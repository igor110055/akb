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
#include <linux/reboot.h>

#include "ulib.h"
#include "mxc_sdk.h"
#include "buffer.h"
#include "config.h"
#include "zopt.h"

typedef void (*ECBF)(struct ev_loop *l, ev_io *w);

static int    ncfginit = 0;
static ev_timer  tmeow;
static ev_io  listenw;
static ev_io  cmdw;
static int    tmeo_active = 0;
static int    dlfd = -1;
static int    dlst = 0;
static buffer_t  nbuff;
static int    npfd[2] = {-1};

static pthread_t  nthr = 0;

static int   dlrun = 0;

static pthread_mutex_t  dmtx = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t   dcnd = PTHREAD_COND_INITIALIZER;

#define ncfg_wait()     pthread_cond_wait(&dcnd, &dmtx)
#define ncfg_signal()   pthread_cond_signal(&dcnd)
#define ncfg_lock()     pthread_mutex_lock(&dmtx)
#define ncfg_unlock()   pthread_mutex_unlock(&dmtx)

static char   peerip[16] = {0};
static char   ftpusr[16] = {0};
static char   ftpasswd[16] = {0};


////////////////////////////////////////////////////////////
#if 0
{
#endif

static int    ftprun = 0;

static netbuf *uconn = NULL;
static pthread_t  ftpthr;
static pthread_mutex_t  dlmtx = PTHREAD_MUTEX_INITIALIZER;

#define  dl_lock()      pthread_mutex_lock(&dlmtx)
#define  dl_unlock()    pthread_mutex_unlock(&dlmtx)

static unsigned int  gdlc = 0;

static char  *rpath[] = {"pzone.db", "zone.db", "map.zip"};
static char  *lpath[] = {"/tmp/pzone.db", "/tmp/zone.db", "/tmp/map.zip"};
static char  *tpath[] = {"/usr/etc/pzone.db", "/usr/etc/zone.db", "/usr/etc/map.zip"};

static int    ftpsize[] = {1024, 1024, 32*1024};

static void   download_finished(int r);

static int
ftp_connect(char *serip, char *usr, char *pas)
{
	if (uconn) return 1;
	if (!FtpConnect(serip, &uconn))
	{
		printf("Unable to connect to server:%s\n%s\n", serip, ftplib_lastresp);
		return 0;
	}

	if (!FtpLogin(usr, pas, uconn))
	{
		printf("Login failure\n%s\n", FtpLastResponse(uconn));
		FtpClose(uconn, 0);
		uconn = NULL;
		return 0;
	}
	return 1;
}

static int
get_file_size(char *path)
{
	int  size = 0;
	if (ftp_connect(peerip, ftpusr, ftpasswd))
	{
		if (FtpSize(path, &size, 'I', uconn))
		{
			return size;
		}
		else
		{
			printf("ftp error\n%s\n",FtpLastResponse(uconn));
		}
	}
	return 0;
}

static int
get_file(char *d, char *s, int mode)
{
	int  r = 0;
	if (ftp_connect(peerip, ftpusr, ftpasswd))
	{
		if (FtpGet(d, s, mode, uconn))
		{
			r = 1;
		}
		else
		{
			printf("ftp error\n%s\n",FtpLastResponse(uconn));
		}
	}
	return r;
}

static void
ftp_cleanup(void)
{
	if (uconn)
	{
		FtpClose(uconn, 0);
		uconn = NULL;
	}
}

static int
ftp_cb(netbuf *nControl, int xfered, void *arg)
{
//	DBG("--- xfered = %d and test cancel ---\n", xfered);
	pthread_testcancel();
	return 1;
}

static void
ftp_option(int ms, long arg, long cbytes)
{
	FtpOptions(FTPLIB_CALLBACK, (long)ftp_cb, uconn);
	FtpOptions(FTPLIB_IDLETIME, (long)ms, uconn);
	FtpOptions(FTPLIB_CALLBACKARG, arg, uconn);
	FtpOptions(FTPLIB_CALLBACKBYTES, cbytes, uconn);
}

static void
ftp_clear_option(void)
{
	FtpOptions(FTPLIB_CALLBACK, 0, uconn);
	FtpOptions(FTPLIB_IDLETIME, 0, uconn);
	FtpOptions(FTPLIB_CALLBACKARG, 0, uconn);
	FtpOptions(FTPLIB_CALLBACKBYTES, 0, uconn);
}

/* DONE */
static int
dl_file(int i)
{
	int   size;
	int   r = 0;

	if ((size = get_file_size(rpath[i])) > 0)
	{
		ftp_option(1000, 0, ftpsize[i]);
		r = get_file(lpath[i], rpath[i], 'I');
		DBG("--- DOWLOADING %s: %s ---\n", rpath[i], r > 0?"success":"failed");
		ftp_clear_option();
	}
	return r;
}

/* DONE */
static int
save_file(unsigned int dlc)
{
	int  i;
	int  r = 0;

	while (dlc) {
		i = __builtin_ctz(dlc);
		DBG("--- saving %s ---\n", lpath[i]);
		dlc &= ~(1 << i);
		if (u_copy(lpath[i], tpath[i]) <= 0) {
			++r;
		}
	}

	return r;
}

static inline void
cleanup_file(void)
{
	unlink(lpath[0]);
	unlink(lpath[1]);
	unlink(lpath[2]);
}

static void
cleanup_dl(void *arg)
{
	DBG("--- ftp thread cancelled and cleanup the resource ---\n");
	ftp_cleanup();
	cleanup_file();
	DBG("--- leaving ftp loop ---\n");
}

static char  cwd[32] = {0};

static void*
ftp_loop(void *arg)
{
	unsigned int  dlc;
	int   i;
	int   r = 0;
	int   len;

	dlc = gdlc;

	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
	pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL);

	if (ftp_connect(peerip, ftpusr, ftpasswd) == 0) {
		DBG("--- ftp connect error: download failed ---\n");
		download_finished(0);
		pthread_exit(NULL);
	}

	memset(cwd, 0, sizeof(cwd));
	if (FtpPwd(cwd, sizeof(cwd), uconn) == 0) {
		ftp_cleanup();
		download_finished(0);
		pthread_exit(NULL);
	}

	len = strlen(cwd);
	if (cwd[len-1] == '/') {
		cwd[len-1] = 0;
	}

	FtpChdir(cwd, uconn);

	pthread_cleanup_push(cleanup_dl, NULL);
	DBG("--- entering ftp loop ---\n");
	while (dlc) {
		i = __builtin_ctz(dlc);
		DBG("--- DOWNLOADING %s ---\n", rpath[i]);
		dlc &= ~(1 << i);
		pthread_testcancel();
		r = dl_file(i);
		pthread_testcancel();
		if (r == 0) break;
	}
	pthread_cleanup_pop(0);
	pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
	ftp_cleanup();
	if (r > 0) {
		r = !save_file(gdlc);
	}
	cleanup_file();
	download_finished(!!r);
	DBG("--- leaving ftp loop ---\n");
	pthread_exit(NULL);
}

