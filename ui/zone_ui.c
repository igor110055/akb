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
#include "layout.h"
#include "gui.h"
#include "stb.h"
#include "config.h"
#include "zopt.h"

typedef int (*FIINT)(int);

static UtkWindow   *zonew;
static UtkWindow   *zst[GLB_ROWS];
static UtkListBox  *zlb;
static id_col_t     zidc;
static char         zbuf[GLB_ROWS][4];
static char        *idbuf[GLB_ROWS];

static int          selectedid = 0;
static int          zuit = 0;

static UtkWindow   *zarmbn0;
static UtkWindow   *zarmbn1;
static UtkWindow   *zbpbn0;
static UtkWindow   *zbpbn1;

static tip_t        prterr;
static tip_t        zoptip;
static myent_t      zient;
static kbar_ent_t   zike;
static char         zinq[6] = {0};
static int          kbarup = 0;
static int          zselected[MAXZ] = {0};
static int          sel_zcnt = 0;

static char *zastr[] = {
	[Z_UNKNOWN-1] = "未知防区,布防失败",
	[Z_TMRED-1] = "定时防区,不能手动布防",
	[Z_STERR-1] = "该防区状态不能布防",
	[Z_24HUR-1] = "24小时防区,无需布防"
};

static int zaelen[4];

static char *zdstr[] = {
	[Z_UNKNOWN-1] = "未知防区,撤防失败",
	[Z_TMRED-1] = "定时防区,不能手动撤防",
	[Z_STERR-1] = "该防区状态不能撤防",
	[Z_24HUR-1] = "24小时防区,不能撤防"
};
static int  zdelen[4];

static char *zbpstr[] = {
	[Z_UNKNOWN-1] = "未知防区,旁路失败",
	[Z_TMRED-1] = "定时防区,旁路失败",
	[Z_STERR-1] = "该防区状态不能旁路",
	[Z_24HUR-1] = "24小时防区不能旁路",
};
static int  zbplen[4];

static char *zbpre = "旁路恢复失败";
static int   zbprlen;

static char *znrv = "请输入有效防区号";
static int   zrlen = 0;

static char *psuc = "部分操作成功";
static int   pslen = 0;

static char *asuc = "全部操作成功";
static int   aslen = 0;

static void
ztiplen_init(void)
{
	zaelen[0] = strlen(zastr[0]);
	zaelen[1] = strlen(zastr[1]);
	zaelen[2] = strlen(zastr[2]);
	zaelen[3] = strlen(zastr[3]);

	zdelen[0] = strlen(zdstr[0]);
	zdelen[1] = strlen(zdstr[1]);
	zdelen[2] = strlen(zdstr[2]);
	zdelen[3] = strlen(zdstr[3]);

	zbplen[0] = strlen(zbpstr[0]);
	zbplen[1] = strlen(zbpstr[1]);
	zbplen[2] = strlen(zbpstr[2]);
	zbplen[3] = strlen(zbpstr[3]);

	zbprlen = strlen(zbpre);
	zrlen = strlen(znrv);
	pslen = strlen(psuc);
	aslen = strlen(asuc);
}

static inline void
show_zerrtip(char *err, int len, int tmeo)
{
	show_strtip(&zoptip, err, len, tmeo, tip_w, qry_tip_y);
}

static void
after_opr(void *arg)
{
	if (selectedid > 0) {
		utk_list_box_unselect_all(zlb);
		selectedid = 0;
	}

	myent_clear(&zient);
}

static void
show_aerr(int r)
{
	show_zerrtip(zastr[r], zaelen[r], 2);
}

static void
show_derr(int r)
{
	show_zerrtip(zdstr[r], zdelen[r], 2);
}

static void
show_berr(int r)
{
	show_zerrtip(zbpstr[r], zbplen[r], 2);
}

static void
show_brerr(int r)
{
	show_zerrtip(zbpre, zbprlen, 2);
}

static FVINT  zopr[] = {zone_arm, zone_disarm, zone_bypass, zone_bypassr};
static FVINT  showerr[] = {show_aerr, show_derr, show_berr, show_brerr};

