#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <sys/ipc.h>
#include <pthread.h>
#include "mxc_sdk.h"
#include "ulib.h"
#include "config.h"
#include "zopt.h"
#include "xfer.h"

#define ARP_PRT     9001

typedef struct mcast_if {
	ev_io  mw;
	ev_io  pw;
	int    fd;
	int    cmdfd[2];
	int    inited;
} mcast_if_t;

typedef struct  ipreq {
	unsigned short  magic; //('I'<< 8)|'R';
	unsigned short  devid;
}__attribute__((packed)) ipreq_t;

typedef struct  ipack {
	unsigned short  magic; //('I'<< 8)|'R';
	unsigned short  devid;	
	char  ip[16];
}__attribute__((packed)) ipack_t;

static pthread_mutex_t     gmtx = PTHREAD_MUTEX_INITIALIZER;
#define  grp_lock()        pthread_mutex_lock(&gmtx)
#define  grp_unlock()      pthread_mutex_unlock(&gmtx)

static int    gret = -1;
static pthread_t  grpid = 0;

// Choose a address in the range [232.0.1.0, 232.255.255.255)
// i.e., [0xE8000100, 0xE8FFFFFF)
#define  MAKE_MCADDR(id)    ((unsigned long)0xE8000100 + (id))

static mcast_if_t  g_mi = {
	.fd = -1,
	.inited = 0,
};

static void
mcast_if_init(mcast_if_t *mi)
{
	if (mi->inited == 0) {
		if (init_pipe(mi->cmdfd) == 0)
			mi->inited = 1;
	}
}

static void
mcast_if_uninit(mcast_if_t *mi)
{
	if (mi->inited == 1) {
		close(mi->cmdfd[0]);
		close(mi->cmdfd[1]);
		mi->inited = 0;
	}
}


//////////////////////////////////////////////////////////////
//       master routine
//////////////////////////////////////////////////////////////

