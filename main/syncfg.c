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
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <linux/reboot.h>

#include "mxc_sdk.h"
#include "ulib.h"
#include "buffer.h"
#include "conn.h"
#include "config.h"
#include "zopt.h"
#include "pzone.h"
#include "xfer.h"
#include "gui.h"

#define FTPRT       8999
#define IBUF_SZ     32
#define OBUF_SZ     (100*32)

//#define ZST_SZ     (MAXZ + MAXPZ)

typedef void (*FVOID)(void);

typedef struct _fi {
	char *name;
	int   fd;     //file descriptor
	int   ref;
	unsigned int  size;  //file size
	unsigned int  crc;
	void (*reload) (void);
} fi_t;

typedef struct  _fclnt {
	UListHdr  list;
	ev_io     nw;
	ev_timer  tw;
	int   nfd;
	int   nt;
	int   idx;  //index of element in fi_t array.
	int   off;
	int   first;
	buffer_t ibuf;
	buffer_t obuf;
} fclnt_t;

#define clear_buffer(f)   do { \
	buffer_reset(&(f)->ibuf);  \
	buffer_reset(&(f)->obuf);  \
} while (0)

static int    g_ft = -1;
static pthread_t  g_fid = 0;

static pthread_mutex_t    fmtx = PTHREAD_MUTEX_INITIALIZER;
#define  ft_lock()        pthread_mutex_lock(&fmtx)
#define  ft_unlock()      pthread_mutex_unlock(&fmtx)

static UListHdr   free_clnt = {&free_clnt, &free_clnt};
static UListHdr   clnt_list = {&clnt_list, &clnt_list};

static int    fpfd[2] = {-1, -1};
static ev_io  g_mw;

static int    ftx_disable = 0;

static void   add_ntimer_ev(struct ev_loop *l, fclnt_t *f);
static void   reload_mapkt(void);
static void   reload_zmapdb(void);
static void   reload_zmpos(void);

static fi_t   g_fi[] = {
	[IfConfId] = {
		.name = IFTCONF,
		.fd = -1,
		.ref = 0,
		.size = 0,
		.crc = 0,
		.reload = reload_ifconf,
	},
	[ZoneDbId] = {
		.name = ZONEDB,
		.fd = -1,
		.ref = 0,
		.size = 0,
		.crc = 0,
		.reload = reload_zonedb,
	},
	[PzneDbId] = {
		.name = PZONEDB,
		.fd = -1,
		.ref = 0,
		.size = 0,
		.crc = 0,
		.reload = reload_pzonedb,
	},
	[UsrDbId] = {
		.name = USRDB,
		.fd = -1,
		.ref = 0,
		.size = 0,
		.crc = 0,
		.reload = reload_usrdb,
	},
	[GenDbId] = {
		.name = GENDB,
		.fd = -1,
		.ref = 0,
		.size = 0,
		.crc = 0,
		.reload = reload_gendb,
	},
	[PzTmDbId] = {
		.name = PZTMRDB,
		.fd = -1,
		.ref = 0,
		.size = 0,
		.crc = 0,
		.reload = reload_pztmr,
	},
	[MapktId] = {
		.name = MAPKT,
		.fd = -1,
		.ref = 0,
		.size = 0,
		.crc = 0,
		.reload = reload_mapkt,
	},
	[ZmapDbId] = {
		.name = ZMAPDB,
		.fd = -1,
		.ref = 0,
		.size = 0,
		.crc = 0,
		.reload = reload_zmapdb,
	},
	[ZmPosId] = {
		.name = ZMPOS,
		.fd = -1,
		.ref = 0,
		.size = 0,
		.crc = 0,
		.reload = reload_zmpos,
	},
};

static int   zmapflg = 0;

static void
reload_mapkt(void)
{
	zmapflg |= 1;
}

static void
reload_zmapdb(void)
{
	zmapflg |= (1 << 1);
}

static void
reload_zmpos(void)
{
	zmapflg |= (1 << 2);
}

int
cfg_crc_diff(int i, unsigned int  crc)
{
	return (g_fi[i].crc != crc);
}

int
make_crc_buf(char *buf, int len)
{
	int  total = 0;
	int  n;
	int  i;
	char  *ptr = buf;

	for (i = 0; i < ARR_SZ(g_fi)&&len > 0; ++i) {
		n = snprintf(ptr, len, "|%d:%u", i, g_fi[i].crc);
		total += n;
		ptr += n;
		len -= n;
	}
	
	*ptr++ = '\r';
	*ptr++ = '\n';
	*ptr = 0;

	return total+2;
}


