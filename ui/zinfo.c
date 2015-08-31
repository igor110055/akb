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

static UtkWindow   *zwin;
static UtkListBox  *slb;
static id_col_t     zidc;
static char         zbuf[GLB_ROWS][4];
static char        *idbuf[GLB_ROWS];

static UtkWindow   *zarrowz;
static UtkWindow   *zarrowp;
static int  zarrst = 0;
static int  parrst = 0;

static int  zinf[MAXZ];
static int  zicnt = 0;

static int  zpinf[MAXZ];  //used for sorint result
static int  zp = 0;

static void
reverse_array(int *arr, int n)
{
	int  i;
	int  tmp;
	
	for (i = 0; i < (n>>1); ++i) {
		tmp = arr[i];
		arr[i] = arr[n-1-i];
		arr[n-1-i] = tmp; 
	}
}

static inline void
reload_listbox(void)
{
	utk_list_box_reload_item(slb, zicnt);
	id_colum_reset_base(&zidc);
	id_colum_set_maxid(&zidc, zicnt);
	id_colum_update(&zidc);
}

static void
scroll_up(void* data)
{
	if (utk_list_box_scroll_pageup(slb)) {
		id_colum_pageup(&zidc);
	}
}

static void
scroll_down(void* data)
{
	if (utk_list_box_scroll_pagedn(slb)) {
		id_colum_pagedn(&zidc);
	}
}

static void
sort_by_znr(void *data)
{
	reverse_array(zinf, zicnt);
	if (zp == 1) {
		utk_list_box_set_hash_array(slb, zinf, zicnt);
		zp = 0;
	}
	reload_listbox();
	zarrst ^= 1;
	utk_widget_update_state(zarrowz, zarrst);
}

static void
sort_by_pnr(void *data)
{
	reverse_array(zpinf, zicnt);
	if (zp == 0) {
		utk_list_box_set_hash_array(slb, zpinf, zicnt);
		zp = 1;
	}
	reload_listbox();
	parrst ^= 1;
	utk_widget_update_state(zarrowp, parrst);
}

static void
id_col_sel(void *data)
{
	int i = (int)data;
	utk_list_box_toggle_item((UtkWindow*)slb, i);
}

static void
inc_arr_bypnr(int *out, int *in, int n)
{
	int  i;
	int  j;
	int  k;

	out[0] = in[0];
	j = 1;
	for (i = 1; i < n; ++i) {
		if (zonedb[in[i]].pnr >= zonedb[out[j-1]].pnr) {
			out[j] = in[i];
		} else {
			for (k = 0; k < j; ++k) {
				if (zonedb[in[i]].pnr < zonedb[out[k]].pnr)
					break;
			}
			memmove(&out[k+1], &out[k], (j-k)*sizeof(int));
			out[k] = in[i];
		}
		++j;
	}
}

static void
_load_zinf(void)
{
	if (zarcnt > 0) {
		memcpy(zinf, zarray, zarcnt*sizeof(int));
		inc_arr_bypnr(zpinf, zarray, zarcnt);
	} else {
		memset(zinf, 0, MAXZ*sizeof(int));
		memset(zpinf, 0, MAXZ*sizeof(int));
	}
	zicnt = zarcnt;
}

void
build_zinfo_gui(void)
{
	int  i;
	rect_t  r  = {.x = bn_ret_x, .y = bn_bar_y + 1, .w = bn_ret_w, .h = bn_bar_h};
	rect_t  rm = {.x = ZILB_X, .y = 67, .w = 110, .h = row_h};
	rect_t	rid = {.x = ziid_col_x, .y = ziid_col_y, .w = ziid_col_w, .h = row_h};
	rect_t	fr = {.x = ZILB_X, .y = ZILB_Y, .w = ZILB_W, .h = row_h};

	int   cx[5] = {0};
	int   co[5] = {0};
	int   cw[] = {110, 330, 110, 126, 126};
	int   chr[] = {4, 26, 2, 10, 8};
	long  off[] = {U_STRUCT_OFFSET(zone_t, zstr), U_STRUCT_OFFSET(zone_t, zname), U_STRUCT_OFFSET(zone_t, pstr), 0, U_STRUCT_OFFSET(zone_t, dname)};
	int   fw[] = {8, 12};
	int   dw = 0;
	int   ft = ZLBX_FT;

	_load_zinf();

	zwin = utk_window_derive();
#ifndef MAPDLL
	utk_widget_resident(zwin);
#endif

	utk_window_set_bg(zwin, BG_ZINFO);

	stb_new(zwin, show_main_win);

	zarrowz = utk_label_new();
	utk_label_set_bg(zarrowz, ARROW_UP);
	utk_label_set_bg(zarrowz, ARROW_DN);
	utk_widget_add_cntl(zwin, zarrowz, zarrow_x, arrow_y);

	zarrowp = utk_label_new();
	utk_label_set_bg(zarrowp, ARROW_UP);
	utk_label_set_bg(zarrowp, ARROW_DN);
	utk_widget_add_cntl(zwin, zarrowp, parrow_x, arrow_y);

	map_new(zwin, &rm, sort_by_znr, NULL, NULL);
	rm.x = 602;
	map_new(zwin, &rm, sort_by_pnr, NULL, NULL);

	for (i = 0; i < GLB_ROWS; ++i) {
		idbuf[i] = &zbuf[i][0];
	}

	id_colum_init(&zidc, GLB_ROWS, zarcnt, 4);
	id_colum_new(&zidc, zwin, idbuf, &rid, id_col_sel);

again:
	for (i = 0; i < 5; ++i) {
		co[i] = cw[i] - chr[i]*fw[ft];
		if (co[i] < 0) {
			if (ft > 0) {
				DBG("--- not enough space in %d cols ---\n", i+1);
				--ft;
				goto again;
			}
		}
		co[i] >>= 1;
	}
	
	for (i = 0; i < 5; ++i) {
		cx[i] = dw + co[i];
		dw += cw[i] + 2;
	}

	slb = listbox_new(zwin, &fr, GLB_ROWS);
	listbox_set_font(slb, ft);
	listbox_set_cols(slb, ARR_SZ(cx), cx, chr, off);
	listbox_set_content(slb, zonedb, sizeof(zone_t), MAXZ, 0);
	listbox_add_rows(slb, NULL, GLB_ROWS);
	utk_list_box_set_hash_array(slb, zinf, zicnt);
	utk_list_box_set_text_clr(slb, LBX_TXT_CLR);
	utk_list_box_set_col_cb(slb, 3, show_ztp);
	utk_widget_disable((UtkWindow*)slb);

	////////////////////////////////////////////////////////////
	button_new(zwin, BN_RET184, &r, (void *)go_home, (void*)0);

	r.w = bn_scroll_w;
	r.x = bn_up_x;
	button_new(zwin, BN_PU182, &r, (void *)scroll_up, (void*)0);

	r.x = bn_dn_x;
	button_new(zwin, BN_PD182, &r, (void *)scroll_down, (void*)0);
}

void
show_zinfo_win(void)
{
	if (utk_window_top() != zwin) {
		utk_window_show(zwin);
	}
}

void
load_zinf(void)
{
	_load_zinf();
	reload_listbox();
}


