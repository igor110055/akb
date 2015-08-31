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
#include "utkmstbutton.h"
#include "config.h"
#include "layout.h"
#include "gui.h"
#include "stb.h"
#include "zopt.h"

static UtkWindow   *pzuiw;
static UtkWindow   *pst[GLB_ROWS];
static UtkListBox  *plb;
static id_col_t     pidc;
static char         pbuf[GLB_ROWS][4];
static char        *idbuf[GLB_ROWS];

static int          selectedid = -1;
static int          puit = 0;

static UtkWindow *parmbn0;
static UtkWindow *parmbn1;
static UtkWindow *pbpbn0;
static UtkWindow *pbpbn1;

static UtkWindow *fdialog;

static tip_t     prterr;
static tip_t     optip;

static char *amerr[PZ_MAXERR] = {
	[PZ_SYSF-1]    = "系统故障,分区布防失败",
	[PZ_NRDY-1]    = "分区未准备,无法布防",
	[PZ_ALRT-1]    = "分区有报警,无法布防",
	[PZ_ARMED-1]   = "分区已布防",
	[PZ_BPASSED-1] = "分区旁路,无法布防",
	[PZ_TMRED-1]   = "定时分区,无法手动布防",
	[PZ_NARM-1]    = "该分区无防区可被布防",
	[PZ_24HUR-1]   = "24小时分区,无需布防"
};

static int   amlen[PZ_MAXERR] = {0};

static char  *bperr[] = {
	[0] = "分区旁路失败",
	[1] = "分区旁路恢复失败",
};

static int   belen[2];

static char  *daerr = "分区撤防失败";
static int    delen;

static pzone_t  *cur_pz = NULL;

void
build_mstbn_col(void *g[], void *parent, int x, int y, int rows, void *f)
{
	int   i;
	int   j;
	char *bgx0[] = {
		PST_UNKNOWN0,
		PST_DISARM0,
		PST_PTBYPASS0,
		PST_BYPASS0,
		PST_NRDY0,
		PST_PTARM0,
		PST_ARM0,
		PST_FRCARM0,
		PST_TMRARM0,
		PST_ALARM0,
		PST_ALARMR0
	};
	
	char *bgx1[] = {
		PST_UNKNOWN1,
		PST_DISARM1,
		PST_PTBYPASS1,
		PST_BYPASS1,
		PST_NRDY1,
		PST_PTARM1,
		PST_ARM1,
		PST_FRCARM1,
		PST_TMRARM1,
		PST_ALARM1,
		PST_ALARMR1
	};

	for (i = 0; i < rows; ++i) {
		g[i] = utk_mst_button_new();
		utk_mst_button_set_type((UtkMSTButton *)g[i]);
		for (j = 0; j < ARR_SZ(bgx0); ++j) {
			utk_mst_button_set_bg((UtkMSTButton*)g[i], bgx0[j]);
			utk_mst_button_set_bg((UtkMSTButton*)g[i], bgx1[j]);
		}
		utk_signal_connect(g[i], "clicked", f, (void*)i);
		utk_widget_add_cntl(parent, g[i], x, y+i*row_h);
	}
}


static inline int
get_selected_pitemid(void)
{
	int id;

	if (selectedid < 0) return -1;
	id = utk_list_box_get_id(plb, selectedid);
	if (id >= pzcnt) return -1;

	return id;
}

static void
tiplen_init(void)
{
	int  i;

	for (i = 0; i < PZ_MAXERR; ++i) {
		amlen[i] = strlen(amerr[i]);
	}
	belen[0] = strlen(bperr[0]);
	belen[1] = strlen(bperr[1]);
	delen = strlen(daerr);
}

static inline void
show_perrtip(char *err, int len, int tmeo)
{
	show_strtip(&optip, err, len, tmeo, tip_w, qry_tip_y);
}

static void 
pz_arm(void)
{
	int  id;

	if ((id = get_selected_pitemid()) < 0) return;
	if (!pz_get(pzarr[id]+1)) {
		show_tip(&prterr);
		return;
	}
	cur_pz = &pzone[pzarr[id]];
	pzone_arm(pzarr[id]+1);
	if (selectedid >= 0) {
		utk_list_box_unselect(plb, selectedid);
		selectedid = -1;
	}
}

