#ifndef _ZNODE_H_
#define _ZNODE_H_

extern void   zhead_init(void);
extern void   nready_put(int z);
extern void   nready_del(int z);
extern void   nready_clear(void);
extern int    nready_get(int *last);
extern int    nready_yet(void);

extern void   alarmst_put(int z, int a);
extern void   alarmst_del(int z);
extern void   alarmst_clear(void);
extern int    alarmst_get(int *last);
extern void   alarmst_clearpz(int p);

extern void   znode_remove(int z);


#endif