/////////////////////////////////////////////////////////
//       the following functions being called by ncfg_loop     //
/////////////////////////////////////////////////////////
/* return value: 0-success, 1-failed. */
static int    
start_download(int dlc)
{
	int  r = 1;

	dl_lock();
	if (ftprun == 0) {
		gdlc = dlc;
		r = pthread_create(&ftpthr, NULL, ftp_loop, NULL);
		if (r == 0)
			ftprun = 1;
		else 
			r = 1;
	}
	dl_unlock();
	return r;
}

static void
dl_cleanup(void)
{
	pthread_join(ftpthr, NULL);
	dl_lock();
	ftprun = 0;
	dl_unlock();
}

static void    
cancel_download(void)
{
	dl_lock();
	if (ftprun == 1) {
		pthread_cancel(ftpthr);
		pthread_join(ftpthr, NULL);
		ftprun = 0;
	}
	dl_unlock();
}

#if 0
}
#endif

//////////////////////////////////////////////////////////////////

#if 0
{
#endif
static void
ncfg_v(int i)
{
	ncfg_lock();
	dlrun = i;
	ncfg_signal();
	ncfg_unlock();
}

static void
ev_cmd_remove(struct ev_loop *l, ev_io *w)
{
	ev_io_stop(l, w);
	close(w->fd);
	dlfd = -1;
	dlst = 0;
	buffer_reset(&nbuff);
	DBG("--- remove watcher ---\n");
}

static void
ntmeo_cb(struct ev_loop *loop, ev_timer *w, int revents)
{
	ev_cmd_remove(loop, &cmdw);
    ev_timer_stop(loop, w);
	tmeo_active = 0;
}

static void
ev_tmeo_remove(struct ev_loop *loop, ev_timer *w)
{
	if (tmeo_active) {
		ev_timer_stop(loop, w);
		tmeo_active = 0;
	}
}

static void
ev_tmeo_add(struct ev_loop *loop, int sec)
{
	if (tmeo_active == 0) {
		ev_timer_init(&tmeow, ntmeo_cb, sec, 0);
    	ev_timer_start(loop, &tmeow);
		tmeo_active = 1;
	}
}

static int
handle_msg(char *buf, int fd)
{
	char  *ptr = buf;
	char  *p = NULL;
	int    cmd;
	int    r;
	char   sndbuf[32] = {0};
	int    n;
	unsigned int  dlc = 0;

	cmd = strtol(buf, &ptr, 10);
	if (cmd == 0&&ptr == buf) {
		DBG("--- wrong cmd format ---\n");
		return 0;
	}

	++ptr;
	if (cmd == 1000) {
		if (dlst != 1) {
			DBG("--- wrong state ---\n");
			return 0;
		}
		p = strchr(ptr, '|');
		if (p == NULL) {
			DBG("--- no password ---\n");
			return 0;
		}
		*p++ = 0;
		r = verify_user(ptr, p);
		n = sprintf(sndbuf, "#1001|%d$", r);
		DBG("--- SEND Ack: %s ---\n", sndbuf);
		if (write(fd, sndbuf, n) < 0) {
			DBG("--- SEND Ack: %s failed ---\n", sndbuf);
			return 0;
		}
		if (r != 0) {
			DBG("--- user login failed: %d ---\n", r);
			return 0;
		}
		DBG("--- user login successfull---\n");
		return 2;
	}
	else if (cmd == 1002) {
		if (dlst != 2) {
			DBG("--- user not login ---\n");
			return 0;
		}
		r = strtol(ptr, &p, 10);
		if (r == 0&&p == ptr) {
			ignore_result(write(fd, "#1003|1$", 8));
			DBG("--- format error: #1003|1$ ---\n");
			return 0;
		}
		dlc |= !!r;
#ifdef DEBUG
		if (r == 0) {
			DBG("--- pzone.db not downloadable ---\n");
		} else {
			DBG("--- pzone.db downloadable ---\n");
		}
#endif
		ptr = ++p;
		r = strtol(ptr, &p, 10);
		if (r == 0&&p == ptr) {
			ignore_result(write(fd, "#1003|1$", 8));
			DBG("--- format error: #1003|1$ ---\n");
			return 0;
		}
		dlc |= (!!r) << 1;
#ifdef DEBUG
		if (r == 0) {
			DBG("--- zone.db not downloadable ---\n");
		} else {
			DBG("--- zone.db downloadable ---\n");
		}
#endif
		ptr = ++p;
		r = strtol(ptr, &p, 10);
		if (r == 0&&p == ptr) {
			ignore_result(write(fd, "#1003|1$", 8));
			DBG("--- format error: #1003|1$ ---\n");
			return 0;
		}
		dlc |= (!!r) << 2;
#ifdef DEBUG
		if (r == 0) {
			DBG("--- map.zip not downloadable ---\n");
		} else {
			DBG("--- map.zip downloadable ---\n");
		}
#endif
		if (dlc == 0) {
			DBG("--- no file can be downloadable ---\n");
			return 0;
		}

		CLEARA(ftpusr);
		CLEARA(ftpasswd);
		if (*p == 0) {
			strcpy(ftpusr, "anonymous");
		} else if (*p == '|') {
			ptr = ++p;
			p = strrchr(ptr, '|');
			if (p) {
				*p++ = 0;
			}
			strcpy(ftpusr, ptr);
			if (p) {
				strcpy(ftpasswd, p);
			}
		}
		DBG("--- ftpusr = \"%s\", ftpasswd = \"%s\" ---\n", ftpusr, ftpasswd);

		r = start_download(dlc);
		n = sprintf(sndbuf, "#1003|%d$", r);
		DBG("--- send ack: %s ---\n", sndbuf);
		if (write(fd, sndbuf, n) < 0) {
			DBG("--- send ack: %s failed ---\n", sndbuf);
			if (r == 0) {
				DBG("--- send ack failed and trying to cancel the downloading ---\n");
				cancel_download();
			}
			return 0;
		}
		if (r) return 0;
		return 3;
	}
	return 0;
}

/* DONE */
static void
handle_netcmd(struct ev_loop *loop, ev_io *w, int revents)
{
	int   n;
	char *h = NULL;
	char *t = NULL;
	int   r;

	ev_tmeo_remove(loop, &tmeow);
    if (revents & EV_ERROR) {
        DBG("--- SOCKET ERROR ---\n");
        ev_cmd_remove(loop, w);
        return;
    }

    if (revents & EV_READ) {
        n = buffer_read(&nbuff, w->fd);
		if (n <= 0) {
			DBG("--- READ ERROR ---\n");
			ev_cmd_remove(loop, w);
        	return;
		}
		DBG("--- received: %s ---\n", nbuff.rptr);
		h = buffer_find_chr(&nbuff, '#');
		if (h == NULL) {
			DBG("--- unknown cmd message ---\n");
			ev_cmd_remove(loop, w);
			return;
		}
		t = buffer_find_rchr(&nbuff, '$');
		if (t == NULL) {
			DBG("--- no end flag and continue reading ---\n");
			ev_tmeo_add(loop, 10);
			return;
		}
		*t = 0;
		++h;
		r = handle_msg(h, w->fd);
		if (r == 0) {
			ev_cmd_remove(loop, w);
			return;
		} else {
			buffer_reset(&nbuff);
			dlst = r;
			if (dlst == 2) {
				ev_tmeo_add(loop, 10);
			}
		}
    }
}

static void
listen_cb(struct ev_loop *loop, ev_io *w, int revents)
{
	int   conFd = -1;
	struct sockaddr_in addr;
	socklen_t  addrsize;

	addrsize = sizeof(addr);
	conFd = accept(w->fd, (struct sockaddr *)&addr, &addrsize);
	if (conFd < 0) return;
    memset(peerip, 0, 16);
	inet_ntop(AF_INET, &addr.sin_addr, peerip, 16);
	DBG("--- connection accepted: peerip = %s ---\n", peerip);
	if (dlfd != -1||dlst > 0) {
		close(conFd);
		return;
	}

	dlfd = conFd;
	dlst = 1;
	buffer_reset(&nbuff);
	u_set_nonblock(conFd);
	u_tcp_set_nodelay(conFd);
	ev_io_init(&cmdw, handle_netcmd, conFd, EV_READ);
	ev_io_start(loop, &cmdw);
	ev_tmeo_add(loop, 10);
}

static void
pipe_cb(struct ev_loop *loop, ev_io *w, int revents)
{
	char  cmd;

	if (revents & EV_READ) {
		if (read(npfd[0], &cmd, sizeof(cmd)) > 0) {
		#if 0
			if (cmd == 'q') {
				DBG("--- RECEIVED THE DOWNLOAD EXIT CMD ---\n");
				if (dlst > 0) {
					ev_cmd_remove(loop, &cmdw);
				}
				ev_tmeo_remove(loop, &tmeow);
				ev_io_stop(loop, &listenw);
				ev_io_stop(loop, w);
				ev_break(loop, EVBREAK_ALL);
			} else if (cmd == 'f') {
				if (dlst == 3) {
					ignore_result(write(cmdw.fd, "#1004|1$", 8));
					ev_cmd_remove(loop, &cmdw);
					DBG("--- download failed: #1004|1$ ---\n");
					dl_cleanup();
				}
			} else if (cmd == 'o') {
				if (dlst == 3) {
					ignore_result(write(cmdw.fd, "#1004|0$", 8));
					ev_cmd_remove(loop, &cmdw);
					DBG("--- download success: #1004|0$ ---\n");
					dl_cleanup();
					sleep(1);
					reboot(LINUX_REBOOT_CMD_RESTART);
				}
			}
		#else
			switch (cmd) {
			case 0:
				if (dlst == 3) {
					ignore_result(write(cmdw.fd, "#1004|1$", 8));
					ev_cmd_remove(loop, &cmdw);
					DBG("--- download failed: #1004|1$ ---\n");
					dl_cleanup();
				}
				break;
			case 1:
				if (dlst == 3) {
					ignore_result(write(cmdw.fd, "#1004|0$", 8));
					ev_cmd_remove(loop, &cmdw);
					DBG("--- download success: #1004|0$ ---\n");
					dl_cleanup();
					sleep(1);
					reboot(LINUX_REBOOT_CMD_RESTART);
				}
				break;
			case 2:
				DBG("--- RECEIVED THE DOWNLOAD EXIT CMD ---\n");
				if (dlst > 0) {
					ev_cmd_remove(loop, &cmdw);
				}
				ev_tmeo_remove(loop, &tmeow);
				ev_io_stop(loop, &listenw);
				ev_io_stop(loop, w);
				ev_break(loop, EVBREAK_ALL);
				break;
			default:
				break;
			}
		#endif
		}
	}
}

/* DONE */
static void*
ncfg_loop(void *arg)
{
	struct ev_loop *n_loop;
	int    fd;
	ev_io  ev;

	pthread_detach(pthread_self());
	fd = u_tcp_serv(NULL, 9990);
	if (fd < 0) {
		ncfg_v(-1);
		pthread_exit(NULL);
	}

	u_set_nonblock(fd);

	n_loop = ev_loop_new(EVFLAG_AUTO);
	if (n_loop == NULL) {
		close(fd);
		ncfg_v(-1);
		pthread_exit(NULL);
	}

	ncfg_v(1);

	flush_pipe(npfd);
	ev_io_init(&listenw, listen_cb, fd, EV_READ);
	ev_io_start(n_loop, &listenw);

	ev_io_init(&ev, pipe_cb, npfd[0], EV_READ);
	ev_io_start(n_loop, &ev);

	DBG("--- enter download loop ---\n");
	ev_run(n_loop, 0);
	close(fd);

	ev_loop_destroy(n_loop);

	DBG("--- leave download loop ---\n");

	pthread_exit(NULL);
}

static void
ncfg_init(void)
{
	if (ncfginit == 0) {
		if (init_pipe(npfd) < 0) return;
		buffer_init_fix(&nbuff, 64);
		ncfginit = 1;
	}
}

#if 0
void
ncfg_uninit(void)
{
	if (ncfginit == 1) {
		close(npfd[0]);
		close(npfd[1]);
		buffer_base_free(&nbuff);
		ncfginit = 0;
	}
}
#endif

void
start_ncfg_thread(void)
{
	ncfg_init();
	ncfg_lock();
	if (dlrun == 0) {
		if (pthread_create(&nthr, NULL, ncfg_loop, NULL) < 0) {
			ncfg_unlock();
			return;
		}
		while (dlrun == 0)
			ncfg_wait();
		if (dlrun == -1) {
			pthread_join(nthr, NULL);
			dlrun = 0;
		}
	}
	ncfg_unlock();
}

/* r: 0, 1 */
static void
download_finished(int r)
{
	char  c = (char)r;

	ncfg_lock();
	if (dlrun == 1) {
		ignore_result(write(npfd[1], &c, sizeof(c)));
	}
	ncfg_unlock();
}

void
stop_ncfg_thread(void)
{
	download_finished(2);
}

#if 0
}
#endif