static void
update_pzst(int p, int y)
{
	int  id;
	int  i;

	id = utk_list_box_get_id(plb, 0);
	i = pzone[p].idx - id;
	if (i >= 0&&i < GLB_ROWS) {
		utk_widget_update_state(pst[i], y<<1);
	}
}

static void
pzarm_result(int res)
{
	int  r = res & 0xff;

	if (r > 0) {
		DBG("--- pz_arm: r = %d ---\n",  r);
		if (r <= PZ_NRDY) {
			cur_pz = &pzone[res >> 8];
			show_model_dialog(fdialog);
		} else {
			show_perrtip(amerr[r-1], amlen[r-1], 2);
		}
	}
}

static void 
pz_disarm(void)
{
	int  id;

	if ((id = get_selected_pitemid()) < 0) return;
	if (!pz_get(pzarr[id]+1)) {
		show_tip(&prterr);
		return;
	}
	pzone_disarm(pzarr[id]+1);
	if (selectedid >= 0) {
		utk_list_box_unselect(plb, selectedid);
		selectedid = -1;
	}
}

static void
pzdisarm_result(int res)
{
	if ((res & 0xff) > 0) {
		show_perrtip(daerr, delen, 2);
	}
}


static void
pz_bypass(void *data)
{
	int  id;

	if ((id = get_selected_pitemid()) < 0) return;
	if (!pz_get(pzarr[id]+1)) {
		show_tip(&prterr);
		return;
	}
	pzone_bypass(pzarr[id]);
	if (selectedid >= 0) {
		utk_list_box_unselect(plb, selectedid);
		selectedid = -1;
	}
}

static void
pzbp_result(int res)
{
	if ((res & 0xff) > 0) {
		show_perrtip(bperr[0], belen[0], 2);
	}
}

static void
pz_bypassr(void *data)
{
	int  id;

	if ((id = get_selected_pitemid()) < 0) return;
	if (!pz_get(pzarr[id]+1)) {
		show_tip(&prterr);
		return;
	}
	pzone_bypassr(pzarr[id]);
	if (selectedid >= 0) {
		utk_list_box_unselect(plb, selectedid);
		selectedid = -1;
	}
}

static void
pzbpr_result(int res)
{
	if ((res & 0xff) > 0) {
		show_perrtip(bperr[1], belen[1], 2);
	}
}

static void
pzfarm_result(int res)
{
	int  r = res & 0xff;

	if (r > 0) {
		--r;
		show_perrtip(amerr[r], amlen[r], 2);
	}
}

static void (*pz_result[])(int) = {pzarm_result, pzfarm_result, pzdisarm_result, pzbp_result, pzbpr_result};

static void
update_pst_col(void)
{
	int  id;
	int  st;
	int  i;
	int  j;
	int  maxr;
	int  maxid;

	id = utk_list_box_get_id(plb, 0);
	
	maxr = id + GLB_ROWS;
	if (pzcnt <= id) {
		maxid = id;
	} else if (pzcnt <= maxr) {
		maxid = pzcnt;
	} else {
		maxid = maxr;
	}

	for (i = id; i < maxid; ++i) {
		st = (int)get_pzone_state_direct(pzarr[i]+1);
		utk_widget_update_state(pst[i-id], st<<1);
	}

	for (i = maxid; i < maxr; ++i) {
		j = i - id;
		utk_widget_set_state(pst[j], -1);
		utk_widget_hide(pst[j]);
	}
}

static void 
page_up(void)
{
	if (utk_list_box_scroll_pageup(plb)) {
		id_colum_pageup(&pidc);
		update_pst_col();
	}
}

static void
page_down(void)
{
	if (utk_list_box_scroll_pagedn(plb)) {
		id_colum_pagedn(&pidc);
		update_pst_col();
	}
}

static PVOID    goparent[] = {show_arment_win, show_bpent_win, show_stent_win};

static void
go_back(void)
{
	(*goparent[puit])();
}

