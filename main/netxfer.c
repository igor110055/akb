#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "mxc_sdk.h"
#include "ulib.h"
#include "buffer.h"
#include "conn.h"
#include "config.h"
#include "znode.h"
#include "zopt.h"
#include "stb.h"
#include "xfer.h"
#include "pztimer.h"
#include "gui.h"


#define MPORT      9000
#define BUF_SZ     64

#define HBITVL     5.0

typedef struct  _sess {
	UListHdr  list;
	ev_io     nw;
	ev_io     pw;
	ev_timer  tw;
	int       nfd;
	int       id;
	char      ip[16];
} sess_t;

typedef struct cli_msg {
	unsigned int  cmdid;  //devid|(cmd<<16),cmd:0-login,1--operation,2-heartbeat,3-alarm clear..
	unsigned int  data;
}__attribute__((packed)) cli_msg_t;

static int        g_nt = -1;
static pthread_t  g_nid = 0;

static pthread_mutex_t     nmtx = PTHREAD_MUTEX_INITIALIZER;
#define net_lock()         pthread_mutex_lock(&nmtx)
#define net_unlock()       pthread_mutex_unlock(&nmtx)

static UListHdr   free_clist = {&free_clist, &free_clist};
static UListHdr   sess_head = {&sess_head, &sess_head};

static int    npfd[2] = {-1, -1};
static ev_io  g_mw;

static int    sys_busy = 0;

static char  *usrptr = NULL;

static unsigned short   g_cfgmap = 0;

static sess_t  *g_sess[99] = {NULL};

void
cfg_map_set(int i)
{
	g_cfgmap |= (1 << i);
}

void
cfg_map_clr(void)
{
	g_cfgmap = 0;
}

