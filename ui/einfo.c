#include "ulib.h"
#include "utk.h"
#include "utksignal.h"
#include "utkwindow.h"
#include "utkbutton.h"
#include "utklabel.h"
#include "utkclock.h"
#include "utkmstlbl.h"
#include "utkvisitem.h"
#include "utklistbox.h"
#include "utktimeout.h"
#include "layout.h"
#include "gui.h"
#include "stb.h"
#include "config.h"
#include "query.h"
#include "rs485.h"

#define  MAXEROWS      10

static UtkWindow      *ewin;
static UtkWindow      *ddialog;
static UtkListBox     *elb;
static bn_col_t        ebc;

static int             selectedid = 0;

einf_t         einf[MAXE];
static tip_t   prterr;

static void  equery_clr_result(qry_ui_t *q);
static void  equery_set_result(qry_ui_t *q, int n);

static qry_ui_t  equ = {
	.type  = 0,
	.data  = einf,
	.icnt  = 0,
	.qres  = qresult,
	.qcnt  = 0,
	.clr_result = equery_clr_result,
	.set_result = equery_set_result,
};

static char   *ev_name[] = {
	[EV_RBT]    = "系统重启",
	[EV_LOGIN]  = "系统登录",
	[EV_LOGOUT] = "系统退出",
	[EV_PRG]    = "编程更改",

	[EV_NETR]   = "网络恢复",
	[EV_NETE]   = "网络故障",
	[EV_RS485R] = "RS485恢复",
	[EV_RS485E] = "RS485故障",
	[EV_APR]    = "主机交流恢复",	
	[EV_APF]    = "主机交流故障",
	[EV_DPR]    = "主机电池恢复",
	[EV_DPF]    = "主机电池故障",
	[EV_BLR]    = "主机电池电压恢复",
	[EV_BL]     = "主机电池电压低",
	[EV_HBUSR]  = "主机总线恢复",	
	[EV_HBUSE]  = "主机总线故障",
	[EV_A120DPR] = "A120防拆恢复",
	[EV_A120DP]  = "A120防拆报警",

	[EV_A120ACP] = "A120交流供电",
	[EV_A120DCP] = "A120电池供电",
	[EV_HPENT]  = "主机进入编程",
	[EV_HPEXT]  = "主机退出编程",
	[EV_HCONN]  = "主机联结",
	[EV_HDCON]  = "主机联结失败",
};

/////////////////////////////////////////////////

/* DONE */
void
load_einfo(void)
{
	load_info_file(&equ);
}

/* DONE */
static void
set_ev_name(einf_t *e, int ev, int hid)
{
	e->eid = ev;
	e->hid = hid;
	memset(e->ename, 0, 24);
	if (ev < EV_HCONN) {
		strncpy(e->ename, ev_name[ev], 24);
	} else {
		snprintf(e->ename, 24, "%d号%s", hid, ev_name[ev]);
	}
}

static void
bn_idc_update(int rebase, int cnt)
{
	bn_colum_set_maxid(&ebc, cnt);
	if (rebase) {
		bn_colum_reset_base(&ebc);
	}
	bn_colum_update(&ebc);
}

/* DONE */
static void
reindex_einfo(int from)
{
	int  i;

	for (i = from; i < equ.icnt; ++i) {
		einf[i].idx = i;
		memset(einf[i].idstr, 0, 8);
		snprintf(einf[i].idstr, 8, "%d", i);
	}
}

static void
transfer_event(int id, int t)
{
	if (xfer_event()&&!xfer_noway()) {
		if (xfer_byuart()) {
			uart_xfer_realt(id, t);
		}
	}
}

