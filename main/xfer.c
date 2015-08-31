#include <pthread.h>
#include "ulib.h"
#include "config.h"
#include "zopt.h"
#include "pzone.h"

static struct list_hdr   free_plist = {&free_plist, &free_plist};
static struct list_hdr	 plist_head = {&plist_head, &plist_head};
static pthread_mutex_t   xbmtx = PTHREAD_MUTEX_INITIALIZER;

#define xbuf_lock()      pthread_mutex_lock(&xbmtx)
#define xbuf_unlock()    pthread_mutex_unlock(&xbmtx)

static struct list_hdr   free_xlist = {&free_xlist, &free_xlist};
static struct list_hdr	 xlist_head = {&xlist_head, &xlist_head};
static pthread_mutex_t   xrmtx = PTHREAD_MUTEX_INITIALIZER;

#define xres_lock()      pthread_mutex_lock(&xrmtx)
#define xres_unlock()    pthread_mutex_unlock(&xrmtx)

static xbuf_t *
xfer_buf_new(void)
{
	xbuf_t *x;
	struct list_hdr *list;

	xbuf_lock();
	if (u_list_empty(&free_plist)) {
		x = calloc(sizeof(xbuf_t), 1);
		if (x) {
			U_INIT_LIST_HEAD(&x->list);
		}
	} else {
		list = free_plist.next;
		u_list_del(list);
		x = (xbuf_t*)list;
		memset(x->buf, 0, 4096);
		x->n = 0;
	}
	xbuf_unlock();
	return x;
}

void
xfer_buf_free(xbuf_t *x)
{
	xbuf_lock();
	u_list_append(&x->list, &free_plist);
	xbuf_unlock();
}

xbuf_t *
xfer_buf_get(void)
{
	xbuf_t *x = NULL;
	struct list_hdr *list;

	xbuf_lock();
	list = plist_head.next;
	if (list != &plist_head) {
		u_list_del(list);
		x = (xbuf_t*)list;
	}
	xbuf_unlock();

	return x;
}

static void
xfer_buf_put(xbuf_t *x)
{
	xbuf_lock();
	u_list_append(&x->list, &plist_head);
	xbuf_unlock();
}

/* #9|devid|id\r\n */
void
netxfer_alarmclr(int id)
{
	xbuf_t *x;

	if (!is_standalone()&&xfer_alarm()&&xfer_bynet()) {
		x = xfer_buf_new();
		if (x != NULL) {
			x->n = sprintf(x->buf, "#%d|%d|%d\r\n", AlrmClrCmd, g_devid, id);
			xfer_buf_put(x);
			xfer_netcmd();
		}
	}
}

void
netxfer_pztmr(int i, int o)
{
	xbuf_t  *x;

	if (!is_standalone()&&xfer_alarm()&&xfer_bynet()) {
		x = xfer_buf_new();
		if (x != NULL) {
			x->n = sprintf(x->buf, "#%d|%d|%d|%d|%d\r\n", TmerOpCmd, g_devid, g_uid, o, i);
			xfer_buf_put(x);
			xfer_netcmd();
		}
	}
}

/* 
 * #4|devid|usrid|op|t|pznr:pzst|z0:zst0|z1:zst1бн..\r\n
 * o: 0-disarm,1-arm,2-bypass restore,3-bypass
*/
void
netxfer_pzopt(int p, int o, int t)
{
	xbuf_t  *x;

	if (!is_standalone()&&xfer_alarm()&&xfer_bynet()) {
		x = xfer_buf_new();
		if (x != NULL) {
			x->n = sprintf(x->buf, "#%d|%d|%d|%d|%d|%d:%d", PzOptCmd, g_devid, g_uid, o, t, p, pstat[p]);
			pzopt_zones_st(p, x);
			x->n += sprintf(x->buf+x->n, "\r\n");
			xfer_buf_put(x);
			xfer_netcmd();
		}
	}
}

/* #6|devid|usrid|op|znr:zst|pzn:pzst\r\n */
/* o: 0-disarm,1-arm,2-bypassr,3-bypass */
/* 4-alarm,5-alarm restore,6-fault,7-fault restore. */
void
netxfer_zopt(int z, int o)
{
	xbuf_t  *x;
	int      p;

	p = pnrofz(z);
	if (!is_standalone()&&xfer_alarm()&&xfer_bynet()) {
		x = xfer_buf_new();
		if (x != NULL) {
			x->n = sprintf(x->buf, "#%d|%d|%d|%d|%d:%d|%d:%d\r\n", ZoneOpCmd, g_devid, g_uid, o, z, zstat[zoneidx(z)], p, pstat[p]);
			xfer_buf_put(x);
			xfer_netcmd();
		}
	}
}

/* #8|devid|eid|hid\r\n */
void
netxfer_event(int eid, int hid)
{
	xbuf_t  *x;

	if (!is_standalone()&&xfer_event()&&xfer_bynet()) {
		x = xfer_buf_new();
		if (x != NULL) {
			x->n = sprintf(x->buf, "#%d|%d|%d|%d\r\n", EvtNtfCmd, g_devid, eid, hid);
			xfer_buf_put(x);
			xfer_netcmd();
		}
	}
}

/* 
  #3|devid|usrid|op|type\r\n
  type: 0-normal; 1-forced.
*/
/* o: 0-disarm,1-arm */
void
netxfer_allarm(int o, int t, int id)
{
	xbuf_t *x;

	if (!is_standalone()&&xfer_alarm()&&xfer_bynet()) {
		x = xfer_buf_new();
		if (x != NULL) {
			x->n = sprintf(x->buf, "#%d|%02d%02d|%d|%d|%d\r\n", AllOptCmd, id, g_devid, g_uid, o, t);
			xfer_buf_put(x);
			xfer_netcmd();
		}
	}
}

////////////////////////////////////////////////////////////

static xres_t *
xr_buf_new(void)
{
	xres_t *x;
	struct list_hdr *list;

	xres_lock();
	if (u_list_empty(&free_xlist)) {
		x = calloc(sizeof(xres_t), 1);
		if (x) {
			U_INIT_LIST_HEAD(&x->list);
		}
	} else {
		list = free_xlist.next;
		u_list_del(list);
		x = (xres_t*)list;
		memset(x->buf, 0, 32);
		x->n = 0;
		x->id = 0;
	}
	xres_unlock();
	return x;
}

void
xr_buf_free(xres_t *x)
{
	xres_lock();
	u_list_append(&x->list, &free_xlist);
	xres_unlock();
}

xres_t *
xr_buf_get(void)
{
	xres_t *x = NULL;
	struct list_hdr *list;

	xres_lock();
	list = xlist_head.next;
	if (list != &xlist_head) {
		u_list_del(list);
		x = (xres_t*)list;
	}
	xres_unlock();

	return x;
}

static void
xr_buf_put(xres_t *x)
{
	xres_lock();
	u_list_append(&x->list, &xlist_head);
	xres_unlock();
}

/* #9|devid|result|cmd|arg\r\n */
void
netxfer_result(int cmd, int arg, int r, int id)
{
	xres_t  *x;

	if (!is_standalone()&&xfer_alarm()&&xfer_bynet()) {
		x = xr_buf_new();
		if (x != NULL) {
			x->n = sprintf(x->buf, "#%d|%d|%d|%d|%d\r\n", AckOfCmd, g_devid, r, cmd, arg);
			x->id = id;
			xr_buf_put(x);
			xfer_result();
		}
	}
}


