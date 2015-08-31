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
#include "buffer.h"
#include "config.h"
#include "hashtbl.h"
#include "zopt.h"

#define  OPEN_MODE   (S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH)

#ifndef ARR_SZ
#define ARR_SZ(x)     (sizeof(x)/sizeof(x[0]))
#endif

#define UART_BUFSZ    (4*1024)
#define NBUF_SZ       (2*1024)
#define UPORT          8889

#define UARTDEV      "/dev/ttySAC1"

typedef int (*RECV)(int, buffer_t*, int);

struct _recv;
struct _iface;

typedef struct _prot {
#ifdef DEBUG
	char  *name;
#endif
	int    id;
	int    speed;
	int   (*recv)(int, buffer_t*, int);
} prot_t;

typedef struct  _iface {
	char  *name;
	struct _recv  *rcvr;
	void  (*init)(void);
	int   (*open)(struct  _iface *);
	void  (*fdcb)(struct ev_loop *, ev_io*, int);
	void  (*cleanup)(struct ev_loop *);
	void  (*uninit)(void);
} iface_t;

typedef struct _recv {
	ev_io     mw;
	ev_io     pw;
	prot_t   *prot;
	iface_t  *ifc;
	int       ift;
} recv_t;

typedef struct _ev_nio {
	struct list_hdr  list;
	ev_io  nw;
	int    id;
	int    fd;
    buffer_t  *buffer;
} ev_nio_t;

static ev_nio_t   ev_nio[MAXHOST];
static recv_t     g_rcver = {
	.ift = -1,
};

static int        ufd[2] = {-1, -1};

static pthread_t  utid = 0;
static int        rcvthr = -1;

static int        ndisabled = 0;

static pthread_mutex_t   rcv_mtx = PTHREAD_MUTEX_INITIALIZER;

#define recv_lock()      pthread_mutex_lock(&rcv_mtx)
#define recv_unlock()    pthread_mutex_unlock(&rcv_mtx)

static buffer_t    u_buf;

extern int  u_tty_set(int fd, int speed, int datasz, int stopbits, int fc, int vmin, int vtm);
static int  handle_DS7400(int fd, buffer_t *rbuf, int id);
static int  handle_VISTA128(int fd, buffer_t *rbuf, int id);
static int  handle_A120(int fd, buffer_t *rbuf, int id);

static prot_t   g_prot[] = {
	[0] = {
	#ifdef DEBUG
		.name  = "DS7400",
	#endif
		.id    = 0,
		.speed = 2400,
		.recv  = handle_DS7400,
	},
	[1] = {
	#ifdef DEBUG
		.name  = "VISTA128",
	#endif
		.id    = 1,
		.speed = 9600,
		.recv  = handle_VISTA128,
	},
	[2] = {
	#ifdef DEBUG
		.name  = "A120",
	#endif
		.id    = 2,
		.speed = 9600,
		.recv  = handle_A120,
	},
};

static int   ndev_open(iface_t *d);
static int   uart_open(iface_t *d);
static void  uart_cb(struct ev_loop *loop, ev_io *w, int revents);
static void  ndev_cb(struct ev_loop *loop, ev_io *w, int revents);
static void  ev_nio_cleanup(struct ev_loop *l);

static int
dummy_open(iface_t *ifc)
{
	return -1;
}

static void
dummy_cb(struct ev_loop *loop, ev_io *w, int revents)
{
}

static void
dummy_init(void)
{
}

static void
uartif_init(void)
{
	buffer_init_fix(&u_buf, UART_BUFSZ);
}

static void
netif_init(void)
{
	int  i;

	CLEARA(ev_nio);
	for (i = 0; i < ARR_SZ(ev_nio); ++i) {
		ev_nio[i].fd = -1;
		ev_nio[i].id = i + 1;
		U_INIT_LIST_HEAD(&ev_nio[i].list);
	}
	host_load();
}

static void
dummy_uninit(void)
{
}

static void
uartif_uninit(void)
{
	buffer_base_free(&u_buf);
}

static void
netif_uninit(void)
{
	int  i;

	for (i = 0; i < ARR_SZ(ev_nio); ++i) {
		buffer_free(ev_nio[i].buffer);
		ev_nio[i].buffer = NULL;
	}
	hashtable_clear();
	hashtable_destroy();
	host_unload();
}

