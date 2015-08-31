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
#include "config.h"
#include "gui.h"
#include "stb.h"
#include "zopt.h"
#include "pzone.h"


static UtkWindow   *pzwin;
static UtkWindow   *zst[GLB_ROWS];
static UtkWindow   *from;

static UtkListBox  *zlb;
static id_col_t     zidc;
static char         zbuf[GLB_ROWS][4];
static char        *idbuf[GLB_ROWS];

static int          curpz = -1;
static int         *pzinf;
static int          pzic = 0;

static void
load_pzinf(int p)
{
	int  id;
	int  st;
	int  i;
	int  maxid;
	int  maxr;

	pzic = 0;
	pzinf = pzinfo_get(p, &pzic);
	DBG("--- pzone-%d has %d zones ---\n", p+1, pzic);
	if (pzic == 0||pzinf == NULL) return;

	curpz = p;
	utk_list_box_set_hash_array(zlb, pzinf, pzic);
	id_colum_set_maxid(&zidc, pzic);
	id_colum_reset_base(&zidc);
	id_colum_update(&zidc);

	id = utk_list_box_get_id(zlb, 0);
	maxr = id + GLB_ROWS;
	if (id >= pzic) {
		maxid = id;
	} else if (pzic <= maxr) {
		maxid = pzic;
	} else {
		maxid = maxr;
	}

	for (i = id; i < maxid; ++i) {
		st = (int)get_zone_state_direct(pzinf[i]);
		utk_widget_update_state(zst[i-id], st);
	}

	for (i = maxid; i < maxr; ++i) {
		utk_widget_update_state(zst[i-id], -1);
	}
}

////////////////////////////////////////////////
/* called when scroll listbox */
static void
update_pzst_col(void)
{
	int  id;
	int  st;
	int  i;
	int  j;
	int  maxid;
	int  maxr;

	id = utk_list_box_get_id(zlb, 0);
	maxr = id + GLB_ROWS;
	if (id >= pzic) {
		maxid = id;
	} else if (pzic <= maxr) {
		maxid = pzic;
	} else {
		maxid = maxr;
	}

	for (i = id; i < maxid; ++i) {
		j = i - id;
		st = (int)get_zone_state_direct(pzinf[i]);
		utk_widget_update_state(zst[j], st);
		DBG("--- show zst-%d = %d ---\n", j, st);
	}

	for (i = maxid; i < maxr; ++i) {
		j = i - id;
		utk_widget_set_state(zst[j], -1);
		utk_widget_hide(zst[j]);
		DBG("--- hide zst-%d ---\n", j);
	}
}

static void 
pz_pageup(void)
{
	if (utk_list_box_scroll_pageup(zlb)) {
		id_colum_pageup(&zidc);
		update_pzst_col();
	}
}

static void
pz_pagedn(void)
{
	if (utk_list_box_scroll_pagedn(zlb)) {
		id_colum_pagedn(&zidc);
		update_pzst_col();
	}
}

static void
go_back(void)
{
	utk_window_show(from);
}

static PVOID  bn_cb[] = {go_back, pz_pageup, pz_pagedn};

static void
do_zopt(void *data)
{
	int i = (int)data;
	bn_cb[i]();
}

/////////////////////////////////////////

static void
quit_pzck(void *data)
{
	curpz = -1;
}

static void
handle_pzck_state(UtkWindow *w, int x, int y)
{
	if ((x > 0&&x-1 == curpz)||(x == 0&&curpz >= 0)) {
		update_pzst_col();
	}
}

void
build_pzck_gui(void)
{
	int   i;
	int   bw[] = {184, 182, 182};
	int   bx[] = {420, 420+184, 420+184+182};
	char *bg[] = {BN_RET184, BN_PU182, BN_PD182};
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

	pzwin = utk_window_derive();
#ifndef MAPDLL
	utk_widget_resident(pzwin);
#endif
	utk_window_set_bg(pzwin, BG_PZCK);

	utk_widget_async_evt_handler(pzwin, handle_pzck_state);
	utk_widget_hide_before(pzwin, quit_pzck, (void*)0);

	stb_new(pzwin, show_main_win);

	for (i = 0; i < GLB_ROWS; ++i) {
		idbuf[i] = &zbuf[i][0];
	}

	id_colum_init(&zidc, GLB_ROWS, 0, 4);
	id_colum_new(&zidc, pzwin, idbuf, &r, NULL);

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

	zlb = listbox_new(pzwin, &fr, GLB_ROWS);
	listbox_set_font(zlb, 1);
	listbox_set_cols(zlb, ARR_SZ(cx), cx, chr, off);
	listbox_set_content(zlb, zonedb, sizeof(zone_t), MAXZ, 0);
	listbox_add_rows(zlb, NULL, GLB_ROWS);
	utk_list_box_set_text_clr(zlb, LBX_TXT_CLR);
	utk_list_box_set_col_cb(zlb, 2, show_ztp);
	utk_widget_disable((UtkWindow*)zlb);

	build_label_col((void**)zst, pzwin, 834, zid_col_y, row_h, GLB_ROWS, bgx, ARR_SZ(bgx));
	for (i = 0; i < GLB_ROWS; ++i) {
		utk_widget_set_state(zst[i], -1);
	}

	////////////////////////////////////////////////////////////
	build_button_array2(pzwin, ARR_SZ(bw), bn_bar_h, bn_bar_y, bw, bx, do_zopt, bg);
}

/* p: [0~MAXPZ) */
void
show_pzck_gui(int p)
{
	from = utk_window_top();
	load_pzinf(p);
	utk_window_show(pzwin);
}

void
update_pzck_pstat(int p)
{
	utk_widget_send_async_event(pzwin, p, 0);
}

void
update_pzck_state(int p, int z, int st)
{
	int  id;
	int  i;

	if (curpz == p) {
		id = utk_list_box_get_id(zlb, 0);
		i = zonedb[z].pidx;
		DBG("--- zone-%d: pidx = %d ---\n", z, i);
		if (i < id) return;
		i -= id;
		if (i < GLB_ROWS) {
			utk_widget_update_state(zst[i], st);
		}
	}
}



