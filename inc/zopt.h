#ifndef __ZOPT_H__
#define __ZOPT_H__

#include <time.h>

#define ZST_UNKNOWN     0
#define ZST_DISARM      1
#define ZST_NRDY        2
#define ZST_BYPASS      3
#define ZST_ARM         4
#define ZST_ALARM       5
#define ZST_ALARMR      6
#define ZST_MAX         7

#define PST_UNKNOWN     0
#define PST_DISARM      1
#define PST_PTBPASS     2
#define PST_BYPASS      3
#define PST_NRDY        4
#define PST_PTARM       5
#define PST_ARM         6
#define PST_FRCARM      7
#define PST_TMRARM      8
#define PST_ALARM       9
#define PST_ALARMR      10
#define PST_MAX         11

#define PZ_SYSF         1
#define PZ_NRDY         2
#define PZ_ALRT         3
#define PZ_ARMED        4
#define PZ_BPASSED      5
#define PZ_TMRED        6
#define PZ_NARM         7
#define PZ_24HUR        8

#define ALLARMED        15

#define PZ_MAXERR       PZ_24HUR

#define Z_UNKNOWN       1
#define Z_TMRED         2
#define Z_STERR         3
#define Z_24HUR         4

#define SYST_INVALID_TM    600

/////////////////////////////////////////
#define ODAM         0
#define OARM         1
#define OZBP         2
#define OZBPR        3
#define OPZAM        4
#define OPZFA        5
#define OPZDA        6
#define OPZBP        7
#define OPZBPR       8

////////////////////////////
//timer zone disable
#define OTMRDN       9
//timer zone enable
#define OTMREN       10

#define OPZTMR       11

#define OZALT        12
#define OZALTR       13
#define OEVT         14
/////////////////////////////

#define OZST         15
#define OPZST        16

#define OSYST        17

#define MAXOPT       OSYST
#define OSVRST       OTMREN

#define OCLRALR      19

#define DEVID_SFT    24
#define ARG_MASK     0xffff

#define CfgChgCmd     1
#define AllOptCmd     2
#define PzOptCmd      3
#define ZoneOpCmd     4
#define TmerOpCmd     5
#define AlrmClrCmd    6
#define EvtNtfCmd     7
#define AckOfCmd      8
#define BusyCmd       9

/* for sync.c */
#define ReqCfgCmd     15

#define ZOP_DA        0
#define ZOP_AM        1
#define ZOP_BPR       2
#define ZOP_BP        3
#define ZOP_ALM       4
#define ZOP_ALR       5
#define ZOP_FLT       6
#define ZOP_FLR       7

#define test_bit(v, n)   (((v) >> (n)) & 1)
#define set_bit(v, n)    (v) |= (1<<(n))
#define clr_bit(v, n)    (v) &= ~(1<<(n))

extern unsigned int   tmslice[4];
extern unsigned short  _syst;

extern void  zopt_start(void);
extern void  zopt_exit(void);
extern void  pz_timer_add(unsigned int t, void *cb);
extern int   pz_timer_remove(void);

extern void  ztimer_start(void *w);
extern void  ztimer_stop(void *w);


/* called in UI thread. */
extern void   zone_arm(int z);     //z: 0~511
extern void   zone_disarm(int z);  //z: 0~511
extern void   zone_bypass(int z);  //z: 0~511
extern void   zone_bypassr(int z);
extern void   pzone_arm(int p);
extern void   pzone_farm(int p);
extern void   pzone_disarm(int p);
extern void   pzone_bypass(int p);
extern void   pzone_bypassr(int p);

/* called in UART thread. */
extern void   zone_alert(int z);  //z: 0~511
extern void   zone_alertr(int z); //z: 0~511

extern void   simul_alert(int z);
extern void   simul_alertr(int z);

extern void  send_event(int ev, int id);
extern void  update_syst(int id, int st);

extern void   stop_pzone_timer(void);
extern void   start_pzone_timer(void);
extern void   enable_pzone_timer(int i);
extern void   disable_pzone_timer(int i);

extern void  update_zoneui_state(int zi, int st);
extern int   get_zone_state(int z);
extern int   get_zone_state_direct(int z);


extern int   get_pzone_state(int p);//p:[0, MAXPZ]
extern int   get_pzone_state_direct(int p);//p:[0, MAXPZ]

extern unsigned char  my_cksum(unsigned char *buf, int len);

extern unsigned int  get_daytm(struct tm *ntm);
extern int   pzopt_zones(int p, int (*cb)(int, int));

extern int   get_syst(void);

extern void  clear_rs485_flag(void);

extern void  init_pztimer(int i);

extern void  init_pzone_info(void);

extern void    send_zcmd(void *buf, int n);
extern int     recv_cmd_ack(int fd, int ms);

extern void    send_evt_slave(int eid, int hid);


////////////////////////////////////////////////////
extern void  relay_init(void);
extern void  relay_free(void);
extern void  relay_on(void);
extern void  relay_off(void);

////////////////////////////////////////////////////

extern void  clear_pzstat(void);

/*
 * called by zopt thread.
 * op: 0-all disarm, 1-all arm, 2-forced all arm;
 * res: 0-success, nonzero-failed.
 */
extern void  allopr_return(int op, int res);

/*
 * op: 0-pz_arm, 1-pz_farm, 2-pz_disarm, 3_pz_bypass, 4-pz_bypassr.
 * r:  0-success,1-errno.
 */
extern void  pzopr_return(int op, int p, int r);

/*
 * op: 0-zarm, 1-zdisarm, 2-zbypass, 3-zbypassr.
 * r:  0-success,1-errno.
 */
extern void  zopr_return(int op, int z, int r);

extern void  zdisarm_ack(int arg, int r);
extern void  zarm_ack(int arg, int r);
extern void  zbp_ack(int arg, int r);
extern void  zbpr_ack(int arg, int r);
extern void  pzarm_ack(int arg, int r);
extern void  pzfarm_ack(int arg, int r);
extern void  pzdisarm_ack(int arg, int r);	
extern void  pzbp_ack(int arg, int r);
extern void  pzbpr_ack(int arg, int r);

extern int   cannot_zdisarm(int arg);
extern int   cannot_zarm(int arg);
extern int   cannot_zbpass(int arg);
extern int   cannot_zbpassr(int arg);
extern int   try_pzarm(int arg);
extern int   try_fpzarm(int arg);
extern int   try_pzdisarm(int arg);	
extern int   cannot_pzbp(int arg);
extern int   cannot_pzbpr(int arg);
extern void  enable_scrsvr(void);
extern void  disable_scrsvr(void);
extern void  clear_alarm(int id);

///////////////////////////////////////////////////////////////

extern void  pzone_opr(int p, int op, int t);
extern void  zone_opr(int op, int z, int zst, int p, int pst);
extern void  handle_allopt(int op, int d);

extern void  calc_zonef(void);
extern void  calc_mapst(void);


#endif