#if 0
{
#endif

/* DONE */
static void
ipaddr_ack(int fd, struct sockaddr_in* sa)
{
	ipack_t  ipack;

	CLEARV(ipack);
	ipack.magic = (('I') << 8)|('R');
	ipack.devid = g_devid;
	memcpy(ipack.ip, g_ipaddr, 15);
	sendto(fd, &ipack, sizeof(ipack), 0, (struct sockaddr*)sa, sizeof(struct sockaddr_in));
}

/* DONE */
static void
handle_mcast(struct ev_loop *l, ev_io *w, int revents)
{
	struct sockaddr_in  sa;
	socklen_t   len;
	int   n;
	ipreq_t  ipreq;
#ifdef DEBUG
	char  peerip[16] = {0};
#endif

	if (revents & EV_READ) {
		len = sizeof(sa);
		memset(&sa, 0, len);
		n = recvfrom(w->fd, &ipreq, sizeof(ipreq), 0, (struct sockaddr*)&sa, &len);

#ifdef DEBUG
		inet_ntop(AF_INET, &sa.sin_addr, peerip, 16);
		DBG("--- received discovery message from: %s ---\n", peerip);
#endif
		if (n > 0) {
			if ((((ipreq.magic >> 8)&0xff) == 'I')
				&&((ipreq.magic & 0xff) == 'R')
				&&(ipreq.devid == g_devid)) {
				ipaddr_ack(w->fd, &sa);
			}
		}
	}
}

/* DONE */
static int
add_mcast_if(struct ev_loop *l, mcast_if_t *mi)
{
	int  fd = -1;
	struct in_addr  mcaddr;

	CLEARV(mcaddr);
	mcaddr.s_addr = htonl(MAKE_MCADDR(0));
	fd = u_mcast_recv_sock("eth0", mcaddr, ARP_PRT);
	if (fd < 0) return -1;
	mi->fd = fd;
	ev_io_init(&mi->mw, handle_mcast, fd, EV_READ);
	ev_io_start(l, &mi->mw);

	return 0;
}

/* DONE */
static void
del_mcast_if(struct ev_loop *l, mcast_if_t *mi)
{
	struct in_addr  mcaddr;

	if (mi->fd != -1) {
		CLEARV(mcaddr);
		mcaddr.s_addr = htonl(MAKE_MCADDR(0));
		u_mcast_quit(mi->fd, mcaddr);
		ev_io_stop(l, &mi->mw);
		mi->fd = -1;
	}
}

/* DONE */
static void
handle_gpipe(struct ev_loop *l, ev_io *w, int revents)
{
	char  cmd;
	mcast_if_t  *mi = U_CONTAINER_OF(w, struct mcast_if, pw);

	if (revents & EV_READ) {
		if (read(w->fd, &cmd, sizeof(cmd)) == 1) {
			if (cmd == 'q') {
				ev_io_stop(l, w);
				del_mcast_if(l, mi);
				ev_break(l, EVBREAK_ALL);
				DBG("--- going to quit the grp_loop ---\n");
			}
		}
	}
}

/* DONE */
static void
add_pipe_ev(struct ev_loop *l, mcast_if_t *mi)
{
	ev_io_init(&mi->pw, handle_gpipe, mi->cmdfd[0], EV_READ);
	ev_io_start(l, &mi->pw);
}

/* DONE */
static void*
grp_loop(void* arg)
{
	struct ev_loop *l = NULL;

	l = ev_loop_new(EVFLAG_AUTO);

	if (add_mcast_if(l, &g_mi) < 0) {
		DBG("--- u_mcast_recv_sock() failed ---\n");
		exit(1);
	}
	add_pipe_ev(l, &g_mi);

	DBG("--- enter group loop ---\n");
	ev_run(l, 0);

	ev_loop_destroy(l);

	DBG("--- quit group loop ---\n");

	pthread_exit(NULL);
}

/* DONE */
void
grp_start(void)
{
	if (is_standalone()||!is_master()) return;
	grp_lock();
	if (gret == -1) {
		mcast_if_init(&g_mi);
		DBG("---------- START THE GROUP LOOP --------\n");
		gret = pthread_create(&grpid, NULL, grp_loop, NULL);
		if (gret < 0)
			mcast_if_uninit(&g_mi);
	}
	grp_unlock();
}

/* DONE */
void
grp_end(void)
{
	grp_lock();
	if (gret == 0) {
		ignore_result(write(g_mi.cmdfd[1], "q", 1));
		pthread_join(grpid, NULL);
		mcast_if_uninit(&g_mi);
		grpid = 0;
		gret = -1;
		DBG("---------- EXIT THE GROUP LOOP --------\n");
	}
	grp_unlock();
}

#if 0
}
#endif

