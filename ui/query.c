#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "query.h"
#include "config.h"
#include "mxc_sdk.h"
#include "gui.h"
#include "layout.h"

#define evlbl_w     160
#define evlbl_h     16
#define evlbl_x     (190+(1024-678)/2)
#define evlbl_y     (238+(600-370)/2+(52-16)/2)

typedef struct _qry_inp {
	time_t   stm;
	time_t   etm;
	int      idx;  //zone id or event id.
	int      type;
	qry_ui_t *qu;
} qry_inp_t;

typedef void  (*VQRY)(qry_inp_t *);

int    qresult[MAXDB] = {0};
int    prtarr[MAXPRT] = {0};

static pthread_mutex_t    qmtx = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t     qcnd = PTHREAD_COND_INITIALIZER;

#define  query_lock()     pthread_mutex_lock(&qmtx)
#define  query_trylock()  pthread_mutex_trylock(&qmtx)
#define  query_unlock()   pthread_mutex_unlock(&qmtx)

#define  query_p()        pthread_cond_wait(&qcnd, &qmtx)
#define  query_v()        pthread_cond_signal(&qcnd)

static qry_inp_t    g_query;
static qry_inp_t   *g_qry = NULL;

static int      qid = -1;   //zone id or event id.

static char    *qrying = "正在查询,请稍候!";
static int      qrylen;

static char    *nores = "没有找到符合条件的事件!";
static int      noreslen;

static char    *ierr = "请输入查询条件!";
static int      ierrlen = 0;

static char     yr[2][6] = {{0},};
static char     mon[2][4] = {{0},};
static char     day[2][4] = {{0},};
static char     hur[2][4] = {{0},};
static char     min[2][4] = {{0},};
static char     sec[2][4] = {{0},};
static char    *pstr[2][6];

static char     zinp[6] = {0};
static UtkWindow *evlbl;

static int   earr[] = {EV_NETR, EV_RS485R, EV_APR, EV_DPR, EV_BLR, EV_HBUSR, EV_HPENT, EV_HCONN};

static char *infname[] = {EINFDB, AINFDB};
static unsigned short  elesz[] = {sizeof(einf_t), sizeof(zainf_t)};

/* DONE */
void
load_info_file(qry_ui_t *q)
{
	FILE  *fp;
	int    cnt;

	if ((fp = fopen(infname[q->type], "r")) == NULL) return;
	if (fread(&cnt, sizeof(cnt), 1, fp) != 1) {
		fclose(fp);
		return;
	}

	cnt = (cnt > MAXE) ? MAXE : cnt;
	if (fread(q->data, elesz[q->type], cnt, fp) == cnt) {
		q->icnt = cnt;
	}
	DBG("--- LOAD %s INFO: %d ---\n", (q->type?"ALARM":"EVENT"), cnt);
	fclose(fp);
}

/* DONE */
void
store_info_file(qry_ui_t *q)
{
	FILE  *fp;

	if ((fp = fopen(infname[q->type], "w")) != NULL) {
		ignore_result(fwrite(&q->icnt, sizeof(q->icnt), 1, fp));
		ignore_result(fwrite(q->data, elesz[q->type], q->icnt, fp));
		fflush(fp);
		fclose(fp);
	}
}

/* DONE */
static int
_search_einfo_raw(qry_inp_t *q)
{
	int        i;
	int        rcnt = 0;
	qry_ui_t  *qu = q->qu;
	einf_t    *ei = (einf_t*)qu->data;
	int        j = qu->icnt;

	DBG("--- query: event info ---\n");
	if (q->stm > 0) {
		if (q->etm == 0) {
			time(&q->etm);
		}

		for (i = 0; i < j; ++i) {
			if (ei[i].etm >= q->stm)
				break;
		}
		if (i == j) return 0;
		for (; j > i; --j) {
			if (ei[j-1].etm <= q->etm)
				break;
		}
		if (j == i) return 0;
	}

	if (q->idx >= 0&&q->idx < 4) {
		for (; i < j; ++i) {
			if (ei[i].eid == q->idx) {
				qu->qres[rcnt] = i;
				++rcnt;
			}
		}
	} else if (q->idx >= 4) {
		for (; i < j; ++i) {
			if (ei[i].eid == earr[q->idx-4]||ei[i].eid == earr[q->idx-4]+1) {
				qu->qres[rcnt] = i;
				++rcnt;
			}
		}
	} else {
		for (; i < j; ++i) {
			qu->qres[rcnt] = i;
			++rcnt;
		}
	}

	return rcnt;
}

