#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>

#include "ulib.h"
#include "mxc_sdk.h"
#include "config.h"
#include "proto.h"
#include "rs485.h"
#include "zopt.h"

#define RDEBUG       0
#define XDEBUG       1

#define XFER_SZ      256

#define MAXQ         1024
#define XFERTMO      0.1
#define POLLTMO      0.1
#define XBUFSZ       32

#define RETRY        3

#define TMSTP_CMD    ((unsigned int)0xf0000000)
#define LNK_CMD      ((unsigned int)0xe0000000)


#define OPEN_MODE   (S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH)

#define RS485        "/dev/ttySAC2"

typedef void (*VIIF)(int, int);

typedef struct _xfer_buf {
	unsigned int *xdata;
	int   size;
	int   xr;
	int   xw;
} xfer_buf_t;

extern void  rs485_clear(void);
extern void *memrchr(const void *s, int c, size_t n);

static pthread_mutex_t   rsmtx = PTHREAD_MUTEX_INITIALIZER;
#define rs485_lock()     pthread_mutex_lock(&rsmtx)
#define rs485_unlock()   pthread_mutex_unlock(&rsmtx)

static pthread_t    rtid;
static int   rret = -1;

static struct ev_loop *rbase = NULL;
static ev_io        pollw;   //poll write watcher.

static ev_timer     rcvtw;   //reading timeout watcher.
static ev_timer    *prcvw = NULL;

static ev_timer     sndtw;   //reading timeout watcher.
static ev_timer    *psndtw = NULL;

static ev_timer     tsw;     //timestamp timer watcher.
static ev_timer    *rtw = NULL;

static ev_timer     lnkw;    //linkage timer
static ev_timer    *plkw = NULL;

static int          lnkopt = 0;
static unsigned int  stm = 0;
static unsigned int  etm = 0;
static unsigned int  lnktm[2];

extern int  u_tty_set(int fd, int speed, int datasz, int stopbits, int fc, int vmin, int vtm);

static int  rfd[2] = {-1, -1};
static int  rs_fd = -1;

static int  rs_st = 1; //state of 485 bus.
static int  nack = 0;  //number of the ack received in one polling loop.

static ucnf_t   *ucf = NULL;

static int  rtpr = 0;
static int  rlnk = 0;
static int  ifrpr = 0;

static int  rserr = 0;

static void  send_cmd(struct ev_loop *loop, ev_io *w, int revents);
static void  add_wev(struct ev_loop *loop, int fd, void *cb);
static void  add_lnkage_timer(struct ev_loop *loop, ev_tstamp t);
static void  add_tstamp_timer(struct ev_loop *loop, ev_tstamp t);
static void  poll_timeout(struct ev_loop *loop, ev_timer *w, int revents);

static unsigned char  devaddr[80];
static int            ndev = 0;
static int            pollidx = 0;

static char  zopcode[] = {
	[ZALR]   = XFER_ZALR,
	[ZALT]   = XFER_ZALT,
	[ZFLR]   = XFER_ZFLR,
	[ZFLT]   = XFER_ZFLT,
	[ZARM]   = XFER_ZARM,
	[ZDAM]   = XFER_ZDAM,
	[ZBPA]   = XFER_ZBP,
	[ZBPR]   = XFER_ZBPR,
	[AARM+MAXZCMD] = XFER_PARM,	
	[FARM+MAXZCMD] = XFER_PARM,
	[ADAM+MAXZCMD] = XFER_PDAM,
	[PARM+MAXZCMD] = XFER_PARM,
	[PFAM+MAXZCMD] = XFER_PARM,
	[PDAM+MAXZCMD] = XFER_PDAM,
	[PBPA+MAXZCMD] = XFER_PBP,
	[PBPR+MAXZCMD] = XFER_PBPR,
	[PTAM+MAXZCMD] = XFER_PARM,
	[PTDM+MAXZCMD] = XFER_PDAM,
};

/////////////////////////////////////////////////////////////
static xfer_buf_t  *xbuf = NULL;