static iface_t  g_ifc[] = {
	[0] = {
		.name  = "dummy",
		.rcvr  = &g_rcver,
		.init  = dummy_init,
		.open  = dummy_open,
		.fdcb  = dummy_cb,
		.cleanup = NULL,
		.uninit = dummy_uninit,
	},
	[1] = {
		.name  = UARTDEV,
		.rcvr  = &g_rcver,
		.init  = uartif_init,
		.open  = uart_open,
		.fdcb  = uart_cb,
		.cleanup = NULL,
		.uninit = uartif_uninit,
	},
	[2] = {
		.name  = "eth0",
		.rcvr  = &g_rcver,
		.init  = netif_init,
		.open  = ndev_open,
		.fdcb  = ndev_cb,
		.cleanup = ev_nio_cleanup,
		.uninit = netif_uninit,
	},
};

static int   flush_sock_buffer(int fd, buffer_t *b, int i);

static RECV  do_nrecv[] = {NULL, flush_sock_buffer};


//////////////////////////////////////////////////////
#if 0
{
#endif

#if 0
static int
set_prop(int fd, int speed)
{
	struct termios newtio;
	int  name_arr[]  = {115200,  38400,  19200,  9600,  4800,  2400,  1200,  300, 0};
	int  speed_arr[] = {B115200, B38400, B19200, B9600, B4800, B2400, B1200, B300, 0};
	int  i;

	memset(&newtio, 0, sizeof (newtio));
	for (i = 0; name_arr[i]; ++i) { 
		if (speed == name_arr[i]) {
			cfsetispeed(&newtio, speed_arr[i]);
			cfsetospeed(&newtio, speed_arr[i]);
			break;   
		}  
	}

	
	newtio.c_cflag |= CS8;
	
	/* Set the parity */
	newtio.c_cflag &= ~PARENB;
	
	/* Set the number of stop bits */
	newtio.c_cflag &= (~CSTOPB);
	
	/* Selects raw (non-canonical) input and output */
	newtio.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
	newtio.c_oflag &= ~OPOST;
	newtio.c_iflag |= IGNPAR;
	/* Ignore parity errors!!! Windows driver does so why shouldn't I? */
	/* Enable receiber, hang on close, ignore control line */
	newtio.c_cflag |= CREAD | HUPCL | CLOCAL;

	/* Read 1 byte minimun, no timeout specified */
	newtio.c_cc[VMIN] = 1;
	newtio.c_cc[VTIME] = 0;

	if (tcsetattr (fd, TCSANOW, &newtio) < 0)
		return 0;

	return 1;
}
#endif

int
open_uart(char *fname, int speed)
{
	int fd;

	fd = open(fname, O_RDWR|O_NOCTTY|O_SYNC|O_NONBLOCK);
	if (fd < 0) return -1;
	u_tty_set(fd, speed, 8, 1, 0, 0, 0);
	u_set_nonblock(fd);
#if 0
	if (set_prop(fd, speed) == 0) {
		printf("---  set uart failed --\n");
		close(fd);
		return -1;
	}
#endif
	return fd;
}

static int
uart_open(iface_t *ifc)
{
	prot_t *p = ifc->rcvr->prot;
	return open_uart(ifc->name, p->speed);
}

static int
ndev_open(iface_t *ifc)
{
	int  fd;
	char localip[16] = {0};

	u_if_get_ip(ifc->name, localip, sizeof(localip), NULL);
	DBG("--- tcp server: %s:%d ---\n", localip, UPORT);
	fd = u_tcp_serv(localip, UPORT);
	if (fd > 0) {
		u_set_nonblock(fd);
	}
	return fd;
}

#if 0
}
#endif

#define IDTAG       "Device"
#define IDTAGLen     6

static int
get_devid(char *s)
{
	char *ptr;
	int   id = 1;
	
	ptr = strstr(s, IDTAG);
	if (ptr) {
		ptr += IDTAGLen;
		id = strtol(ptr, NULL, 10);
		if (id == 0)
			id = 1;
	}
	return id;
}