//////////////////////////////////////////////////////////////
//       slave routine
//////////////////////////////////////////////////////////////
#if 0
{
#endif

#define PROBETMO     3.0

typedef struct _probev {
	ev_io  iow;
	ev_io  pw;
	ev_timer  rtw;
	int    pt;
	int    fd;
	int    result;
	struct sockaddr_in  addr;
} probev_t;

static void   probe_ipaddr(struct ev_loop *l, ev_io *w, int revents);

static inline void
add_probev(struct ev_loop *l, probev_t *pe)
{
	ev_io_init(&pe->iow, probe_ipaddr, pe->fd, EV_WRITE);
	ev_io_start(l, &pe->iow);
}

static void
read_ack(struct ev_loop *l, ev_io *w, int revents)
{
	probev_t *p = U_CONTAINER_OF(w, struct _probev, iow);
	socklen_t   len;
	int   n;
	int   fd = w->fd;
	ipack_t  ipack;
#ifdef DEBUG
	struct sockaddr_in sa;
	char  peerip[16] = {0};
#endif

	ev_timer_stop(l, &p->rtw); //stop the receiver timer watcher.
	p->pt = 0;
	ev_io_stop(l, w);  //stop the receiver watcher.

	if (revents & EV_READ) {
		len = sizeof(sa);
		memset(&sa, 0, len);
		memset(&ipack, 0, sizeof(ipack));
		n = recvfrom(fd, &ipack, sizeof(ipack), 0, (struct sockaddr*)&sa, &len);

#ifdef DEBUG
		inet_ntop(AF_INET, &sa.sin_addr, peerip, 16);
		DBG("--- received ack from: %s ---\n", peerip);
#endif

		if (n > 0) {
			if ((((ipack.magic >> 8)&0xff) == 'I')
				&&((ipack.magic & 0xff) == 'R')
				&&(ipack.devid == masterid)) {
				update_masterip(ipack.ip);
				ev_io_stop(l, &p->pw);
				ev_break(l, EVBREAK_ALL);
				DBG("--- master ip: %s, and will exit the probing loop ---\n", ipack.ip);
				return;
			}
		}
	}

	add_probev(l, p);
}

/* DONE */
static void
probe_timeout(struct ev_loop *l, ev_timer *w, int revents)
{
	probev_t *p = U_CONTAINER_OF(w, struct _probev, rtw);

	DBG("--- probe master timeout: no response ---\n");
	p->pt = 0;
	//automatic stopped.
	ev_io_stop(l, &p->iow); //timeout and stop the receiver watcher.
	add_probev(l, p);  //continue probing.
}

/* DONE */
static void
probe_ipaddr(struct ev_loop *l, ev_io *w, int revents)
{
	probev_t *p = U_CONTAINER_OF(w, struct _probev, iow);
	int  fd = w->fd;
	ipreq_t   ipreq;

	if (revents & EV_WRITE) {
		ev_io_stop(l, w);
		ipreq.magic = (('I')<< 8)|('R');
		ipreq.devid = masterid;
		sendto(fd, &ipreq, sizeof(ipreq), 0, (struct sockaddr*)&p->addr, sizeof(struct sockaddr_in));

		ev_io_init(w, read_ack, fd, EV_READ);
		ev_io_start(l, w);

		p->pt = 1;
		ev_timer_init(&p->rtw, probe_timeout, PROBETMO, 0.);
		ev_timer_start(l, &p->rtw);
	}
}

/* DONE */
static void
handle_qcmd(struct ev_loop *l, ev_io *w, int revents)
{
	unsigned int  cmd;
	probev_t  *pev = U_CONTAINER_OF(w, struct _probev, pw);

	if (revents & EV_READ) {
		if (read(w->fd, &cmd, sizeof(cmd)) == sizeof(cmd)) {
			if ((cmd >> NCMDSFT) == 3) {
				ev_io_stop(l, w);
				ev_io_stop(l, &pev->iow);
				if (pev->pt == 1) {
					ev_timer_stop(l, &pev->rtw);
					pev->pt = 0;
				}
				pev->result = 1;
				ev_break(l, EVBREAK_ALL);
				DBG("--- going to quit the client_loop ---\n");
			}
		}
	}
}

/* DONE */
static void
add_cmdev(struct ev_loop *l, probev_t *pev, int fd)
{
	pev->result = 0;
	ev_io_init(&pev->pw, handle_qcmd, fd, EV_READ);
	ev_io_start(l, &pev->pw);
}

/* DONE */
static int
init_probev(probev_t *pev)
{
	struct in_addr ia;
#ifdef DEBUG
	char  ip[16];
#endif

	memset(pev, 0, sizeof(probev_t));
	pev->addr.sin_family = AF_INET;
	pev->addr.sin_port = htons(ARP_PRT);
	pev->addr.sin_addr.s_addr = htonl(MAKE_MCADDR(0));

#ifdef DEBUG
	u_if_get_ip("eth0", ip, sizeof(ip), &ia);
	DBG("--- eth0: %s-%s ---\n", g_ipaddr, ip);
#else
	u_if_get_ip("eth0", NULL, 0, &ia);
#endif

	pev->fd = u_mcast_sndsock(ia);
	return pev->fd;
}

#define uninit_probev(p)    close((p)->fd)

/* DONE */
int
probe_masterip(void *loop, int fd)
{
	probev_t   pev;
	struct ev_loop *l = loop;

	if (init_probev(&pev) < 0) exit(1);

	add_cmdev(l, &pev, fd);
	add_probev(l, &pev);

	DBG("--- enter probing loop ---\n");
	ev_run(l, 0);

	uninit_probev(&pev);
	DBG("--- quit probing loop ---\n");

	return pev.result;
}

#if 0
}
#endif