/* DONE */
static int
_search_einfo_nested(qry_inp_t *q)
{
	int        i;
	int        rcnt = 0;
	qry_ui_t  *qu = q->qu;
	einf_t    *ei = (einf_t*)qu->data;
	int        j  = qu->qcnt;
	int       *htbl = qu->qres;

	DBG("--- nested query: event info ---\n");
	if (q->stm > 0) {
		if (q->etm == 0) {
			time(&q->etm);
		}

		for (i = 0; i < j; ++i) {
			if (ei[htbl[i]].etm >= q->stm)
				break;
		}
		if (i == j) return 0;

		for (; j > i; --j) {
			if (ei[htbl[j-1]].etm <= q->etm)
				break;
		}
		if (j == i) return 0;
	}

	if (q->idx >= 0&&q->idx < 4) {
		for (; i < j; ++i) {
			if (ei[htbl[i]].eid == q->idx) {
				htbl[rcnt] = htbl[i];
				++rcnt;
			}
		}
	} else if (q->idx >= 4) {
		for (; i < j; ++i) {
			if (ei[htbl[i]].eid == earr[q->idx-4]||ei[htbl[i]].eid == earr[q->idx-4]+1) {
				htbl[rcnt] = htbl[i];
				++rcnt;
			}
		}
	} else {
		rcnt = j - i;
		if (i > 0) {
			memmove(&htbl[0], &htbl[i], rcnt*sizeof(int));
		}
	}

	return rcnt;
}

/* DONE */
static void
search_einfo(qry_inp_t *qi)
{
	int  n;

	if (qi->qu->qcnt == 0) {
		n = _search_einfo_raw(qi);
	} else {
		n = _search_einfo_nested(qi);
	}
	utk_widget_send_async_event_p(qi->qu->qryw, n, 0);
}

/* DONE */
static int
_search_ainfo_raw(qry_inp_t *q)
{
	int        i;
	int        rcnt = 0;
	qry_ui_t  *qu = q->qu;
	zainf_t   *zi = (zainf_t*)qu->data;
	int        j  = qu->icnt;

	DBG("--- query: alarm info ---\n");
	if (q->stm > 0) {
		if (q->etm == 0) {
			time(&q->etm);
		}

		for (i = 0; i < j; ++i) {
			if (zi[i].etm >= q->stm)
				break;
		}
		if (i == j) return 0;
		for (; j > i; --j) {
			if (zi[j-1].etm <= q->etm)
				break;
		}
		if (j == i) return 0;
	}

	if (q->idx >= 0) {
		for (; i < j; ++i) {
			if (zi[i].znr == q->idx) {
				qu->qres[rcnt++] = i;
			}
		}
	} else {
		for (; i < j; ++i) {
			qu->qres[rcnt++] = i;
		}
	}

	return rcnt;
}

/* DONE */
static int
_search_ainfo_nested(qry_inp_t *q)
{
	int        i;
	int        rcnt = 0;
	qry_ui_t  *qu = q->qu;
	zainf_t   *zi = (zainf_t*)qu->data;
	int        j  = qu->qcnt;
	int       *htbl = qu->qres;

	DBG("--- nested query: alarm info ---\n");
	if (q->stm > 0) {
		if (q->etm == 0) {
			time(&q->etm);
		}

		for (i = 0; i < j; ++i) {
			if (zi[htbl[i]].etm >= q->stm)
				break;
		}
		if (i == j) return 0;

		for (; j > i; --j) {
			if (zi[htbl[j-1]].etm <= q->etm)
				break;
		}
		if (j == i) return 0;
	}

	if (q->idx >= 0) {
		for (; i < j; ++i) {
			if (zi[htbl[i]].znr == q->idx) {
				htbl[rcnt++] = htbl[i];
			}
		}
	} else {
		rcnt = j - i;
		if (i > 0) {
			memmove(&htbl[0], &htbl[i], rcnt*sizeof(int));
		}
	}

	return rcnt;
}

