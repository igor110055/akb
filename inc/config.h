#ifndef _AKB_CONFIG_H_
#define _AKB_CONFIG_H_

#include <time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "ulib.h"

#ifndef ignore_result
#define ignore_result(x) ({ typeof(x) z = x; (void)sizeof z; })
#endif

#ifndef  SECS_PER_DAY
#define  SECS_PER_DAY   (24*3600)
#endif

#define MAXSLV     9
#define MAXHOST    32


//#define  RTC_CLOCK   1

//#define MAPDLL    1

#define ENABLE_SCRSVR     1

#if ENABLE_SCRSVR
#define SCRSVR    120
#endif
#define DQTMEO    15

#define HSTNR     6
#define MAXST     (2+HSTNR*2)

#define  MAXPLEN     6
#define  MINPLEN     4

#define SUPASSWD      "357159"

typedef void (*PVOID)(void);
typedef void (*VINT2)(int, int);


/* fixed data */
#define  NETCONF     "/etc/eth0-setting"
#define  HOSTCONF    "/usr/etc/host.conf"
#define  GENCONF     "/usr/etc/general.conf"
#define  PZTMRDB     "/usr/etc/pztmr.conf"
#define  UARTCONF    "/usr/etc/uart.conf"

#define  DEVID       "/usr/etc/devid"


#define  IFTCONF     "/usr/etc/iftyp.conf"

#define  GENDB       "/usr/etc/genchr.conf"
#define  USRDB       "/usr/etc/usr.db"
#define  ZONEDB      "/usr/etc/zone.db"
#define  PZONEDB     "/usr/etc/pzone.db"

/* dynamic data */
#define  EINFDB      "/usr/db/einfo.db"
#define  AINFDB      "/usr/db/ainfo.db"

#define  ALRMDB      "/usr/db/alrm.db"
#define  SYSTDB      "/usr/db/syst.db"

#define  MAPKT       "/usr/etc/map.zip"
#define  ZMAPDB      "/usr/etc/zmap.db"
#define  ZMPOS       "/usr/etc/zmapos.db"

#define  MAP_PATH      "/usr/etc/map"
#define  MAP_RES_FILE  "/usr/etc/zmap.img"


#define  MAXE          10240
#define  MAXZA         10240

#define  MAXNAME       32
#define  MAXZ          (MAXHOST*99)
#define  MAXPZ         10
#define  ZBNUM         (MAXZ>>5)

#define  XFER_ALARM     0
#define  XFER_SYSEV     1
#define  XFER_ZNEST     2
#define  XFER_NONE      3
#define  XFER_UART      4
#define  XFER_NET       5
#define  RC             6

#define  MAXUSR         8

#define  IFACE_A120      0
#define  IFACE_DS7400    1
#define  IFACE_VISTA120  2

#define  PRI_PG      (1<<0)
#define  PRI_FN      (1<<1)
#define  PRI_BP      (1<<2)
#define  PRI_FARM    (1<<3)
#define  PRI_DEL     (1<<4)
#define  PRI_ARM     (1<<5)

/* in UI thread send to einfo.c */
/* transfer by UI thread to rs485 or network. */
#define EV_RBT             0
#define EV_LOGIN           1
#define EV_LOGOUT          2
#define EV_PRG             3

/* from net detect thread to zopt.c, _syst:bit0 */
#define EV_NETR            4
#define EV_NETE            5

/* from 485 thread to zopt.c, _syst:bit1 */
#define EV_RS485R          6
#define EV_RS485E          7

/* from uart thread to zopt.c */
#define EV_HPENT           8
#define EV_HPEXT           9
#define EV_APR             10
#define EV_APF             11
#define EV_DPR             12
#define EV_DPF             13
#define EV_BLR             14
#define EV_BL              15
#define EV_HBUSR           16
#define EV_HBUSE           17
#define EV_A120DPR         18
#define EV_A120DP          19

#define EV_A120ACP         20
#define EV_A120DCP         21

#define EV_HCONN           22
#define EV_HDCON           23

// for pztm_t and sys_tmr.c 
#define MAXTPZ             6

/////////////////////////////////////
//         for  send_ainfo()              //
/////////////////////////////////////
#define ZALR         0
#define ZALT         1
#define ZFLR         2
#define ZFLT         3
#define ZARM         4
#define ZDAM         5
#define ZBPA         6
#define ZBPR         7

#define MAXZCMD      8

#define AARM         0
#define FARM         1
#define ADAM         2
#define PARM         3
#define PFAM         4
#define PDAM         5
#define PBPA         6
#define PBPR         7
#define PTAM         8
#define PTDM         9

#define IfConfId     0
#define ZoneDbId     1
#define PzneDbId     2
#define UsrDbId      3
#define GenDbId      4
#define PzTmDbId     5
#define MapktId      6
#define ZmapDbId     7
#define ZmPosId      8