////////////////////////////////////////////////////////////////
//             master routine
////////////////////////////////////////////////////////////////
#if 0
{
#endif
static void   recv_cb(struct ev_loop *l, ev_io *w, int revents);
static void   add_hb_timer(struct ev_loop *l, sess_t  *s);

void
set_usrptr(char *p)
{
	usrptr = p;
}

/* DONE */
static sess_t*
sess_new(void)
{
    sess_t* s = NULL;
    
    if (u_list_empty(&free_clist)) {
        s = calloc(1, sizeof(sess_t));
        if (s != NULL) {
            U_INIT_LIST_HEAD(&s->list);
			s->nfd = -1;
			s->id = 0;
        }
    } else {
        s = (sess_t*)(free_clist.next);
        u_list_del(&s->list);
		memset(s, 0, sizeof(sess_t));
        U_INIT_LIST_HEAD(&s->list);
		s->nfd = -1;
		s->id = 0;
    }
    return s;
}

/* DONE */
static void
sess_free(sess_t* s)
{
    if (!u_list_empty(&s->list)) {
        u_list_del(&s->list);
    }
    U_INIT_LIST_HEAD(&s->list);
    u_list_append(&s->list, &free_clist);
}

/* DONE */
static void
sess_del(struct ev_loop *l, sess_t *s)
{
	ev_timer_stop(l, &s->tw);
	ev_io_stop(l, &s->nw);
	u_tcp_quit(s->nfd);
	sess_free(s);
}

#if 0
/* DONE */
static void
sess_cleanup(void)
{
	UListHdr *list;

	while (!u_list_empty(&free_clist)) {
		list = free_clist.next;
		u_list_del(list);
		free(list);
	}
}
#endif

/////////////////////////////////////////////////////////
static inline void
clear_slave(sess_t *s)
{
	if (s->id > 0) {
		clr_slave_ui(s->id);
		if (s->id <= MAXSLV) {
			stb_update_slave(s->id, 0);
		}
	}
}

/* DONE */
static void
wait_timeout(struct ev_loop *l, ev_timer *w, int revents)
{
	sess_t *s = U_CONTAINER_OF(w, struct _sess, tw);

	DBG("--- connection timeout: no heartbeat received ---\n");
	//timer watcher automatically stopped.
	clear_slave(s);
	sess_del(l, s);
}

/* DONE */
static void
add_hb_timer(struct ev_loop *l, sess_t  *s)
{
	ev_timer_init(&s->tw, wait_timeout, 20., 0.);
	ev_timer_start(l, &s->tw);
}

/* DONE */
static void
handle_login(struct ev_loop *l, sess_t *s, cli_msg_t *m)
{
	char  buf[256] = {0};
	int   n;

	if (sys_busy) return;
	n = sprintf(buf, "#0|%d|%s", g_devid, usrptr);
	n += make_crc_buf(buf+n, sizeof(buf)-n);
	DBG("---  send login ack: %s ---\n", buf);
	if (write(s->nfd, buf, n) < 0) {
		DBG("---  send login ack failed ---\n");
		sess_del(l, s);
		return;
	}
	s->id = m->cmdid & 0xffff;
	s->id /= 100;
	if (s->id > 0) {
		g_sess[s->id-1] = s;
		set_slave_ui(s->id, s->ip);
		if (s->id <= MAXSLV) {
			stb_update_slave(s->id, 1);
		}
		add_hb_timer(l, s);
	} else {
		sess_del(l, s);
	}
}

/* DONE */
static void
handle_opt(struct ev_loop *l, sess_t *s, cli_msg_t *m)
{
	unsigned int cmd;

	if (sys_busy) return;
	cmd = (m->data)|(s->id << 24);
	send_zcmd(&cmd, sizeof(cmd));
	add_hb_timer(l, s);
}

/* DONE */
static void
handle_hb(struct ev_loop *l, sess_t *s, cli_msg_t *m)
{
//	DBG("--- received the heartbeat of slave-%d ---\n", s->id);
	add_hb_timer(l, s);
}

static void
handle_alrmclr(struct ev_loop *l, sess_t *s, cli_msg_t *m)
{
	struct list_hdr *list;
	sess_t  *sess;
	char   buf[32] = {0};
	int    n;
	int    id;

	if (m->data == 0xffff) {
		clear_alarm(-1);
		id = -1;
	} else {
		clear_alarm(m->data);
		id = m->data;
	}

	list = sess_head.next;
	while (list != &sess_head) {
		sess = (sess_t*)list;
		list = list->next;
		if (sess != s) {
			ev_timer_stop(l, &sess->tw);
			n = sprintf(buf, "#%d|%d|%d\r\n", AlrmClrCmd, g_devid, id);
			if (u_writen(sess->nfd, buf, n) < 0) {
				DBG("--- xfer error ---\n");
				clear_slave(sess);
				sess_del(l, sess);
			} else {
				add_hb_timer(l, sess);
			}
		}
	}
	add_hb_timer(l, s);
}

static void (*do_rcmd[])(struct ev_loop *, sess_t *, cli_msg_t*) = {handle_login, handle_opt, handle_hb, handle_alrmclr};

/* DONE */
static void
handle_rcmd(struct ev_loop *l, sess_t *s)
{
	cli_msg_t  msg;
	int   n;
	int   cmd;
	int   devid;

	n = read(s->nfd, &msg, sizeof(msg));
	if (n <= 0) {
		DBG("--- read error ---\n");
		clear_slave(s);
		sess_del(l, s);
		return;
	} else {
		cmd = (msg.cmdid >> 16) & 3;
		devid = msg.cmdid & 0xffff;
		if (devid % 100 != g_devid) {
			DBG("--- slave devid error ---\n");
			clear_slave(s);
			sess_del(l, s);
			return;
		} else {
			do_rcmd[cmd](l, s, &msg);
		}
	}
}

/* DONE */
static void
recv_cb(struct ev_loop *l, ev_io *w, int revents)
{
    sess_t *s = U_CONTAINER_OF(w, struct _sess, nw);

    if (revents & EV_ERROR) {
        DBG("--- SOCKET ERROR ---\n");
		clear_slave(s);
		sess_del(l, s);
        return;
    }

	ev_timer_stop(l, &s->tw);
    if (revents & EV_READ) {
        handle_rcmd(l, s);
    }
}

/* DONE */
static void
listen_cb(struct ev_loop *l, ev_io *w, int revents)
{
    int   conFd = -1;
    sess_t  *s = NULL;
	struct sockaddr_in addr;
	socklen_t  addrsize;

	addrsize = sizeof(addr);
	conFd = accept(w->fd, (struct sockaddr *)&addr, &addrsize);
	if (conFd >= 0)
	{
		if (sys_busy) {
			close(conFd);
			return;
		}
	    u_set_nonblock(conFd);
		u_tcp_set_nodelay(conFd);

		s = sess_new();
		if (s)
		{
			s->nfd = conFd;
			inet_ntop(AF_INET, &addr.sin_addr, s->ip, 16);
			DBG("--- connection accepted: peerip = %s ---\n", s->ip);
			u_list_append(&s->list, &sess_head);
			ev_io_init(&s->nw, recv_cb, conFd, EV_READ);
            ev_io_start(l, &s->nw);
//			add_hb_timer(l, s);
		}
		else
		{
			DBG("--- sess_new failed ---\n");
			close(conFd);
		}
	}
}

/* DONE */
static void
cleanup_iow(struct ev_loop *l)
{
	UListHdr *list;
	sess_t   *s;

	while (!u_list_empty(&sess_head)) {
		list = sess_head.next;
		u_list_del(list);
		s = (sess_t*)list;
		sess_del(l, s);
	}
	clr_slave_ui(0);
//	stb_display_slave(0);
}

/* DONE */
static void
xfer_buf(struct ev_loop *l, char *buf, int n)
{
	struct list_hdr *list;
	sess_t  *s;

	list = sess_head.next;
	while (list != &sess_head) {
		s = (sess_t*)list;
		list = list->next;
		DBG("--- send: %s ---\n", buf);
		ev_timer_stop(l, &s->tw);
		if (u_writen(s->nfd, buf, n) < 0) {
			DBG("--- xfer error ---\n");
			clear_slave(s);
			sess_del(l, s);
		} else {
			add_hb_timer(l, s);
		}
	}
}

/* DONE */
static void
xfer_cmd(struct ev_loop *l)
{
	xbuf_t  *x;

	x = xfer_buf_get();
	while (x) {
		DBG("--- xfer cmd: %s ---\n", x->buf);
		xfer_buf(l, x->buf, x->n);
		xfer_buf_free(x);
		x = xfer_buf_get();
	}
}

/* DONE */
static void
xfer_ack(struct ev_loop *l)
{
	xres_t  *x;
	sess_t  *s;

	x = xr_buf_get();
	while (x) {
		s = g_sess[x->id-1];
		if (s) {
			ev_timer_stop(l, &s->tw);
			if (u_writen(s->nfd, x->buf, x->n) < 0) {
				DBG("--- xfer error ---\n");
				clear_slave(s);
				sess_del(l, s);
				g_sess[x->id-1] = NULL;
			} else {
				add_hb_timer(l, s);
			}
			DBG("--- xfer ack: %s to slave-%d ---\n", x->buf, x->id);
		}
		xr_buf_free(x);
		x = xr_buf_get();
	}
}

/* DONE */
static void
xfer_cfg_changed(struct ev_loop *l, int map)
{
	char    buf[32] = {0};
	int     n;

	n = sprintf(buf, "#%d|%d|%d\r\n", CfgChgCmd, g_devid, map);
	DBG("--- config changed: %s ---\n", buf);
	xfer_buf(l, buf, n);
}

/* DONE */
static void
handle_mcmd(struct ev_loop *l, ev_io *w, int revents)
{
	unsigned short  v;
	int   map;
	int   id;

	if (revents & EV_READ) {
		if (read(w->fd, &v, sizeof(v)) == sizeof(v)) {
			id = v >> 12;
			switch (id) {
			case 0:
				ev_io_stop(l, w);
				ev_io_stop(l, &g_mw);
				cleanup_iow(l);
				ev_break(l, EVBREAK_ALL);
				DBG("--- going to quit the file server loop ---\n");
				break;
			case 1: //transfer cmd.from zopt engine thread.
				xfer_cmd(l);
				break;
			case 2: //transfer cfg changed cmd.from UI thread when exit the system programing.
				map = v & ((1 << 12) - 1);
				if (!u_list_empty(&sess_head)&&map != 0) {
					xfer_cfg_changed(l, map);
				}
				sys_busy = 0;
				enable_syncfg();
				break;
			case 3: //xfer result.
				xfer_ack(l);
				break;
			case 4: 
				sys_busy = 1;
				DBG("--- enter into system programing. ---\n");
				break;
			default:
				break;
			}
		}
	}
}

/* DONE */
static void*
master_loop(void* arg)
{
	struct ev_loop   *l;
	int  listenFd;
	ev_io  pw;

	l = ev_loop_new(EVFLAG_AUTO);
	if (l == NULL) exit(1);

	DBG("--- going to enter master loop ---\n");
	listenFd = u_tcp_serv(NULL, MPORT);
	if (listenFd < 0)
	{
		DBG("--- create tcp server failed ---\n");
		ev_loop_destroy(l);
		exit(1);
	}
	u_set_nonblock(listenFd);

	ev_io_init(&g_mw, listen_cb, listenFd, EV_READ);
	ev_io_start(l, &g_mw);

	ev_io_init(&pw, handle_mcmd, npfd[0], EV_READ);
	ev_io_start(l, &pw);

	ev_run(l, 0);

	close(listenFd);

    ev_loop_destroy(l);
	DBG("--- exit master loop ---\n");

    pthread_exit(NULL);
}

/* v:0-quit,1-operation,2-cfg changed. */
static void
inform_master(int v)
{
	unsigned short cmd = (unsigned short)v;
	net_lock();
	if (g_nt == 0) {
		ignore_result(write(npfd[1], &cmd, sizeof(cmd)));
	}
	net_unlock();
}

/* called by zopt thread. */
void
xfer_result(void)
{
	inform_master(3 << 12);
}

/* called by zopt thread. */
void
xfer_netcmd(void)
{
	inform_master(1 << 12);
}

/* called by UI thread when exit the system programing. because g_cfgmap is accessed by UI thread when config changed. */
void
xfer_cfgcmd(void)
{
	g_cfgmap &= ((1 << 12) - 1);
	inform_master((2 << 12)|g_cfgmap);
	g_cfgmap = 0;
}

/* called by syncfg thread before config file being changed. */
void
set_busy(void)
{
	inform_master(4 << 12);
}

#if 0
}
#endif