/* DONE */
static void
search_ainfo(qry_inp_t *qi)
{
	int  n;

	if (qi->qu->qcnt == 0) {
		n = _search_ainfo_raw(qi);
	} else {
		n = _search_ainfo_nested(qi);
	}
	utk_widget_send_async_event_p(qi->qu->qryw, n, 0);
}

static VQRY   fsearch[] = {search_einfo, search_ainfo};

/* DONE */
static void*
query_thread(void *arg)
{
	pthread_detach(pthread_self());
	while (1) {
		query_lock();
		while (g_qry == NULL)
			query_p();

		DBG("--- begin querying: %s ---\n", (g_qry->type?"alarm info":"event info"));	
		(*fsearch[g_qry->type])(g_qry);
		DBG("--- end querying: %s ---\n", (g_qry->type?"alarm info":"event info"));

		g_qry = NULL;
		qid = -1;
		query_unlock();
	}
	pthread_exit(NULL);
}

/* DONE */
void
start_query_thread(void)
{
	pthread_t  tid;

	qrylen = strlen(qrying);
	noreslen = strlen(nores);
	ierrlen = strlen(ierr);

	pstr[0][0] = yr[0];
	pstr[0][1] = mon[0];
	pstr[0][2] = day[0];
	pstr[0][3] = hur[0];
	pstr[0][4] = min[0];
	pstr[0][5] = sec[0];
	
	pstr[1][0] = yr[1];
	pstr[1][1] = mon[1];
	pstr[1][2] = day[1];
	pstr[1][3] = hur[1];
	pstr[1][4] = min[1];
	pstr[1][5] = sec[1];
	pthread_create(&tid, NULL, query_thread, NULL);
}

/////////////////////////////////////////////////////////
/* DONE */
static time_t
get_time_from_str(char *str[])
{
	struct tm  ntm;
	int   v;
	char *p;

	CLEARV(ntm);
	v = strtol(str[0], &p, 10);
#ifndef DEBUG
	if (v < 2013) return 0;
#endif
	ntm.tm_year = v - 1900;

	v = strtol(str[1], &p, 10);
	if (v < 1||v > 12) return 0;
	ntm.tm_mon = v - 1;

	v = strtol(str[2], &p, 10);
	if (v < 1||v > 31) return 0;
	ntm.tm_mday = v;

	v = strtol(str[3], &p, 10);
	if ((v == 0&&p == str[3])||(v > 23)) return 0;
	ntm.tm_hour = v;

	v = strtol(str[4], &p, 10);
	if ((v == 0&&p == str[4])||(v > 59)) return 0;
	ntm.tm_min = v;

	v = strtol(str[5], &p, 10);
	if ((v == 0&&p == str[5])||(v > 59)) return 0;
	ntm.tm_sec = v;

	DBG("--- %d-%02d-%02d %02d:%02d:%02d ---\n", ntm.tm_year+1900, ntm.tm_mon+1,
		     ntm.tm_mday, ntm.tm_hour, ntm.tm_min, ntm.tm_sec);
	return mktime(&ntm);
}

/* DONE */
static void
build_time_entry(qry_ui_t *qu, int w[], int h, int x[], int y, int idx)
{
	int      i;
	rect_t   r;
	myent_t *e;

	r.h = h;
	r.y = y;
	r.x = x[0];
	r.w = w[0];
	e = &qu->te[idx][0];
	myent_new(&e[0], qu->qryw, &r, 0);
	myent_add_caret(&e[0], RGB_BLACK);
	myent_set_cache(&e[0], pstr[idx][0], 6, 4);
	myent_add_kbar(&e[0], &qu->qke);

	r.w = w[1];
	for (i = 1; i < 6; ++i) {
		r.x = x[i];
		myent_new(&e[i], qu->qryw, &r, 0);
		myent_add_caret(&e[i], RGB_BLACK);
		myent_set_cache(&e[i], pstr[idx][i], 4, 2);
		myent_add_kbar(&e[i], &qu->qke);
	}
}