static xfer_buf_t*
xbuf_new(int sz)
{
	xfer_buf_t *x;
	
	x = calloc(1, sizeof(xfer_buf_t));
	if (x) {
		memset(x, 0, sizeof(xfer_buf_t));
		x->xdata = calloc(sz, sizeof(unsigned int));
		if (x->xdata != NULL) {
			x->size = sz;
		} else {
			free(x);
			x = NULL;
		}
	}
	return x;
}

static void
xbuf_free(xfer_buf_t **x)
{
	if (*x) {
		free((*x)->xdata);
		memset(*x, 0, sizeof(xfer_buf_t));
		free(*x);
		*x = NULL;
	}
}

#define xbuf_reset(x)   \
	do { \
		(x)->xw = 0;  \
		(x)->xr = 0;  \
	} while (0)

static void
xbuf_put(xfer_buf_t *x, unsigned int d)
{
	if (((x->xw + 1)&(x->size-1)) == x->xr) { //xbuf full
		DBG("--- xfer buffer no space to write ---\n");
		if (x->size == MAXQ) {
			DBG("--- xfer buffer reached limits, lost one cmd. ---\n");
			x->xr = (x->xr + 1) & (x->size - 1);
		} else {
			void *ptr;
			ptr = realloc(x->xdata, (x->size<<1)*sizeof(unsigned int));
			if (ptr) {
				x->xdata = ptr;
				x->size <<= 1;
			} else {
				DBG("--- xfer buffer reached limits and realloc failed, lost one cmd. ---\n");
				x->xr = (x->xr + 1)&(x->size - 1);
			}
		}
	}
	x->xdata[x->xw] = d;
	x->xw = (x->xw + 1)&(x->size - 1);
}

#define xbuf_get(x)  ({unsigned int cmd;cmd = x->xdata[x->xr];x->xr = (x->xr + 1) & (x->size - 1);cmd;})

////////////////////////////////////////////////////////

static int
uart_dev_init(ucnf_t *cf)
{
	int   i;
	int   n;

	ndev = 0;
	if (cf->siom > 0) {
		for (i = cf->siom; i > 0; --i) {
			devaddr[i-1] = i;
		}
		ndev = cf->siom;
	}

	if (cf->relay > 0) {
		n = cf->relay;
		i = 65;
		while (n) {
			devaddr[ndev++] = i++;
			--n;
		}
	}

	if (cf->dev & 1) {
		devaddr[ndev++] = 73;
	}

	return ndev;
}

////////////////////////////////////////////////////////
/* DONE: remove receiver watcher when close the rs485. */
static inline void
remove_tev(struct ev_loop *loop)
{
	if (prcvw != NULL) {
		ev_timer_stop(loop, prcvw);
		prcvw = NULL;
	}
}

static inline void
remove_ptev(struct ev_loop *loop)
{
	if (psndtw != NULL) {
		ev_timer_stop(loop, psndtw);
		psndtw = NULL;
	}
}

/* DONE */
static void
linkage_opt(struct ev_loop *loop, ev_timer *w, int revents)
{
	xbuf_put(xbuf, LNK_CMD);
}

/* DONE */
static void
add_lnkage_timer(struct ev_loop *loop, ev_tstamp t)
{
	if (plkw == NULL) {
		plkw = &lnkw;
		ev_init(plkw, linkage_opt);
	}
	plkw->repeat = t;
	ev_timer_again(loop, plkw);
}

static void
remove_lnkage_timer(struct ev_loop *loop)
{
	if (plkw != NULL) {
		ev_timer_stop(loop, plkw);
		plkw = NULL;
	}
}

/* DONE */
static inline void
turn_onoff_linkage(int fd, int on)
{
	unsigned char  cmd[] = {0x85, 0x00, 0x8f};

	cmd[0] = 0x85 - on;
	ignore_result(write(fd, cmd, sizeof(cmd)));
	u_mdelay(100);
	ignore_result(write(fd, cmd, sizeof(cmd)));
	u_mdelay(100);
	ignore_result(write(fd, cmd, sizeof(cmd)));
	DBG("--- turn %s linkage ---\n", on ? "on":"off");
}