//////////////////////////////////////////////////////
//                      DS7400                      //
//////////////////////////////////////////////////////
#if 0
{
#endif

#define APF      "AC Power Failure"
#define APR      "AC Power Restore"
#define DPF      "DC Power Failure"
#define DPR      "DC Power Restore"
#define BL       "Lower Battery"
#define BLR      "Lower Battery Restore"
#define PAK      "Program Accessed From Keypad"
#define ZA       "Zone Alarm"
#define ZR       "Zone Restore"

#define APF_ID     0
#define APR_ID     1
#define DPF_ID     2
#define DPR_ID     3
#define BL_ID      4
#define BLR_ID     5
#define PAK_ID     6
#define ZA_ID      7
#define ZR_ID      8

static char *dscmd[] = {
	[APF_ID] = APF,
	[APR_ID] = APR,
	[DPF_ID] = DPF,
	[DPR_ID] = DPR,
	[BL_ID]  = BL,
	[BLR_ID] = BLR,
	[PAK_ID] = PAK,
	[ZA_ID] = ZA,
	[ZR_ID] = ZR,
};

static int tklen[] = {
	[APF_ID] = 16,
	[APR_ID] = 16,
	[DPF_ID] = 16,
	[DPR_ID] = 16,
	[BL_ID]  = 13,
	[BLR_ID] = 21,
	[PAK_ID] = 28,
	[ZA_ID]  = 10,
	[ZR_ID]  = 12,
};

static int  evid[] = {
	[APF_ID] = EV_APF,
	[APR_ID] = EV_APR,
	[DPF_ID] = EV_DPF,
	[DPR_ID] = EV_DPR,
	[BL_ID]  = EV_BL,
	[BLR_ID] = EV_BLR,
	[PAK_ID] = EV_HPEXT,
};

static void
handle_ds7400_alt(char *ptr, int id)
{
	char *p = NULL;
	int   z;

	ptr += tklen[ZA_ID];
	z = strtol(ptr, &p, 10);
	if (z == 0) return;
	if (id == 0) {
		id = get_devid(p);
	}
#if OLDZ
	zone_alert(z+((id-1)<<8)-1);
#else
	zone_alert((id-1)*99+z-1);
#endif
	DBG("--- host-%d: zone %d alarm ---\n", id, z);
}

static void
handle_ds7400_alr(char *ptr, int id)
{
	char *p = NULL;
	int   z;

	ptr += tklen[ZR_ID];
	z = strtol(ptr, &p, 10);
	if (z == 0) return;
	if (id == 0) {
		id = get_devid(p);
	}
#if OLDZ
	zone_alertr(z+((id-1)<<8)-1);
#else
	zone_alertr((id-1)*99+z-1);
#endif

	DBG("--- host-%d: zone %d alarm restore ---\n", id, z);
}

static inline void
handle_ds7400_evt(char *ptr, int i, int id)
{
	ptr += tklen[i];
	if (id == 0) {
		id = get_devid(ptr);
	}
	send_event(evid[i], id);
	DBG("--- host-%d: %s ---\n", id, dscmd[i]);
}

static void
_handle_DS7400(char *buf, int id)
{
	char *ptr;

	DBG("--- handle ds7400 data: %s ---\n", buf);
	if ((ptr = strstr(buf, dscmd[ZA_ID])) != NULL) {
		handle_ds7400_alt(ptr, id);
	} else if ((ptr = strstr(buf, dscmd[ZR_ID])) != NULL) {
		handle_ds7400_alr(ptr, id);
	} else if ((ptr = strstr(buf, dscmd[APF_ID])) != NULL) {
		handle_ds7400_evt(ptr, APF_ID, id);
	} else if ((ptr = strstr(buf, dscmd[APR_ID])) != NULL) {
		handle_ds7400_evt(ptr, APR_ID, id);
	} else if ((ptr = strstr(buf, dscmd[DPF_ID])) != NULL) {
		handle_ds7400_evt(ptr, DPF_ID, id);
	} else if ((ptr = strstr(buf, dscmd[DPR_ID])) != NULL) {
		handle_ds7400_evt(ptr, DPR_ID, id);
	} else if ((ptr = strstr(buf, dscmd[BL_ID])) != NULL) {
		handle_ds7400_evt(ptr, BL_ID, id);
	} else if ((ptr = strstr(buf, dscmd[BLR_ID])) != NULL) {
		handle_ds7400_evt(ptr, BLR_ID, id);
	} else if ((ptr = strstr(buf, dscmd[PAK_ID])) != NULL) {
		handle_ds7400_evt(ptr, PAK_ID, id);
	}
}

static int
handle_DS7400(int fd, buffer_t *rbuf, int id)
{
	char *p = NULL;
	const char  endflag[] = {0x0d,0x0a};

	if (buffer_read(rbuf, fd) <= 0) {
		DBG("--- READ ERROR ---\n");
		buffer_reset(rbuf);
		return -1;
	}

	do {
		p = buffer_find(rbuf, endflag, sizeof(endflag));
		if (p == NULL) {
			return 0;
		}

		*p++ = 0;
		*p++ = 0;

		_handle_DS7400(BUFPTR_DATA(rbuf), id);
	
		buffer_drain_ptr(rbuf, p);
	} while (BUFPTR_LENGTH(rbuf) > 0);
	return 0;
}

#if 0
}
#endif

