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
#include "zopt.h"

static UtkWindow *awin;
static UtkEntry  *pent;
static UtkWindow *go[8];

static UtkWindow *fdialog;

static tip_t     armed;
static tip_t     disarmed;
//static tip_t     nzone;

static char  pbuf[18] = {0};

static tip_t     pwderr;
static tip_t     aaok;
static tip_t     adok;
static tip_t     prterr;
static tip_t     fatip;

static my_kb_t  *my_kb;
static char  kval[12] = {'0','1','2','3','4','5','6','7','8','9','.','#'};

static char  *fae[] = {"强制布防成功", "强制布防失败"};
static int    faelen[2];
static int    fares = 1;

static char  *sysalt = "系统报警,无法布防!";
static int    salen = 0;

static char  *armf = "系统布防失败!";
static int    aflen = 0;

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
show_zarm_win(void)
{
	show_zone_gui(0);
}

static void
show_ptarm_win(void)
{
	show_pzone_gui(0);
}

static void
clear_inpent(void *arg)
{
	utk_entry_clear_text(pent);
}

static void
all_arm_opt(void)
{
	if (pz_get(0)) {
		pzone_arm(0);
	} else {
		show_tip(&prterr);
	}
}

static void
all_disarm_opt(void)
{
	if (pz_get(0)) {
		pzone_disarm(0);
	} else {
		show_tip(&prterr);
	}
}

static void
handle_amres(UtkWindow *win, int  op, int r)
{
	switch (op) {
	case 0: //all disarm result.
		if (r == 0) {
			show_tip(&adok);
		} else {
			show_tip(&disarmed);
		}
		break;
	case 1: //all arm result.
		if (r == 0) {
			show_tip(&aaok);
		} else if (r == PZ_ALRT) {
			fares = 1;
			show_strtip(&fatip, sysalt, salen, 2, tip_w, qry_tip_y);
		} else if (r == ALLARMED) {
			show_tip(&armed);
		} else if (r == PZ_SYSF) {
			show_model_dialog(fdialog);
		} else {
			fares = 1;
			show_strtip(&fatip, armf, aflen, 2, tip_w, qry_tip_y);
		}
		break;
	case 2: //forced all arm result.
		fares = r & 1;
		show_strtip(&fatip, fae[fares], faelen[fares], 2, tip_w, qry_tip_y);
		break;
	default:
		break;
	}
}

static PVOID aam_opt[] = {all_arm_opt, all_disarm_opt};
static PVOID am_cb[] = {show_ptarm_win, show_zarm_win, show_tmr_win};

static void
select_amopt(void *data)
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
	if (i < 2) {
		aam_opt[i]();
	} else {
		i >>= 1;
		i--;
		am_cb[i]();
	}
}

static void
faarm_opt(void *data)
{
	hide_model_dialog(fdialog);
	if (priority_get(PRI_FARM)&&pz_get(0)) {
		pzone_farm(0);
	} else {
		show_tip(&prterr);
	}
}

static void
cancel_all_arm(void *data)
{
	hide_model_dialog(fdialog);
}

static void
goto_main(void *arg)
{
	show_main_win();
}

static void
handle_farm(void *arg)
{
	if (fares == 0) {
		show_main_win();
	}
	fares = 1;
}

void
build_arment_gui(void)
{
	kb_para_t   par;
	int  i;
	int  x[] = {am_bn_x0, am_bn_x1};
	int  y[] = {am_bn_y0, am_bn_y1, am_bn_y2, am_bn_y3};
	int  w[] = {am_bn_w0, am_bn_w1};
	char *bg[] = {BN_ALLARM1, BN_ALLDISARM1, BN_PARTARM1, BN_PDISARM1, BN_ZARM1, BN_ZDISARM1, BN_TMRDN1, BN_TMREN1};
	rect_t  r = {.x = bn_del_x, .y = bn_del_y, .w = bn_del_w, .h = bn_del_h};
	rect_t  r0 = {.x = pwd_ent_x, .y = pwd_ent_y, .w = pwd_ent_w, .h = pwd_ent_h};
	char *bgok[] = {YES_BN0, YES_BN1};
	char *bgcl[] = {NO_BN0, NO_BN1};

	faelen[0] = strlen(fae[0]);
	faelen[1] = strlen(fae[1]);
	salen = strlen(sysalt);
	aflen = strlen(armf);

	awin = utk_window_derive();
#ifndef MAPDLL
	utk_widget_resident(awin);
#endif
	utk_window_set_bg(awin, BG_AMPWD);
	utk_widget_show_before(awin, clear_inpent, NULL);
	utk_widget_async_evt_handler(awin, handle_amres);

	stb_new(awin, show_main_win);

	build_tip_lbl(&pwderr, awin, (LCD_W-tip_w)/2, (LCD_H-tip_h)/2, LBL_PWDERR, 1);
	build_tip_lbl(&aaok, awin, (LCD_W-tip_w)/2, (LCD_H-tip_h)/2, LBL_AARMOK, 1);
	tip_set_hide_cb(&aaok, goto_main, NULL);
	build_tip_lbl(&adok, awin, (LCD_W-tip_w)/2, (LCD_H-tip_h)/2, LBL_ADAMOK, 1);
	tip_set_hide_cb(&adok, goto_main, NULL);
	build_tip_lbl(&prterr, awin, (LCD_W-tip_w)/2, (LCD_H-tip_h)/2, LBL_PRTERR, 1);
	build_tip_lbl(&fatip, awin, (LCD_W-tip_w)/2, (LCD_H-tip_h)/2, BG_TIP, 0);
	tip_set_hide_cb(&fatip, handle_farm, NULL);
	build_tip_lbl(&armed, awin, (LCD_W-tip_w)/2, (LCD_H-tip_h)/2, LBL_ARMED, 2);
	build_tip_lbl(&disarmed, awin, (LCD_W-tip_w)/2, (LCD_H-tip_h)/2, LBL_DISARMED, 2);

	fdialog = build_model_dialog(awin, BG_FARMD, bgok, bgcl, faarm_opt, cancel_all_arm);

	pent = build_passwd_entry(awin, 0, &r0, pbuf, 18, RGB_BLACK);

	button_new(awin, BN_DEL, &r, (void *)del_entry, (void*)NULL);

	kb_para_init(&par, key_pressed, arm_kb_x, arm_kb_y, 0);
	my_kb = kb_new((void*)awin, &par);

	r.h = am_bn_h;
	for (i = 0; i < 8; ++i) {
		r.w = w[i&1];
		r.x = x[i&1];
		r.y = y[i>>1];
		go[i] = button_new(awin, bg[i], &r, (void *)select_amopt, (void*)i);
	}
}

void
show_arment_win(void)
{
	if (utk_window_top() != awin) {
		CLEARA(pbuf);
		utk_window_show(awin);
	}
}

/*
 * called by zopt thread.
 * op: 0-all disarm, 1-all arm, 2-forced all arm;
 * res: 0-success, nonzero-failed.
 */
void
allopr_return(int op, int res)
{
	utk_widget_send_async_event(awin, op, res);
}



