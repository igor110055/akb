#ifndef _QTIMER_H_
#define _QTIMER_H_

#ifdef __cplusplus
extern "C" {
#endif

#define QTMEOUT    30.0

extern void   init_qtimer(void);
extern void   register_qtimer(void *w);

#ifdef __cplusplus
}
#endif

#endif


