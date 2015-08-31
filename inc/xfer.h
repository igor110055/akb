#ifndef __XFER_H__
#define __XFER_H__

#include "ulib.h"

#define NCMDSFT     24

typedef struct _xbuf {
	struct list_hdr  list;
	char   buf[4096];
	int    n;
} xbuf_t;

typedef struct _xres {
	struct list_hdr  list;
	char   buf[32];
	int    n;
	int    id;
} xres_t;

extern xbuf_t *xfer_buf_get(void);
extern void    xfer_buf_free(xbuf_t *x);
extern void    netxfer_alarmclr(int id);
extern void    netxfer_pztmr(int i, int o);
extern void    netxfer_pzopt(int p, int o, int t);
extern void    netxfer_zopt(int z, int o);
extern void    netxfer_event(int eid, int hid);
extern void    netxfer_allarm(int o, int t, int id);
extern void    netxfer_alarmclr_slave(int id);

extern xres_t *xr_buf_get(void);
extern void    xr_buf_free(xres_t *x);
extern void    netxfer_result(int cmd, int arg, int r, int id);

extern void    start_master(void);
extern void    end_master(void);

extern void    start_slave(void);
extern void    end_slave(void);
extern void    do_xfer_nack(int cid, unsigned short arg);

extern void    finish_syncfg(void);

extern void    set_busy(void);
extern void    xfer_result(void);
extern void    xfer_netcmd(void);
extern void    xfer_cfgcmd(void);


#endif