////////////////////////////////////////////////////////////////
//             slave routine
////////////////////////////////////////////////////////////////
#if 0
{
#endif
#define RBUFSZ     4096

typedef struct _scli {
	conn_t   con;
	ev_io    pw;
	buffer_t rbuf;
	int      ready;
	int      result;
} scli_t;

static void  try_login(struct ev_loop *l, conn_t *con);
static void  conn_exit(struct ev_loop *l, conn_t *con);

static scli_t   g_slv = {
	.con = {
		.fd = -1,
		.host = g_masterip,
		.port = MPORT,
		.tmeo = 3.0,
		.connected = try_login,
		.exit = conn_exit,
	},
	.ready = 0,
	.result = 0,
};

static void  recv_cmd(struct ev_loop *l, ev_io *w, int revents);

/* DONE */
static void
send_hb(struct ev_loop *l, ev_timer *w, int revents)
{
	conn_t *con = U_CONTAINER_OF(w, struct _conn, hw);
	cli_msg_t   msg;

	if (revents & EV_TIMER) {
//		DBG("--- time to send heartbeat. ---\n");		
		msg.cmdid = g_devid|(2 << 16);
		msg.data = 0;

		if (tcp_conn_send(con, &msg, sizeof(msg)) < 0) {
			DBG("--- tcp send error ---\n");
			tcp_conn_retry(l, con, 1);
			return;
		}
		tcp_conn_add_hw(l, con, send_hb, HBITVL);
	}
}

////////////////////////////////////////////////////////////////////
#if 0
{
#endif

typedef void (*PCHAR)(scli_t *, char *, int);

static int  allflag = 0;

/* DONE */
static void
handle_cfg(scli_t *s, char *buf, int id)
{
	unsigned int  map;

	map = (unsigned int)strtol(buf, NULL, 10);
	if (map) {
		s->ready = 0;
		map |= (1 << MAXCFGID);
		start_syncfg(map);
	}
	DBG("--- master cfg changed, map = %u ---\n", map);
}

/* #2|devid|usrid|op|t\r\n */
static void
all_armopt(scli_t *s, char *buf, int id)
{
	int    usrid;
	int    op;
	int    t;
	int    d;
	char  *p;
	char  *ptr = buf;
	unsigned int  map;

	usrid = strtol(ptr, &p, 10);
	if (usrid == 0&&p == ptr) return;
	ptr = p + 1;

	op = strtol(ptr, &p, 10);
	if (op == 0&&p == ptr) return;
	ptr = p + 1;

	t = strtol(ptr, &p, 10);
	if (t == 0&&p == ptr) return;

	DBG("--- master cmd: all opt: op = %d, t = %d ---\n", op, t);

	d = (id == g_devid) ? 1 : 0; //if d == 1,then display success message.
	allflag = (op + 1 + t)|(d << 4);
	s->ready = 0;
	map = (1 << MAXCFGID);
	start_syncfg(map);
}

/* DONE */
/* #3|devid|usrid|op|t|pznr:pzst|z0:zst0|z1:zst1бн..\r\n */
/* type: 0-normal, 1-timerop, 2-forcedop */
/* op: 0-disarm,1-arm,2-bypass restore,3-bypass */
static void
pzone_opt(scli_t *s, char *buf, int unused)
{
	int   usrid;
	int   op;
	int   t;
	char *ptr = buf;
	char *p;
	int   pz;
	int   z;
	int   st;

	usrid = strtol(ptr, &p, 10);
	if (usrid == 0&&p == ptr) return;
	ptr = p + 1;

	
	op = strtol(ptr, &p, 10);
	if (op == 0&&p == ptr) return;
	ptr = p + 1;

	t = strtol(ptr, &p, 10);
	if (t == 0&&p == ptr) return;
	ptr = p + 1;

	pz = strtol(ptr, &p, 10);
	if (pz == 0&&p == ptr) return;
	ptr = p + 1;

	st = strtol(ptr, &p, 10);
	ptr = p;

	pstat[pz] = (char)st;
	if (op != 1) {
		while (*ptr == '|') {
			++ptr;
			z = strtol(ptr, &p, 10);
	    	if (z == 0&&p == ptr) return;
			ptr = p + 1;

			st = strtol(ptr, &p, 10);
			if (st == 0&&p == ptr) return;
			ptr = p;
			zstat[zoneidx(z)] = (char)st;
			if (st == ZST_NRDY)
				nready_put(z);
			else
				nready_del(z);
		}
	} else { //pzone arm.
		while (*ptr == '|') {
			++ptr;
			z = strtol(ptr, &p, 10);
	    	if (z == 0&&p == ptr) return;
			ptr = p + 1;

			st = strtol(ptr, &p, 10);
			if (st == 0&&p == ptr) return;
			ptr = p;
			zstat[zoneidx(z)] = (char)st;
			if (st == ZST_NRDY)
				nready_put(z);
			else
				nready_del(z);
		}
	}

	DBG("--- master cmd: pzone opt: op = %d, t = %d, pznr = %d ---\n", op, t, pz);
	pzone_opr(pz, op, t);
}

/* DONE */
/* #4|devid|usrid|op|znr:zst|pzn:pzst\r\n */
static void
zone_opt(scli_t *s, char *buf, int unused)
{
	char *ptr = buf;
	char *pp;
	int   op;
	int   z;
	int   st;
	int   usrid;
	int   p;
	int   pst;

	usrid = strtol(ptr, &pp, 10);
	if (usrid == 0&&pp == ptr) return;
	ptr = pp + 1;

	op = strtol(ptr, &pp, 10);
	if (pp == ptr&&op == 0) return;
	ptr = pp + 1;

	z = strtol(ptr, &pp, 10);
	if (z == 0&&pp == ptr) return;
	ptr = pp + 1;

	st = strtol(ptr, &pp, 10);
	if (st == 0&&pp == ptr) return;
	ptr = pp + 1;

	p = strtol(ptr, &pp, 10);
	if (p == 0&&pp == ptr) return;
	ptr = pp + 1;

	pst = strtol(ptr, &pp, 10);
	if (pst == 0&&pp == ptr) return;

	DBG("--- zone opt: op = %d, z = %d, zst = %d, pz = %d, pst = %d ---\n", op, z, st, p, pst);

	zone_opr(op, z, st, p, pst);
}

/* DONE */
static void
timer_opr(scli_t *s, char *buf, int unused)
{
	char *ptr = buf;
	char *pp;
	int   usrid;
	int   op;
	int   i;
	
	usrid = strtol(ptr, &pp, 10);
	if (usrid == 0&&pp == ptr) return;
	ptr = pp + 1;

	op = strtol(ptr, &pp, 10);
	if (op == 0&&pp == ptr) return;
	ptr = pp + 1;
	
	i = strtol(ptr, &pp, 10);
	if (i == 0&&pp == ptr) return;

	DBG("--- pzone timer opt: op = %d, z = %d ---\n", op, i);

	tmer_opt(op, i);
}

/* DONE */
/* #6|devid|id\r\n */
static void
clear_alarminfo(scli_t *s, char *buf, int unused)
{
	char *pp;
	int   id;
	
	id = strtol(buf, &pp, 10);
	if (id == 0&&pp == buf) return;

	DBG("--- alarm clear: id = %d ---\n", id);
	clear_alarm(id);
}

/* DONE */
/* #7|devid|evid|hid\r\n */
static void
event_notify(scli_t *s, char *buf, int unused)
{
	char *ptr = buf;
	char *p;
	int   eid;
	int   hid;

	eid = strtol(ptr, &p, 10);
	if (eid == 0&&p == ptr) return;
	ptr = p + 1;
	hid = strtol(ptr, &p, 10);
	if (hid == 0&&p == ptr) return;

	send_evt_slave(eid, hid);
	DBG("--- event: eid = %d, hid = %d ---\n", eid, hid);
}

///////////////////////////////////////////////////////////////
static void
disable_pztmr_ack(int arg, int r)
{
	return;
}

static void
enable_pztmr_ack(int arg, int r)
{
	return;
}

static void
pztmr_init_ack(int arg, int r)
{
	return;
}

static void
za_ack(int arg, int r)
{
	return;
}

static void
zar_ack(int arg, int r)
{
	return;
}

static void  (*do_ack[])(int, int) = {
	[ODAM]	 = zdisarm_ack,
	[OARM]   = zarm_ack,
	[OZBP]   = zbp_ack,
	[OZBPR]  = zbpr_ack,
	[OPZAM]  = pzarm_ack,
	[OPZFA]  = pzfarm_ack,
	[OPZDA]  = pzdisarm_ack,
	[OPZBP]  = pzbp_ack,
	[OPZBPR] = pzbpr_ack,
	[OTMRDN] = disable_pztmr_ack,
	[OTMREN] = enable_pztmr_ack,
	[OPZTMR] = pztmr_init_ack,
	[OZALT]  = za_ack,
	[OZALTR] = zar_ack,
};

/* #9|devid|result|cmd|arg\r\n */
static void
cmd_result(scli_t *s, char *buf, int id)
{
	char *ptr = buf;
	char *p;
	int   res;
	int   cmd;
	int   arg;

	res = strtol(ptr, &p, 10);
	if (res == 0&&p == ptr) return;
	ptr = p + 1;

	cmd = (unsigned short)strtol(ptr, &p, 10);
	if (cmd == 0&&p == ptr) return;
	ptr = p + 1;

	arg = (unsigned short)strtol(ptr, &p, 10);
	
	DBG("--- cmd ack: res = %d, cmd = %d, arg = %d ---\n", res, cmd, arg);
	if (cmd <= OPZBPR) { 
		(*do_ack[cmd])(arg, res);
	}
}

static PCHAR   fn_mcmd[] = {
	[CfgChgCmd-CfgChgCmd] = handle_cfg,   /* #1|devid|map\r\n */
	[AllOptCmd-CfgChgCmd] = all_armopt,   /* #2|devid|usrid|op|length\r\n+data */
	[PzOptCmd-CfgChgCmd]  = pzone_opt,    /* #3|devid|usrid|op|pznr:pzst|z0:zst0|z1:zst1бн..\r\n */
	[ZoneOpCmd-CfgChgCmd] = zone_opt,     /* #4|devid|usrid|op|znr:zst|pzn:pzst\r\n */
	[TmerOpCmd-CfgChgCmd] = timer_opr,    /* #5|devid|op|pznr\r\n */
	[AlrmClrCmd-CfgChgCmd] = clear_alarminfo, /* #6|devid|id\r\n */
	[EvtNtfCmd-CfgChgCmd] = event_notify, /* #7|devid|evid|hid\r\n */
	[AckOfCmd-CfgChgCmd]  = cmd_result,   /* #8|devid|result|cmd|arg\r\n */
};

/* DONE */
static void
cmd_timeout(struct ev_loop *l, ev_timer *w, int revents)
{
	conn_t *con = U_CONTAINER_OF(w, struct _conn, tw);
	scli_t *s = (scli_t *)con;

	if (revents & EV_TIMER) {
		DBG("--- receiving cmd timeout, will reconnect. ---\n");
		buffer_reset(&s->rbuf);
		tcp_conn_retry(l, con, 1);
	}
}

static void
link_quit(struct ev_loop *l, scli_t *s)
{
	tcp_conn_del(l, &s->con);
	ev_io_stop(l, &s->pw);
	ev_break(l, EVBREAK_ALL);
	s->ready = 0;
	s->result = 1;
}

/* DONE */
static void
parse_mcmd(struct ev_loop *l, scli_t *s)
{
	char *ptr;
	char *p;
	char *pt;
	int   cmd;
	int   id;

	if (buffer_read(&s->rbuf, s->con.fd) <= 0) {
		DBG("--- READ ERROR when receiving cmd. ---\n");
		buffer_reset(&s->rbuf);
		tcp_conn_retry(l, &s->con, 1);
		return;
	}

again:
	DBG("--- received %d bytes: %s ---\n", BUFPTR_LENGTH(&s->rbuf), BUFPTR_DATA(&s->rbuf));
	ptr = buffer_find_chr(&s->rbuf, '#');
	if (ptr == NULL) {
		DBG("--- no start flag found and ignore it. ---\n");
		buffer_reset(&s->rbuf);
		return;
	}

	pt = buffer_find(&s->rbuf, "\r\n", 2);
	if (pt == NULL) {
		DBG("--- Not completed and continue receiving cmd. ---\n");
		tcp_conn_add_tev(l, &s->con, cmd_timeout, 5.0);
		return; //not completed and continue receiving.
	}
	pt += 2;

	++ptr;
	cmd = (int)strtol(ptr, &p, 10);
	if (cmd == 0&&p == ptr) {
		DBG("--- cmd error, ignore it. ---\n");
		buffer_reset(&s->rbuf);
		return;
	}

	ptr = p + 1;
	id = (int)strtol(ptr, &p, 10);
	if (id % 100 != masterid) {
		DBG("--- devid = %d error, ignore it. ---\n", id);
		buffer_reset(&s->rbuf);
		link_quit(l, s);
		return;
	}

	ptr = p + 1;
	if (s->ready == 1&&cmd >= CfgChgCmd&&cmd < BusyCmd) {
		(*fn_mcmd[cmd-CfgChgCmd])(s, ptr, id);
		buffer_drain_ptr(&s->rbuf, pt);
		if (BUFPTR_LENGTH(&s->rbuf) > 0) {
			DBG("--- there is another cmd in buffer ---\n");
			goto again;
		}
	} else {
		buffer_reset(&s->rbuf);
	}
}

/* DONE */
static void
recv_cmd(struct ev_loop *l, ev_io *w, int revents)
{
	conn_t *con = U_CONTAINER_OF(w, struct _conn, nw);
	scli_t *s = (scli_t *)con;

	if (revents & EV_ERROR) {
		DBG("--- SOCKET ERROR  when receiving cmd. ---\n");
		tcp_conn_retry(l, con, 1);
		return;
	}

	tcp_conn_del_tev(l, con);
	if (revents & EV_READ) {
		parse_mcmd(l, s);
	}
}

#if 0
}
#endif


////////////////////////////////////////////////////////////////
#if 0
{
#endif

/* DONE */
static void
lack_timeout(struct ev_loop *l, ev_timer *w, int revents)
{
	conn_t *con = U_CONTAINER_OF(w, struct _conn, tw);

	if (revents & EV_TIMER) {
		DBG("--- receiving ack timeout, will retry login. ---\n");
		tcp_conn_del_rev(l, con);
		try_login(l, con);
	}
}

/* DONE */
static int
parse_cfg(scli_t *s, char *buf)
{
	char *ptr = buf;
	char *p;
	int   id;
	unsigned int  crc;
	unsigned int  map = 0;

	p = strchr(ptr, '|');
	if (p == NULL) return 2;
	*p = 0;
	update_username(ptr); //should think twice.
	DBG("--- user name: %s ---\n", ptr);

	ptr = p;
	do {
		++ptr;
		id = (int)strtol(ptr, &p, 10);
		if (ptr == p||*p != ':') return 3;
		ptr = p + 1;
		crc = (unsigned int)strtoll(ptr, &p, 10);
		if (ptr == p) return 3;
		DBG("--- %d: %x ---\n", id, crc);
		if (crc == 0) {
			fi_clear(id);
		} else if (crc != 0&&cfg_crc_diff(id, crc)) {
			map |= (1 << id);
		}
		ptr = p;
	} while (*ptr);

	map |= (1 << MAXCFGID); //flag of the zsync.
	start_syncfg(map);

	return 1;
}

/* DONE
 * return: 0-receiving not completed.
 *         1-receiving ack completed.
 *         2-socket error,need to reconnect.
 *         3-cmd/format error, 
 *         4-devid error.
 * #0|devid|cfgidx:crc32|cfgidx:crc32бн.\r\n
 */
static int
parse_lack(scli_t *s)
{
	char *ptr;
	char *p;
	char *pt;
	int   v;

	if (buffer_read(&s->rbuf, s->con.fd) <= 0) {
		DBG("--- READ ERROR, when login. ---\n");
		return 2;
	}

	ptr = buffer_find_chr(&s->rbuf, '#');
	if (ptr == NULL) {
		DBG("--- no start flag found ---\n");
		return 3;
	}

	pt = buffer_find(&s->rbuf, "\r\n", 2);
	if (pt == NULL) return 0; //not completed and continue receiving.
	*pt = 0;

	DBG("--- received: %s ---\n", ptr);
	++ptr;
	v = (int)strtol(ptr, &p, 10);
	if (v != 0||p == ptr) {
		DBG("--- cmd = %d error ---\n", v);
		return 3;
	}

	ptr = p + 1;
	v = (int)strtol(ptr, &p, 10);
	if (v != masterid) {
		DBG("--- devid = %d error ---\n", v);
		return 4;
	}

	return parse_cfg(s, p + 1);
}

/* DONE */
static void
handle_lack(struct ev_loop *l, ev_io *w, int revents)
{
	conn_t  *con = U_CONTAINER_OF(w, struct _conn, nw);
	scli_t  *s = (scli_t *)con;
	int      r;

	if (revents & EV_ERROR) {
        DBG("--- SOCKET ERROR, when login. ---\n");
        tcp_conn_retry(l, con, 1);
        return;
    }

	tcp_conn_del_tev(l, con);
	if (revents & EV_READ) {
		r = parse_lack(s);
		switch (r) {
		case 0:
			DBG("--- Not completed and continue receiving login ack. ---\n");
			tcp_conn_add_tev(l, con, lack_timeout, 5.0);
			break;
		case 1: //finish receiving ack.
			DBG("--- login success ---\n");
			buffer_reset(&s->rbuf);
			tcp_conn_del_rev(l, con);
			tcp_conn_add_rev(l, con, recv_cmd);
			tcp_conn_add_hw(l, con, send_hb, HBITVL);
			break;
		default: //reconnect
			DBG("--- read error: reprobing master ---\n");
			link_quit(l, s);
			break;
		}
	}
}

/* DONE: connected callback. */
static void
try_login(struct ev_loop *l, conn_t *con)
{
	scli_t *s = (scli_t*)con;
	cli_msg_t	msg;

	msg.cmdid = g_devid;
	msg.data = 0;

	DBG("--- connected to master for login. ---\n");
	if (tcp_conn_send(con, &msg, sizeof(msg)) < 0) {
		DBG("--- write error when connected. ---\n");
		tcp_conn_retry(l, con, 0);
		return;
	}
	DBG("--- send login msg ---\n");

	s->ready = 0;
	buffer_reset(&s->rbuf);
	tcp_conn_add_rev(l, con, handle_lack);
	tcp_conn_add_tev(l, con, lack_timeout, 5.0);
}

#if 0
}
#endif


//////////////////////////////////////////////////////////
static int
disable_pztmr(int p)
{
	return 0;
}

static int
enable_pztmr(int p)
{
	return 0;
}

static int
pztmr_init(int p)
{
	return 1;
}

static int
do_za(int p)
{
	return 0;
}

static int
do_zar(int p)
{
	return 0;
}

static int  (*cannot_rop[])(int) = {
	[ODAM]	 = cannot_zdisarm,
	[OARM]   = cannot_zarm,
	[OZBP]   = cannot_zbpass,
	[OZBPR]  = cannot_zbpassr,
	[OPZAM]  = try_pzarm,
	[OPZFA]  = try_fpzarm,
	[OPZDA]  = try_pzdisarm,  //done
	[OPZBP]  = cannot_pzbp,   //done
	[OPZBPR] = cannot_pzbpr,  //done
	[OTMRDN] = disable_pztmr,
	[OTMREN] = enable_pztmr,
	[OPZTMR] = pztmr_init,
	[OZALT]  = do_za,
	[OZALTR] = do_zar,
};

/*
 *  cmd: bit31~bit24:cmd type:0-zone/pzone cmd,1-alarm clear,2-sync finished,3-quit.
 *       bit23~bit16:cmd id.
 *       bit15~bit0: cmd arg,for example:zone/pzone number.
 */
/* DONE */
static void
handle_pcmd(struct ev_loop *l, ev_io *w, int revents)
{
	unsigned int  cmd;
	scli_t *s = U_CONTAINER_OF(w, struct _scli, pw);	
	cli_msg_t	msg;
	int         cid;
	int         arg;
	int         r;
	int         t;

	if (revents & EV_READ) {
		if (read(w->fd, &cmd, sizeof(cmd)) == sizeof(cmd)) {
			t = cmd >> NCMDSFT;
			switch (t) {
			case 0: //zone/pzone operation.
				if (s->ready == 1) {
					cid = (cmd >> 16)&0xff;
					arg = cmd & 0xffff;

					if (cid >= OZALTR) return;

					r = (*cannot_rop[cid])(arg);
					if (r != 0) {
						(*do_ack[cid])(arg, r);
						return;
					}
					DBG("--- xfer slave cmd. ---\n");
					tcp_conn_del_hw(l, &s->con);
					msg.cmdid = g_devid|(1 << 16);
					msg.data = cmd;

					if (tcp_conn_send(&s->con, &msg, sizeof(msg)) < 0) {
						DBG("--- write error when send xfer cmd. ---\n");
						tcp_conn_retry(l, &s->con, 1);
						return;
					}
					tcp_conn_add_hw(l, &s->con, send_hb, HBITVL);
				}
				break;
			case 1: //alarm clear operation
				if (s->ready == 1) {
					tcp_conn_del_hw(l, &s->con);
					msg.cmdid = g_devid|(3 << 16);
					msg.data = cmd & 0xffff;
					if (tcp_conn_send(&s->con, &msg, sizeof(msg)) < 0) {
						DBG("--- write error when send xfer cmd. ---\n");
						tcp_conn_retry(l, &s->con, 1);
						return;
					}
					tcp_conn_add_hw(l, &s->con, send_hb, HBITVL);
				}
				break;
			case 2: //sync finished cmd from sync thread.
				DBG("--- cfg sync finished. ---\n");
				s->ready = 1;
				if (allflag > 0) {
					handle_allopt((allflag&0xf) - 1, allflag >> 4);
					allflag = 0;
				} else {
					DBG("--- before calc_zonef(). ---\n");
					calc_zonef();
					DBG("--- before update_zmapst(). ---\n");
					update_zmapst(MAXZ, 2);
					DBG("--- after update_zmapst(). ---\n");
				}
				break;
			case 3: //quit
				tcp_conn_del(l, &s->con);
				ev_io_stop(l, w);
				ev_break(l, EVBREAK_ALL);
				s->ready = 0;
				s->result = 0;
				DBG("--- going to quit the client loop ---\n");
				break;
			default:
				break;
			}
		}
	}
}

static void
conn_exit(struct ev_loop *l, conn_t *con)
{
	scli_t *s = (scli_t *)con;

	ev_io_stop(l, &s->pw);
	ev_break(l, EVBREAK_ALL);
	s->ready = 0;
	s->result = 1;
}

/* DONE */
static int
client_loop(struct ev_loop *l, scli_t *s)
{
	s->result = 0;
	ev_io_init(&s->pw, handle_pcmd, npfd[0], EV_READ);
	ev_io_start(l, &s->pw);
	tcp_connect(l, &s->con);
	ev_run(l, 0);
	return s->result;
}

/* DONE */
static void*
slave_loop(void* arg)
{
	struct ev_loop *l = NULL;
	int   r = 0;

	l = ev_loop_new(EVFLAG_AUTO);
	if (l == NULL) exit(1);
	buffer_init(&g_slv.rbuf, RBUFSZ);

	do {
		if (probe_masterip(l, npfd[0])) {
			DBG("--- probing master cancelled ---\n");
			buffer_base_free(&g_slv.rbuf);
			pthread_exit(NULL);
		}
		DBG("--- master ip: %s ---\n", g_masterip);
		r = client_loop(l, &g_slv);
	} while (r);

	buffer_base_free(&g_slv.rbuf);
    ev_loop_destroy(l);
	l = NULL;
	DBG("--- exit file client loop ---\n");

    pthread_exit(NULL);
}

#if 0
}
#endif