#define MAXCFGID     (ZmPosId+1)

#ifndef ARR_SZ
#define ARR_SZ(x)     (sizeof(x)/sizeof(x[0]))
#endif

#ifndef CLEARV
#define CLEARV(x)     memset(&x, 0, sizeof(x))
#endif

#ifndef CLEARA
#define CLEARA(x)     memset(&x[0], 0, sizeof(x))
#endif

typedef struct _ucnf {
	int  relay;
	int  siom;
	int  dev;
	unsigned char tm[6];
} ucnf_t;

typedef struct _gencfg {
	int             rprt; //real time print
	char            user[48];
	unsigned char   vol;
	unsigned char   nrh;
	unsigned char   cid;
	unsigned char   aud;
	unsigned char   atp;
	unsigned char   etp;
} gencfg_t;

//////////////////////////////////
typedef struct _tclk {
	unsigned char hur;
	unsigned char min;
} tclk_t;

typedef struct _pztm {
	int           we;
	tclk_t        tm[4];
	int           npzt;
	unsigned char pz[MAXTPZ]; //element value: [1, MAXPZ]
	unsigned char pt[MAXTPZ];
} pztm_t;
///////////////////////////////////

typedef struct _usr {
	char           name[24];
	char           passwd[16];
	unsigned short pri;
	unsigned short id;
	unsigned short pz;
} usr_t;

typedef struct _zone {
	char     zstr[6];
	char     pstr[4];
	char     zname[MAXNAME];   //name of zone
	char     dname[16];        //name of detector
	char     vip[16];
	unsigned short znr;        //1~512
	unsigned char  pnr;        //1~MAXPZ
	unsigned char  ztp;        //zone type
	unsigned short prot;
	unsigned int   idx;        //index of position in zarray[]
	unsigned int   pidx;       //index of position in pzone
} zone_t;

typedef struct _pzone {
	char  pstr[4];
	char  pname[MAXNAME];
	int   pnr;      //1~16
	unsigned int  idx;
} pzone_t;

typedef struct _genchr {
	unsigned char itm[2];
	unsigned char otm;
	unsigned char xfer;
} genchr_t;

typedef struct _zainf {
	char     idstr[8];
	char     zstr[6];
	char     zname[MAXNAME];
	char     pstr[4];
	char     inf[16];
	char     tmstr[20];
	unsigned short opcode;
	time_t   etm;
	unsigned int pnr;
	unsigned int znr;
	unsigned int idx;
} zainf_t;

typedef struct _einf {
	char     idstr[8];
	char     ename[24];
	char     tmstr[20];
	time_t   etm;
	int      eid;
	int      hid;   //host id
	unsigned int idx;
} einf_t;

typedef void  (*FVINT)(int);
typedef void  (*FVINT2)(int, int);
typedef int   (*FINT2)(int, int);

#define  zone_type(z)      (zonedb[z].ztp)

extern genchr_t genchr;
extern usr_t    g_usr[MAXUSR];
extern int      g_ucnt;
extern usr_t   *curr_usr;
extern int      g_uid;

extern zone_t   zonedb[MAXZ];
extern int      zarray[MAXZ];
extern int      zarcnt;

/* pzone list */
extern pzone_t  pzone[MAXPZ];
extern int      pzarr[MAXPZ];
extern int      pzcnt;

extern int      g_tflag;

extern zainf_t  ainf[MAXZA];
extern einf_t   einf[MAXE];

#define ZFLTMAX     1024
extern unsigned short  zflt[ZFLTMAX];

extern unsigned short  g_devid;
extern unsigned short  masterid;
extern char     g_masterip[16];
extern char     g_ipaddr[16];
extern int      mdev;
extern int      miface;
extern unsigned int   hmask;
extern unsigned int   hnet;

#define  pnrofz(z)    (zonedb[z].pnr-1)
#define  zoneidx(z)   (zonedb[z].idx)

extern char    *zstat;
extern char     pstat[MAXPZ];


///////////////////////////////////////////
//                gencfg.c                //
///////////////////////////////////////////
#define  xfer_alarm()    (genchr.xfer & 1)
#define  xfer_event()    (genchr.xfer & (1 << 1))
#define  xfer_zstat()    (genchr.xfer & (1 << 2))
#define  xfer_noway()    (genchr.xfer & (1 << 3))
#define  xfer_byuart()   (genchr.xfer & (1 << 4))
#define  xfer_bynet()    (genchr.xfer & (1 << 5))

extern void    start_service(void);
//extern void    stop_service(void);

extern void    start_net(void);
extern void    stop_net(void);

extern int     open_uart(char *fname, int speed);
extern void    resume_master(void);
extern void    pause_master(void);
extern void    sysf_quit(void);
extern void    sysp_exit(void);
extern int     is_valid_unip(char *val);
extern void    netconf_load(void);
extern void    uart_conf(void);
extern void    general_conf(void);
extern void    load_pztmr(int ud);
extern pztm_t *get_pztm_conf(void);