//////////////////////////////////////////////////////
//                     VISTA120                     //
//////////////////////////////////////////////////////
#if 0
{
#endif

#define VAPF       "AC LOSS"
#define VAPR       "AC RESTORE"
#define VDPF       "BATTERY FAIL"
#define VDPR       "BAT TST FAIL"
#define VBL        "LOW BATTERY"
#define VBLR       "LOW BATT RST"
#define VPGENT     "PROGRM ENTRY"
#define VPGEXT     "PROGRAM EXIT"
#define VBUSE      "RPM SUPR"
#define VBUSR      "RPM RST"

#define VPANIC     "PANIC"
#define VPNCR      "PNC RST"
#define VTRBL      "TROUBLE"
#define VTRBLR     "TRBL RST"
#define VBURG      "BURGLARY"
#define VBURGR     "BURG RST"

#define VAPF_ID       0
#define VAPR_ID       1
#define VDPF_ID       2
#define VDPR_ID       3
#define VBL_ID        4
#define VBLR_ID       5
#define VPGENT_ID     6
#define VPGEXT_ID     7
#define VBUSE_ID      8
#define VBUSR_ID      9

static char *vcode[] = {
	[VAPF_ID] = VAPF,
	[VAPR_ID] = VAPR,
	[VDPF_ID] = VDPF,
	[VDPR_ID] = VDPR,
	[VBL_ID]  = VBL,
	[VBLR_ID] = VBLR,
	[VPGENT_ID] = VPGENT,
	[VPGEXT_ID] = VPGEXT,
	[VBUSE_ID]  = VBUSE,
	[VBUSR_ID]  = VBUSR,
};

static int   vclen[] = {
	[VAPF_ID] = 7,
	[VAPR_ID] = 10,
	[VDPF_ID] = 12,
	[VDPR_ID] = 12,
	[VBL_ID]  = 11,
	[VBLR_ID] = 12,
	[VPGENT_ID] = 12,
	[VPGEXT_ID] = 12,
	[VBUSE_ID]  = 8,
	[VBUSR_ID]  = 7,
};

static int   vevid[] = {
	[VAPF_ID] = EV_APF,
	[VAPR_ID] = EV_APR,
	[VDPF_ID] = EV_DPF,
	[VDPR_ID] = EV_DPR,
	[VBL_ID]  = EV_BL,
	[VBLR_ID] = EV_BLR,
	[VPGENT_ID] = EV_HPENT,
	[VPGEXT_ID] = EV_HPEXT,
	[VBUSE_ID]  = EV_HBUSE,
	[VBUSR_ID]  = EV_HBUSR,
};

static inline void
_handle_vista_evt(char *ptr, int i, int id)
{
	if (id == 0) {
		id = get_devid(ptr+vclen[i]);
	}
	send_event(vevid[i], id);
	DBG("--- host-%d: %s ---\n", id, vcode[i]);
}

static inline void
handle_vista_alr(char *s, int id)
{
	char *p;
	int   z;

	z = strtol(s, &p, 10);
	if (id == 0) {
		id = get_devid(p);
	}
#if OLDZ
	zone_alertr(z+((id-1)<<8)-1);
#else
	zone_alertr((id-1)*99+z-1);
#endif
}

static inline void
handle_vista_alt(char *s, int id)
{
	char *p;
	int   z;

	z = strtol(s, &p, 10);
	if (id == 0)
		id = get_devid(p);
#if OLDZ
	zone_alert(z+((id-1)<<8)-1);
#else
	zone_alert((id-1)*99+z-1);
#endif
}

/*
P1 01/01/00 00:38   PANIC        002
50 31 20 30 31 2F 30 31 2F 30 30 20 30 30 3A 33 38 20 20 20 50 41 4E 49 43 20 20 20 20 20 20 20 20 30 30 32 20 20 20 20 20 20 20 0D 0A 00 00
P1 01/01/00 00:38   PNC RST      002
50 31 20 30 31 2F 30 31 2F 30 30 20 30 30 3A 33 38 20 20 20 50 4E 43 20 52 53 54 20 20 20 20 20 20 30 30 32 20 20 20 20 20 20 20 0D 0A 00 00
P1 01/01/00 00:41   BURGLARY     001
50 31 20 30 31 2F 30 31 2F 30 30 20 30 30 3A 34 31 20 20 20 42 55 52 47 4C 41 52 59 20 20 20 20 20 30 30 31 20 20 20 20 20 20 20 0D 0A 00 00
P1 01/01/00 00:41   BURG RST     001
50 31 20 30 31 2F 30 31 2F 30 30 20 30 30 3A 34 31 20 20 20 42 55 52 47 20 52 53 54 20 20 20 20 20 30 30 31 20 20 20 20 20 20 20 0D 0A 00 00
*/
static void
_handle_VISTA128(char *buf, int id)
{
	char *ptr = NULL;

	DBG("--- handle vista128: %s ---\n", buf);
	if ((ptr = strstr(buf, VPANIC)) != NULL) {
		ptr += 13;
		handle_vista_alt(ptr, id);
	} else if ((ptr = strstr(buf, VPNCR)) != NULL) {
		ptr += 13;
		handle_vista_alr(ptr, id);
	} else if ((ptr = strstr(buf, VTRBL)) != NULL) {
		ptr += 13;
		handle_vista_alt(ptr, id);
	} else if ((ptr = strstr(buf, VTRBLR)) != NULL) {
		ptr += 13;
		handle_vista_alr(ptr, id);
	} else if ((ptr = strstr(buf, VBURG)) != NULL) {
		ptr += 13;
		handle_vista_alt(ptr, id);
	} else if ((ptr = strstr(buf, VBURGR)) != NULL) {
		ptr += 13;
		handle_vista_alr(ptr, id);
	} else if ((ptr = strstr(buf, vcode[VAPF_ID])) != NULL) {
		_handle_vista_evt(ptr, VAPF_ID, id);
	} else if ((ptr = strstr(buf, vcode[VAPR_ID])) != NULL) {
		_handle_vista_evt(ptr, VAPR_ID, id);
	} else if ((ptr = strstr(buf, vcode[VDPF_ID])) != NULL) {
		_handle_vista_evt(ptr, VDPF_ID, id);
	} else if ((ptr = strstr(buf, vcode[VDPR_ID])) != NULL) {
		_handle_vista_evt(ptr, VDPR_ID, id);
	} else if ((ptr = strstr(buf, vcode[VBL_ID])) != NULL) {
		_handle_vista_evt(ptr, VBL_ID, id);
	} else if ((ptr = strstr(buf, vcode[VBLR_ID])) != NULL) {
		_handle_vista_evt(ptr, VBLR_ID, id);
	} else if ((ptr = strstr(buf, vcode[VPGENT_ID])) != NULL) {
		_handle_vista_evt(ptr, VPGENT_ID, id);
	} else if ((ptr = strstr(buf, vcode[VPGEXT_ID])) != NULL) {
		_handle_vista_evt(ptr, VPGEXT_ID, id);
	} else if ((ptr = strstr(buf, vcode[VBUSE_ID])) != NULL) {
		_handle_vista_evt(ptr, VBUSE_ID, id);
	} else if ((ptr = strstr(buf, vcode[VBUSR_ID])) != NULL) {
		_handle_vista_evt(ptr, VBUSR_ID, id);
	}
}

#if 0
}
#endif