static void
do_zopr(void *arg)
{
	int     i;
	int     t = (int)arg;
	int     z;
	int     pz;
	unsigned short pzm = 0;

	if (kbarup) return;
	sel_zcnt = utk_list_box_selected_array(zlb, zselected, MAXZ);
	DBG("--- sel_zcnt = %d ---\n", sel_zcnt);

	for (i = 0; i < sel_zcnt; ++i) {
		z = zarray[zselected[i]];
		pz = pnrofz(z);
		pzm |= (1 << pz);
	}

	if (!pzg_get(pzm)) { //pzone operation permission
		show_tip(&prterr);
		return;
	}

	for (i = 0; i < sel_zcnt; ++i) {
		zopr[t](zarray[zselected[i]]);
	}
	after_opr(0);
}

////////////////////////////////////////////////
/* called when scroll listbox */

static void
update_zst_col(void)
{
	int  id;
	int  st;
	int  i;
	int  j;
	int  maxr;
	int  maxid;

	id = utk_list_box_get_id(zlb, 0);
	maxr = id + GLB_ROWS;
	if (zarcnt <= id) {
		maxid = id;
	} else if (zarcnt <= maxr) {
		maxid = zarcnt;
	} else {
		maxid = maxr;
	}

	for (i = id; i < maxid; ++i) {
		st = (int)get_zone_state_direct(zarray[i]);
		utk_widget_update_state(zst[i-id], st);
	}
	
	for (i = maxid; i < maxr; ++i) {
		j = i - id;
		utk_widget_set_state(zst[j], -1);
		utk_widget_hide(zst[j]);
	}
}

static void 
zone_pageup(void)
{
	if (kbarup) return;
	if (utk_list_box_scroll_pageup(zlb)) {
		id_colum_pageup(&zidc);
		update_zst_col();
	}
}

static void
zone_pagedown(void)
{
	if (kbarup) return;
	if (utk_list_box_scroll_pagedn(zlb)) {
		id_colum_pagedn(&zidc);
		update_zst_col();
	}
}

static void
go_back(void)
{
	if (kbarup) return;
	if (zuit == 0) {
		show_arment_win();
	} else {
		show_bpent_win();
	}
}

static PVOID  bn_cb[] = {go_back, zone_pageup, zone_pagedown};

static void
do_zopt(void *data)
{
	int i = (int)data;
	bn_cb[i]();
}