/* DONE: called when received the relay's ack */
static void
start_linkage_timer(struct ev_loop *loop, int fd)
{
	unsigned int  now;
	unsigned int  t;

	now = get_daytm(NULL);
	if (stm < etm) {
		if (now >= stm&&now < etm) {
			turn_onoff_linkage(fd, 1);
			lnkopt = 1;
			add_lnkage_timer(loop, etm - now + 0.0);
			DBG("--- linkage on now,and linkage off after %u secs ---\n", etm - now);
		} else {
			turn_onoff_linkage(fd, 0);
			t = (SECS_PER_DAY + stm - now) % SECS_PER_DAY;
			lnkopt = 0;
			add_lnkage_timer(loop, t + 0.0);
			DBG("--- linkage off now, and linkage on after %u secs ---\n", t);
		}
	} else {
		if (now >= etm&&now < stm) {
			turn_onoff_linkage(fd, 0);
			lnkopt = 0;
			add_lnkage_timer(loop, stm - now + 0.0);
			DBG("--- linkage off now, and linkage on after %u secs ---\n", stm - now);
		} else {
			turn_onoff_linkage(fd, 1);
			t = (SECS_PER_DAY + etm - now) % SECS_PER_DAY;
			lnkopt = 1;
			add_lnkage_timer(loop, t + 0.0);
			DBG("--- linkage on, and linkage off after %u secs ---\n", t);
		}
	}
}
//////////////////////////////////////////////////////////
/* DONE */
static void
send_timestamp(int fd)
{
	unsigned char ts_cmd[] = {0xee, 0x49, 0x09, 0x0e, 0x00, 0x00, 0x00, 0x0f, 0x00, 0x00, 0x00, 0x00, 0xdd};
	struct tm nowtm;

	get_tm(&nowtm, NULL);
	ts_cmd[4] = (nowtm.tm_year+1900-2000)&0xff;
	ts_cmd[5] = nowtm.tm_mon+1;
	ts_cmd[6] = nowtm.tm_mday;
	ts_cmd[8] = nowtm.tm_hour;
	ts_cmd[9] = nowtm.tm_min;
	ts_cmd[10] = nowtm.tm_sec;
	ts_cmd[11] = my_cksum(&ts_cmd[1], 10);
	ignore_result(write(fd, ts_cmd, sizeof(ts_cmd)));
	DBG("--- SEND RPRT TIMESTAMP: 0xee 0x49 0x09 0x0e 0x%02x 0x%02x 0x%02x 0x0f 0x%02x 0x%02x 0x%02x 0x%02x 0xdd ---\n",
		ts_cmd[4], ts_cmd[5], ts_cmd[6], ts_cmd[8], ts_cmd[9], ts_cmd[10], ts_cmd[11]);
}

/* DONE */
static inline void
turn_onoff_rtprt(int fd, int on)
{
	unsigned char  cmd[] = {0x00, 0x49, 0x8f};

	cmd[0] = 0x87 - on;
	ignore_result(write(fd, cmd, sizeof(cmd)));
	u_mdelay(100);
	ignore_result(write(fd, cmd, sizeof(cmd)));
	u_mdelay(100);
	ignore_result(write(fd, cmd, sizeof(cmd)));
	DBG("--- turn %s real time print ---\n", on ? "on":"off");
}

static void
try_send_cmd(struct ev_loop *loop, ev_timer *w, int revents)
{
	//automatic stopped
	add_wev(loop, rs_fd, send_cmd); //poll next device.
}

#if 1
/* DONE */
static void
add_tev(struct ev_loop *loop, ev_tstamp t, void *cb)
{
	if (prcvw == NULL) {
		prcvw = &rcvtw;
		ev_timer_init(prcvw, cb, t, 0.);
		ev_timer_start(loop, prcvw);
	} else {
		ev_timer_set(prcvw, t, 0.);
		ev_timer_start(loop, prcvw);
	}
}