////////////////////////////////////////////////////////////////
//             master routine
////////////////////////////////////////////////////////////////
#if 0
{
#endif
static void   recv_fcb(struct ev_loop *l, ev_io *w, int revents);
static void   add_ntimer_ev(struct ev_loop *l, fclnt_t  *f);

void
fi_clear(int i)
{
	fi_t  *fi = &g_fi[i];

	fi->fd = -1;
	fi->ref = 0;
	fi->size = 0;
	fi->crc = 0;
	unlink(fi->name);
}

/* called when configfile changed. */
/* DONE */
void
fi_update(int i)
{
	unsigned int  res = 0;
	fi_t  *fi = &g_fi[i];
	struct stat  st;

	if (stat(fi->name, &st) < 0) {
		fi->fd = -1;
		fi->ref = 0;
		fi->size = 0;
		fi->crc = 0;
		return;
	}

	fi->ref = 0;
	fi->size = st.st_size;

	if (u_file_crc32(fi->name, &res) == 0) {
		fi->crc = res;
	} else {
		fi->crc = 0;
	}
	DBG("--- %s: crc32 = %x, size = %d ---\n", fi->name, fi->crc, fi->size);
	cfg_map_set(i);
}

/* called when system start in init.c */
/* DONE */
void
fi_save(void)
{
	int  i;

	for (i = 0; i < ARR_SZ(g_fi); ++i) {
		fi_update(i);
	}
}

///////////////////////////////////////////////////////
static int
fi_open(int i)
{
	fi_t  *fi;

	if (i < 0||i >= 10||g_fi[i].size == 0) return -1;
	fi = &g_fi[i];
	if (fi->ref == 0) {
		fi->fd = open(fi->name, O_RDONLY);
		if (fi->fd < 0) return -1;
	}
	fi->ref++;
	return 0;
}

static void
fi_close(int i)
{
	fi_t  *fi;

	if (i < 0||i >= 10) return;
	fi = &g_fi[i];
	if (fi->ref > 0) {
		fi->ref--;
		if (fi->ref == 0) {
			close(fi->fd);
		}
	}
}

/* DONE */
static fclnt_t*
fclnt_new(void)
{
    fclnt_t* f = NULL;
    
    if (u_list_empty(&free_clnt)) {
        f = calloc(1, sizeof(fclnt_t));
        if (f != NULL) {
            U_INIT_LIST_HEAD(&f->list);
			buffer_init_fix(&f->ibuf, IBUF_SZ);
			buffer_init_fix(&f->obuf, OBUF_SZ);
			f->idx = -1;
			f->first = 1;
        }
    } else {
        f = (fclnt_t*)(free_clnt.next);
        u_list_del(&f->list);
        U_INIT_LIST_HEAD(&f->list);
		f->off = 0;
		f->idx = -1;
		f->nt = 0;
		f->first = 1;
    }
    return f;
}

/* DONE */
static void
fclnt_free(fclnt_t* f)
{
    if (!u_list_empty(&f->list)) {
        u_list_del(&f->list);
    }
    U_INIT_LIST_HEAD(&f->list);
    u_list_append(&f->list, &free_clnt);
}

/*
 * return: 0-sending, -1-send error, 1-completed.
 */
static int
file_send(fclnt_t *f)
{
	fi_t  *fi = &g_fi[f->idx];
	buffer_t  *b = &f->obuf;
	int  nread;
	int  nsp;
	int  nleft;

	if (BUFPTR_LENGTH(b) == 0) {
		nleft = fi->size - f->off;
		if (nleft == 0) {
			DBG("--- %s:%d bytes transfer completed ---\n", fi->name, fi->size);
			buffer_reset(b);
			return 1;
		}

		nsp = BUFPTR_SPACE(b);
		nread = nleft > nsp ? nsp : nleft;
//		DBG("--- %s will be transferd %d bytes ---\n", fi->name, nread);
		lseek(fi->fd, f->off, SEEK_SET);
		buffer_readn(b, fi->fd, nread);
		f->off = lseek(fi->fd, 0, SEEK_CUR);
	}

	if (buffer_write(b, f->nfd) < 0)
		return -1;
	return 0;
}

static void
fclnt_del(struct ev_loop *l, fclnt_t *f)
{
	ev_io_stop(l, &f->nw);
	u_tcp_quit(f->nfd);
	fclnt_free(f);
}

static void
send_file(struct ev_loop *l, ev_io *w, int revents)
{
	fclnt_t *f = U_CONTAINER_OF(w, struct _fclnt, nw);
	fi_t    *fi = &g_fi[f->idx];
	char     buf[256] = {0};
	int      n;
	int      r;

	if (revents & EV_ERROR) {
        DBG("--- SOCKET ERROR ---\n");
		fi_close(f->idx);
		fclnt_del(l, f);
        return;
    }

    if (revents & EV_WRITE) {
		if (f->first) {
			n = sprintf(buf, "#%d|%d|%d|%u|%x\r\n\r\n", ReqCfgCmd, g_devid, f->idx, fi->size, fi->crc);
			buffer_add(&f->obuf, buf, n);
			f->first = 0;
			DBG("--- send %s: crc = %x ---\n", fi->name, fi->crc);
		}

		r = file_send(f);
    	switch (r) {
		case -1: //transfer error.
			fi_close(f->idx);
			fclnt_del(l, f);
			break;
		case 1: //transfer done.write watcher--->read watcher.
			fi_close(f->idx);
			ev_io_stop(l, w);
			ev_io_init(w, recv_fcb, f->nfd, EV_READ);
            ev_io_start(l, w);
			add_ntimer_ev(l, f);
			break;
		default: //continue.
			break;
    	}
    }
}

static void
add_send_ev(struct ev_loop *l, fclnt_t *f)
{
	ev_io_init(&f->nw, send_file, f->nfd, EV_WRITE);
	ev_io_start(l, &f->nw);
}

static void    add_sendz_ev(struct ev_loop *l, fclnt_t *f);

static void
send_zstat(struct ev_loop *l, ev_io *w, int revents)
{
	fclnt_t *f = U_CONTAINER_OF(w, struct _fclnt, nw);
	char     buf[1024] = {0};
	int      n;

	if (revents & EV_ERROR) {
        DBG("--- SOCKET ERROR ---\n");
		fclnt_del(l, f);
        return;
    }

	ev_io_stop(l, w);
    if (revents & EV_WRITE) {
		if (f->first) {
			n = sprintf(buf, "#%d|%d|%d|%u|0\r\n\r\n", ReqCfgCmd, g_devid, f->idx, MAXPZ+zarcnt);
			buffer_add(&f->obuf, buf, n);
			buffer_add(&f->obuf, pstat, MAXPZ);
			buffer_add(&f->obuf, zstat, zarcnt); //modified on 2014..5.06
			f->first = 0;
		}

		if (BUFPTR_LENGTH(&f->obuf) > 0) {
			if (buffer_write(&f->obuf, f->nfd) < 0) {
				fclnt_del(l, f);
				DBG("---  write zstat error ---\n");
				return;
			}
		}

		if (BUFPTR_LENGTH(&f->obuf) == 0){
			DBG("---  write zstat completed. ---\n");
			ev_io_init(w, recv_fcb, f->nfd, EV_READ);
            ev_io_start(l, w);
			add_ntimer_ev(l, f);
		} else {
			DBG("---  will continue sending zstat. ---\n");
			add_sendz_ev(l, f);
		}
    }
}

static void
add_sendz_ev(struct ev_loop *l, fclnt_t *f)
{
	ev_io_init(&f->nw, send_zstat, f->nfd, EV_WRITE);
	ev_io_start(l, &f->nw);
}

static void
send_nack(struct ev_loop *l, ev_io *w, int revents)
{
	fclnt_t *f = U_CONTAINER_OF(w, struct _fclnt, nw);
	char     buf[256] = {0};
	int      n;

	if (revents & EV_ERROR) {
        DBG("--- SOCKET ERROR ---\n");
		fclnt_del(l, f);
        return;
    }

	ev_io_stop(l, w);
    if (revents & EV_WRITE) {
		n = sprintf(buf, "#%d|%d|%d|0|0\r\n\r\n", ReqCfgCmd, g_devid, f->idx);
		if (write(f->nfd, buf, n) < 0) {
			fclnt_del(l, f);
        	return;
		}

		ev_io_init(w, recv_fcb, f->nfd, EV_READ);
		ev_io_start(l, w);
		add_ntimer_ev(l, f);
    }
}

static void
add_nack_ev(struct ev_loop *l, fclnt_t *f)
{
	ev_io_init(&f->nw, send_nack, f->nfd, EV_WRITE);
	ev_io_start(l, &f->nw);
}

static void
handle_req(struct ev_loop *l, fclnt_t *f)
{
    char *ph = NULL;
	char *pt = NULL;
	int   fd = f->nw.fd;
	int   cmd;

	if (buffer_read(&f->ibuf, fd) <= 0) {
		DBG("--- READ ERROR ---\n");
		clear_buffer(f);
		fclnt_del(l, f);
		return;
	}

	ph = buffer_find_chr(&f->ibuf, '#');
	if (ph == NULL) {
		DBG("--- no start flag found, fake client ---\n");
		clear_buffer(f);
		fclnt_del(l, f);
		return;
	}

	pt = buffer_find(&f->ibuf, "\r\n", 2);
	if (pt == NULL) {
		DBG("--- no end flag found, continue reading. ---\n");
		add_ntimer_ev(l, f);
		return;
	}

	*pt++ = 0;
	*pt++ = 0;

	++ph;
	cmd = (int)strtol(ph, &pt, 10);
	if (cmd != ReqCfgCmd) {
		DBG("--- wrong cmd ---\n");
		clear_buffer(f);
		fclnt_del(l, f);
		return;
	}

	++pt;
	cmd = strtol(pt, &ph, 10);
	if (cmd != g_devid) {
		DBG("--- wrong devid ---\n");
		clear_buffer(f);
		fclnt_del(l, f);
		return;
	}
	++ph;
	cmd = strtol(ph, &pt, 10);
	if (cmd == 0&&pt == ph) {
		DBG("--- no file idx ---\n");
		clear_buffer(f);
		fclnt_del(l, f);
		return;
	}

	f->idx = cmd;

	clear_buffer(f);
	f->first = 1;
	f->off = 0;

	if (cmd < ARR_SZ(g_fi)) {
		DBG("--- send the file: %s. crc32 = %x ---\n", g_fi[cmd].name, g_fi[cmd].crc);
		if (fi_open(f->idx) == 0) {
			ev_io_stop(l, &f->nw);
			add_send_ev(l, f);
		} else {
			DBG("--- no such file ---\n");
			ev_io_stop(l, &f->nw);
			fi_clear(f->idx);
			add_nack_ev(l, f);
		}
	} else if (cmd == ARR_SZ(g_fi)) {
		DBG("--- send the zstat. ---\n");
		ev_io_stop(l, &f->nw);
		add_sendz_ev(l, f);
	} else {
		fclnt_del(l, f);
	}
}

/* DONE */
static void
recv_fcb(struct ev_loop *l, ev_io *w, int revents)
{
    fclnt_t *f = U_CONTAINER_OF(w, struct _fclnt, nw);

    if (f->nt != 0) {
        ev_timer_stop(l, &f->tw);
		f->nt = 0;
    }

    if (revents & EV_ERROR) {
        DBG("--- SOCKET ERROR ---\n");
		fclnt_del(l, f);
        return;
    }

    if (revents & EV_READ) {
        handle_req(l, f);
    }
}

/* DONE */
static void
wait_timeout(struct ev_loop *l, ev_timer *w, int revents)
{
	fclnt_t *f = U_CONTAINER_OF(w, struct _fclnt, tw);

	DBG("--- connection timeout: no request received ---\n");
	//timer watcher automatically stopped.
	f->nt = 0;
	fclnt_del(l, f);
}

/* DONE */
static void
add_ntimer_ev(struct ev_loop *l, fclnt_t  *f)
{
	f->nt = 1;
	ev_timer_init(&f->tw, wait_timeout, 10., 0.);
	ev_timer_start(l, &f->tw);
}

/* DONE */
static void
listen_cb(struct ev_loop *l, ev_io *w, int revents)
{
    int   conFd = -1;
    fclnt_t  *f = NULL;

#ifdef DEBUG
	struct sockaddr_in addr;
	socklen_t  addrsize;
    char peerip[16];

	addrsize = sizeof(addr);
	conFd = accept(w->fd, (struct sockaddr *)&addr, &addrsize);
#else
    conFd = accept(w->fd, NULL, NULL);
#endif
	if (conFd >= 0)
	{
#ifdef DEBUG
        memset(peerip, 0, 16);
		inet_ntop(AF_INET, &addr.sin_addr, peerip, 16);
		printf("--- connection accepted: peerip = %s ---\n", peerip);
#endif
		if (ftx_disable) {
			DBG("--- file transfer disabled. close the socket ---\n");
			close(conFd);
			return;
		}
	    u_set_nonblock(conFd);
		u_tcp_set_nodelay(conFd);

		f = fclnt_new();
		if (f)
		{
			f->nfd = conFd;
			ev_io_init(&f->nw, recv_fcb, conFd, EV_READ);
            ev_io_start(l, &f->nw);
			add_ntimer_ev(l, f);
			u_list_append(&f->list, &clnt_list);
		}
		else
		{
			DBG("--- fclnt_new failed ---\n");
			close(conFd);
		}
	}
}

/* DONE */
static void
cleanup_nw(struct ev_loop *l)
{
	UListHdr *list;
	fclnt_t  *f;

	while (!u_list_empty(&clnt_list)) {
		list = clnt_list.next;
		u_list_del(list);
		f = (fclnt_t*)list;
		if (f->nt) {
			ev_timer_stop(l, &f->tw);
			f->nt = 0;
		}
		fclnt_del(l, f);
	}
}

/* DONE */
static void
handle_fqcmd(struct ev_loop *l, ev_io *w, int revents)
{
	unsigned int  cmd;
	int   i;

	if (revents & EV_READ) {
		if (read(w->fd, &cmd, sizeof(cmd)) == sizeof(cmd)) {
			if (cmd == 0) {
				ev_io_stop(l, w);
				ev_io_stop(l, &g_mw);
				cleanup_nw(l);
				ev_break(l, EVBREAK_ALL);
				DBG("--- going to quit the file server loop ---\n");
			} else {
				i = (cmd >> 16) - 1;
				if (i) {
					if (ftx_disable == 0) {
						cleanup_nw(l);
						set_busy(); //inform master loop thread.
						DBG("--- cancel transfer cfg file: %d ---\n", i);
					}
					ignore_result(write(w->fd, &i, sizeof(i))); //return ack to UI thread.
				}
				ftx_disable = i;
			}
		}
	}
}

/* DONE */
static void*
file_server_loop(void* arg)
{
	struct ev_loop   *l;
	int  listenFd;
	ev_io  pw;

	l = ev_loop_new(EVFLAG_AUTO);
	if (l == NULL) exit(1);

	DBG("--- going to enter file server loop ---\n");
	listenFd = u_tcp_serv(NULL, FTPRT);
	if (listenFd < 0)
	{
		DBG("--- create tcp server failed ---\n");
		ev_loop_destroy(l);
		exit(1);
	}
	u_set_nonblock(listenFd);

	ev_io_init(&g_mw, listen_cb, listenFd, EV_READ);
	ev_io_start(l, &g_mw);

	ev_io_init(&pw, handle_fqcmd, fpfd[0], EV_READ);
	ev_io_start(l, &pw);

	ev_run(l, 0);

	close(listenFd);

    ev_loop_destroy(l);
	DBG("--- exit file server loop ---\n");

    pthread_exit(NULL);
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
#define SYNC_SZ     8192

typedef struct _ftx {
	conn_t   con;
	ev_io    pw;
	unsigned int  cmap;
	unsigned int  omap;
	int      flag;
	buffer_t rbuf;
	int      idx;
	int      fd;
	unsigned int  crc;
	unsigned int  fsize;
	unsigned int  nread;
} ftx_t;

#define ftx_one_clear(f)   do { \
	(f)->idx = -1;   \
	(f)->crc = 0;    \
	(f)->fsize = 0;  \
	(f)->nread = 0;  \
	} while (0)

static void  ftx_connected_action(struct ev_loop *l, conn_t *con);
static void  read_fdata(struct ev_loop *l, ev_io *w, int revents);

static ftx_t   g_ftx = {
	.con = {
		.fd = -1,
		.host = g_masterip,
		.port = FTPRT,
		.tmeo = 6.0,
		.connected = ftx_connected_action,
	},
	.cmap = 0,
	.flag = 0,
	.fd   = -1,
	.crc  = 0,
	.fsize = 0,
	.nread = 0,
};

#ifdef DEBUG
static void
dump_ftx(void)
{
	printf("---- g_ftx ------\n");
	printf("-- cmap  = %u ---\n", g_ftx.cmap);
	printf("-- flag  = %d ---\n", g_ftx.flag);
	printf("-- crc   = %x ---\n", g_ftx.crc);
	printf("-- fsize = %u ---\n", g_ftx.fsize);
	printf("-- nread = %u ---\n", g_ftx.nread);
}

#endif

static void
ftx_cancel(struct ev_loop *l, ftx_t *f)
{
	if (f->flag) {
		tcp_conn_del(l, &f->con);
		f->flag = 0;
	}

	if (f->fd != -1) {
		close(f->fd);
		f->fd = -1;
		unlink(g_fi[f->idx].name);
	}

	ftx_one_clear(f);
	f->cmap = 0;
	f->omap = 0;
}

/* DONE */
static void
fack_timeout(struct ev_loop *l, ev_timer *w, int revents)
{
	conn_t *con = U_CONTAINER_OF(w, struct _conn, tw);
	ftx_t  *f = (ftx_t *)con;

	if (revents & EV_TIMER) {
		DBG("--- receiving timeout, cancel th ftx. ---\n");
		ftx_cancel(l, f);
	}
}

static void
ftx_finished(ftx_t *f)
{
	fi_t  *fi = &g_fi[f->idx];

	f->cmap &= ~(1 << f->idx);
	fi->crc = f->crc;
	fi->size = f->fsize;
	fi->reload();
	ftx_one_clear(f);
}

/* DONE
 * return: 0-receiving not completed.
 *         1-sync file completed.
 *         2-receiving completed but crc error.need to retransition.
 *         3-open file error.exit.
 */
static int
handle_rbuf(ftx_t  *f)
{
	unsigned int  nwriten = 0;
	unsigned int  n;
	int   x;

	if (f->fd == -1) {
		f->fd = open(g_fi[f->idx].name, O_WRONLY|O_CREAT, 0777);
		if (f->fd < 0) {
			DBG("--- OPEN %s FAILED, errno = %d: %s ---\n", g_fi[f->idx].name, errno, strerror(errno));
			return 3;
		}
	}

	while (BUFPTR_LENGTH(&f->rbuf) > 0) {
		x = buffer_write(&f->rbuf, f->fd);
		if (x > 0)
			nwriten += (unsigned int)x;
		else
			break;
	}

	f->nread += nwriten;
	if (f->nread == f->fsize) { //finish receiving.
		fsync(f->fd);
		close(f->fd);
		f->fd = -1;

		u_file_crc32(g_fi[f->idx].name, &n);
		if (n != f->crc) {
			DBG("--- finish sync[%s]: crc error: received %x, should be %x. ---\n", g_fi[f->idx].name, n, f->crc);
			unlink(g_fi[f->idx].name);
			return 2;
		} else {
			DBG("--- finish sync[%s]: crc correct: %x. ---\n", g_fi[f->idx].name, n);
			ftx_finished(f);
			return 1;
		}
	}
	return 0;
}

static void
reload_map(void)
{
	if (zmapflg & 1) {
		reboot(LINUX_REBOOT_CMD_RESTART);
		exit(0);
	}
	if ((zmapflg >> 1) > 0) {
		reload_mapdb();
	}
	zmapflg = 0;
}

/* DONE */
static void
save_fdata(struct ev_loop *l, ftx_t *f)
{
	int  r;

	r = handle_rbuf(f);
	switch (r) {
	case 0: //continue receiving.
		tcp_conn_add_rev(l, &f->con, read_fdata);
		tcp_conn_add_tev(l, &f->con, fack_timeout, 10.0);
		break;
	case 1: //finish sync a file.
	case 2:
		if (f->cmap == 0) {
			reload_map();
			finish_syncfg();
		} else {
			ftx_connected_action(l, &f->con);
		}
		break;
	default:
		ftx_cancel(l, f);
		break;
	}
}

/* DONE */
static void
read_fdata(struct ev_loop *l, ev_io *w, int revents)
{
	conn_t *con = U_CONTAINER_OF(w, struct _conn, nw);
	ftx_t  *ftx = &g_ftx;

	if (revents & EV_ERROR) {
		DBG("--- SOCKET ERROR  when receiving data. ---\n");
		ftx_cancel(l, ftx);
		return;
	}

	tcp_conn_del_tev(l, con);
	tcp_conn_del_rev(l, con);
	if (revents & EV_READ) {
		if (buffer_read(&ftx->rbuf, ftx->con.fd) <= 0) {
			DBG("--- READ ERROR when receiving data. ---\n");
			ftx_cancel(l, ftx);
			return;
		}
		DBG("--- received %d bytes ---\n", BUFPTR_LENGTH(&ftx->rbuf));
		save_fdata(l, ftx);
	}
}

//////////////////////////////////////////////////////////
static void   handle_zdata(struct ev_loop *l, ftx_t *f);

/* DONE */
static void
finish_zsync(struct ev_loop *l, ftx_t *f)
{
	reload_map();
	if ((f->omap&(1 << ZoneDbId))&&!(f->omap&(1 << PzneDbId))) {
		DBG("--- reload_pzonedb ---\n");
		reload_pzonedb();
	}

	memcpy(pstat, BUFPTR_DATA(&f->rbuf), MAXPZ);
	buffer_drain(&f->rbuf, MAXPZ);
//	memcpy(zstat, BUFPTR_DATA(&f->rbuf), zarcnt);
	memcpy(zstat, BUFPTR_DATA(&f->rbuf), f->fsize - MAXPZ);
	buffer_reset(&f->rbuf);

	sync_zone_ui();
	sync_pzone_ui();
	DBG("--- finish syncing zstat ---\n");
}

/* DONE */
static void
recv_data(struct ev_loop *l, ev_io *w, int revents)
{
	conn_t *con = (conn_t*)w;
	ftx_t *f;

	f = (ftx_t*)con;
	if (buffer_read(&f->rbuf, f->con.fd) <= 0) {
		DBG("--- READ ERROR when receiving zstat data. ---\n");
		ftx_cancel(l, f);
		return;
	}

	tcp_conn_del_tev(l, con);
	tcp_conn_del_rev(l, con);
	handle_zdata(l, f);
}

/* DONE */
static void
handle_zdata(struct ev_loop *l, ftx_t *f)
{
	if (f->fsize > BUFPTR_LENGTH(&f->rbuf)) {
		DBG("--- not completed, len = %d ---\n", BUFPTR_LENGTH(&f->rbuf));
		tcp_conn_add_rev(l, &f->con, recv_data);
		tcp_conn_add_tev(l, &f->con, fack_timeout, 5.0);
	} else {
		finish_zsync(l, f);
		buffer_reset(&f->rbuf);
		tcp_conn_del(l, &f->con);
		f->cmap = 0;
		f->flag = 0;     //don't remember this.
		finish_syncfg(); //inform slave loop thread.
	}
}

static void
parse_fack(struct ev_loop *l, ftx_t *f)
{
	char *ptr;
	char *p;
	char *pt;
	int   v;
	unsigned int x;

	if (buffer_read(&f->rbuf, f->con.fd) <= 0) {
		DBG("--- READ ERROR ---\n");
		ftx_cancel(l, f);
		return;
	}

	ptr = buffer_find_chr(&f->rbuf, '#');
	if (ptr == NULL) {
		DBG("--- no start flag found ---\n");
		ftx_cancel(l, f);
		return;
	}

	pt = buffer_find(&f->rbuf, "\r\n\r\n", 4);
	if (pt == NULL) {
		DBG("--- header not completed ---\n");
		tcp_conn_add_tev(l, &f->con, fack_timeout, 10.0);
		return;
	}
	pt += 4;

	++ptr;
	v = (int)strtol(ptr, &p, 10);
	if (v != ReqCfgCmd) {
		DBG("--- cmd = %d error ---\n", v);
		ftx_cancel(l, f);
		return;
	}

	ptr = p + 1;
	v = (int)strtol(ptr, &p, 10);
	if (v != masterid) {
		DBG("--- devid = %d error ---\n", v);
		ftx_cancel(l, f);
		return;
	}

	ptr = p + 1;
	v = (int)strtol(ptr, &p, 10);
	if (v != f->idx) {
		DBG("--- idx = %d error ---\n", v);
		ftx_cancel(l, f);
		return;
	}

	//////////////////////////////////////////
	ptr = p + 1;
	x = (unsigned int)strtol(ptr, &p, 10);
	if (x == 0&&p == ptr) {
		DBG("--- no file size found ---\n");
		ftx_cancel(l, f);
		return;
	}

	f->fsize = x;

	ptr = p + 1;
	x = (unsigned int)strtoll(ptr, &p, 16);
	if (x == 0&&p == ptr) {
		DBG("--- no crc value found ---\n");
		ftx_cancel(l, f);
		return;
	}

	DBG("--- crc32 = %x ---\n", x);
	f->crc = x;
	f->nread = 0;
	buffer_drain_ptr(&f->rbuf, pt);

	tcp_conn_del_rev(l, &f->con);
	dump_ftx();
	if (f->idx < ARR_SZ(g_fi)) {
		DBG("--- start to get file: %s: size = %u, crc32 = %x ---\n", g_fi[f->idx].name, f->fsize, f->crc);
		if (f->fsize == 0&&f->crc == 0) {
			unlink(g_fi[f->idx].name);
			ftx_finished(f);
			if (f->cmap == 0) {
				finish_syncfg();
			} else {
				ftx_connected_action(l, &f->con);
			}
		} else {
			save_fdata(l, f);
		}
	} else {
		DBG("--- start to get zstat: size = %u ---\n", f->fsize);
		handle_zdata(l, f);
	}
}

/* DONE */
static void
handle_fack(struct ev_loop *l, ev_io *w, int revents)
{
	conn_t *con = U_CONTAINER_OF(w, struct _conn, nw);
	ftx_t  *f = (ftx_t *)con;

	if (revents & EV_ERROR) {
        DBG("--- SOCKET ERROR ---\n");
		ftx_cancel(l, f);
        return;
    }

	tcp_conn_del_tev(l, con);

	if (revents & EV_READ) {
		parse_fack(l, f);
	}
}

/* DONE */
static void
ftx_connected_action(struct ev_loop *l, conn_t *con)
{
	char   buf[32] = {0};
	int    n;
	ftx_t *f = (ftx_t *)con;

	DBG("--- connected to master for config file. ---\n");
	if (f->cmap == 0) return;
	f->idx = __builtin_ctz(f->cmap);
	n = sprintf(buf, "#%d|%d|%d\r\n", ReqCfgCmd, masterid, f->idx);
	if (tcp_conn_send(con, buf, n) < 0) {
		DBG("--- write error when connected. ---\n");
		tcp_conn_del(l, con);
		return;
	}

#ifdef DEBUG
	if (f->idx < ARR_SZ(g_fi))
		DBG("--- send: %s for %s ---\n", buf, g_fi[f->idx].name);
	else
		DBG("--- send: %s for zstat ---\n", buf);
#endif
	buffer_reset(&f->rbuf);
	tcp_conn_add_rev(l, con, handle_fack);
	tcp_conn_add_tev(l, con, fack_timeout, 5.0);
}

/* DONE */
static void
handle_fcmd(struct ev_loop *l, ev_io *w, int revents)
{
	unsigned int  cmd;
	ftx_t *f = U_CONTAINER_OF(w, struct _ftx, pw);

	if (revents & EV_READ) {
		if (read(w->fd, &cmd, sizeof(cmd)) == sizeof(cmd)) {
			if (cmd == 0) {
				ftx_cancel(l, f);
				ev_io_stop(l, w);
				ev_break(l, EVBREAK_ALL);
				DBG("--- going to quit the file client loop ---\n");
			} else {
				ftx_cancel(l, f);
				if (cmd < (1<<11)) {
					f->flag = 1;
					f->cmap = cmd;
					f->omap = cmd;
					DBG("--- going to request file: map = %04x ---\n", cmd);
					zmapflg = 0;
					tcp_connect(l, &f->con);
				} 
			}
		}
	}
}

/* DONE */
static void*
file_client_loop(void* arg)
{
	struct ev_loop *l;

	l = ev_loop_new(EVFLAG_AUTO);
	if (l == NULL) exit(1);

	buffer_init(&g_ftx.rbuf, SYNC_SZ);
	DBG("--- going to enter file client loop ---\n");

	ev_io_init(&g_ftx.pw, handle_fcmd, fpfd[0], EV_READ);
	ev_io_start(l, &g_ftx.pw);

	ev_run(l, 0);

	buffer_base_free(&g_ftx.rbuf);
    ev_loop_destroy(l);

	DBG("--- exit file client loop ---\n");

    pthread_exit(NULL);
}

#if 0
}
#endif

