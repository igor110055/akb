#ifndef _STB_H_
#define _STB_H_

#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

void*    stb_new(void *parent, void *f);
void     stb_add_update_cb(void *arg, void *ud, void *load, int itvl);
int      stb_start_clock(void *arg);
int      stb_stop_clock(void *arg);
void     stb_update_clock(void *arg);
void     get_timestr(time_t *t, char *buf, int len);

void     start_netif(void);
void     stop_netif(void);

void     stb_set_user(char *s);
/* id: [1, 4] */
void     stb_update_slave(int id, int st);
void     stb_display_slave(int disp);


#ifdef __cplusplus
}
#endif

#endif