/* DONE */
static void
add_ptev(struct ev_loop *loop, ev_tstamp t, void *cb)
{
	if (psndtw == NULL) {
		psndtw = &sndtw;
		ev_timer_init(psndtw, cb, t, 0.);
		ev_timer_start(loop, psndtw);
	} else {
		ev_timer_set(psndtw, t, 0.);
		ev_timer_start(loop, psndtw);
	}
}

#else
/* DONE */
static void
add_poll_timer(struct ev_loop *loop, ev_tstamp t)
{
	if (prcvw == NULL) {
		prcvw = &rcvtw;
		ev_init(prcvw, poll_timeout);
	}
	prcvw->repeat = t;
	ev_timer_again(loop, prcvw);
}

/* DONE */
static void
add_send_timer(struct ev_loop *loop, ev_tstamp t)
{
	if (psndtw == NULL) {
		psndtw = &sndtw;
		ev_init(psndtw, poll_timeout);
	}
	psndtw->repeat = t;
	ev_timer_again(loop, psndtw);
}
#endif

static void
stop_poll_timer(struct ev_loop *loop)
{
	if (prcvw) {
		ev_timer_stop(loop, prcvw);
	}
}

static void
stop_send_timer(struct ev_loop *loop)
{
	if (psndtw) {
		ev_timer_stop(loop, psndtw);
	}
}

/* DONE */
static void
recv_ack(struct ev_loop *loop, ev_io *w, int revents)
{
	unsigned char buf[64];
#if RDEBUG
	int   i;
#endif
	char *ptr;
	int   k;
	int   n;
	int   fd = w->fd;

#if 0
	if (prcvw) {  //stop the receiver timer.
		ev_timer_stop(loop, prcvw);
	}
#else
	stop_poll_timer(loop);
#endif
	
	ev_io_stop(loop, w);  //stop the receiver watcher.

	if (revents & EV_READ) {
		n = read(fd, buf, sizeof(buf));
		if (n > 0) {
		#if RDEBUG
			printf("--- HAVE DATA:");
			for (i = 0; i < n; ++i) {
				printf(" 0x%02x", buf[i]);
			}
			printf(" ---\n");
		#endif
	
			ptr = memrchr(buf, (int)0x9f, n); //found the end flag.
			if (ptr) {
				k = *(ptr-1);
//				DBG("--- poll device: %d, device: %d ack ---\n", devaddr[pollidx], k);
				if (k == 73) { //check whether the real time print should be turned on.
					if (rtpr == 0) {
						turn_onoff_rtprt(fd, ifrpr);
						if (ifrpr) {
							send_timestamp(fd);
							add_tstamp_timer(loop, 5.0);
						}
						rtpr = 1;
						DBG("--- printer online ---\n");
					}
				} else if (k > 0&&k < 65) { //check whether the linkage timer should be created.
					if (rlnk == 0) {
						if (ucf->tm[0] != 0xff) { //start the linkage timer.
							start_linkage_timer(loop, fd);
						} else {
							turn_onoff_linkage(fd, 0);
						}
						rlnk = 1;
						DBG("--- single io modules online ---\n");
					}
				}
				if (rs_st == 0) {
					send_event(EV_RS485R, 0);
					rs_st = 1;
				}
				++nack;
				rserr = 0;
			}
		}
	}
	add_ptev(loop, XFERTMO, try_send_cmd);
}

/* DONE */
static void
poll_timeout(struct ev_loop *loop, ev_timer *w, int revents)
{
//	DBG("--- timeout: no device-%d ---\n", devaddr[pollidx]);
	//automatic stopped.
	ev_io_stop(loop, &pollw); //timeout and stop the receiver watcher.
	add_wev(loop, rs_fd, send_cmd); //poll next device.
}

/* DONE: add receiver watcher. */
static void
add_rev(struct ev_loop *loop, int fd)
{
	ev_io_init(&pollw, recv_ack, fd, EV_READ);
	ev_io_start(loop, &pollw);
}