///////////////////////////////////////////////////////////////
/* DONE */
static void
start_ft_loop(void *cb, int mst)
{
	if ((is_master() != mst)||is_standalone()) return;
	ft_lock();
	if (g_ft == -1) {
		init_pipe(fpfd);
		DBG("---------- START THE FILE SERVER LOOP --------\n");
		g_ft = pthread_create(&g_fid, NULL, cb, NULL);
	}
	ft_unlock();
}

/* DONE */
void
stop_ft_loop(void)
{
	unsigned int cmd = 0;
	ft_lock();
	if (g_ft == 0) {
		ignore_result(write(fpfd[1], &cmd, sizeof(cmd)));
		pthread_join(g_fid, NULL);
		g_fid = 0;
		g_ft = -1;
		close(fpfd[0]);
		close(fpfd[1]);
		DBG("---------- EXIT THE FILE SERVER LOOP --------\n");
	}
	ft_unlock();
}

/* DONE */
void
start_file_server(void)
{
	start_ft_loop((void*)file_server_loop, 1);
}

/* DONE */
void
start_file_client(void)
{
	start_ft_loop((void*)file_client_loop, 0);
}

static void
syncfg_cmd_nr(unsigned int v)
{
	ft_lock();
	if (g_ft == 0) {
		ignore_result(write(fpfd[1], &v, sizeof(v)));
	}
	ft_unlock();
}

static void
syncfg_cmd(unsigned int v)
{
	int  res;

	ft_lock();
	if (g_ft == 0) {
		ignore_result(write(fpfd[1], &v, sizeof(v)));
		res = recv_cmd_ack(fpfd[1], -1);
	}
	ft_unlock();
}

/* slave only: called by slave loop thread. */
void
start_syncfg(unsigned int v)
{
	if (v > 0&&v < (1<<11)) {
		syncfg_cmd_nr(v);
	}
}

/* slave only: called by slave loop thread. */
void
cancel_syncfg(void)
{
	syncfg_cmd_nr((unsigned int)~0);
}

/* master only: called by UI thread. */
void
disable_syncfg(void)
{
	syncfg_cmd(2 << 16);
}

/* master only: called by net_tcp thread. */
void
enable_syncfg(void)
{
	syncfg_cmd_nr(1 << 16);
}



