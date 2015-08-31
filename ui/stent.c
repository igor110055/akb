#include "ulib.h"
#include "mxc_sdk.h"
#include "utksignal.h"
#include "utkbutton.h"
#include "utkmstbutton.h"
#include "utkspinbutton.h"
#include "utklabel.h"
#include "utkclock.h"
#include "layout.h"
#include "gui.h"
#include "stb.h"

//////////////////////////////////////////
#define  BG_ST        "/opt/ui/bg_status.jpg"
#define  LBL_SYST     "/opt/ui/lbl_syst1.png"
#define  LBL_PZST     "/opt/ui/lbl_pzst1.png"

#define  st_w         160
#define  st_h         46
#define  st_x         342
#define  st_y0        271
#define  st_y1        (st_y0 + 76)

#define  BN_CK1       "/opt/ui/bn_ck1.png"
#define  bn_ck_w      118
#define  bn_ck_h      46

#define  bn_ck_x       563
#define  bn_ck_y0      st_y0
#define  bn_ck_y1      st_y1

#define  bn_ckret_x    (1024-(156+56))
#define  bn_ckret_y    (600-66)

///////////////////////////////////////////

static UtkWindow  *stwin;
static UtkWindow  *st0;
static UtkWindow  *st1;

static void
check_curstat(void *data)
{
	int i = (int)data;

	if (i == 0) {
		show_syst_win();
	} else {
		show_pzone_gui(2);
	}
}

static void
handle_nreadyout(UtkWindow *win, int  x, int y)
{
	if (win != stwin) return;
	if (utk_widget_state(st1) != x) {
		utk_widget_update_state(st1, x);
	}
}

void
build_stent_gui(void)
{
	rect_t  rt0 = {.x = st_x, .y = st_y0, .w = st_w, .h = st_h};
	rect_t  rt1 = {.x = st_x, .y = st_y1, .w = st_w, .h = st_h};
	rect_t  r0 = {.x = bn_ck_x, .y = bn_ck_y0, .w = bn_ck_w, .h = bn_ck_h};
	rect_t  r1 = {.x = bn_ck_x, .y = bn_ck_y1, .w = bn_ck_w, .h = bn_ck_h};
	rect_t  r2 = {.x = bn_ckret_x, .y = bn_ckret_y, .w = 156, .h = 42};

	stwin = utk_window_derive();
#ifndef MAPDLL
	utk_widget_resident(stwin);
#endif
	utk_window_set_bg(stwin, BG_ST);
	utk_widget_async_evt_handler(stwin, handle_nreadyout);
	
	stb_new(stwin, show_main_win);

	st0 = label_new(stwin, LBL_SYST, NULL, 0, &rt0);
	st1 = label_new(stwin, LBL_PZST, NULL, 0, &rt1);
//	utk_widget_set_state(st0, 1);

	button_new(stwin, BN_CK1, &r0, (void*)check_curstat, (void*)0);
	button_new(stwin, BN_CK1, &r1, (void*)check_curstat, (void*)1);
	button_new(stwin, BN_RET156, &r2, (void*)go_home, (void*)0);
}

/* note: reentrance problem */
/* called by other thread */
void
show_stent_win(void)
{
	if (utk_window_top() != stwin)
	{
		utk_window_show(stwin);
	}
}

/* called by UI thread in syst.c */
void
update_syst_label(int st)
{
	DBG("--- update label to %d ---\n", st);
	if (utk_widget_state(st0) != st) {
		utk_widget_update_state(st0, st);
	}
}

/* called by zopt thread in zopt.c, when nready in or out. */
void
update_pzst_label(int st)
{
	utk_widget_send_async_event(stwin, st, 0);
}

void
clear_pzst_label(void)
{
	if (utk_widget_state(st1) != 0) {
		utk_widget_update_state(st1, 0);
	}
}