/* DONE */
static void
build_zinp_entry(qry_ui_t *qu)
{
	rect_t   r = {.x = (1024-678)/2+190, .y = (600-370)/2+236, .w = 60, .h = 50};

	myent_new(&qu->zinp, qu->qryw, &r, 0);
	myent_add_caret(&qu->zinp, RGB_BLACK);
	myent_set_cache(&qu->zinp, zinp, 6, 4);
	myent_set_text_max(&qu->zinp, 4);
	myent_add_kbar(&qu->zinp, &qu->qke);
}

static char *evtn[] = {
	"系 统 重 启",
	"系 统 登 录",
	"系 统 登 出",
	"编 程 更 改",
	"网 络 事 件",
	"RS485事件",
	"主机交流事件",
	"主机电池事件",
	"主机电池电压事件",
	"主机总线事件",
	"主机编程事件",
	"主机联结事件",
};

static void
select_evid(int i)
{
	DBG("--- zdm: %d ---\n", i);
	qid = i;
	utk_label_set_text((UtkLabel*)evlbl, evtn[i]);
	utk_label_update(evlbl);
}

static void
build_evid_menu(qry_ui_t *q)
{
	rect_t   r = {.x = (1024-678)/2+4, .y = (600-408)/2, .w = 160, .h = 34};
	build_pop_menu(&q->dm, q->qryw, BG_EVT, &r, 12, select_evid);
	menu_set_grab(&q->dm);
}

static void
prepare_evid(void *arg)
{
	qry_ui_t *q = arg;

	if (kbar_ent_showed(&q->qke) == 0) {
		show_menu(&q->dm);
	}
}

static void
build_evid_label(qry_ui_t *qu)
{
	qu->evid = utk_label_new_fp();
	utk_widget_set_size(qu->evid, evlbl_w, evlbl_h);
	utk_label_set_text_size((UtkLabel*)qu->evid, 20);
	utk_label_set_text_color((UtkLabel*)qu->evid, RGB_BLACK);
	utk_label_set_text((UtkLabel*)qu->evid, NULL);
	utk_widget_add_cntl(qu->qryw, qu->evid, evlbl_x, evlbl_y);
	evlbl = qu->evid;
}

static void
build_qry_info(qry_ui_t *qu)
{
	int  w[] = {qry_ent_yw, qry_ent_tw};
	int  x[] = {qry_ent_x0, qry_ent_x1, qry_ent_x2, qry_ent_x3, qry_ent_x4, qry_ent_x5};
	rect_t  r = {.x = (1024-678)/2+164, .y = (600-370)/2+236, .w = 476, .h = 52};

	build_entry_kbar(&qu->qke, qu->qryw, (600-370)/2+370, 0);
	build_time_entry(qu, w, qry_ent_h, x, qry_ent_y0, 0);
	build_time_entry(qu, w, qry_ent_h, x, qry_ent_y1, 1);
	if (qu->type == 0) {
		build_evid_menu(qu);
		map_new(qu->qryw, &r, NULL, prepare_evid, (void *)qu);
		build_evid_label(qu);
	} else {
		build_zinp_entry(qu);
	}
}

static inline void
show_qerr_tip(qry_ui_t *q, int tmeo)
{
	tip_lbl_set_text(&q->qryt, ierr, ierrlen, RGB_BLACK, (tip_w-ierrlen*8)/2, qry_tip_y);
	tip_set_timeout(&q->qryt, tmeo);
	show_tip(&q->qryt);
}

static void
qry_ui_clear(void *arg)
{
	int  i;
	qry_ui_t *q = arg;
	
	for (i = 0; i < 6; ++i) {
		myent_clear(&q->te[0][i]);
		myent_clear(&q->te[1][i]);
	}

	if (q->type == 0) {
		hide_menu(&q->dm);
		utk_label_set_text((UtkLabel*)q->evid, NULL);
		utk_label_update(q->evid);
	} else {
		myent_clear(&q->zinp);
	}
}

static void
query_cancel(void *arg)
{
	qry_ui_t *q = arg;
	kbar_entry_cancel(&q->qke);
	qry_ui_clear(arg);
	utk_window_show(q->parent);
}