static void
add_event(int ev, int hid)
{
	einf_t  *e;
	int      wrap = 0;

	if (equ.icnt == MAXE) {
		memmove(&einf[0], &einf[1], (equ.icnt-1)*sizeof(einf_t));
		--equ.icnt;
		reindex_einfo(0);
		wrap = 1;
	}

	e = &einf[equ.icnt];
	get_timestr(&e->etm, e->tmstr, 20);
	set_ev_name(e, ev, hid);
	e->idx = equ.icnt;
	memset(e->idstr, 0, 8);
	snprintf(e->idstr, 8, "%d", e->idx);

	++equ.icnt;

	if (equ.qcnt == 0) { //add while not in query.
		if (wrap == 0) {
			utk_list_box_add_item(elb, 1);
			bn_idc_update(0, equ.icnt);
		} else {
			utk_list_box_reload_item(elb, equ.icnt);
			bn_idc_update(1, equ.icnt);
		}
	}
	store_info_file(&equ);

	if (ev >= EV_HPENT&&ev <= EV_BL&&hid > 0) {
		transfer_event(e->idx, EINFT);
	}
}

static void
pr_einf(void)
{
	int  i;
	int  ecnt;
	einf_t  *e;
	int  id;

	if (equ.icnt == 0||selectedid == 0) return;
	ecnt = utk_list_box_selected_array(elb,  prtarr, MAXPRT);
	if (ecnt == 0) return;
	if (equ.qcnt > 0) {
		for (i = 0; i < ecnt; ++i) {
			id = qresult[prtarr[i]];
			e = &einf[id];
			if (e->eid >= EV_HPENT&&e->eid <= EV_BL&&e->hid > 0) {
				uart_xfer_print(id, EINFT);
			}
		}
	} else {
		for (i = 0; i < ecnt; ++i) {
			id = prtarr[i];
			e = &einf[id];
			if (e->eid >= EV_HPENT&&e->eid <= EV_BL&&e->hid > 0) {
				uart_xfer_print(id, EINFT);
			}
		}
	}
}

static void
del_opt(void *data)
{
	int  r;
	int  id;

	hide_model_dialog(ddialog);
	id = utk_list_box_first_selected(elb);
	if (id == -1) return;

	r = utk_list_box_del_selected(elb);
	equ.icnt -= r;
	DBG("--- first id = %d, %d items deleted ---\n", id, r);

	if (equ.icnt > 0) {
		reindex_einfo(id);
		store_info_file(&equ);
	} else {
		remove(EINFDB);
	}
	utk_list_box_update((UtkWindow*)elb);
	if (equ.qcnt > 0) {
		equ.qcnt -= r;
		if (equ.qcnt == 0) {
			bn_idc_update(1, equ.icnt);
		} else {
			bn_idc_update(1, equ.qcnt);
		}
	} else {
		bn_idc_update(1, equ.icnt);
	}
	selectedid = 0;
}

static void
cancel_del(void *data)
{
	hide_model_dialog(ddialog);
	utk_list_box_unselect_all(elb);
	selectedid = 0;
}

static void
del_einf(void)
{
	if (selectedid > 0) {
		if (priority_get(PRI_DEL) == 0) {
			show_tip(&prterr);
			return;
		}
		show_model_dialog(ddialog);
	}
}

static void
einf_qry(void)
{
	if (equ.icnt == 0) return;
	show_qry_ui(&equ);
}

static void
einf_pageup(void)
{
	int id;
	int tail;

	if (utk_list_box_scroll_pageup(elb)) {
		bn_colum_pageup(&ebc);
	} else {
		tail = equ.icnt % 10;
		tail = tail > 0 ? tail : 10;
		id = equ.icnt - tail;
		bn_colum_set_base(&ebc, id);
		utk_list_box_scroll_to(elb, id);
		bn_colum_update(&ebc);
	}
}

static void
einf_pagedn(void)
{
	if (utk_list_box_scroll_pagedn(elb)) {
		bn_colum_pagedn(&ebc);
	} else {
		utk_list_box_scroll_to(elb, 0);
		bn_idc_update(1, equ.icnt);
	}
}

static void
einf_back(void)
{
	if (equ.qcnt > 0) {
		utk_list_box_clr_content_part(elb);
		utk_list_box_update((UtkWindow*)elb);
		bn_idc_update(1, equ.icnt);
		equ.qcnt = 0;
	} else {
		show_main_win();
	}
}

static PVOID  einf_cb[] = {einf_back, pr_einf, del_einf, einf_qry, einf_pageup, einf_pagedn};

static void
do_einfo_opt(void *data)
{
	einf_cb[(int)data]();
}

