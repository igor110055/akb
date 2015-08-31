#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include "mxc_sdk.h"
#include "ulib.h"
#include "utkscrsvr.h"
#include "config.h"
#include "pzone.h"
#include "zopt.h"
#include "znode.h"
#include "rs485.h"
#include "xfer.h"
#include "zdelay.h"
#include "pztimer.h"
#include "gui.h"


typedef int  (*FUINT) (unsigned int);


static pthread_mutex_t   zmtx = PTHREAD_MUTEX_INITIALIZER;
#define zopt_lock()      pthread_mutex_lock(&zmtx)
#define zopt_unlock()    pthread_mutex_unlock(&zmtx)

static pthread_t  ztid = 0;
static int        zret = -1;

//////////////////////////////////////////

//////////////////////////////////////////
static struct ev_loop *z_base = NULL;
static int    zfd[2] = {-1, -1};

unsigned short  _syst = 0;//defaut states[fault] after boot.

static ev_timer  armw;
static ev_timer *pztw = NULL;

#define TTSAVEST     (5.)

#if ENABLE_SCRSVR

static int  scrsv = 0;

void
enable_scrsvr(void)
{
	if (scrsv == 0) {
		DBG("--- enable screen saver ---\n");
		utk_start_scrsvr(SCRSVR);
		scrsv = 1;
	}
}

void
disable_scrsvr(void)
{
	if (scrsv == 1) {
		DBG("--- disable screen saver ---\n");
		utk_stop_scrsvr();
		scrsv = 0;
	}
}

#endif

void  
ztimer_start(void *w)
{
	ev_timer_start(z_base, (ev_timer*)w);
}

void
ztimer_stop(void *w)
{
	ev_timer_stop(z_base, (ev_timer*)w);
}

void
pz_timer_add(unsigned int t, void *cb)
{
	if (pztw != NULL) {
		pztw->repeat = t + 0.0;
		ev_timer_again(z_base, pztw);
		DBG("--- restart the timer(%u seconds) to trigger the arm/disarm ---\n", t);
	} else {
		pztw = &armw;
		ev_init(pztw, cb);
		pztw->repeat = t + 0.0;
		ev_timer_again(z_base, pztw);
		DBG("--- start the timer(%u seconds) to trigger the arm/disarm ---\n", t);
	}
}

int
pz_timer_remove(void)
{
	int  r = 1;

	if (pztw != NULL) {
		ev_timer_stop(z_base, pztw);
		pztw = NULL;
		r = 0;
	}

	return r;
}


