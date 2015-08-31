#include <time.h>
#include <string.h>
#include "ulib.h"
#include "unotifier.h"
#include "utksignal.h"
#include "utkwindow.h"
#include "utkmap.h"
#include "utkbutton.h"
#include "utklabel.h"
#include "utktimeout.h"
#include "layout.h"
#include "gui.h"
#include "stb.h"
#include "config.h"
#include "zopt.h"

typedef struct _dst {
	UtkWindow    *lbl_net;
	UtkWindow    *slv[MAXSLV];
	UtkWindow    *clk;
	UtkWindow    *hlbl;
	UtkWindow    *usr;
	unsigned int  timerid;
	unsigned int  itvl;
	struct  tm    tm;
	char          tmstr[32];  //yyyy-mm-dd hh:mm
	void         (*f)(void);
	void         (*update)(void*);
	void         (*load)(void*);
	struct u_notifier_node  ntf;
	struct u_notifier_node  ntf1;
} dst_t;

static dst_t *g_dst = NULL;
static struct u_notifier_head  ntf_head;
static struct u_notifier_head  ntf_head1;
static int    ntf_head_init = 0;

static char   uname[34] = {0};

static char  *wday[] = {
	"星期天",
	"星期一",
	"星期二",
	"星期三",
	"星期四",
	"星期五",
	"星期六",
};

static void
stb_get_tm(dst_t *d)
{
	get_tm(&d->tm, NULL);
	memset(d->tmstr, 0, 32);
	snprintf(d->tmstr, 32, "%d-%02d-%02d %02d:%02d:%02d %s", d->tm.tm_year+1900, d->tm.tm_mon+1,
		     d->tm.tm_mday, d->tm.tm_hour, d->tm.tm_min, d->tm.tm_sec, wday[d->tm.tm_wday]);
}

static int
update_net_label(struct u_notifier_node* node, unsigned long v, void *data)
{
	dst_t *d = node->data;

//	v = ((v != 0) ? 1 : 0);
	utk_widget_update_state(d->lbl_net, !!v);

	return 0;
}

static void
hide_slave(void *arg)
{
	dst_t *d = arg;
	int    i;

	for (i = 0; i < MAXSLV; ++i) {
		utk_widget_update_state(d->slv[i], 0);
		utk_widget_hide(d->slv[i]);
	}
}

static void
show_slave(void *arg)
{
	dst_t *d = arg;
	int    i;

	for (i = 0; i < MAXSLV; ++i) {
		utk_widget_show(d->slv[i]);
	}
}

static void   (*disp_slv[])(void*) = {hide_slave, show_slave};

/* v: 0-offline,1-online,2-hide,3-show. */
static int
update_slave_label(struct u_notifier_node* node, unsigned long v, void *data)
{
	dst_t *d = node->data;
	int    id = (int)data;

	DBG("---- v = %lu ---\n", v);
	if (id <= MAXSLV&&id > 0) {
		utk_widget_update_state(d->slv[id-1], !!v);
	} else if (id == 0) {
		(*disp_slv[!!v])(node->data);
	}
	return 0;
}

static void
pressed(void *data)
{
	dst_t  *d = (dst_t*)data;
	utk_widget_update_state(d->hlbl, 1);
}

static void
return_home(void *data)
{
	dst_t  *d = (dst_t*)data;
	utk_widget_update_state(d->hlbl, 0);
	d->f();
}

static void
prepare_show_stbar(void *d)
{
	g_dst = d;
	stb_get_tm(d);
	if (g_dst->load) {
		g_dst->load(&g_dst->tm);
	}
}