/* idx: [0, MAXTPZ-1], p: [0, MAXPZ-1] */
extern void    del_pzone_from_pztm(int p, int idx);
extern ucnf_t*  get_uart_conf(void);
extern void    update_titvl(int i);
extern void    load_iftype(void);
extern void    load_usr(void);
extern void    add_usr(usr_t *u, int id);
extern void    load_genchr(void);
extern void    load_alarm(void);
extern void    load_ainfo(void);

extern void    load_alarm(void);

extern int     passwd_verify(char *pass);
extern int     priority_get(int opt);
extern int     pz_get(int pz);
extern int     pzg_get(unsigned short pzm);
extern int     usr_priority(int uid, int opt);
extern usr_t  *get_superuser(void);
extern int     verify_user(char *name, char *md5str);

extern void    load_zone(void);
extern void    load_zinf(void);
extern int     store_zone(zone_t *z, int add);
extern int     load_pzone_file(void);
extern void    load_pzonedb(void);

extern int     pzone_exist(int pnr);
extern void    load_einfo(void);

extern void    add_alarm_info(zainf_t *za);

extern void    send_alarm(int opt, int arg);
extern void    send_event_db(int ev, int id);

/* znr: 1~512 */
extern int     mk_zainf(zainf_t *zi, int opt, unsigned short arg);
/* p: 1~MAXPZ */
extern int     mk_painf(zainf_t *zi, int opt, unsigned short arg);
extern void    mk_aainf(zainf_t *zi, int opt);

extern void    send_ainfo(int opt, int arg);

extern void    sysp_ref(void);

extern int     get_host_nr(void);
extern void    update_hostnr(int n);
/* called when arming opt */
extern int     if_host_normal(int i);
extern int     if_host_connected(int i);

extern int     init_pipe(int pfd[2]);

extern int     get_alarm_count(void);

extern int     get_real_print(void);

extern char*   get_owner_ptr(void);
extern void    handle_netifloop(void);

//extern void    ncfg_init(void);
//extern void    ncfg_uninit(void);
extern void    start_ncfg_thread(void);
extern void    stop_ncfg_thread(void);

extern void    devid_conf(void);

////////////////////////////////////////////////////
extern int     is_master(void);
extern int     is_standalone(void);

extern void    update_master_label(int m);
/* called when master is discovered. */
extern void    update_masterip(char *ip);

extern void    fi_clear(int i);
extern void    fi_save(void);
extern void    fi_update(int i);

extern void    grp_start(void);
extern void    grp_end(void);
extern int     probe_masterip(void *loop, int fd);

extern void    start_file_server(void);
extern void    start_file_client(void);
extern void    stop_ft_loop(void);
extern void    start_syncfg(unsigned int v);
extern void    cancel_syncfg(void);

/* called by UI thread. */
extern void    disable_syncfg(void);

/* called by master net_tcp thread. */
extern void    enable_syncfg(void);
extern int     cfg_crc_diff(int i, unsigned int  crc);
extern int     make_crc_buf(char *buf, int len);
extern void    set_usrptr(char *p);
extern void    update_username(char *buf);

///////////////////////////////////////////////////
//           slave routine           //
///////////////////////////////////////////////////
extern void    reload_ifconf(void);
extern void    reload_zonedb(void);
extern void    reload_usrdb(void);
extern void    reload_gendb(void);
extern void    reload_pztmr(void);
extern void    reload_mapdb(void);

extern void    cfg_map_clr(void);
extern void    cfg_map_set(int i);
extern int     reboot(int cmd);

extern void    set_slave_ui(int id, char *ip);
extern void    clr_slave_ui(int id);

extern void    update_hostip(int id, unsigned int hostp);
extern void    hashtbl_clear(void);
extern void    hashtbl_save(int id, unsigned int m);
extern int     save_hip(int id, char *ip);
extern void    clear_sysfst(void);

extern void    host_load(void);
extern void    host_unload(void);

static inline unsigned int
get_net_hostp(char *ip)
{
	struct in_addr  ia;
	inet_aton(ip, &ia);
	return ntohl(ia.s_addr) & hmask;
}

static inline void
get_tm(struct tm *tm, time_t *t)
{
	time_t now;

	time(&now);
	if (t) {
		*t = now;
	}
	if (tm) {
		localtime_r(&now, tm);
	}
}

/* znr: 0-511 */
static inline int
get_pzone_by_znr(int znr)
{
	return zonedb[znr].pnr;
}

static inline int
zone_valid(int z)
{
	return (z >= 0&&z < MAXZ&&zonedb[z].znr == z+1);
}

static inline void
flush_pipe(int *pfd)
{
	u_flush_fd(pfd[0]);
	u_flush_fd(pfd[1]);
}

#endif