static int
handle_VISTA128(int fd, buffer_t *rbuf, int id)
{
	char *p = NULL;
	const char  endflag[] = {0x0d,0x0a};

	if (buffer_read(rbuf, fd) <= 0) {
		DBG("--- READ ERROR ---\n");
		buffer_reset(rbuf);
		return -1;
	}

	do {
		p = buffer_find(rbuf, endflag, sizeof(endflag));
		if (p == NULL) {
			return 0;
		}

		*p++ = 0;
		*p++ = 0;

		_handle_VISTA128(BUFPTR_DATA(rbuf), id);
	
		buffer_drain_ptr(rbuf, p+2);
	} while (BUFPTR_LENGTH(rbuf) > 0);
	return 0;
}

unsigned char
my_cksum(unsigned char *buf, int len)
{
	unsigned short sum = 0;
	int      i;

	for (i = 0; i < len; ++i) {
		sum += buf[i];
	}
	sum &= ~(1<<7);
	return (unsigned char)(sum&0xff);
}

static int
_handle_A120(unsigned char *p, int len, int id)
{
	int  n;
	unsigned char  s;
	int  be[] = {EV_DPR, EV_DPF};

	n = p[3] + 5;
	if (n > len) {
		DBG("----- not enough data, len = %d, real = %d -----\n", n, len);
		return 0;
	}

	s = my_cksum(&p[1], n-2);
	if (s != p[n-1]) {
		DBG("--- checksum error: sum = %02x, real = %02x ---\n", s, p[n-1]);
		return n;
	}

	if (id == 0)
		id = p[1] + 1;

	switch (p[2]) {
	case 0x21:
		DBG("--- host-%d,zone-%d: alarm ---\n", id, p[4]);
	#if OLDZ
		zone_alert(p[4] - 1 + ((id-1)<<8));
	#else
		zone_alert((id-1)*99+p[4]-1);
	#endif
		break;
	case 0x22:
		DBG("--- host-%d,zone-%d: alarm restore ---\n", id, p[4]);
	#if OLDZ
		zone_alertr(p[4] - 1 + ((id-1)<<8));
	#else
		zone_alertr((id-1)*99+p[4]-1);
	#endif
		break;
	case 0x23: //防拆报警
		send_event(EV_A120DP, id);
		break;
	case 0x24: //防拆恢复
		send_event(EV_A120DPR, id);
		break;
	case 0x25: //模块离线/在线
		if (p[5]) {
			DBG("--- host-%d,zone-%d: alarm[module offline] ---\n", id, p[4]);
		#if OLDZ
			zone_alert(p[4] - 1 + ((id-1)<<8));
		#else
			zone_alert((id-1)*99+p[4]-1);
		#endif
		} else {
			DBG("--- host-%d,zone-%d: alarm restore [module online] ---\n", id, p[4]);
		#if OLDZ
			zone_alertr(p[4] - 1 + ((id-1)<<8));
		#else
			zone_alertr((id-1)*99+p[4]-1);
		#endif
		}
		break;
	case 0x31: //防区故障/正常
		if (p[6]) {
			DBG("--- host-%d,zone-%d: alarm[zone failed] ---\n", id, p[4]);
		#if OLDZ
			zone_alert(p[4] - 1 + ((id-1)<<8));
		#else
			zone_alert((id-1)*99+p[4]-1);
		#endif
		} else {
			DBG("--- host-%d,zone-%d: alarm restore [zone restored] ---\n", id, p[4]);
		#if OLDZ
			zone_alertr(p[4] - 1 + ((id-1)<<8));
		#else
			zone_alertr((id-1)*99+p[4]-1);
		#endif
		}
		break;
	case 0x08:
		send_event(EV_HPENT, id);
		break;
	case 0x09:
		send_event(EV_HPEXT, id);
		break;
	case 0x2b: //电池故障/恢复
		send_event(be[(int)p[6]], id);
		break;
	case 0x2c: //交流供电
//		send_event(EV_APR, hid);
		send_event(EV_A120ACP, id);
		break;
	case 0x2d: //电池供电
//		send_event(EV_APF, hid);
		send_event(EV_A120DCP, id);
		break;
	case 0x2e: //电池电压低
		send_event(EV_BL, id);
		break;
	default:
		break;
	}

	return n;
}