static int
get_zinp(void)
{
	int  z;

	if (strlen(zinp) == 0) return -1;
	z = strtol(zinp, NULL, 10);
	if ((z > 256&&z <= 1000)||z > 1256) return -1;
	if (z > 1000) {
		z -= 1000;
		z += 256;
	}
	return z;
}

static void
qry_start(void *arg)
{
	qry_ui_t *q = arg;
	time_t  tm0 = 0;
	time_t  tm1 = 0;
	int     id;

	if (kbar_entry_save_prepared(&q->qke) == 0) return;
	if (q->type == 0) {
		hide_menu(&q->dm);
	}

	if (strlen(pstr[0][0]) > 0)
	{
		tm0 = get_time_from_str(pstr[0]);
		if (tm0 == 0) {
			show_tip(&q->terr);
			return;
		}
	}

	if (strlen(pstr[1][0]) > 0)
	{
		tm1 = get_time_from_str(pstr[1]);
		if (tm1 == 0) {
			show_tip(&q->terr);
			return;
		}
	}
	
	if (q->type == 1) {
		id = get_zinp(); //get zone number.
		DBG("--- zid = %d ---\n", id);
	} else {
		id = qid;        //event id.
	}

	if (tm0 == 0&&id < 0) {
		show_qerr_tip(q, 2);
		return;
	}

	if (query_trylock() != 0) {
		show_qerr_tip(q, 2);
		return;
	}

	if (g_qry == NULL) {
		g_query.stm = tm0;
		g_query.etm = tm1;
		g_query.idx = id;
		g_query.type = q->type;

		g_qry = &g_query;
		g_qry->qu = q;
		query_v();
	}
	query_unlock();

	tip_lbl_set_text(&q->qryt, qrying, qrylen, RGB_BLACK, (tip_w-qrylen*8)/2, qry_tip_y);
	tip_set_timeout(&q->qryt, 0);
	show_tip(&q->qryt);
}

static void
handle_query(UtkWindow *w, int x, int y)
{
	qry_ui_t *q = g_qry->qu;

	DBG("--- query result: %d ---\n", x);
	if (x == 0) {
		q->clr_result(q);
		tip_lbl_set_text(&q->qryt, nores, noreslen, RGB_BLACK, (tip_w-noreslen*8)/2, qry_tip_y);
		tip_set_timeout(&q->qryt, 2);
		tip_update(&q->qryt);
	} else {
		hide_tip(&q->qryt);
		q->set_result(q, x);
	}
}

/* DONE */
void
qry_ui_new(qry_ui_t *qu, void *parent, char *bg)
{
	int   bx[]  = {eqry_ok_x, eqry_cl_x};
	char *bgcl[] = {BN_RET1500, BN_RET1501};
	char *bgok[] = {BN_GOK0, BN_GOK1};

	qu->parent = parent;
	qu->qryw   = dialog_new(parent, bg, bx, eqry_bn_y, bgok, bgcl);

	/* add on 2013.12.06 */
#if 0
	qu->qt.sec = DQTMEO;
	init_qtimer(&qu->qt, (void *)qu->qryw, (void *)query_cancel);
#endif

	utk_widget_async_evt_handler(qu->qryw, handle_query);
	dialog_ok_event(qu->qryw, qry_start, qu);
	dialog_cancel_event(qu->qryw, query_cancel, qu);

	
	build_tip_lbl(&qu->terr, qu->qryw, (LCD_W-tip_w)/2, (LCD_H-tip_h)/2, LBL_TINVLD, TIP_SPLASH_TIME);
	tip_set_hide_cb(&qu->terr, (void*)qry_ui_clear, (void *)qu);

	build_tip_lbl(&qu->qryt, qu->qryw, (LCD_W-tip_w)/2, (LCD_H-tip_h)/2, BG_TIP, 0);
	tip_set_hide_cb(&qu->qryt, (void*)qry_ui_clear, (void *)qu);
	build_qry_info(qu);
}

/* DONE */
void
show_qry_ui(qry_ui_t *q)
{
	utk_window_show(q->qryw);
}



