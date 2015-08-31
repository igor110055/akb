#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "ulib.h"
#include "utksignal.h"
#include "utkbutton.h"
#include "utkentry.h"
#include "utkfont.h"
#include "layout.h"
#include "mxc_sdk.h"
#include "kb_ui.h"
#include "gui.h"
#include "stb.h"
#include "config.h"

static UtkWindow *bwin;
static UtkEntry  *pent;
static UtkWindow *go[4];

static char      pbuf[18];

static tip_t     pwderr;
static my_kb_t  *my_kb;
static char  kval[12] = {'0','1','2','3','4','5','6','7','8','9','.','#'};

static void
key_pressed(void *data)
{
	int  i = (int)data;
	utk_entry_add_char(pent, kval[i]);
}

static void
del_entry(void *data)
{
	utk_entry_del_char(pent);
}

static void
show_zbypass_win(void)
{
	show_zone_gui(1);
}

static void
show_bypass_win(void)
{
	show_pzone_gui(1);
}


static PVOID bp_cb[] = {show_zbypass_win, show_bypass_win};

static void
select_bpopt(void *data)
{
	int i = (int)data;

	if (!passwd_verify(pbuf)) {
		utk_entry_clear_text(pent);
		utk_entry_update((UtkWindow *)pent);
		show_tip(&pwderr);
		return;
	}

	utk_entry_clear_text(pent);
	utk_entry_update((UtkWindow *)pent);

	if (priority_get(PRI_BP) == 0) {
//		show_tip(&prierr);
		return;
	}

	bp_cb[i>>1]();
}

static void
clear_inpent(void *arg)
{
	utk_entry_clear_text(pent);
}

void
build_bpent_gui(void)
{
	kb_para_t   par;
	int    i;
	rect_t r = {.x = bn_del_x, .y = bn_del_y, .w = bn_del_w, .h = bn_del_h};
	rect_t r0 = {.x = pwd_ent_x, .y = pwd_ent_y, .w = pwd_ent_w, .h = pwd_ent_h};
	int    x[] = {bp_bn_x0, bp_bn_x1};
	int    y[] = {bp_bn_y0, bp_bn_y1};
	int    w[] = {bp_bn_w0, bp_bn_w1};
	char *bg[] = {BN_ZBPASS, BN_ZBPASSR, BN_PBPASS, BN_PBPASSR};

	bwin = utk_window_derive();
#ifndef MAPDLL
	utk_widget_resident(bwin);
#endif
	utk_window_set_bg(bwin, BG_BPENT);
	utk_widget_show_before(bwin, clear_inpent, NULL);
	
	stb_new(bwin, show_main_win);
	
	build_tip_lbl(&pwderr, bwin, (LCD_W-tip_w)/2, (LCD_H-tip_h)/2, LBL_PWDERR, 2);
	pent = build_passwd_entry(bwin, 0, &r0, pbuf, 18, RGB_BLACK);
	button_new(bwin, BN_DEL1, &r, del_entry, (void*)NULL);

	/***** keyboard *****/
	kb_para_init(&par, key_pressed, bp_kb_x, bp_kb_y, 1);
	my_kb = kb_new((void*)bwin, &par);

	/***** side menu buttion *****/
	r.h = bp_bn_h;
	for (i = 0; i < 4; ++i) {
		r.w = w[i&1];
		r.x = x[i&1];
		r.y = y[i>>1];
		go[i] = button_new(bwin, bg[i], &r, (void *)select_bpopt, (void*)i);
	}
}

/* note: reentrance problem */
/* called by other thread */
void
show_bpent_win(void)
{
	if (utk_window_top() != bwin)
	{
		CLEARA(pbuf);
		utk_window_show(bwin);
	}
}