///////////////////////////////////////////////////////////////
/* DONE */
static void
start_net_loop(void *cb, int mst)
{
	if ((is_master() != mst)||is_standalone()) return;
	net_lock();
	if (g_nt == -1) {
		init_pipe(npfd);
		DBG("---------- START THE NET LOOP --------\n");
		g_nt = pthread_create(&g_nid, NULL, cb, NULL);
	}
	net_unlock();
}

/* called in syncfg.c when config file synchronized. */
void
finish_syncfg(void)
{
	unsigned int  cmd = (2 << NCMDSFT);
	net_lock();
	if (g_nt == 0) {
		ignore_result(write(npfd[1], &cmd, sizeof(cmd)));
		DBG("---------- finish syncfg --------\n");
	}
	net_unlock();
}

/* DONE */
void
start_master(void)
{
	start_net_loop((void*)master_loop, 1);
}

/* DONE */
void
start_slave(void)
{
	start_net_loop((void*)slave_loop, 0);
}

////////////////////////////////////////////////////////////
static void
slave_quit(void)
{
	unsigned int  cmd = (3 << NCMDSFT);
	ignore_result(write(npfd[1], &cmd, sizeof(cmd)));
}

static void
master_quit(void)
{
	unsigned short cmd = 0;
	ignore_result(write(npfd[1], &cmd, sizeof(cmd)));
}

