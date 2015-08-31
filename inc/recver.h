#ifndef __RECVER_H__
#define __RECVER_H__

extern void   recver_init(void);
extern void   recver_uninit(void);
extern void   recver_prepare(void);
extern void   recver_finish(void);
extern void   recver_start(void);
extern void   recver_exit(void);

extern void   enable_netdev(int en);

#endif


