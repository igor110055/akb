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
#include "config.h"
#include "query.h"
#include "gui.h"
#include "stb.h"
#include "rs485.h"

static UtkWindow   *alrwin;
static UtkListBox  *alb;
static UtkWindow   *ddialog;

static int       selectedid = 0;

zainf_t   ainf[MAXZA];
static tip_t     prterr;

static zainf_t   g_za;

static unsigned int    stid;

static void  aquery_clr_result(qry_ui_t *q);
static void  aquery_set_result(qry_ui_t *q, int n);

static qry_ui_t  aqu = {
	.type  = 1,
	.data  = ainf,
	.icnt  = 0,
	.qres  = qresult,
	.qcnt  = 0,
	.clr_result = aquery_clr_result,
	.set_result = aquery_set_result,
};

unsigned short  zflt[ZFLTMAX] = {0};
static int   zflidx = 0;

/* DONE */
void
load_ainfo(void)
{
	load_info_file(&aqu);
}

static void
save_ainfo(void *arg)
{
	store_info_file(arg);
}

static void
reindex_ainfo(int fr)
{
	int i;

	for (i = fr; i < aqu.icnt; ++i) {
		ainf[i].idx = i;
		memset(ainf[i].idstr, 0, 8);
		snprintf(ainf[i].idstr, 8, "%d", i);
	}
}

/* arg: 0~511, zone number */
static void
transfer_alarm(int id)
{
	if (xfer_alarm()&&!xfer_noway()) {
		if (xfer_byuart()) {
			uart_xfer_realt(id, AINFT);
		}
	}
}

/* DONE: called in zalarm.c */
void
add_alarm_info(zainf_t *za)
{
	int  mv = 0;
	int  left;
	int  fr;

	left = MAXZA - aqu.icnt;

	if (left == 0) {
		memmove(&ainf[0], &ainf[1], (aqu.icnt - 1)*sizeof(zainf_t));
		aqu.icnt -= 1;
		left += 1;
		reindex_ainfo(0);
		mv = 1;
	}

	fr = aqu.icnt;
	memcpy(&ainf[aqu.icnt], za, sizeof(zainf_t));
	aqu.icnt += 1;
	reindex_ainfo(fr);

	if (aqu.qcnt == 0) { //add while not in query.
		if (mv == 0) {
			utk_list_box_add_item(alb, 1);
		} else {
			utk_list_box_reload_item(alb, aqu.icnt);
		}
	}

	/* transfer alarm info through rs485 */
	transfer_alarm(aqu.icnt-1);
	utk_timeout_add_once(1000, stid);
}

static void 
pr_ainf(void)
{
	int  i;
	int  acnt;

	if (aqu.icnt == 0||selectedid == 0) return;
	acnt = utk_list_box_selected_array(alb,  prtarr, MAXPRT);
	if (acnt == 0) return;
	if (aqu.qcnt > 0) {
		for (i = 0; i < acnt; ++i) {
			uart_xfer_print(qresult[prtarr[i]], AINFT);
		}
	} else {
		for (i = 0; i < acnt; ++i) {
			uart_xfer_print(prtarr[i], AINFT);
		}
	}
}

static void
del_opt(void *data)
{
	int  id;
	int  r;

	hide_model_dialog(ddialog);
	id = utk_list_box_first_selected(alb);
	if (id == -1) return;

	r = utk_list_box_del_selected(alb);

	aqu.icnt -= r;

	if (aqu.icnt > 0) {
		reindex_ainfo(id);
		utk_timeout_add_once(1000, stid);
	} else {
		remove(AINFDB);
	}
	utk_list_box_update((UtkWindow*)alb);

	if (aqu.qcnt > 0) {
		aqu.qcnt -= r;
		/*
		if (aqu.qcnt == 0) {
			utk_list_box_clr_content_part(alb);
			utk_list_box_update((UtkWindow*)alb);
		}*/
	}
	selectedid = 0;
}

static void
cancel_del(void *data)
{
	hide_model_dialog(ddialog);
	utk_list_box_unselect_all(alb);
	selectedid = 0;
}


static void 
del_ainf(void)
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
query_ainf(void)
{
	if (aqu.icnt == 0) return;
	show_qry_ui(&aqu);
}

static void
video_check(void)
{
}

static void
ainf_pageup(void)
{
	int  id;
	int  tail;

	if (utk_list_box_scroll_pageup(alb) == 0) {
		tail = aqu.icnt % 10;
		tail = tail > 0 ? tail : 10;
		id = aqu.icnt - tail;
		utk_list_box_scroll_to(alb, id);
	}
}

static void
ainf_pagedn(void)
{
	if (utk_list_box_scroll_pagedn(alb) == 0) {
		utk_list_box_scroll_to(alb, 0);
	}
}

static void
ainf_back(void)
{
	if (aqu.qcnt > 0) {
		utk_list_box_clr_content_part(alb);
		utk_list_box_update((UtkWindow*)alb);
		aqu.qcnt = 0;
	} else {
		show_main_win();
	}
}

