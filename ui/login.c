#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ulib.h"
#include "mxc_sdk.h"
#include "utksignal.h"
#include "utkbutton.h"
#include "utklabel.h"
#include "utkcontainer.h"
#include "utkmap.h"
#include "utktimeout.h"
#include "utkdesktop.h"
#include "layout.h"
#include "gui.h"
#include "config.h"
#include "stb.h"
#include "zopt.h"

static UtkWindow *mwin;
static UtkWindow *ulbl[MAXUSR+1];
static UtkWindow *ump[MAXUSR+1];
static UtkWindow *uinp;
static UtkWindow *umap;
static UtkWindow *pmap;
static UtkWindow *pdot[MAXPLEN];
static UtkWindow *cfm;

static tip_t      pwderr;
static keybar_t   lkbar;

static char       passbuf[MAXPLEN+2] = {0};
static int        passlen = 0;

static usr_t      su;

static usr_t     *who = NULL;
usr_t *curr_usr = NULL;
int    g_uid = -1;

usr_t *
get_superuser(void)
{
	return &su;
}

int
priority_get(int opt)
{
	return (who->pri & opt);
}

int
pz_get(int pz)
{
	if (pz == 0) {
		return (who->pz == ((1 << MAXPZ) - 1));
	} else if (pz > 0&&pz <= MAXPZ) {
		--pz;
		return (who->pz & (1 << pz));
	}
	return 0;
}

int
pzg_get(unsigned short pzm)
{
	return ((who->pz & pzm) == pzm);
}

int
usr_priority(int uid, int opt)
{
	if (uid == 0) return 1;
	return ((g_usr[uid-1].pri & opt) ? 1 : 0);
}

int
passwd_verify(char *pass)
{
	return !strcmp(pass, who->passwd);
}

static void
load_superusr(void)
{
	memset(&su, 0, sizeof(usr_t));
	strcpy(su.name, "超级管理员");
	strcpy(su.passwd, SUPASSWD);
	su.pri = 0xffff;
	su.id = 0;
	su.pz = (1 << MAXPZ) - 1;
}

static void
clear_usrinp(void)
{
	utk_label_set_text((UtkLabel*)uinp, NULL);
	utk_label_update(uinp);
}

static void
hide_user(void)
{
	int  i;

	for (i = 0; i < g_ucnt+1; ++i) {
		utk_widget_disable(ump[i]);
		utk_label_set_text((UtkLabel*)ulbl[i], NULL);
		utk_widget_update_state(ulbl[i], 0);
	}
}

static void
show_user(void *arg)
{
	int  i;

	utk_widget_enable(ump[0]);
	utk_label_set_text((UtkLabel*)ulbl[0], su.name);
	utk_widget_update_state(ulbl[0], 1);

	for (i = 1; i < g_ucnt+1; ++i) {
		utk_widget_enable(ump[i]);
		utk_label_set_text((UtkLabel*)ulbl[i], g_usr[i-1].name);
		utk_widget_update_state(ulbl[i], 1);
	}
}

static void
select_user(void *arg)
{
	int  i = (int)arg;

	if (i == 0)
		who = &su;
	else
		who = &g_usr[i-1];
	g_uid = i;
	hide_user();
	utk_label_set_text((UtkLabel*)uinp, who->name);
	utk_label_update(uinp);
}

static void
reset_passwd_lbl(void)
{
	int i;

	for (i = 0; i < MAXPLEN; ++i) {
		utk_widget_hide(pdot[i]);
	}
}

static int
pent_bp(void)
{
	if (passlen > 0) {
		--passlen;
		DBG("--- hide the pdot[%d] ---\n", passlen);
		utk_widget_hide(pdot[passlen]);
		passbuf[passlen] = 0;
	}
	return passlen;
}

static void
pent_fw(int v)
{
	if (passlen < MAXPLEN) {
		DBG("--- show the pdot[%d] ---\n", passlen);
		utk_widget_show(pdot[passlen]);
		passbuf[passlen] = (char)v;
		++passlen;
	}
}

static void
disable_widget(void)
{
/*
	utk_widget_disable(umap);
	utk_widget_disable(pmap);
	utk_widget_disable(cfm);
*/
	keybar_grab(&lkbar);
}