void*
stb_new(void *parent, void *f)
{
	int         i;
	dst_t      *d;
	rect_t      r = {.x = clock_x, .y = clock_y, .w = 208, .h = 16};
	rect_t      r0 = {.x = 1024-64, .y = 0, .w = 64, .h = 39};
	char       *bg0[] = {SLV1_0, SLV2_0, SLV3_0, SLV4_0, SLV5_0, SLV6_0, SLV7_0, SLV8_0, SLV9_0};
	char       *bg1[] = {SLV1_1, SLV2_1, SLV3_1, SLV4_1, SLV5_1, SLV6_1, SLV7_1, SLV8_1, SLV9_1};

	if (ntf_head_init == 0) {
		u_notifier_head_init(&ntf_head);
		u_notifier_head_init(&ntf_head1);
		ntf_head_init = 1;
	}

	d = calloc(1, sizeof(dst_t));
	if (!d) return NULL;

	d->clk = label_new(parent, NULL, d->tmstr, RGB_WHITE, &r);
	d->itvl = 100;  //default: 10s.
	d->f = f;
	d->load = NULL;
	d->update = NULL;

	r.x = clock_x+208;
	r.w = sizeof(uname)*8;
	d->usr = label_new(parent, NULL, uname, RGB_WHITE, &r);

	for (i = 0; i < MAXSLV; ++i) {
		d->slv[i]= utk_label_new();
		utk_label_set_bg(d->slv[i], bg0[i]);
		utk_label_set_bg(d->slv[i], bg1[i]);
		utk_widget_add_cntl(parent, d->slv[i], slv_x+i*slv_itvl, slv_y);
		if (!is_master()) {
			utk_widget_disable(d->slv[i]);
		}
	}

	d->lbl_net = utk_label_new();
	utk_label_set_bg(d->lbl_net, LBL_NETUNCONN);
	utk_label_set_bg(d->lbl_net, LBL_NETCONN);
	utk_widget_add_cntl(parent, d->lbl_net, st_net_x, st_net_y);

	if (f) {
		d->hlbl = utk_label_new();
		utk_label_set_bg(d->hlbl, HOME0);
		utk_label_set_bg(d->hlbl, HOME1);
		utk_widget_add_cntl((UtkWindow*)parent, d->hlbl, 1024-64, 6);

		map_new(parent, &r0, pressed, return_home, (void*)d);
	}

	u_notifier_node_init(&d->ntf, update_net_label, 0, d);
	u_notifier_chain_register(&ntf_head, &d->ntf);

	u_notifier_node_init(&d->ntf1, update_slave_label, 0, d);
	u_notifier_chain_register(&ntf_head1, &d->ntf1);
	
	d->timerid = utk_widget_timer(parent, stb_update_clock, (void*)d);
	utk_widget_show_before(parent, prepare_show_stbar, (void*)d);
	utk_widget_show_after(parent, stb_start_clock, (void*)d);
	utk_widget_hide_before(parent, stb_stop_clock, (void*)d);

	return d;
}

/* id: [1, 4] */
void
stb_update_slave(int id, int st)
{
	u_notifier_call_chain(&ntf_head1, st, (void*)id);
}

void
stb_display_slave(int st)
{
	u_notifier_call_chain(&ntf_head1, st, (void*)0);
}

void
stb_update_clock(void *arg)
{
	dst_t *d = (dst_t*)arg;
	stb_get_tm(d);
	utk_label_update(d->clk);
	if (d->update) {
		d->update(&d->tm);
	}
}

void
stb_add_update_cb(void *arg, void *ud, void *l, int itvl)
{
	dst_t  *d = arg;
	d->update = ud;
	d->load = l;
	if (itvl != 0)
		d->itvl = itvl;
}

int
stb_start_clock(void *arg)
{
	dst_t *d = arg;
	utk_timeout_add(d->itvl, d->timerid);
	return 0;
}

int
stb_stop_clock(void *arg)
{
	dst_t *d = arg;
	utk_timeout_remove(d->timerid);
	return 0;
}

void
get_timestr(time_t *t, char *buf, int len)
{
	struct tm  tm;
	get_tm(&tm, t);
	memset(buf, 0, len);
	snprintf(buf, len, "%d-%02d-%02d %02d:%02d:%02d", tm.tm_year+1900, tm.tm_mon+1,
		     tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
}

///////////////////////////////////////////////////////
static int  nquit = 0;
static int  nret = -1;
static pthread_t nthr_tid = (pthread_t)0;

static void*
netif_loop(void *arg)
{
	int  st = 0;
	int  ost = 0;
	int  err[] = {EV_NETE, EV_NETR};

	st = u_if_is_up("eth0");
	u_notifier_call_chain(&ntf_head, st, NULL);
	send_event(err[st], 0);
	ost = st;
	nquit = 0;
	DBG("--- enter netif loop ---\n");
	while (!nquit)
	{
		st = u_if_is_up("eth0");
		if (st >= 0&&st != ost) {
			u_notifier_call_chain(&ntf_head, st, NULL);
			ost = st;
			send_event(err[st], 0);
		}

		u_sleep(1);
	}
	if (st == 0) {
		send_event(EV_NETR, 0);
	}
	pthread_exit(NULL);
}

/* called after all gui builded. */
void
start_netif(void)
{
	if (nret == -1) {
		nret = pthread_create(&nthr_tid, NULL, netif_loop, NULL);
	}
}

void
stop_netif(void)
{
	if (nret == 0) {
		nquit = 1;
		pthread_join(nthr_tid, NULL);
		nthr_tid = 0;
		nret = -1;
		u_notifier_call_chain(&ntf_head, 0, NULL);
	}
}

void
stb_set_user(char *s)
{
	memset(uname, 0, sizeof(uname));
	snprintf(uname, sizeof(uname), "当前用户: %s", s);
}