///////////////////////////////////////////////////////////
/* DONE */
static void
xfer_tstamp(struct ev_loop *loop, ev_timer *w, int revents)
{
	DBG("--- time to send timestamp ---\n");
	xbuf_put(xbuf, TMSTP_CMD);
}

/* DONE */
static void
add_tstamp_timer(struct ev_loop *loop, ev_tstamp t)
{
	if (rtw == NULL) {
		rtw = &tsw;
		ev_init(rtw, xfer_tstamp);
	}
	rtw->repeat = t;
	ev_timer_again(loop, rtw);
}

/*
static inline void
stop_tstamp_timer(struct ev_loop *loop)
{
	if (rtw != NULL) {
		ev_timer_stop(loop, rtw);
	}
}
*/

/* DONE */
static inline void
remove_tstamp_timer(struct ev_loop *loop)
{
	if (rtw != NULL) {
		ev_timer_stop(loop, rtw);
		rtw = NULL;
	}
}

/////////////////////////////////////////////////////////////
#ifdef XDEBUG

static void
dump_hex(unsigned char *buf, int n)
{
	int  i;

	printf("--- send rs485: ");
	for (i = 0; i < n; ++i) {
		printf(" 0x%02x", buf[i]);
	}
	printf(" ---\n");
}

#endif

/* DONE */
static void
send_print_ainfo(int fd, int id)
{
	zainf_t *za = &ainf[id];
	unsigned char cmd[] = {0xee, 0x49, 0x0d, 0x4e, 0x00, 0x00, 0x00, 0x4f, 0x00, 0x00, 0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0xdd};
	struct tm  ntm;

	localtime_r(&za->etm, &ntm);
	cmd[4]  = (ntm.tm_year+1900-2000)&0xff;
	cmd[5]  = (unsigned char)ntm.tm_mon+1;
	cmd[6]  = (unsigned char)ntm.tm_mday;
	cmd[8]  = (unsigned char)ntm.tm_hour;
	cmd[9]  = (unsigned char)ntm.tm_min;
	cmd[10] = (unsigned char)ntm.tm_sec;

	cmd[11] = (unsigned char)zopcode[za->opcode]+0x40;
	if (zopcode[za->opcode] < XFER_ZALT) {
		cmd[12] = (unsigned char)g_uid;
		if (za->znr > 0) {
			cmd[13] = (unsigned char)(za->znr & 0xff);
			cmd[14] = (unsigned char)((za->znr >> 8) + 1);
		} else {
			cmd[13] = (unsigned char)(za->pnr & 0xff);
			if (za->opcode == PTAM+MAXZCMD) {
				DBG("--- rs485 xfer: 定时布防 ---\n");
				cmd[14] = 1;
			} else if (za->opcode == PTDM+MAXZCMD) {
				DBG("--- rs485 xfer: 定时撤防 ---\n");
				cmd[14] = 3;
			} else if (za->opcode == PFAM+MAXZCMD) {
				DBG("--- rs485 xfer: 强制布防 ---\n");
				cmd[14] = 2;
			} else {
				cmd[14] = 0;
			}
		}
	} else { //alarm info
		cmd[12] = (unsigned char)(za->znr & 0xff);
		cmd[13] = (unsigned char)((za->znr >> 8) + 1);
		cmd[14] = 0;
	}

	cmd[15] = my_cksum(&cmd[1], 14);
	ignore_result(write(fd, cmd, sizeof(cmd)));
#ifdef XDEBUG
	dump_hex(cmd, sizeof(cmd));
#endif
}