static void
enable_widget(void)
{
/*
	utk_widget_enable(umap);
	utk_widget_enable(pmap);
	utk_widget_enable(cfm);
*/
	keybar_ungrab(&lkbar);
}

static void (*wopt[])(void) = {enable_widget, disable_widget};

static void
widget_opt(int i)
{
	(*wopt[i])();
}

static void
prepare_pinput(void *data)
{
	reset_passwd_lbl();
	CLEARA(passbuf);
	passlen = 0;
	hide_user();
	show_keybar(&lkbar);
}

static void
user_identify(void *data)
{
	reset_passwd_lbl();	
	if (who == NULL) {
		show_tip(&pwderr);
		CLEARA(passbuf);
		passlen = 0;
		return;
	}

	if ((passlen > 0)&&(strcmp(passbuf, who->passwd) == 0)) {
		clear_usrinp();
		stb_set_user(who->name);
		curr_usr = who;
		if (get_alarm_count() == 0) {
			show_main_win();
		} else {
			show_zalrm_win();
		}
		send_event(EV_LOGIN, 0);
	} else {
		show_tip(&pwderr);
	}
	CLEARA(passbuf);
	passlen = 0;
}

static keybar_ops_t  pkop = {
	.fw = pent_fw,
	.bp = pent_bp,
	.show = widget_opt,
};

static void
prepare_login(void *data)
{
	hide_user();
	hide_keybar(&lkbar);
	CLEARA(passbuf);
	passlen = 0;
	curr_usr = NULL;
}

static void
build_passwd_dot(void *parent, void *d[], int n, int x, int y)
{
	int   i;

	for (i = 0; i < n; ++i)
	{
		d[i] = utk_label_new_fp();
		utk_label_set_bg((UtkWindow*)d[i], ICO_DOTB);
		utk_widget_add_cntl((UtkWindow*)parent, (UtkWindow*)d[i], x+21*i, y);
		utk_widget_disable((UtkWindow*)d[i]);
	}
}

void
build_login_gui()
{
	int  i;
	rect_t  r0 = {.x = login_usr_x, .y = login_usr_y, .w = login_usr_w, .h = login_usr_h};
	rect_t  r = {.x = login_ent_x, .y = login_ent_y0, .w = login_ent_w, .h = login_ent_h};
	
	load_superusr();

	mwin = utk_desktop_new();
	utk_widget_show_before(mwin, prepare_login, NULL);

	build_tip_lbl(&pwderr, mwin, (LCD_W-tip_w)/2, (LCD_H-tip_h)/2, LBL_PWDERR, 2);
	for (i = 0; i < MAXUSR+1; ++i) {
		r0.y = login_usr_y - i*login_usr_h;
		ulbl[i] = label_new(mwin, LBL_USR, NULL, RGB_BLACK, &r0);
		ump[i] = map_new(mwin, &r0, NULL, (void *)select_user, (void*)i);
		utk_widget_disable(ump[i]);
	}

	uinp = label_new(mwin, NULL, NULL, RGB_BLACK, &r);
	umap = map_new(mwin, &r, NULL, (void *)show_user, (void*)0);

	r.y = login_ent_y1;
	pmap = map_new(mwin, &r, NULL, (void *)prepare_pinput, (void*)0);

	build_passwd_dot(mwin, (void**)pdot, MAXPLEN, login_ent_x + 8, login_ent_y1 + (login_ent_h - 14)/2);
	build_keybar(&lkbar, mwin, (LCD_W - keybar_w)>>1, (174 - keybar_h)/2 + (LCD_H - 174), &pkop, 0);

	cfm = utk_button_new();
	utk_button_set_bg(cfm, BN_LOGIN0);
	utk_button_set_bg(cfm, BN_LOGIN1);
	utk_signal_connect(cfm, "clicked", user_identify, 0);
	utk_widget_add_cntl(mwin, cfm, bn_login_x, bn_login_y);
}

/* called by other threads */
void
show_login(void)
{
	if (utk_window_top() != mwin) {
		utk_window_show(mwin);
	}
}

UtkWindow*
desktop_window(void)
{
	return mwin;
}


