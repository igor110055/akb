#include <arpa/inet.h>
#include <string.h>
#include "ulib.h"
#include "utk.h"
#include "utksignal.h"
#include "utkwindow.h"
#include "utkbutton.h"
#include "utklabel.h"
#include "utkmap.h"
#include "utkentry.h"
#include "layout.h"
#include "stb.h"
#include "gui.h"
#include "config.h"
#include "zopt.h"
#include "qtimer.h"


static UtkWindow  *arwin;
static UtkWindow  *cbx[4];
static kbar_ent_t  ake;
static myent_t     zent;
static tip_t       zptip;

static char  aznr[6] = {0};
static char  g_aznr[6] = {0};

static char  acx = 0;

static PVOID  sa_cb[] = {show_sysnet_win, show_sysuart_win, show_sysgen_win, show_systmr_win};

static void
simulate_alarm(void)
{
	int  v;
	int  z;
	int  id;
#ifdef DEBUG
	char  *sopt[] = {"²¼·À", "³··À", "±¨¾¯", "±¨¾¯»Ö¸´"};
#endif

	v = atoi(aznr);
	id = v / 100;
	z  = v % 100;
	if (id == 0||id > MAXHOST||z == 0) {
		show_tip(&zptip);
		return;
	}

	v = (id - 1)*99 + z - 1;
	if (zonedb[v].znr != v + 1) {
		show_tip(&zptip);
		return;
	}

	if ((acx>>2)&1) {
		DBG("--- %sºÅ·ÀÇø: %s ---\n", aznr, sopt[2]);
		simul_alert(v);
	} else if ((acx>>3)&1) {
		DBG("--- %sºÅ·ÀÇø: %s ---\n", aznr, sopt[3]);
		simul_alertr(v);
	}
}

static void
sysalr_save(void *data)
{
	if (kbar_entry_save_prepared(&ake) == 0) return;
	simulate_alarm();
}

static void
sysalr_cancel(void *data)
{
	int  i;

	kbar_entry_cancel(&ake);
	for (i = 0; i < 4; ++i) {
		if ((acx>>i)&1) {
			utk_widget_update_state(cbx[i], 0);
		}
	}
	acx = 0;
}

static void
sysf_anav(void *data)
{
	sa_cb[(int)data]();
}

static inline void
cancel_sel(int i)
{
	if (utk_widget_state(cbx[i]) != 0) {
		utk_widget_update_state(cbx[i], 0);
	}
	acx &= ~(1<<i);
}

static void
select_opt(void *data)
{
	int  i = (int)data;

	if (utk_widget_state(cbx[i]) == 0) {
		acx &= ~(1<<i);
		return;
	}
	switch (i) {
	case 0:
		cancel_sel(1);
		acx |= (1<<0);
		break;
	case 1:
		cancel_sel(0);
		cancel_sel(2);
		cancel_sel(3);
		acx |= (1<<1);
		break;
	case 2:
		cancel_sel(3);
		acx |= (1<<2);
		break;
	case 3:
		cancel_sel(2);
		acx |= (1<<3);
		break;
	default:
		break;
	}
}

void
build_sysalr_gui(void)
{
	int  bw[] = {sys_w0, sys_w1, sys_w2, sys_w3};
	int  bx[] = {sys_x0, sys_x1, sys_x2, sys_x3};
	rect_t  r = {.x = bn_gret_x, .y = bn_sys_y, .w = gret_w, .h = bn_sys_h};
	rect_t  r0 = {.x = bn_sv_x, .y = bn_sys_y, .w = bn_sys_w, .h = bn_sys_h};
	rect_t  re = {.x = azent_x, .y = azent_y, .w = azent_w, .h = azent_h};

	arwin = utk_window_derive();
#ifndef MAPDLL
	utk_widget_resident(arwin);
#endif
	utk_window_set_bg(arwin, BG_SYSALR);
	utk_widget_hide_before(arwin, sysalr_cancel, NULL);

	register_qtimer((void*)arwin);

	stb_new(arwin, sysf_quit);
	build_map_array(arwin, ARR_SZ(bx), bw, bx, sys_y, sys_h, sysf_anav);
	build_tip_lbl(&zptip, arwin, (LCD_W-tip_w)/2, (LCD_H-tip_h)/2, LBL_UNDEFZ, 2);
	
	build_entry_kbar(&ake, arwin, sysalr_kbar_y, 0);
	DBG("--- alr entry_kbar = %p ---\n", &ake);

	myent_new(&zent, arwin, &re, 0);
	myent_add_caret(&zent, RGB_BLACK);
	myent_set_cache(&zent, aznr, sizeof(aznr), 4);
	myent_set_buffer(&zent, g_aznr, sizeof(g_aznr));
	myent_add_kbar(&zent, &ake);

	cbx[0] = build_checkbox(arwin, sysalr_cbox_x0, sysalr_cbox_y0, select_opt, (void*)0);
	cbx[1] = build_checkbox(arwin, sysalr_cbox_x1, sysalr_cbox_y0, select_opt, (void*)1);
	cbx[2] = build_checkbox(arwin, sysalr_cbox_x2, sysalr_cbox_y1, select_opt, (void*)2);
	cbx[3] = build_checkbox(arwin, sysalr_cbox_x3, sysalr_cbox_y1, select_opt, (void*)3);
	
	button_new(arwin, BN_RET1501, &r, go_home, (void*)1);
	button_new(arwin, BN_SND, &r0, (void *)sysalr_save, (void *)0);

	r0.x = bn_cancel_x;
	button_new(arwin, BN_CL150, &r0, (void *)sysalr_cancel, (void *)0);
}

/* DONE */
void
show_sysalr_win()
{
	if (utk_window_top() != arwin) {
		utk_window_show(arwin);
	}
}