static int
handle_A120(int fd, buffer_t *rbuf, int id)
{
	unsigned char *p = NULL;
	int   n;

	if (buffer_read(rbuf, fd) <= 0) {
		DBG("--- READ ERROR ---\n");
		buffer_reset(rbuf);
		return -1;
	}

	p = buffer_find_chr(rbuf, 0xcc);
	if (p == NULL) {
		DBG("----- no more data, continue waiting -----\n");
		buffer_reset(rbuf);
		return 0;
	}
	buffer_drain_ptr(rbuf, (char*)p);

	while (BUFPTR_LENGTH(rbuf) > 4) {
		n = _handle_A120(p, BUFPTR_LENGTH(rbuf), id);
		if (n == 0) {
			return 0;
		}
		buffer_drain(rbuf, n);

		p = buffer_find_chr(rbuf, 0xcc);
		if (p == NULL) {
			DBG("----- no more data, continue waiting -----\n");
			buffer_reset(rbuf);
			return 0;
		}
		buffer_drain_ptr(rbuf, (char*)p);
	}
	return 0;
}

////////////////////////////////////////////////////////////////////////////

static void
uart_cb(struct ev_loop *l, ev_io *w, int revents)
{
	recv_t  *r = U_CONTAINER_OF(w, struct _recv, mw);
	prot_t  *p = r->prot;

	if (revents & EV_READ) {
		p->recv(w->fd, &u_buf, 0);
    }
}

/* DONE */
static void
pipe_cb(struct ev_loop *l, ev_io *w, int revents)
{
	unsigned int  cmd;
	recv_t  *r = U_CONTAINER_OF(w, struct _recv, pw);
	iface_t *ifc = r->ifc;
	int      key;
	int      value;

	if (revents & EV_READ) {
		if (read(ufd[0], &cmd, sizeof(cmd)) == sizeof(cmd)) {
			if (cmd == (unsigned int)~0) {
				DBG("--- RECEIVED THE RECVER EXIT CMD ---\n");
				ev_io_stop(l, &r->mw);
				ev_io_stop(l, w);
				if (ifc->cleanup) {
					ifc->cleanup(l);
				}
				ev_break(l, EVBREAK_ALL);
			} else if (cmd == 0) {
				DBG("--- clear hashtable ---\n");
				hashtable_clear();
			} else if (cmd == 1) {
				DBG("--- disable netdev ---\n");
				ndisabled = 1;
			} else if (cmd == 2) {
				DBG("--- enable netdev ---\n");
				ndisabled = 0;
			} else {
				value = cmd >> 24;
				key = ((1<<24)-1) & cmd;
				hashtable_set(key, value);
			}
		}
	}
}

