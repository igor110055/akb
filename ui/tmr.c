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
#include "config.h"
#include "layout.h"
#include "gui.h"
#include "stb.h"
#include "zopt.h"
#include "pztimer.h"

static UtkWindow   *tmrw = NULL;
static UtkWindow   *pst[MAXTPZ] = {NULL};
static UtkListBox  *plb = NULL;
static UtkWindow   *psel[MAXTPZ] = {NULL};

int    tpzarr[MAXTPZ];
int    tpzcnt = 0;
static int    tpzidx[MAXPZ] = {0};

static void
update_tpst_col(void)
{
	int  st;
	int  i;

	for (i = 0; i < MAXTPZ; ++i) {
		if (i < tpzcnt) {
			st = (int)get_pzone_state_direct(tpzarr[i]+1);
			DBG("--- pzone-%d: st = %d ---\n", tpzarr[i]+1, st);
			utk_widget_update_state(pst[i], st<<1);
			DBG("--- pst[%d] = %p ---\n", i, pst[i]);
		} else {
			DBG("--- hide pst[%d] ---\n", i);
			utk_widget_hide(pst[i]);
		}
	}
}

static void
update_psel_col(void)
{
	int  i;
	int  st;

	for (i = 0; i < MAXTPZ; ++i) {
		if (i >= tpzcnt) {
			utk_widget_set_state(psel[i], -1);
			utk_widget_hide(psel[i]);
		} else {
			st = pztimer_enabled(tpzarr[i]);
			utk_widget_update_state(psel[i], !!st);
		}
	}
}

void
tmrui_clr_content(int ud)
{
	CLEARA(tpzarr);
	CLEARA(tpzidx);
	tpzcnt = 0;

	if (ud != 0) {
		if (plb) {
			utk_list_box_clear_all((UtkWindow*)plb);
		}
		reload_tmrck_ui();
		update_tpst_col();
		update_psel_col();
	}
}

/* called in sys_tmr.c when save or load the configuration. */
void
tmrui_set_content(void *p, int ud)
{
	int  i;
	unsigned char nr;
	pztm_t *pzt = p;

	CLEARA(tpzarr);
	CLEARA(tpzidx);
	tpzcnt = pzt->npzt;
	for (i = 0; i < tpzcnt; ++i) {
		nr = pzt->pz[i];
		tpzarr[i] = (int)nr-1;
		tpzidx[tpzarr[i]] = i+1;
		DBG("--- timer pzone = %d, idx = %d ---\n", nr, i);
	}
	DBG("--- total timer pzone = %d ---\n", tpzcnt);

	if (ud != 0) {
		if (plb) {
			utk_list_box_reload_item(plb, tpzcnt);
		}
		reload_tmrck_ui();
		update_tpst_col();
		update_psel_col();
	}
}

static void 
go_back(void *data)
{
	show_arment_win();
}

static void
go_tmrchk(void *data)
{
	show_tmrck_win();
}

static void
tmr_onoff(void *data)
{
	int  i = (int)data;
	int  st;

	if (i >= tpzcnt) return;
	st = utk_widget_state(psel[i]);
	st ^= 1;
	utk_widget_update_state(psel[i], st);
	if (st == 0) {
		DBG("--- disable tmr pzone-%d: st = %d ---\n", tpzarr[i]+1, st);
		disable_pzone_timer(i);
	} else {
		DBG("--- enable tmr pzone-%d: st = %d ---\n", tpzarr[i]+1, st);
		enable_pzone_timer(i);
	}
}

static void
check_tzonest(void *arg)
{
	int  i = (int)arg;

	if (i < tpzcnt) {
		show_pzck_gui(tpzarr[i]);
	}
}

static void
get_pzst(void *arg)
{
	int  i;
	int  st;

	for (i = 0; i < tpzcnt; ++i) {
		st = (int)get_pzone_state_direct(tpzarr[i]+1);
		utk_widget_set_state(pst[i], st<<1);
		st = pztimer_enabled(tpzarr[i]);
		utk_widget_set_state(psel[i], !!st);
	}

	for (i = tpzcnt; i < MAXTPZ; ++i) {
		utk_widget_set_state(pst[i], -1);
		utk_widget_set_state(psel[i], -1);
	}
}

