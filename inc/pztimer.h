#ifndef __PZTIMER_H__
#define __PZTIMER_H__

extern int    tpzarr[MAXTPZ];
extern int    tpzcnt;

extern unsigned int    tmrpzmap;       //timer arm flag, set when arming and clr when disarming.
extern unsigned int    pztmrflg;       //pzone timer disabled or enabled.need to be saved

#define set_timer_pzflg(p)     pztmrflg |= (1<<(p))
#define clr_timer_pzflg(p)     pztmrflg &= ~(1<<(p))
#define pztimer_enabled(p)     (pztmrflg & (1<<(p)))

extern void  init_tmrpz_node(void);
extern void  stop_pz_timer(void);
extern int   pztimer_init(unsigned int var);
extern int   enable_pztimer(unsigned int var);
extern int   disable_pztimer(unsigned int var);
extern void  update_tmrpz_ui(int p, int st);

extern void  set_tpz_flag(int i, int st);

extern void  tmrui_set_content(void *pzt, int ud);
extern void  tmrui_clr_content(int ud);

extern void  tmer_opt(int op, int i);



#endif


