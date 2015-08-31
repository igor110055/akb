#ifndef _PZONE_H_
#define _PZONE_H_

#include "config.h"
#include "xfer.h"

extern FVINT st_out[7];
extern FVINT st_in[7];

extern unsigned int    frcpzmap;
extern unsigned int    bparmflag;
#define bparmflag_set(p)      bparmflag |= (1<<(p))
#define bparmflag_clr(p)      bparmflag &= ~(1<<(p))
#define bparmflag_isset(p)    (bparmflag & (1<<(p)))

unsigned short  pzaltmap;
#define pzaltmap_set(p)    pzaltmap |= (1 << (p))
#define pzaltmap_clr(p)    pzaltmap &= ~(1 << (p))

extern void  zstate_init(int sz);
extern int  *pzinfo_get(int p, int *n);
extern void  pzinfo_init(void);
extern void  init_pzinfo(void);
extern void  store_pzone(int p, char *name);
extern void  store_pzone_file(void);
extern int   load_pzone(void);
extern void  reload_pzonedb(void);
extern void  update_pzone_state(int p);

extern int   pzopt_zones(int p, int (*cb)(int, int));
extern void  pzopt_zones_st(int p, xbuf_t *x);

extern void  syst_load(void);
extern void  syst_save(void);
extern int   calc_allflag(void);

extern int   cannot_tmrarm(unsigned short p);

extern int   disarmz(int p, int z);
extern int   farmz(int p, int z);

extern void  pzarm_ack(int arg, int r);
extern int   pzarm_opt(unsigned int var);
extern void  pzfarm_ack(int arg, int r);
extern int   pzarm_fopt(unsigned int var);
extern int   pzdisarm_opt(unsigned int var);
extern void  pzdisarm_ack(int arg, int r);
extern int   pzbp_opt(unsigned int var);
extern void  pzbp_ack(int arg, int r);
extern int   pzbpr_opt(unsigned int var);
extern void  pzbpr_ack(int arg, int r);

extern int   zdisarm_opt(unsigned int var);
extern void  zdisarm_ack(int arg, int r);
extern int   zarm_opt(unsigned int var);
extern void  zarm_ack(int arg, int r);
extern int   zbp_opt(unsigned int var);
extern void  zbp_ack(int arg, int r);
extern int   zbpr_opt(unsigned int  var);
extern void  zbpr_ack(int arg, int r);
extern int   za_opt(unsigned int var);
extern int   zar_opt(unsigned int var);
extern int   syst_return(unsigned int p);

#endif