/* DONE */
static void*
recver_loop(void *arg)
{
	struct ev_loop *l;
	int      fd;
	recv_t  *r = arg;
	iface_t *ifc;

	ifc = r->ifc;
	DBG("--- interface: %s, host type = %s ---\n", ifc->name, r->prot->name);
	fd = ifc->open(ifc);
	if (fd < 0) {
		DBG("--- open %s,%s failed ---\n", r->prot->name, ifc->name);
		exit(1);
	}

	l = ev_loop_new(EVFLAG_AUTO);
	if (l == NULL) {
		exit(1);
	}

	flush_pipe(ufd);

	do_nrecv[0] = r->prot->recv;

	ev_io_init(&r->mw, ifc->fdcb, fd, EV_READ);
	ev_io_start(l, &r->mw);

	ev_io_init(&r->pw, pipe_cb, ufd[0], EV_READ);
	ev_io_start(l, &r->pw);

	DBG("--- enter recver event loop ---\n");
	ev_run(l, 0);

	close(fd);

	ev_loop_destroy(l);

	DBG("--- exit recver event loop ---\n");

	pthread_exit(NULL);
}

/* called when slave--->master */
void
recver_init(void)
{
	if (!is_master()) return;
	if (init_pipe(ufd) < 0) {
		exit(1);
	}
}

/* called when master--->slave */
void
recver_uninit(void)
{
	if (ufd[0]!= -1) {
		close(ufd[0]);
		ufd[0] = -1;
	}

	if (ufd[1]!= -1) {
		close(ufd[1]);
		ufd[1] = -1;
	}
}

#if 0
/* master api: DONE */
void
recver_start(void)
{
	if (!is_master()) return;
	if (miface < 0||miface >= ARR_SZ(g_ifc)
	   ||mdev < 0||mdev >= ARR_SZ(g_prot)) return;

	g_rcver.ifc = &g_ifc[miface];
	g_rcver.prot = &g_prot[mdev];

	recv_lock();
	if (rcvthr != 0) {
		g_rcver.ifc->init();
		rcvthr = pthread_create(&utid, NULL, recver_loop, &g_rcver);
		if (rcvthr != 0) {
			printf("--- create the recver thread failed ---\n");
			g_rcver.ifc->uninit();
			recv_unlock();
			exit(1);
		}
	}
	recv_unlock();
	DBG("---------- START THE RECVER THREAD. --------\n");
}

/* DONE: called when enter system programming. */
void
recver_exit(void)
{
	unsigned int  cmd = (unsigned int)~0;
	recv_lock();
	if (rcvthr == 0) {
		ignore_result(write(ufd[1], &cmd, sizeof(cmd)));
		pthread_join(utid, NULL);
		rcvthr = -1;
		g_rcver.ifc->uninit();
		DBG("---------- recver thread exited. --------\n");
	}
	recv_unlock();
}

#else

/* called when slave-->master or iface changed from other to netif. */
void
recver_prepare(void)
{
	if (!is_master()) return;
	if (miface < 0||miface >= ARR_SZ(g_ifc)
	   ||mdev < 0||mdev >= ARR_SZ(g_prot)) return;

	if (g_rcver.ift != miface) {
		if (g_rcver.ift >= 0&&g_rcver.ifc) {
			g_ifc[g_rcver.ift].uninit();
		}
		g_rcver.ifc = &g_ifc[miface];
		g_rcver.prot = &g_prot[mdev];
		g_rcver.ift = miface;
		g_rcver.ifc->init();
	}
}

/* called when master-->slave or iface changed from netif to other. */
void
recver_finish(void)
{
	g_rcver.ifc->uninit();
	memset(&g_rcver, 0, sizeof(recv_t));
	g_rcver.ift = -1;
}

void
recver_start(void)
{
	if (!is_master()||g_rcver.ift == -1) return;
	recv_lock();
	if (rcvthr != 0) {
		rcvthr = pthread_create(&utid, NULL, recver_loop, &g_rcver);
		if (rcvthr != 0) {
			printf("--- create the recver thread failed ---\n");
			recv_unlock();
			exit(1);
		}
	}
	recv_unlock();
	DBG("---------- START THE RECVER THREAD. --------\n");
}

void
recver_exit(void)
{
	unsigned int  cmd = (unsigned int)~0;
	recv_lock();
	if (rcvthr == 0) {
		ignore_result(write(ufd[1], &cmd, sizeof(cmd)));
		pthread_join(utid, NULL);
		rcvthr = -1;
		DBG("---------- recver thread exited. --------\n");
	}
	recv_unlock();
}

/* en: [0, 1] */
void
enable_netdev(int en)
{
	unsigned int  cmd = 1 + en;

	recv_lock();
	if (rcvthr == 0&&miface == 2) {
		ignore_result(write(ufd[1], &cmd, sizeof(cmd)));
	}
	recv_unlock();
}