static void
handle_evt_occured(UtkWindow *win, int  x, int y)
{
	if (win != ewin) return;
	DBG("--- received the event: type:%d-host:%d: %s ---\n", x, y, ev_name[x]);
	add_event(x, y);
}

static void
equery_clr_result(qry_ui_t *q)
{
	if (q->qcnt > 0) {
		utk_list_box_clr_content_part(elb);
		bn_idc_update(1, q->icnt);
		q->qcnt = 0;
	}
}

static void
equery_set_result(qry_ui_t *q, int n)
{
	q->qcnt = n;
	utk_list_box_set_content_part(elb, q->qres, n);
	bn_idc_update(1, n);
	show_einfo_win();
}

static void
selected_item(void *data)
{
	int i = (int)data;
	int vid;

	vid = i&0xff;
	if ((i>>8)&1) {
		++selectedid;
		bn_colum_select(&ebc, vid);
		DBG("--- selected = %d, total = %d ---\n", vid, selectedid);
	} else {
		--selectedid;
		DBG("--- unselected = %d, total = %d ---\n", vid, selectedid);
		bn_colum_unselect(&ebc, vid);
	}
}

static void
select_event(void *arg)
{
	bn_colum_pressed(&ebc, (int)arg);
}

static void
unselect_all(void *arg)
{
	if (selectedid > 0) {
		utk_list_box_unselect_all(elb);
		selectedid = 0;
	}
}

void
build_einfo_gui(void)
{
	int   bw[]  = {160, 156, 156, 156, 142, 142};
	char *bg[]  = {BN_RET160, BN_PR156, BN_DEL156, BN_QRY156, BN_PU142, BN_PD142};
	char *bgc[] = {BN_SEL0, BN_SEL1};
	char *bgok[] = {BN_DELOK0, BN_DELOK1};
	char *bgcl[] = {BN_DELCL0, BN_DELCL1};

	int   cx[]  = {elb_col_x0, elb_col_x1, elb_col_x2};
	int   chr[] = {6, 22, 19};
	long  off[] = {U_STRUCT_OFFSET(einf_t, idstr), U_STRUCT_OFFSET(einf_t, ename), U_STRUCT_OFFSET(einf_t, tmstr)};	

	rect_t  fr  = {.x = ELB_X, .y = ELB_Y, .w = ELB_W, .h = row_h};

	ewin = utk_window_derive();
#ifndef MAPDLL
	utk_widget_resident(ewin);
#endif
	utk_window_set_bg(ewin, BG_EINFO);
	utk_widget_async_evt_handler(ewin, handle_evt_occured);
	utk_widget_hide_before(ewin, unselect_all, (void*)0);

	stb_new(ewin, show_main_win);

	build_tip_lbl(&prterr, ewin, (LCD_W-tip_w)/2, (LCD_H-tip_h)/2, LBL_PRTERR, 1);
	ddialog = build_model_dialog(ewin, BG_DELDIAG, bgok, bgcl, del_opt, cancel_del);

	elb = listbox_new(ewin, &fr, MAXEROWS);
	listbox_set_font(elb, 1);
	listbox_set_cols(elb, ARR_SZ(cx), cx, chr, off);
	listbox_set_content(elb, einf, sizeof(einf_t), MAXE, equ.icnt);
	listbox_add_rows(elb, selected_item, MAXEROWS);
	utk_list_box_set_style(elb, LB_CHECKBOX);
	utk_list_box_set_text_clr(elb, LBX_TXT_CLR);

	bn_colum_init(&ebc, MAXEROWS, equ.icnt, elb);
	bn_colum_new(&ebc, ewin, einf_sel_x, einf_sel_y, bgc, (void*)select_event);

	build_button_array(ewin, ARR_SZ(bw), bbar_h, bbar_y, bw, bbar_x, do_einfo_opt, bg);

	qry_ui_new(&equ, ewin, BG_EQRY);
}

void
show_einfo_win(void)
{
	if (utk_window_top() != ewin) {
		utk_window_show(ewin);
	}
}

void
send_event_db(int ev, int hid)
{
	utk_widget_send_async_event(ewin, ev, hid);
}