static PVOID  bn_cb[] = {go_back, page_up, page_down};

static void
do_ptarm_opt(void *data)
{
	int i = (int)data;
	bn_cb[i]();
}

static void
id_col_sel(void *data)
{
	int i = (int)data;
	utk_list_box_toggle_item((UtkWindow*)plb, i);
}

static void
selected_handle(void *data)
{
	int i = (int)data;

	if ((i>>8)&1) {
		selectedid = i&0xff;
	} else {
		selectedid = -1;
		cur_pz = NULL;
	}
}

static void
detach_pbypass(void)
{
	utk_widget_detach(pbpbn0);
	utk_widget_detach(pbpbn1);
}

static void
detach_parm(void)
{
	utk_widget_detach(parmbn0);
	utk_widget_detach(parmbn1);
}

static void
attach_pbypass(void)
{
	utk_widget_attach(pbpbn0);
	utk_widget_attach(pbpbn1);
}

static void
attach_parm(void)
{
	utk_widget_attach(parmbn0);
	utk_widget_attach(parmbn1);
}

static void
switch_widget(int t)
{
	if (puit == 0) {
		detach_parm();
	} else if (puit == 1) {
		detach_pbypass();
	}

	if (t == 0) {
		attach_parm();
		utk_widget_enable((UtkWindow*)plb); //new add on 2013.12.14
	} else if (t == 1) {
		attach_pbypass();
		utk_widget_enable((UtkWindow*)plb); //new add on 2013.12.14
	} else {
		utk_widget_disable((UtkWindow*)plb); //new add on 2013.12.14
	}

	puit = t;
}

static void
handle_pzone_state(UtkWindow *win, int x, int y)
{
	if (win != pzuiw) return;
	if (x < MAXPZ) {
		update_pzst(x, y);
	} else {
		(*pz_result[x - MAXPZ])(y);
	}
}

static void
check_zonest(void *arg)
{
	int  i = (int)arg;
	int  id;

	id = utk_list_box_get_id(plb, i);
	if (id < pzcnt) {
		show_pzck_gui(pzarr[id]);
	}
}

static void
farm_opt(void *data)
{
	hide_model_dialog(fdialog);
	if (priority_get(PRI_FARM)) {
		if (cur_pz&&pz_get(cur_pz->pnr)) {
			pzone_farm(cur_pz->pnr);
		}
	} else {
		show_tip(&prterr);
	}
	cur_pz = NULL;
}

static void
cancel_parm(void *data)
{
	hide_model_dialog(fdialog);
}

static void
leave_cleanup(void *arg)
{
	if (selectedid >= 0) {
		utk_list_box_unselect(plb, selectedid);
		selectedid = -1;
	}
}