/* DONE */
static void
send_rt_ainfo(int fd, int id)
{
	zainf_t *za = &ainf[id];
	unsigned char cmd[] = {0xee, 0x00, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0xdd};

	cmd[3] = (unsigned char)zopcode[za->opcode];
	if (zopcode[za->opcode] < XFER_ZALT) {
		cmd[4] = (unsigned char)g_uid;
		if (za->znr > 0) {
			cmd[5] = (unsigned char)(za->znr & 0xff);
			cmd[6] = (unsigned char)((za->znr >> 8) + 1);
		} else {
			cmd[5] = (unsigned char)(za->pnr & 0xff);
			if (za->opcode == PTAM+MAXZCMD) {
				DBG("--- rs485 xfer: 定时布防 ---\n");
				cmd[6] = 1;
			} else if (za->opcode == PTDM+MAXZCMD) {
				DBG("--- rs485 xfer: 定时撤防 ---\n");
				cmd[6] = 3;
			} else if (za->opcode == PFAM+MAXZCMD) {
				DBG("--- rs485 xfer: 强制布防 ---\n");
				cmd[6] = 2;
			} else {
				cmd[6] = 0;
			}
		}
	} else { //alarm info
		cmd[4] = (unsigned char)(za->znr & 0xff);
		cmd[5] = (unsigned char)((za->znr >> 8) + 1);
		cmd[6] = 0;
	}

	cmd[7] = my_cksum(&cmd[1], 6);
	ignore_result(write(fd, cmd, sizeof(cmd)));
#ifdef XDEBUG
	dump_hex(cmd, sizeof(cmd));
#endif
}

static unsigned char  ecmd[] = {XFER_PGE, XFER_PGX, XFER_APR, XFER_APF, XFER_BFR, XFER_BF, XFER_BLR, XFER_BL};

/* DONE */
static void
send_print_einfo(int fd, int id)
{
	einf_t  *e = &einf[id];
	unsigned char cmd[] = {0xee, 0x49, 0x0d, 0x4e, 0x00, 0x00, 0x00, 0x4f, 0x00, 0x00, 0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0xdd};
	struct  tm  ntm;

	if (e->eid < EV_HPENT||e->eid > EV_BL) return;
	localtime_r(&e->etm, &ntm);
	cmd[4]  = (ntm.tm_year+1900-2000)&0xff;
	cmd[5]  = (unsigned char)ntm.tm_mon+1;
	cmd[6]  = (unsigned char)ntm.tm_mday;
	cmd[8]  = (unsigned char)ntm.tm_hour;
	cmd[9]  = (unsigned char)ntm.tm_min;
	cmd[10] = (unsigned char)ntm.tm_sec;

	cmd[11] = (unsigned char)ecmd[e->eid-EV_HPENT]+0x40;
	cmd[13] = (unsigned char)e->hid;
	cmd[15] = my_cksum(&cmd[1], 14);
	ignore_result(write(fd, cmd, sizeof(cmd)));
#ifdef XDEBUG
	dump_hex(cmd, sizeof(cmd));
#endif
}

/* DONE */
static void
send_rt_einfo(int fd, int id)
{
	einf_t  *e = &einf[id];
	unsigned char cmd[] = {0xee, 0x00, 0x0d, 0x40, 0x00, 0x00, 0x00, 0x00, 0xdd};

	if (e->eid < EV_HPENT||e->eid > EV_BL) return;

	cmd[3] = (unsigned char)ecmd[e->eid-EV_HPENT];
	cmd[5] = (unsigned char)e->hid;
	cmd[7] = my_cksum(&cmd[1], 6);
	ignore_result(write(fd, cmd, sizeof(cmd)));
#ifdef XDEBUG
	dump_hex(cmd, sizeof(cmd));
#endif

}

static void
send_zstat_info(int fd, int id)
{
	unsigned char  cmd[] = {0xee, 0x00, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0xdd};
	unsigned char  xcmd[] = {XFER_ZFLR, XFER_ZFLT};
	unsigned short z;

	z = zflt[id];
	cmd[3] = xcmd[z&1];
	cmd[4] = (unsigned char)((z >> 1) & 0xff);
	cmd[5] = (unsigned char)((z >> 9) + 1);
	cmd[6] = 0;
	cmd[7] = my_cksum(&cmd[1], 6);
	ignore_result(write(fd, cmd, sizeof(cmd)));
#ifdef XDEBUG
	dump_hex(cmd, sizeof(cmd));
#endif
}

