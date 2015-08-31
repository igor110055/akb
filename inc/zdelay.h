#ifndef __ZDELAY_H__
#define __ZDELAY_H__

#include "config.h"

#define  odelay      genchr.otm
#define  idelay      genchr.itm[0]

extern unsigned int  zodelay[ZBNUM];        //1bit represents a zone.0: cannot alarm, 1: can alarm. used for exit delay.
extern unsigned int  zidelay[ZBNUM];        //alert flag: 1bit represents a zone.0: no alert, 1: alert. used for enter delay.
extern unsigned int  zarflag[ZBNUM];        //alarm restore flag in entering delay mode.
extern unsigned int  zalert[ZBNUM];

#define  zodelay_isset(z)  (zodelay[(z)>>5] & (1<<((z) & 31)))
#define  zodelay_set(z)    zodelay[(z)>>5] |= (1<<((z) & 31))
#define  zodelay_clr(z)    zodelay[(z)>>5] &= ~(1<<((z) & 31))

#define  zidelay_isset(z)  (zidelay[(z)>>5] & (1<<((z) & 31)))
#define  zidelay_set(z)    zidelay[(z)>>5] |= (1<<((z) & 31))
#define  zidelay_clr(z)    zidelay[(z)>>5] &= ~(1<<((z) & 31))

#define  zarflag_isset(z)  (zarflag[(z)>>5] & (1<<((z) & 31)))
#define  zarflag_set(z)    zarflag[(z)>>5] |= (1<<((z) & 31))
#define  zarflag_clr(z)    zarflag[(z)>>5] &= ~(1<<((z) & 31))

#define  zalert_isset(z)  (zalert[(z)>>5] & (1<<((z) & 31)))
#define  zalert_set(z)    zalert[(z)>>5] |= (1<<((z) & 31))
#define  zalert_clr(z)    zalert[(z)>>5] &= ~(1<<((z) & 31))

extern void   start_zodelay(int z);
extern void   start_zidelay(int z);
extern void   finish_zidelay(int z);
extern void   z_timer_cleanup(void);

#endif