void
build_pzone_gui(void)
{
	int   i;
	int   bx[] = {bn_ptbar_x0, bn_ptbar_x3, bn_ptbar_x4};
	int   bw[] = {bn_ptbar_w0, bn_ptbar_w3, bn_ptbar_w4};
	char *bg[] = {BN_RET182, BN_PU180, BN_PD180};

	char *bg0[] = {BN_PARM0, BN_PARM1};
	char *bg1[] = {BN_PZDISARM0, BN_PZDISARM1};
	char *bg2[] = {BN_BPASS0, BN_BPASS1};
	char *bg3[] = {BN_BPASSR0, BN_BPASSR1};

	rect_t rid = {.x = pid_col_x, .y = pid_col_y, .w = pid_col_w, .h = row_h};
	rect_t  fr = {.x = PZLB_X, .y = PZLB_Y, .w = PZLB_W, .h = row_h};
	int   cx[] = {pzlb_col_x1, pzlb_col_x2};
	int   chr[] = {2, 30};
	long  off[] = {U_STRUCT_OFFSET(pzone_t, pstr), U_STRUCT_OFFSET(pzone_t, pname)};
	char *bgok[] = {YES_BN0, YES_BN1};
	char *bgcl[] = {NO_BN0, NO_BN1};

	tiplen_init();

	pzuiw = utk_window_derive();
#ifndef MAPDLL
	utk_widget_resident(pzuiw);
#endif

	utk_window_set_bg(pzuiw, BG_PZUI);
	utk_widget_async_evt_handler(pzuiw, handle_pzone_state);
	utk_widget_hide_before(pzuiw, leave_cleanup, (void*)0);

	stb_new(pzuiw, show_main_win);

	for (i = 0; i < GLB_ROWS; ++i) {
		idbuf[i] = &pbuf[i][0];
	}

	id_colum_init(&pidc, GLB_ROWS, pzcnt, 4);
	id_colum_new(&pidc, pzuiw, idbuf, &rid, id_col_sel);

	plb = listbox_new(pzuiw, &fr, GLB_ROWS);
	listbox_set_font(plb, 1);
	listbox_set_cols(plb, ARR_SZ(cx), cx, chr, off);
	listbox_set_content(plb, pzone, sizeof(pzone_t), MAXPZ, 0);
	listbox_add_rows(plb, selected_handle, GLB_ROWS);
	utk_list_box_set_hash_array(plb, pzarr, pzcnt);
	utk_list_box_set_text_clr(plb, LBX_TXT_CLR);

	build_mstbn_col((void**)pst, pzuiw, pst_col_x, zid_col_y, GLB_ROWS, check_zonest);
	for (i = 0; i < GLB_ROWS; ++i) {
		int  st;
		if (i < pzcnt) {
			st = (int)get_pzone_state_direct(pzarr[i]+1);
			utk_widget_set_state(pst[i], st<<1);
		} else {
			utk_widget_set_state(pst[i], -1);
		}
	}

	build_tip_lbl(&prterr, pzuiw, (LCD_W-tip_w)/2, (LCD_H-tip_h)/2, LBL_PRTERR, 1);
	tip_set_hide_cb(&prterr, (void*)leave_cleanup, (void *)0);
	build_tip_lbl(&optip, pzuiw, (LCD_W-tip_w)/2, (LCD_H-tip_h)/2, BG_TIP, 0);
	tip_set_hide_cb(&optip, (void*)leave_cleanup, (void *)0);
	fdialog = build_model_dialog(pzuiw, BG_FARMD, bgok, bgcl, farm_opt, cancel_parm);

	////////////////////////////////////////////////////////////
	build_button_array2(pzuiw, ARR_SZ(bw), bn_bar_h, bn_bar_y, bw, bx, do_ptarm_opt, bg);

	parmbn0 = button_new0(pzuiw, bg0, bn_ptbar_x1+1, bn_bar_y, pz_arm, NULL);
	parmbn1 = button_new0(pzuiw, bg1, bn_ptbar_x2, bn_bar_y, pz_disarm, NULL);

	pbpbn0 = button_new0(pzuiw, bg2, bn_ptbar_x1+1, bn_bar_y, pz_bypass, NULL);
	pbpbn1 = button_new0(pzuiw, bg3, bn_ptbar_x2, bn_bar_y, pz_bypassr, NULL);

	detach_pbypass();
}

/* t: 0-pzone arm/disarm, 1-pzone bypass/r, 2-pzone state check. */
void
show_pzone_gui(int t)
{
	if (utk_window_top() != pzuiw) {
		if (puit != t) {
			switch_widget(t);
		}
		utk_window_show(pzuiw);
	}
}

void
update_pzoneui_state(int i, int st)
{
	utk_widget_send_async_event(pzuiw, i, st);
}

/* called in zmgr.c when add or delete the pzone */
void
sync_pzone_ui(void)
{
	utk_list_box_reload_item(plb, pzcnt);
	id_colum_reset_base(&pidc);
	id_colum_set_maxid(&pidc, pzcnt);
	id_colum_update(&pidc);
	update_pst_col();
}

/*
 * op: 0-pz_arm, 1-pz_farm, 2-pz_disarm, 3_pz_bypass, 4-pz_bypassr.
 * r: (pznr << 8|result). result:0-success,1-errno.
 */
void
pzopr_return(int op, int p, int r)
{
	utk_widget_send_async_event(pzuiw, op + MAXPZ, (p << 8)|r);
}