static VIIF   msg_send[] = {send_print_ainfo, send_rt_ainfo, send_print_einfo, send_rt_einfo, send_zstat_info};

/* DONE */
/* cmd: bit16: 1-real time transfer, 0-print; bit17: 0-alarm info, 1-event info. */
/* 00: print_ainfo, 01: rt_ainfo, 10: print_einfo, 11: rt_einfo */
static int
send_xfer_cmd(struct ev_loop *loop, int fd)
{
	unsigned int  c;

	if (xbuf->xw == xbuf->xr) return 0;
	c = xbuf_get(xbuf);
	if (c == TMSTP_CMD) { /* DONE */
		send_timestamp(fd);
		add_tstamp_timer(loop, 5.0);
		DBG("--- send the timestamp for real time print. ---\n");
	} else if (c == LNK_CMD) {
		lnkopt ^= 1;
		turn_onoff_linkage(fd, lnkopt);
		add_lnkage_timer(loop, lnktm[lnkopt] + 0.0);
		DBG("--- send the linkage cmd. ---\n");
	} else {
//		(*msg_send[(c>>16)&3])(fd, c&0xffff);
		(*msg_send[(c>>16)&7])(fd, c&0xffff);
	}

	add_ptev(loop, XFERTMO, try_send_cmd);

	return 1;
}

/* DONE */
static void
send_cmd(struct ev_loop *loop, ev_io *w, int revents)
{
	unsigned char cmd[] = {0x81, 0x00, 0x8f};
	int  fd = w->fd;

	ev_io_stop(loop, w);
	if (send_xfer_cmd(loop, fd)) return;

	pollidx = (pollidx + 1) % ndev;
	if (pollidx == 0) {
		if (nack == 0&&rs_st == 1) {
			++rserr;
			if (rserr == RETRY) {
				send_event(EV_RS485E, 0);
				rs_st = 0;
				rserr = 0;
			}
		}
		nack = 0;
	}
	cmd[1] = devaddr[pollidx];
	ignore_result(write(fd, cmd, sizeof(cmd)));

	add_rev(loop, fd);
	add_tev(loop, POLLTMO, poll_timeout);
}

/* DONE */
static void
first_send_cmd(struct ev_loop *loop, ev_io *w, int revents)
{
	unsigned char cmd[] = {0x81, 0x00, 0x8f};

	ev_io_stop(loop, w);
	cmd[1] = devaddr[pollidx];
	ignore_result(write(w->fd, cmd, sizeof(cmd)));

	add_rev(loop, w->fd);
	add_tev(loop, POLLTMO, poll_timeout);
}

/* DONE */
static void
add_wev(struct ev_loop *loop, int fd, void *cb)
{
	ev_io_init(&pollw, cb, fd, EV_WRITE);
	ev_io_start(loop, &pollw);
}

///////////////////////////////////////////////////////////////

/* DONE */
static void
handle_xfercmd(struct ev_loop *loop, ev_io *w, int revents)
{
	unsigned int  cmd;

	if (revents & EV_READ) {
		if (read(w->fd, &cmd, sizeof(cmd)) > 0) {
			if (cmd == 0xffffffff) {
				DBG("--- RECEIVED THE RS485 THREAD EXIT CMD ---\n");
				ev_io_stop(loop, &pollw); //stop read watcher or write watcher.
				remove_tev(loop);
				remove_ptev(loop);
				remove_tstamp_timer(loop);
				remove_lnkage_timer(loop);
				ev_io_stop(loop, w);
				ev_break(loop, EVBREAK_ALL);
			} else {
				stop_send_timer(loop);
				xbuf_put(xbuf, cmd);
				add_ptev(loop, XFERTMO, try_send_cmd);
				DBG("--- QUERY PRINT OR REAL-TIME XFER CMD FROM OTHER THREAD ---\n");
			}
		}
	}
}