static void
id_col_sel(void *data)
{
	int i = (int)data;
	utk_list_box_toggle_item((UtkWindow*)zlb, i);
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
/////////////////////////////////////////

static void
detach_zbypass(void)
{
	utk_widget_detach(zbpbn0);
	utk_widget_detach(zbpbn1);
}

static void
detach_zarm(void)
{
	utk_widget_detach(zarmbn0);
	utk_widget_detach(zarmbn1);
}

static void
attach_zbypass(void)
{
	utk_widget_attach(zbpbn0);
	utk_widget_attach(zbpbn1);
}

static void
attach_zarm(void)
{
	utk_widget_attach(zarmbn0);
	utk_widget_attach(zarmbn1);
}

static void
switch_widget(int t)
{
	if (t == 0) {
		detach_zbypass();
		attach_zarm();
	} else {
		detach_zarm();
		attach_zbypass();
	}
	zuit = t;
}

static void
handle_zone_state(UtkWindow *win, int  z, int st)
{
	int  id;
	int  i;

	if (z < MAXZ) {
		id = utk_list_box_get_id(zlb, 0);
		i = zonedb[z].idx - id;
		if (i >= 0&&i < GLB_ROWS) {
			utk_widget_update_state(zst[i], st);
		}
		update_pzck_state(pnrofz(z), z, st);
	} else {
		i = st & 0xff;
		if (i > 0)
			showerr[z-MAXZ](i-1);
	}
}

//////////////////////////////////////////////////
static void
finish_zinp(void *arg)
{
	int z;

	if (strlen(zinq) == 0) return;
	z = strtol(zinq, NULL, 10);
	if (z == 0||(z > 256&&z <= 1000)||(z > 1256)) {
		show_zerrtip(znrv, zrlen, 2);
		return;
	}
	if (z > 1000) {
		z -= 1000;
		z += 256;
	}
	--z;

	if (zonedb[z].znr != z+1) {
		show_zerrtip(znrv, zrlen, 2);
		return;
	}

	utk_list_box_scroll_to(zlb, zonedb[z].idx);
	utk_list_box_select(zlb, 0);
	id_colum_set_base(&zidc, zonedb[z].idx);
	id_colum_update(&zidc);
	update_zst_col();
}

static int
unselect_zone(void *arg)
{
	if (selectedid > 0) {
		utk_list_box_unselect_all(zlb);
		selectedid = 0;
	}
	return 0;
}

static void
prepare_kbar(int i)
{
	if (i == 0) {
		keybar_ungrab(&zike.kbar);
	} else {
		keybar_grab(&zike.kbar);
	}
	kbarup = i;
}

/* DONE */
static void
build_zinp_entry(UtkWindow *parent)
{
	rect_t   r = {.x = 346, .y = 541, .w = 114, .h = 25};

	build_entry_kbar(&zike, parent, 470, 0);
	kbar_set_show(&zike, prepare_kbar);
	myent_new(&zient, parent, &r, 0);
	myent_add_caret(&zient, RGB_BLACK);
	myent_set_cache(&zient, zinq, 6, 4);
	myent_set_text_max(&zient, 4);
	myent_add_kbar(&zient, &zike);
	myent_set_callback(&zient, unselect_zone, finish_zinp, NULL, NULL);
}
////////////////////////////////////////////////////

static void
hide_before(void *arg)
{
	kbar_entry_cancel(&zike);
	if (selectedid > 0) {
		utk_list_box_unselect_all(zlb);
		selectedid = 0;
	}
	myent_clear(&zient);
}

static void
hide_kbar(void *arg)
{
	kbar_entry_cancel(&zike);
}

static void
unselect_all(void *arg)
{
//	if (selectedid > 0) {
		utk_list_box_unselect_all(zlb);
		selectedid = 0;
//	}
}

void
build_zone_gui(void)
{
	int   i;
	int   bw[] = {bn_zbar_w0, bn_zbar_w4, bn_zbar_w5};
	int   bx[] = {bn_zbar_x0, bn_zbar_x4, bn_zbar_x5};
	char *bg[] = {BN_RET136, BN_PU132, BN_PD131};

	char *bg0[] = {BN_ZZARM0, BN_ZZARM1};
	char *bg1[] = {BN_ZZDISARM0, BN_ZZDISARM1};
	char *bg2[] = {BN_ZBYPASS0, BN_ZBYPASS1};
	char *bg3[] = {BN_ZBYPASSR0, BN_ZBYPASSR1};

	char *bgx[] = {LBL_UNKNOWN, LBL_DISARM, LBL_NRDY, LBL_BYPASS, LBL_ARM, LBL_ALARM, LBL_ALARMR};
	rect_t  r = {.x = zid_col_x, .y = zid_col_y, .w = zid_col_w, .h = row_h};
	rect_t  fr = {.x = zarm_lb_x, .y = zarm_lb_y, .w = zarm_lb_w, .h = row_h};
	int   cx[4] = {0};
	int   co[4] = {0};
	int   cw[] = {330, 84, 136, 134};
	int   chr[] = {26, 4, 10, 8};
	long  off[] = {U_STRUCT_OFFSET(zone_t, zname), U_STRUCT_OFFSET(zone_t, zstr), 0, U_STRUCT_OFFSET(zone_t, dname)};
	int   fw[] = {8, 12};
	int   dw = 0;
	int   ft = ZLBX_FT;

	ztiplen_init();

	zonew = utk_window_derive();
#ifndef MAPDLL
	utk_widget_resident(zonew);
#endif
	utk_window_set_bg(zonew, BG_ZONEUI);
	utk_widget_async_evt_handler(zonew, handle_zone_state);
	utk_widget_show_before(zonew, hide_kbar, (void*)0);
	utk_widget_hide_before(zonew, hide_before, (void*)0);

	stb_new(zonew, show_main_win);

	for (i = 0; i < GLB_ROWS; ++i) {
		idbuf[i] = &zbuf[i][0];
	}
	
again:
	for (i = 0; i < ARR_SZ(cx); ++i) {
		co[i] = cw[i] - chr[i]*fw[ft];
		if (co[i] < 0) {
			if (ft > 0) {
				--ft;
				goto again;
			}
		}
		co[i] >>= 1;
	}

	for (i = 0; i < ARR_SZ(cx); ++i) {
		cx[i] = dw + co[i];
		dw += cw[i] + 2;
	}

	zlb = listbox_new(zonew, &fr, GLB_ROWS);
	listbox_set_font(zlb, ft);
	listbox_set_cols(zlb, ARR_SZ(cx), cx, chr, off);
	listbox_set_content(zlb, zonedb, sizeof(zone_t), MAXZ, 0);
	listbox_add_rows(zlb, selected_handle, GLB_ROWS);
	utk_list_box_set_hash_array(zlb, zarray, zarcnt);
	utk_list_box_set_style(zlb, LB_CHECKBOX);
	utk_list_box_set_text_clr(zlb, LBX_TXT_CLR);
	utk_list_box_set_col_cb(zlb, 2, show_ztp);

	build_label_col((void**)zst, zonew, 834, zid_col_y, row_h, GLB_ROWS, bgx, ARR_SZ(bgx));
	for (i = 0; i < GLB_ROWS; ++i) {
		int  st;
		if (i < zarcnt) {
			st = (int)get_zone_state_direct(zarray[i]);
			utk_widget_set_state(zst[i], st);
		} else {
			utk_widget_set_state(zst[i], -1);
		}
	}

	build_zinp_entry(zonew);
	id_colum_init(&zidc, GLB_ROWS, zarcnt, 4);
	id_colum_new(&zidc, zonew, idbuf, &r, id_col_sel);

	////////////////////////////////////////////////////////////
	build_button_array2(zonew, ARR_SZ(bw), bn_bar_h, bn_bar_y, bw, bx, do_zopt, bg);
	build_tip_lbl(&zoptip, zonew, (LCD_W-tip_w)/2, (LCD_H-tip_h)/2, BG_TIP, 0);
	tip_set_hide_cb(&zoptip, (void*)after_opr, (void *)0);
	build_tip_lbl(&prterr, zonew, (LCD_W-tip_w)/2, (LCD_H-tip_h)/2, LBL_PRTERR, 1);
	tip_set_hide_cb(&prterr, (void*)unselect_all, (void *)0);

	zarmbn0 = button_new0(zonew, bg0, bn_zbar_x2, bn_bar_y, do_zopr, (void*)0);
	zarmbn1 = button_new0(zonew, bg1, bn_zbar_x3, bn_bar_y, do_zopr, (void*)1);
	
	zbpbn0 = button_new0(zonew, bg2, zbar_x2, bn_bar_y, do_zopr, (void*)2);
	zbpbn1 = button_new0(zonew, bg3, zbar_x3, bn_bar_y, do_zopr, (void*)3);

	detach_zbypass();
}

/* t: 0-zone arm, 1-zone bypass */
void
show_zone_gui(int t)
{
	if (utk_window_top() != zonew) {
		if (zuit != t) {
			switch_widget(t);
		}
		utk_window_show(zonew);
	}
}

/* called when zone state changed, zi: -1-all zones. */
void
update_zoneui_state(int zi, int st)
{
	utk_widget_send_async_event(zonew, zi, st);
}

/* called by zmgr.c when add or delete the zone. */
void
sync_zone_ui(void)
{
	utk_list_box_reload_item(zlb, zarcnt);
	id_colum_reset_base(&zidc);
	id_colum_set_maxid(&zidc, zarcnt);
	id_colum_update(&zidc);
	update_zst_col();
}

void
zopr_return(int op, int z, int r)
{
	utk_widget_send_async_event(zonew, MAXZ+op, (z<<8)|r);
}



