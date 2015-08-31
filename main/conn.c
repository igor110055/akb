#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <sys/ipc.h>
#include "mxc_sdk.h"
#include "conn.h"

/* DONE */
static void
try_connect(struct ev_loop *l, ev_timer *w, int revents)
{
	conn_t *con = U_CONTAINER_OF(w, struct _conn, tw);

	DBG("--- trying to connect after %f secs ---\n", con->tmeo);
	if (con->count < MAXRETRY) {
		tcp_connect(l, con);
		con->count++;
	} else {
		con->count = 0;
		if (con->hb) {
			ev_timer_stop(l, &con->hw);
			con->hb = 0;
		}
		(*con->exit)(l, con);
		DBG("--- quit the connection ---\n");
	}
}

/* DONE */
static void
poll_conn(struct ev_loop *l, ev_io *w, int revents)
{
	int  error = 0;
	socklen_t  errsz = sizeof(error);
	conn_t  *con = U_CONTAINER_OF(w, struct _conn, nw);

	ev_timer_stop(l, &con->tw);
	ev_io_stop(l, w);
	if ((revents & EV_READ)||(revents & EV_WRITE))
	{
		if (getsockopt(w->fd, SOL_SOCKET, SO_ERROR, &error, &errsz) < 0||error != 0) {
			close(con->fd);
			con->fd = -1;
			ev_timer_init(&con->tw, try_connect, con->tmeo, 0.);
			ev_timer_start(l, &con->tw);
		} else {
			DBG("+++ nonblocking connect success +++\n");
			con->count = 0;
			(*con->connected)(l, con);
		}
	}
}

/* DONE */
static void
connect_timeout(struct ev_loop *l, ev_timer *w, int revents)
{
	conn_t *con = U_CONTAINER_OF(w, struct _conn, tw);

	DBG("--- nonblocking connect timeout ---\n");
	ev_io_stop(l, &con->nw);
	close(con->fd);
	con->fd = -1;
	ev_timer_init(&con->tw, try_connect, con->tmeo, 0.);
	ev_timer_start(l, &con->tw);
}

/* DONE */
static void
add_connecting_ev(struct ev_loop *l, conn_t *con)
{
	ev_io_init(&con->nw, poll_conn, con->fd, EV_READ|EV_WRITE);
	ev_io_start(l, &con->nw);
	ev_timer_init(&con->tw, connect_timeout, con->tmeo, 0.);
	ev_timer_start(l, &con->tw);
}

/* DONE */
/* make sure that both the timer watcher and the io watcher is not in active or pending state. */
int
tcp_connect(struct ev_loop *l, conn_t *con)
{
	int  n;
	struct sockaddr_in  saddr;

	memset(&saddr, 0, sizeof(saddr));
	saddr.sin_family = AF_INET;
	saddr.sin_port = htons(con->port);
	inet_aton(con->host, &saddr.sin_addr);

	con->fd = socket(AF_INET, SOCK_STREAM, 0);
	if (con->fd < 0) {
		DBG(" +++ tcp socket error +++\n");
		return -1;
	}

	u_set_nonblock(con->fd);	
	if ((n = connect(con->fd, (struct sockaddr*)&saddr, sizeof(struct sockaddr_in))) < 0) {
		if (errno != EINPROGRESS) {
			DBG("+++ nonblocking connect error +++\n");
			close(con->fd);
			con->fd = -1;
			ev_timer_init(&con->tw, try_connect, con->tmeo, 0.);
			ev_timer_start(l, &con->tw);
		} else {
			DBG("+++ nonblocking connecting... +++\n");
			add_connecting_ev(l, con);
		}
	} else if (n >= 0) {
		DBG("+++ nonblocking connect success +++\n");
		con->count = 0;
		(*con->connected)(l, con);
	}
	return 0;
}

/* DONE */
void
tcp_conn_del(struct ev_loop *l, conn_t *con)
{
	if (con->hb) {
		ev_timer_stop(l, &con->hw);
		con->hb = 0;
	}
	ev_timer_stop(l, &con->tw);
	ev_io_stop(l, &con->nw);
	if (con->fd != -1) {
		close(con->fd);
		con->fd = -1;
	}
	con->count = 0;
}

void
tcp_conn_retry(struct ev_loop *l, conn_t *con, int active)
{
	if (active) {
		ev_timer_stop(l, &con->tw);
		ev_io_stop(l, &con->nw);
	}

	if (con->hb) {
		ev_timer_stop(l, &con->hw);
		con->hb = 0;
	}

	if (con->fd != -1) {
		close(con->fd);
		con->fd = -1;
	}
	con->count = 0;
	ev_timer_init(&con->tw, try_connect, con->tmeo, 0.);
	ev_timer_start(l, &con->tw);
}