/* DONE */
static void*
rs485_loop(void* arg)
{
	ev_io  ev;

	rs_fd = open_uart(RS485, 9600);
	if (rs_fd < 0) {
		printf("--- open rs485 failed ---\n");
		exit(1);
	}

	if (ucf->tm[0] != 0xff) {
		stm = ucf->tm[0]*3600 + ucf->tm[1]*60 + ucf->tm[2];
		etm = ucf->tm[3]*3600 + ucf->tm[4]*60 + ucf->tm[5];
		lnktm[0] = (SECS_PER_DAY + stm - etm) % SECS_PER_DAY;
		lnktm[1] = SECS_PER_DAY - lnktm[0];
	}

	xbuf_reset(xbuf);

	rtpr = 0;
	rlnk = 0;
	rserr = 0;
	pollidx = 0;

	psndtw = NULL;
	prcvw = NULL;
	rtw = NULL;
	plkw = NULL;

	flush_pipe(rfd);

	rbase = ev_loop_new(EVFLAG_AUTO);

	ev_io_init(&ev, handle_xfercmd, rfd[0], EV_READ);
	ev_io_start(rbase, &ev);

	add_wev(rbase, rs_fd, first_send_cmd);

	DBG("--- enter rs485 loop ---\n");
	ev_run(rbase, 0);

	ev_loop_destroy(rbase);	

	close(rs_fd);
	DBG("--- quit rs485 loop ---\n");

	pthread_exit(NULL);
}

void
rs485_init(void)
{
//	if (rs_inited == 0) {
		init_pipe(rfd);
		xbuf = xbuf_new(XFER_SZ);
//		rs_inited = 1;
//	}
}

void
rs485_uninit(void)
{
//	if (rs_inited == 1) {
		close(rfd[0]);
		close(rfd[1]);
		xbuf_free(&xbuf);
//		rs_inited = 0;
//	}
}

/* DONE */
void
rs485_start(void)
{
	rs485_lock();
	if (rret != 0) {
		ucf = get_uart_conf();
		if (ucf == NULL||uart_dev_init(ucf) == 0) {
			send_event(EV_RS485R, 0);
			rs_st = 1;
			rs485_unlock();
			return;
		}
		ifrpr = get_real_print();
		DBG("--- realtime print: %s ---\n", ifrpr ? "valid" : "invalid");
		rret = pthread_create(&rtid, NULL, rs485_loop, NULL);
		if (rret != 0) {
			rs485_uninit();
		}
	}
	rs485_unlock();
}

/* DONE */
void
rs485_exit(void)
{
	unsigned int  cmd = 0xffffffff;

	rs485_lock();
	if (rret == 0) {
		ignore_result(write(rfd[1], &cmd, sizeof(cmd)));
		pthread_join(rtid, NULL);
		rret = -1;
		DBG("---------- EXIT THE RS485 LOOP --------\n");
	}
	rs485_unlock();
}

///////////////////////////////////////////////////////////////////
/* called by UI thread */
/* cmd: bit16: 1-real time transfer, 0-print; bit17: 0-alarm info, 1-event info. */
/* t: 0-alarm info, 1-event info */
static inline void
uart_xfer(int id, int rt, int t)
{
	unsigned int  cmd;

	rs485_lock();
	if (rret == 0) {
		cmd = (id & 0xffff)|(rt<<16)|(t<<17);
		ignore_result(write(rfd[1], &cmd, sizeof(cmd)));
	}
	rs485_unlock();
}

/* the 2nd arg:0 not means print msg. */
void
uart_xfer_zstat(int id)
{
	uart_xfer(id, 0, 2);
}

/* called by UI thread */
/* t: 0-alarm info, 1-event info */
void
uart_xfer_realt(int id, int t)
{
	DBG("--- xfer id = %d, t = %d ---\n", id, t);
	uart_xfer(id, 1, t);
}

/* called by UI thread */
/* cmd: bit16: 1-real time transfer, 0-print; bit17: 0-alarm info, 1-event info. */
/* t: 0-alarm info, 1-event info */
void
uart_xfer_print(int id, int t)
{
	uart_xfer(id, 0, t);
}