static void (*net_exit[])(void) = {slave_quit, master_quit};

static void
net_quit(int m)
{
	net_lock();
	if (g_nt == 0) {
		(*net_exit[m])();
		pthread_join(g_nid, NULL);
		g_nid = 0;
		g_nt = -1;
		close(npfd[0]);
		close(npfd[1]);
		DBG("---------- EXIT THE %s LOOP --------\n", m?"MASTER":"SLAVE");
	}
	net_unlock();
}

void
end_master(void)
{
	net_quit(1);
}

void
end_slave(void)
{
	net_quit(0);
}
/////////////////////////////////////////////////////////
/* called by zopt thread. */
void
do_xfer_nack(int cid, unsigned short arg)
{
	unsigned int cmd;

	net_lock();
	if (g_nt == 0) {
		cmd = (cid << 16)|arg;
		ignore_result(write(npfd[1], &cmd, sizeof(cmd)));
	}
	net_unlock();
}

/* called by UI thread. */
void
netxfer_alarmclr_slave(int id)
{
	unsigned int cmd;

	net_lock();
	if (g_nt == 0) {
		cmd = ((unsigned short)id)|(1 << NCMDSFT);
		ignore_result(write(npfd[1], &cmd, sizeof(cmd)));
	}
	net_unlock();
}