static void
update_tpz(UtkWindow *win, int  i, int st)
{
	if (i < MAXTPZ) {
		if (utk_widget_state(psel[i]) != st)
			utk_widget_update_state(psel[i], st);
	} else {
		i -= MAXTPZ;
		DBG("--- pzone-%d: idx = %d, st = %d ---\n", i, tpzidx[i]-1, st);
		if (tpzidx[i] > 0) {
			utk_widget_update_state(pst[tpzidx[i]-1], st<<1);
		}
	}
}

void
build_tmr_gui(void)
{
	int   i;
	char *bgs[] = {ICO_UNSEL, ICO_SEL};

	rect_t  fr = {.x = 178, .y = 107, .w = 610, .h = row_h};
	int   cx[] = {(118-20)/2, (490-300)/2+120};
	int   chr[] = {2, 30};
	long  off[] = {U_STRUCT_OFFSET(pzone_t, pstr), U_STRUCT_OFFSET(pzone_t, pname)};
	rect_t  r   = {.x = bn_tret_x, .y = bn_tbar_y, .w = bn_tret_w, .h = bn_tbar_h};
	rect_t  rm  = {.x = 56, .y = 107, .w = 120, .h = 40};
	int     st;

	tmrw = utk_window_derive();
#ifndef MAPDLL
	utk_widget_resident(tmrw);
#endif
	utk_window_set_bg(tmrw, BG_TMR);
	utk_widget_show_before(tmrw, get_pzst, 0);
	utk_widget_async_evt_handler(tmrw, update_tpz);

	stb_new(tmrw, show_main_win);
	build_label_col((void**)psel, tmrw, 92, (40-24)/2+107, row_h, MAXTPZ, bgs, ARR_SZ(bgs));
	for (i = 0; i < MAXTPZ; ++i) {
		if (i >= tpzcnt) {
			utk_widget_set_state(psel[i], -1);
		} else {
			st = (pztimer_enabled(tpzarr[i]) > 0) ? 1 : 0;
			utk_widget_set_state(psel[i], st);
		}
	}

	build_map_col(tmrw, &rm, MAXTPZ, tmr_onoff);

	plb = listbox_new(tmrw, &fr, MAXTPZ);
	listbox_set_font(plb, 1);
	listbox_set_cols(plb, ARR_SZ(cx), cx, chr, off);
	listbox_set_content(plb, pzone, sizeof(pzone_t), MAXTPZ, 0);
	listbox_add_rows(plb, NULL, MAXTPZ);
	utk_list_box_set_hash_array(plb, tpzarr, tpzcnt);
	utk_widget_disable((UtkWindow*)plb);
	utk_list_box_set_text_clr(plb, LBX_TXT_CLR);

	build_mstbn_col((void**)pst, tmrw, pst_col_x, zid_col_y, MAXTPZ, check_tzonest);
	for (i = 0; i < MAXTPZ; ++i) {
		if (i >= tpzcnt) {
			utk_widget_set_state(pst[i], -1);
		} else {
			st = (int)get_pzone_state_direct(tpzarr[i]+1);
			utk_widget_set_state(pst[i], st<<1);
		}
	}

	////////////////////////////////////////////////////////////
	button_new(tmrw, BN_RET182, &r, (void *)go_back, (void*)0);

	r.x = bn_tmrck_x;
	r.w = bn_tmrck_w;
	button_new(tmrw, BN_TMRCK, &r, (void *)go_tmrchk, (void*)0);
}

void
show_tmr_win(void)
{
	DBG("--- tmrw = %p ---\n", tmrw);
	if (utk_window_top() != tmrw) {
		utk_window_show(tmrw);
	}
}

void
update_tmrpz_ui(int p, int st)
{
	if (tpzidx[p] > 0) {
		utk_widget_send_async_event(tmrw, p+MAXTPZ, st);
	}
}

void
set_tpz_flag(int i, int st)
{
	utk_widget_send_async_event(tmrw, i, st);
}