//////////////////////////////////////////////////////////
#if 0
{
#endif

/* slave only: DONE */
void
send_evt_slave(int eid, int hid)
{
	int id;
	int st;

	DBG("--- host-%d: evtid = %d ---\n", hid, eid);
	if (eid > EV_PRG&&eid < EV_HPENT) { //ev from netloop.c or rs485.c
		id = (eid >> 1) - 2;
		st = eid & 1;
		if (st) {
			_syst |= (1 << id);
		} else {
			_syst &= ~(1 << id);
		}
		st |= (_syst << 1);
		update_syst(id, st);
	} else if (eid > EV_HPEXT&&eid <= EV_A120DP) { //ev from uart.c
		int  id1;
		id = (eid >> 1) - 3 + (hid - 1) * 6;
		st = eid & 1;
		if (st) {
			_syst |= (1 << id);
		} else {
			_syst &= ~(1 << id);
		}

		id1 = 7 + (hid - 1) * 6;
		_syst &= ~(1 << id1);
		
		st |= (_syst << 1);
		update_syst(id, st);
	} else if (eid == EV_HPENT||eid == EV_HPEXT) { //ev from uart.c
		id = 7 + (hid - 1) * 6;
		_syst &= ~(1 << id);
		st = (_syst << 1);
		update_syst(id, st);
	}

#if ENABLE_SCRSVR
	/* new add on 2013.11.15 */
	if (_syst != 0) {
		disable_scrsvr();
	} else if (pzaltmap == 0&&nready_yet() == 0) {
		enable_scrsvr();
	}
#endif
	send_event_db(eid, hid);
}

/* master only: DONE */
static int
handle_evt(unsigned int var)
{
	int eid;
	int hid;
	unsigned short arg = var & 0xffff;

	eid = arg >> 2;
	hid = arg & 3;

	send_evt_slave(eid, hid);
	if (eid >= EV_HPENT&&eid <= EV_BL) {
		netxfer_event(eid, hid);
	}

	return 0;
}

static int
zone_state(unsigned int var)
{
	int  z = var & 0xffff;
	return zstat[zoneidx(z)];
}

static int
pzone_state(unsigned int var)
{
	unsigned short p = var & 0xffff;

	if (p == 0) {
		return calc_allflag();
	}
	return pstat[p-1];
}

static FUINT  zopt_fn[] = {
	[ODAM]	 = zdisarm_opt,
	[OARM]   = zarm_opt,
	[OZBP]   = zbp_opt,
	[OZBPR]  = zbpr_opt,
	[OPZAM]  = pzarm_opt,
	[OPZFA]  = pzarm_fopt,
	[OPZDA]  = pzdisarm_opt,	
	[OPZBP]  = pzbp_opt,
	[OPZBPR] = pzbpr_opt,
	[OTMRDN] = disable_pztimer,
	[OTMREN] = enable_pztimer,
	[OPZTMR] = pztimer_init,
	[OZALT]  = za_opt,
	[OZALTR] = zar_opt,
	[OEVT]   = handle_evt,
	[OZST]   = zone_state,
	[OPZST]  = pzone_state,
	[OSYST]  = syst_return,
};

static void
zopt_receiver(struct ev_loop *loop, ev_io *w, int revents)
{
	unsigned int  cmd;
	int      res;
	int      t;

	if (revents & EV_READ) {
		if (read(zfd[0], &cmd, sizeof(cmd)) > 0) {
			if (cmd == ~0) {
				stop_pz_timer();
				ev_io_stop(loop, w);
				ev_break(loop, EVBREAK_ALL);
				DBG("--- received the stop zopt cmd ---\n");
				return;
			}

			t = (cmd >> 16) & 0xff;
			if (t > MAXOPT) return;
			res = (*zopt_fn[t])(cmd);
			if (t >= OZST) {
				ignore_result(write(zfd[0], &res, sizeof(res)));
			}

			if (t <= OSVRST&&res == 0) {
				syst_save();
			}
		}
	}
}

static void*
zopt_loop(void* arg)
{
	ev_io   ev;

	CLEARA(zidelay);
	z_base = ev_loop_new(EVFLAG_AUTO);
	if (z_base == NULL) exit(1);

	pztimer_init(1);
	ev_io_init(&ev, zopt_receiver, zfd[0], EV_READ);
	ev_io_start(z_base, &ev);

#if ENABLE_SCRSVR
	enable_scrsvr();
#endif

	DBG("--- enter zopt engine loop ---\n");

	ev_run(z_base, 0);

	z_timer_cleanup();
	ev_loop_destroy(z_base);

	DBG("--- quit zopt engine loop ---\n");

	pthread_exit(NULL);
}

void
zopt_start(void)
{
	if (!is_master()) return;
	zopt_lock();
	if (zret == -1) {
		init_pipe(zfd);
		init_tmrpz_node();
		zret = pthread_create(&ztid, NULL, zopt_loop, NULL);
		if (zret != 0) {
			printf("--- create zopt loop thread failed ---\n");
			exit(1);
		}
		DBG("--- zopt_loop started ---\n");
	}
	zopt_unlock();
}

void
zopt_exit(void)
{
	unsigned int cmd = ~0;

	zopt_lock();
	if (zret == 0) {
		ignore_result(write(zfd[1], &cmd, sizeof(cmd)));
		pthread_join(ztid, NULL);
		zret = -1;
		close(zfd[0]);
		close(zfd[1]);
		DBG("--- zopt loop exited ---\n");
	}
	zopt_unlock();
}

int
recv_cmd_ack(int fd, int ms)
{
	int  res = 0;

	if (u_readable_mtimeo(fd, ms) > 0) {
		if (read(fd, &res, sizeof(res)) == sizeof(res)) {
			return res;
		}
	}

	return -1;
}

static int
do_cmd(int cid, unsigned short arg)
{
	unsigned int cmd;

	if (zret == 0) {
		cmd = (cid << 16)|arg;
		if (write(zfd[1], &cmd, sizeof(cmd)) == sizeof(cmd)) {
			return recv_cmd_ack(zfd[1], -1);
		}
	}
	return -1;
}

static void
do_cmd_nr(int cid, unsigned short arg)
{
	unsigned int cmd;

	if (zret == 0) {
		cmd = (cid << 16)|arg;
		ignore_result(write(zfd[1], &cmd, sizeof(cmd)));
	}
}

static void (*snd_cmd_nack[])(int, unsigned short) = {do_xfer_nack, do_cmd_nr};


/* z: [0, MAXPZ]; 0: all */
void
pzone_arm(int z)
{
	int  m;
	if (z < 0||z > MAXPZ) return;
	m = is_master();
	(*snd_cmd_nack[m])(OPZAM, (unsigned short)z);
}

/* z: [0, MAXPZ]; 0: all */
void
pzone_farm(int z)
{
	int  m;
	if (z < 0||z > MAXPZ) return;
	m = is_master();
	(*snd_cmd_nack[m])(OPZFA, (unsigned short)z);
}

/* z: [0, MAXPZ];  0: all */
void
pzone_disarm(int z)
{
	int  m;
	if (z < 0||z > MAXPZ) return;
	m = is_master();
	(*snd_cmd_nack[m])(OPZDA, (unsigned short)z);
}
////////////////////////////////////////////

/* z: 0~511 */
void
zone_arm(int z)
{
	int  m;
	if (z < 0||z >= MAXZ) return;
	m = is_master();
	(*snd_cmd_nack[m])(OARM, (unsigned short)z);
}

/* z: 0~511 */
void
zone_disarm(int z)
{
	int  m;
	if (z < 0||z >= MAXZ) return;
	m = is_master();
	(*snd_cmd_nack[m])(ODAM, (unsigned short)z);
}

/* z: 0~511 */
void
zone_bypass(int z)
{
	int  m;
	if (z < 0||z >= MAXZ) return;
	m = is_master();
	(*snd_cmd_nack[m])(OZBP, (unsigned short)z);
}

/* znr: 0~511 */
void
zone_bypassr(int z)
{
	int  m;
	if (z < 0||z >= MAXZ) return;
	m = is_master();
	(*snd_cmd_nack[m])(OZBPR, (unsigned short)z);
}

/* z: 0-MAXPZ-1 */
void
pzone_bypass(int z)
{
	int  m;
	if (z < 0||z >= MAXPZ) return;
	m = is_master();
	(*snd_cmd_nack[m])(OPZBP, (unsigned short)z);
}

/* z: 0-MAXPZ-1 */
void
pzone_bypassr(int z)
{
	int  m;
	if (z < 0||z >= MAXPZ) return;
	m = is_master();
	(*snd_cmd_nack[m])(OPZBPR, (unsigned short)z);
}

/* z: 0-511 */
void
zone_alert(int z)
{
	if (!zone_valid(z)) {//should prompt the user: undefined zone.
		
		return;
	}
	do_cmd_nr(OZALT, (unsigned short)z);
}

/* z: 0-511 */
void
zone_alertr(int z)
{
	if (!zone_valid(z)) {//should prompt the user
		
		return;
	}
	do_cmd_nr(OZALTR, (unsigned short)z);
}

void
simul_alert(int z)
{
	int  m;
	if (z < 0||z >= MAXZ) return;
	m = is_master();
	(*snd_cmd_nack[m])(OZALT, (unsigned short)z);
}

void
simul_alertr(int z)
{
	int  m;
	if (z < 0||z >= MAXZ) return;
	m = is_master();
	(*snd_cmd_nack[m])(OZALTR, (unsigned short)z);
}


/////////////////////////////////////////////////////////////////
int
get_zone_state_direct(int z)
{
	return (int)zstat[zoneidx(z)];
}

///////////////////////////////////////////////////////////////////
int
get_pzone_state_direct(int p)
{
	if (p > 0) {
		return pstat[p-1];
	}
	return calc_allflag();
}

//////////////////////////////////////////////////////////////////////

static int
syst_get(unsigned int v)
{
	return do_cmd(OSYST, 0);
}

static int (*systr[])(unsigned int) = {syst_return, syst_get};

int
get_syst(void)
{
	int  m;
	m = is_master();
	return (*systr[m])(0);
}

///////////////////////////////////////////////////////////////////////
/* i: index/position of pzone in list, not the pzone number. */
void
enable_pzone_timer(int i)
{
	int  m;
	if (i < 0||i >= MAXTPZ) return;
	m = is_master();
	(*snd_cmd_nack[m])(OTMREN, (unsigned short)i);
}

/* i: index/position of pzone in list, not the pzone number. */
void
disable_pzone_timer(int i)
{
	int  m;
	if (i < 0||i >= MAXTPZ) return;
	m = is_master();
	(*snd_cmd_nack[m])(OTMRDN, (unsigned short)i);
}

/* master only: called when timer arm/disarm cfg changed. */
void
init_pztimer(int i)
{
	do_cmd_nr(OPZTMR, (unsigned short)i);
}

#if 0
}
#endif

static void
send_evt_master(int eid, int hid)
{
	unsigned short  arg = ((eid << 2)|(hid & 3));
	do_cmd_nr(OEVT, arg);
}

static void (*evt_dispatch[])(int, int) = {send_evt_slave, send_evt_master};

/* master only: called by uart thread. */
/* hid: 0-local state, 1/2-host id. */
void
send_event(int eid, int hid)
{
	int  m;
	m = is_master();
	(*evt_dispatch[m])(eid, hid);
}

void
send_zcmd(void *buf, int n)
{
	ignore_result(write(zfd[1], buf, n));
}

////////////////////////////////////////////////////
static int  rlyfd = -1;

void
relay_init(void)
{
	rlyfd = open("/dev/relayc", O_WRONLY);
}

void
relay_free(void)
{
	if (rlyfd >= 0) {
		close(rlyfd);
	}
}

void
relay_on(void)
{
	char  cmd = 1;
	if (rlyfd >= 0) {
		ignore_result(write(rlyfd, &cmd, sizeof(cmd)));
	}
}

void
relay_off(void)
{
	char  cmd = 0;
	if (rlyfd >= 0) {
		ignore_result(write(rlyfd, &cmd, sizeof(cmd)));
	}
}