#endif

/////////////////////////////////////////////////////////////////
#if 0
{
#endif

struct list_hdr  ev_head = {&ev_head, &ev_head};

/* DONE */
static ev_nio_t*
ev_nio_get(unsigned int i, int *s)
{
	int  id;
	ev_nio_t  *ev = NULL;

	*s = 0;
	id = hashtable_get(i);
	if (id > 0&&id <= MAXHOST)
		ev = &ev_nio[id-1];
	else if (id == 0) {
		hashtable_set(i, MAXHOST+1);
		*s = 1;
	}

	return ev;
}

/* DONE */
static void
ev_nio_remove(struct ev_loop *l, ev_nio_t *ev)
{
	ev_io_stop(l, &ev->nw);
	close(ev->fd);
	ev->fd = -1;
	buffer_reset(ev->buffer);
	u_list_del(&ev->list);
	DBG("--- remove watcher-%d ---\n", ev->id);
}

/* DONE */
static void
ev_nio_cleanup(struct ev_loop *l)
{
	struct list_hdr *list;

	while (!u_list_empty(&ev_head)) {
		list = ev_head.next;
		ev_nio_remove(l, (ev_nio_t *)list);
	}
}

static int
flush_sock_buffer(int fd, buffer_t *b, int i)
{
	int   r = 0;

	if (buffer_read(b, fd) <= 0) {
		DBG("--- READ ERROR ---\n");
		r = -1;
	}
	buffer_reset(b);
	return r;
}

/* DONE */
static void
nrecv_cb(struct ev_loop *l, ev_io *w, int revents)
{
    ev_nio_t *ev;
	int   r = 0;

    ev = U_CONTAINER_OF(w, struct _ev_nio, nw);
    if (revents & EV_ERROR) {
        DBG("--- SOCKET ERROR ---\n");
        ev_nio_remove(l, ev);
		update_hostip(ev->id, 0);
        return;
    }

    if (revents & EV_READ) {
		r = (*do_nrecv[ndisabled])(w->fd, ev->buffer, ev->id);
		if (r < 0) {
        	ev_nio_remove(l, ev);
			update_hostip(ev->id, 0);
        }
    }
}

static void
ndev_cb(struct ev_loop *l, ev_io *w, int revents)
{
	int   conFd = -1;
    ev_nio_t *ev = NULL;
	struct sockaddr_in addr;
	socklen_t  addrsize;
	unsigned int  hostp;
	int    d = 0;
#ifdef DEBUG
	char   peerip[16];
#endif

	addrsize = sizeof(addr);
	conFd = accept(w->fd, (struct sockaddr *)&addr, &addrsize);

	if (conFd >= 0) {
#ifdef DEBUG
        memset(peerip, 0, 16);
		inet_ntop(AF_INET, &addr.sin_addr, peerip, 16);
		DBG("--- connection accepted: peerip = %s ---\n", peerip);
#endif
		hostp = ntohl(addr.sin_addr.s_addr)&hmask;
		DBG("--- hostp = %d ---\n", hostp);
		ev = ev_nio_get(hostp, &d);
		if (ev != NULL) {
			DBG("--- host-%d connected ---\n", ev->id);
			if (ev->buffer == NULL) {
				ev->buffer = buffer_new_fixed(NBUF_SZ);
			} else {
				buffer_reset(ev->buffer);
			}
			u_set_nonblock(conFd);
			u_tcp_set_nodelay(conFd);
			ev->fd = conFd;
			ev_io_init(&ev->nw, nrecv_cb, conFd, EV_READ);
			ev_io_start(l, &ev->nw);
			u_list_append(&ev->list, &ev_head);
			update_hostip(ev->id, 1); //online message.
		} else {
			if (d) {
				update_hostip(0, hostp);
			}
			u_tcp_quit(conFd);
		}
	}
}

#if 0
}
#endif

void
hashtbl_clear(void)
{
	unsigned int  cmd = 0;

	recv_lock();
	if (rcvthr == 0&&miface == 2) {
		ignore_result(write(ufd[1], &cmd, sizeof(cmd)));
	} else {
		hashtable_clear();
	}
	recv_unlock();
}

/* DONE: called in runtime. */
void
hashtbl_save(int key, unsigned int value)
{
	unsigned int  cmd;

	if (value <= MAXHOST&&value > 0) {
		recv_lock();
		if (rcvthr == 0&&miface == 2) {
			cmd = (value << 24) | key;
			DBG("--- hash set in recver thread ---\n");
			ignore_result(write(ufd[1], &cmd, sizeof(cmd)));
		} else {
			hashtable_set(key, value);
		}
		recv_unlock();
	}
}