static PVOID  ainf_cb[] = {ainf_back, pr_ainf, del_ainf, query_ainf, video_check, ainf_pageup, ainf_pagedn};

static void
do_alarm_opt(void *data)
{
	ainf_cb[(int)data]();
}

static void
selected_handle(void *data)
{
	int i = (int)data;
	if ((i>>8)&1) {
		++selectedid;
	} else {
		--selectedid;
	}
}

static void
aquery_clr_result(qry_ui_t *q)
{
	if (aqu.qcnt > 0) {
		utk_list_box_clr_content_part(alb);
		aqu.qcnt = 0;
	}
}

static void
aquery_set_result(qry_ui_t *q, int n)
{
	q->qcnt = n;
	utk_list_box_set_content_part(alb, q->qres, n);
	show_ainfo_win();
}

/* arg: 1~512, zone number */
static void
transfer_zstat(int op, int arg)
{
	zflt[zflidx] = (arg << 1)|(op-ZFLR);
	if (xfer_byuart()) {
		uart_xfer_zstat(zflidx);
	}
	zflidx = (zflidx + 1) & (ZFLTMAX-1);
}

static void
add_ainfo(UtkWindow *win, int  op, int arg)
{
	if (op == ZFLR||op == ZFLT) {
		if (xfer_zstat()&&!xfer_noway()) {
			transfer_zstat(op, arg);
		}
	} else {
		if (op > ADAM + MAXZCMD) {
			mk_painf(&g_za, op, arg);
		} else if (op >= MAXZCMD) {
			mk_aainf(&g_za, op);
		} else if (op < MAXZCMD) {
			mk_zainf(&g_za, op, arg);
		} else {
			return;
		}
		add_alarm_info(&g_za);
	}
}

static void
unselect_all(void *arg)
{
	if (selectedid > 0) {
		utk_list_box_unselect_all(alb);
		selectedid = 0;
	}
}

void
build_ainfo_gui(void)
{
	int   bw[] = {bn_abar_w0, bn_abar_w1, bn_abar_w2, bn_abar_w3, bn_abar_w4, bn_abar_w5, bn_abar_w6};
	char *bg[] = {BN_RET160, BN_PR109, BN_DEL109, BN_QRY109, BN_CKV141, BN_PU142, BN_PD142};
	char *bgok[] = {BN_DELOK0, BN_DELOK1};
	char *bgcl[] = {BN_DELCL0, BN_DELCL1};

	rect_t  fr  = {.x = ALB_X, .y = ALB_Y, .w = ALB_W, .h = row_h};
	int   cx[]  = {alb_col_x0, alb_col_x1, alb_col_x2, alb_col_x3, alb_col_x4, alb_col_x5};
	int   chr[] = {6, 4, 30, 2, 12, 19};
	long  off[] = {U_STRUCT_OFFSET(zainf_t, idstr), U_STRUCT_OFFSET(zainf_t, zstr), U_STRUCT_OFFSET(zainf_t, zname), U_STRUCT_OFFSET(zainf_t, pstr),
		           U_STRUCT_OFFSET(zainf_t, inf), U_STRUCT_OFFSET(zainf_t, tmstr)};

	alrwin = utk_window_derive();
#ifndef MAPDLL
	utk_widget_resident(alrwin);
#endif
	utk_window_set_bg(alrwin, BG_AINFO);
	utk_widget_async_evt_handler(alrwin, add_ainfo);
	stid = utk_widget_timer(alrwin, save_ainfo, (void*)&aqu);
	utk_widget_hide_before(alrwin, unselect_all, (void*)0);

	stb_new(alrwin, show_main_win);
	build_tip_lbl(&prterr, alrwin, (LCD_W-tip_w)/2, (LCD_H-tip_h)/2, LBL_PRTERR, 1);
	ddialog = build_model_dialog(alrwin, BG_DELDIAG, bgok, bgcl, del_opt, cancel_del);

	alb = listbox_new(alrwin, &fr, ALB_ROWS);
	listbox_set_font(alb, 0);
	listbox_set_cols(alb, ARR_SZ(cx), cx, chr, off);
	listbox_set_content(alb, ainf, sizeof(zainf_t), MAXZA, aqu.icnt);
	listbox_add_rows(alb, selected_handle, ALB_ROWS);
	utk_list_box_set_style(alb, LB_CHECKBOX);
	utk_list_box_set_text_clr(alb, LBX_TXT_CLR);

	build_button_array(alrwin, ARR_SZ(bw), bn_bar_h, bn_bar_y, bw, bn_abar_x0, do_alarm_opt, bg);

	qry_ui_new(&aqu, alrwin, BG_AQRY);
}

void
show_ainfo_win(void)
{
	if (utk_window_top() != alrwin) {
		utk_window_show(alrwin);
	}
}

void
send_ainfo(int op, int arg)
{
	if (alrwin)
		utk_widget_send_async_event(alrwin, op, arg);
}


