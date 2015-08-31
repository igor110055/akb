#ifndef CONN_H_
#define CONN_H_

#include "ulib.h"

#define MAXRETRY     3

typedef struct _conn {
	ev_io  nw;     //net watcher
	ev_timer  tw;  //timer watcher
	ev_timer  hw;  //timer watcher
	int    hb;
	int    fd;
	int    count;
	char  *host;
	unsigned short port;
	double tmeo;
	void  (*connected)(struct ev_loop *l, struct _conn *con);
	void  (*exit)(struct ev_loop *l, struct _conn *con);
} conn_t;

#define  tcp_connected_set_cb(con, cb)   (con)->connected = (cb)
#define  tcp_connect_set_host(con, ip)   (con)->host = (ip)

extern int   tcp_connect(struct ev_loop *l, conn_t *con);
extern void  tcp_conn_del(struct ev_loop *l, conn_t *con);
extern void  tcp_conn_retry(struct ev_loop *l, conn_t *con, int active);

static inline int
tcp_conn_send(conn_t *con, void *buf, int n)
{
	return write(con->fd, buf, n);
}

static inline void
tcp_conn_add_rev(struct ev_loop *l, conn_t *con, void *cb)
{
	ev_io_init(&con->nw, cb, con->fd, EV_READ);
	ev_io_start(l, &con->nw);
}

static inline void
tcp_conn_del_rev(struct ev_loop *l, conn_t *con)
{
	ev_io_stop(l, &con->nw);
}

static inline void
tcp_conn_add_tev(struct ev_loop *l, conn_t *con, void *cb, double t)
{
	ev_timer_init(&con->tw, cb, t, 0.);
	ev_timer_start(l, &con->tw);
}

static inline void
tcp_conn_del_tev(struct ev_loop *l, conn_t *con)
{
	ev_timer_stop(l, &con->tw);
}

static inline void
tcp_conn_add_hw(struct ev_loop *l, conn_t *con, void *cb, double t)
{
	ev_timer_init(&con->hw, cb, t, 0.);
	ev_timer_start(l, &con->hw);
	con->hb = 1;
}

static inline void
tcp_conn_del_hw(struct ev_loop *l, conn_t *con)
{
	ev_timer_stop(l, &con->hw);
	con->hb = 0;
}

static inline void
tcp_conn_set_iocb(conn_t *con, void *cb)
{
	ev_set_cb(&con->nw, cb);
}

#endif


