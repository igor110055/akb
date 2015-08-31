#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "ulib.h"
#include "utk.h"
#include "utksignal.h"
#include "utkwindow.h"
#include "utkbutton.h"
#include "utklabel.h"
#include "utkentry.h"
#include "utkmap.h"
#include "layout.h"
#include "config.h"
#include "gui.h"
#include "stb.h"
#include "pzone.h"
#include "zopt.h"
#include "qtimer.h"

static UtkWindow   *zcfgw;
static UtkWindow   *idlbl;
static UtkWindow   *vtbn;
static UtkWindow   *prt[3];
static drop_menu_t  zdm;

static tip_t        zerr;
static tip_t        zredef;
static tip_t        zerrtip;

static char     idtxt[6] = {0};
static kbar_ent_t   zke;
static kbar_ent_t   zkb;

static myent_t  znr_ent;
static char     zstr[6] = {0};

static myent_t  pnr_ent;
static char     pstr[4] = {0};

static myent_t  det_ent;
static char     dnstr[16] = {0};

static myent_t  znm_ent;
static char     znmstr[MAXNAME] = {0};

static myent_t  pnm_ent;
static char     pnmstr[MAXNAME] = {0};

static myent_t  vip_ent;
static char     vipstr[16] = {0};

static unsigned char  zt = 0;
static unsigned char  vt = 0;

static zone_t   g_zone;
static int      zid = 0;
static int      znew = 0;

static char *pnrerr = "分区号不能等于0或大于10";
static int   perlen = 0;

static char *dnerr = "请输入探测器名称";
static int   dnelen = 0;

static char *znerr = "请输入防区名称";
static int   znelen = 0;

static char *pnerr = "请输入分区名称";
static int   pnelen = 0;

static char *viperr = "视频IP地址非法";
static int   velen = 0;

static int   perrno = 0;

static void
init_zcfgtiplen(void)
{
	perlen = strlen(pnrerr);
	dnelen = strlen(dnerr);
	znelen = strlen(znerr);
	pnelen = strlen(pnerr);
	velen = strlen(viperr);
}

static void
disable_proto_bn(int v)
{
	if (v) {
		utk_widget_disable(prt[0]);
		utk_widget_disable(prt[1]);
		utk_widget_disable(prt[2]);
	} else {
		utk_widget_enable(prt[0]);
		utk_widget_enable(prt[1]);
		utk_widget_enable(prt[2]);
	}
}

static void
update_proto(int v)
{
	int  i;

	if (v == 0) {
		utk_widget_update_state(prt[0], 0);
		utk_widget_update_state(prt[1], 0);
		utk_widget_update_state(prt[2], 0);
	} else {
		i = v - 1;
		utk_widget_update_state(prt[i], 1);
		i = (i+1)%3;
		utk_widget_update_state(prt[i], 0);
		i = (i+1)%3;
		utk_widget_update_state(prt[i], 0);
	}
}

static void
clear_zone_ui(int ud)
{
	CLEARA(zstr);
	CLEARA(pstr);
	CLEARA(dnstr);
	CLEARA(znmstr);
	CLEARA(pnmstr);
	CLEARA(vipstr);

	if (ud) {
		myent_update(&znr_ent);
		myent_update(&pnr_ent);
		myent_update(&det_ent);
		myent_update(&znm_ent);
		myent_update(&pnm_ent);
		myent_update(&vip_ent);
	}

	zt = 0;
	drop_menu_update_state(&zdm, 0);

	vt = 0;
	utk_widget_update_state(vtbn, 0);
	update_proto(0);
	disable_proto_bn(1);
	myent_disable(&vip_ent);
}

static void
load_zone_inf(zone_t *z, int ud)
{
	int  p;

//	if (ud == 0) {
		memcpy(zstr, z->zstr, 6);
		myent_set_text(&znr_ent, zstr);
		DBG("--- zone nr = %s ---\n", zstr);
//	}

	p = z->pnr;
	if ((p <= MAXPZ)&&(p > 0)&&(pzone[p-1].pnr == p)) {
		memset(pstr, 0, 4);
		snprintf(pstr, 4, "%02d", p);
		memcpy(pnmstr, pzone[p-1].pname, MAXNAME);
		myent_set_text(&pnr_ent, pstr);
		myent_set_text(&pnm_ent, pnmstr);
		DBG("--- pzone nr = %s, pzname = %s ---\n", pstr, pnmstr);
	} else {
		DBG("--- invalid pzone number ---\n");
		CLEARA(pstr);
		CLEARA(pnmstr);
	}

	memcpy(dnstr, z->dname, 16);
	memcpy(znmstr, z->zname, MAXNAME);
	myent_set_text(&det_ent, dnstr);
	myent_set_text(&znm_ent, znmstr);

	zt = z->ztp;
	drop_menu_update_state(&zdm, zt);

	vt = z->prot;
	DBG("--- vt = %d, prot = %d ---\n", vt&1, vt>>1);
	if (vt > 0&&strlen(z->vip) > 0&&is_valid_unip(z->vip)) {
		memcpy(vipstr, z->vip, 16);
		disable_proto_bn(0);
		myent_enable(&vip_ent);
		myent_set_text(&vip_ent, vipstr);
	} else {
		DBG("--- invalid ip address ---\n");
		vt = 0;
		z->prot = 0;
		memset(z->vip, 0, 16);
		CLEARA(vipstr);
		disable_proto_bn(1);
		myent_disable(&vip_ent);
	}
	utk_widget_update_state(vtbn, vt&1);
	update_proto(vt>>1);

	if (ud) {
		myent_update(&znr_ent);
		myent_update(&pnr_ent);
		myent_update(&det_ent);
		myent_update(&znm_ent);
		myent_update(&pnm_ent);
		myent_update(&vip_ent);
	}
}

static void
load_zone_ui(void *data)
{
	zone_t *z = data;
	
	DBG("--- show before: load_zone_ui-0 ---\n");
	if (znew == 1) {
		myent_enable(&znr_ent);
		clear_zone_ui(0);
		DBG("--- new zone ---\n");
	} else {
		myent_disable(&znr_ent);
		load_zone_inf(z, 0);
		DBG("--- modify zone ---\n");
	}
	DBG("--- show before: load_zone_ui-1 ---\n");
}

static void 
go_back(void *data)
{
	show_zmgr_win();
}

static int
is_valid_zcfg(int *id, int *z, int *p)
{
	char *ptr = NULL;
	int   v;

	v = (int)strtol(zstr, &ptr, 10);
	*id = v / 100;
	*z = v % 100;
	if (*id > MAXHOST||*id == 0||*z == 0) {
		show_tip(&zerr);
		return 0;
	}

	*p = (int)strtol(pstr, &ptr, 10);
	if ((*p == 0)||(*p > MAXPZ)) {
		perrno = 1;
		show_strtip(&zerrtip, pnrerr, perlen, 2, tip_w, qry_tip_y);
		return 0;
	}

	if (strlen(dnstr) == 0) {
		perrno = 0;
		show_strtip(&zerrtip, dnerr, dnelen, 2, tip_w, qry_tip_y);
		return 0;
	}

	if (strlen(znmstr) == 0) {
		perrno = 0;
		show_strtip(&zerrtip, znerr, znelen, 2, tip_w, qry_tip_y);
		return 0;
	}

	if (strlen(pnmstr) == 0) {
		perrno = 0;
		show_strtip(&zerrtip, pnerr, pnelen, 2, tip_w, qry_tip_y);
		return 0;
	}

	if ((vt > 0)&&(strlen(vipstr) == 0||!is_valid_unip(vipstr))) {
		perrno = 0;
		show_strtip(&zerrtip, viperr, velen, 2, tip_w, qry_tip_y);
		return 0;
	}

	return 1;
}

static void 
zcfg_save(void *data)
{
	int  p;
	int  z;
	int  hid;

	hide_menu(&zdm);
	if (kbar_entry_save_prepared(&zke) == 0) return;
	if (is_valid_zcfg(&hid, &z, &p) == 0) return;
	if (znew == 0) {
		if (p != (int)(g_zone.pnr)) goto next;
		if (memcmp(pzone[p-1].pname, pnmstr, MAXNAME)) goto next;
		if (g_zone.ztp != zt) goto next;
		if (memcmp(g_zone.dname, dnstr, 16)) goto next;
		if (memcmp(g_zone.zname, znmstr, MAXNAME)) goto next;
		if (memcmp(g_zone.vip, vipstr, 16)) goto next;
		if (g_zone.prot != vt) goto next;
		return;
	}

next:
	disable_syncfg();
	g_zone.znr = (hid - 1)*99 + z;
	g_zone.pnr = p;
	g_zone.ztp = zt;
	memcpy(g_zone.dname, dnstr, 16);
	memcpy(g_zone.zname, znmstr, MAXNAME);
	if (vt > 0)
		memcpy(g_zone.vip, vipstr, 16);
	else
		memset(g_zone.vip, 0, 16);
	g_zone.prot = vt;

	memcpy(g_zone.zstr, zstr, 6);
	snprintf(g_zone.pstr, 4, "%02d", p);

	store_zone(&g_zone, znew);
	store_pzone(p, pnmstr);
	DBG("--- p = %d, z = %s ---\n", p, g_zone.zstr);
	if (znew == 1) {
		memset(&g_zone, 0, sizeof(zone_t));
		++zid;
		CLEARA(idtxt);
		snprintf(idtxt, sizeof(idtxt), "%04d", zid);
		utk_label_update(idlbl);
		clear_zone_ui(1);
	}
}

static void
zcfg_cancel(void *data)
{
	hide_menu(&zdm);
	if (znew == 0) {
		if (zid > 0&&zid <= zarcnt) {
			load_zone_inf(&zonedb[zarray[zid-1]], 1);
		}
	} else {
		clear_zone_ui(1);
	}
}

static void
select_zt(int i)
{
	DBG("--- zdm: %d ---\n", i);
	zt = i;
}

static void
show_ztype(void)
{
	show_menu(&zdm);
}

static void
select_video(void *data)
{
	int  t;

	t = utk_widget_state(vtbn);
	if (t == 0) {
		myent_disable(&vip_ent);
		memset(vipstr, 0, 16);
		myent_update(&vip_ent);
		if ((vt>>1) > 0) {
			utk_widget_update_state(prt[(vt>>1)-1], 0);
		}
		disable_proto_bn(1);
		vt = 0;
	} else {
		myent_enable(&vip_ent);
		if (strlen(g_zone.vip) > 0) {
			memcpy(vipstr, g_zone.vip, 16);
			myent_update(&vip_ent);
		}
		disable_proto_bn(0);
		vt = 1;
	}
}

static void
select_proto(void *arg)
{
	int  i = (int)arg;
	int  t1;
	int  t0;

	t0 = vt >> 1;
	t1 = utk_widget_state(prt[i-1]);
	if (t1 == 0) {
		vt = 1;
	} else {
		if (t0 > 0) {
			utk_widget_update_state(prt[t0-1], 0);
		}
		vt = (i<<1)|0x01;
	}
}

static void
zcfg_quit(void *data)
{
	hide_menu(&zdm);
	hide_imekb(&zkb);
}

static void
change_pnr(void *data)
{
	int  n;

	n = atoi(pstr);
	if (n > 0&&n <= MAXPZ&&pzone[n-1].pnr == n) {
		memcpy(pnmstr, pzone[n-1].pname, MAXNAME);
	} else {
		memset(pnmstr, 0, MAXNAME);
	}

	myent_set_text(&pnm_ent, pnmstr);
	myent_update(&pnm_ent);
}

static void
znr_clear(void *arg)
{
	myent_clear(&znr_ent);
}

static void
pnr_clear(void *arg)
{
	if (perrno == 1) {
		myent_clear(&pnr_ent);
		if (strlen(pnmstr) > 0) {
			myent_clear(&pnm_ent);
		}
		perrno = 0;
	}
}

void
build_zcfg_gui(void)
{
	rect_t  lr = {.x = id_lbl_x, .y = id_lbl_y, .w = id_lbl_w, .h = id_lbl_h};
	rect_t  zr = {.x = zcfg_zt_x, .y = zcfg_zt_y, .w = zcfg_zt_w, .h = zcfg_zt_h};
	rect_t  r = {.x = bn_dm_x, .y = bn_dm_y, .w = bn_dm_w, .h = bn_dm_h};
	rect_t  r0 = {.x = zsel_x0, .y = zsel_y0, .w = psel_w, .h = psel_h};
	rect_t  r1 = {.x = zcfg_bar_x0, .y = zcfg_bar_y, .w = zcfg_bar_w0, .h = zcfg_bar_h};
	char   *bgz[]  = {DM_IZ0, DM_DZ0, DM_AZ0};
	char   *bgz1[] = {DM_IZ1, DM_DZ1, DM_AZ1};
	rect_t  re = {.x = znr_x, .y = znr_y, .w = znr_w, .h = znr_h};

	init_zcfgtiplen();

	zcfgw = utk_window_derive();
#ifndef MAPDLL
	utk_widget_resident(zcfgw);
#endif
	utk_window_set_bg(zcfgw, BG_ZCFG);
	utk_widget_show_before(zcfgw, load_zone_ui, &g_zone);
	utk_widget_hide_before(zcfgw, zcfg_quit, NULL);

	register_qtimer((void*)zcfgw);

	stb_new(zcfgw, sysp_exit);

	idlbl = label_new(zcfgw, NULL, idtxt, RGB_BLACK, &lr);
	build_tip_lbl(&zerr, zcfgw, (LCD_W-tip_w)/2, (LCD_H-tip_h)/2, LBL_ZINVLD, 2);
	tip_set_hide_cb(&zerr, (void*)znr_clear, (void *)0);
	build_tip_lbl(&zredef, zcfgw, (LCD_W-tip_w)/2, (LCD_H-tip_h)/2, LBL_REDEFZ, 2);
	tip_set_hide_cb(&zredef, (void*)znr_clear, (void *)0);
	build_tip_lbl(&zerrtip, zcfgw, (LCD_W-tip_w)/2, (LCD_H-tip_h)/2, BG_TIP, 0);
	tip_set_hide_cb(&zerrtip, (void*)pnr_clear, (void *)0);

	build_drop_menu(&zdm, zcfgw, BG_ZT, &zr, bgz, bgz1, ARR_SZ(bgz), select_zt);

	build_entry_kbar(&zke, zcfgw, zcfg_kbar_y, 0);
	build_entry_keyboard(&zkb, zcfgw, 0, 600-335);

	button_new(zcfgw, BN_DM, &r, show_ztype, (void*)0);

	myent_new(&znr_ent, zcfgw, &re, 0);
	myent_add_caret(&znr_ent, RGB_BLACK);
	myent_set_cache(&znr_ent, zstr, 6, 4);
	myent_add_kbar(&znr_ent, &zke);
//	myent_set_callback(&znr_ent, NULL, check_znr, NULL, NULL);

	re.x = pnr_x;
	re.y = pnr_y;
	re.w = pnr_w;
	re.h = pnr_h;
	myent_new(&pnr_ent, zcfgw, &re, 0);
	myent_add_caret(&pnr_ent, RGB_BLACK);
	myent_set_cache(&pnr_ent, pstr, 4, 3);
	myent_add_kbar(&pnr_ent, &zke);
	myent_set_callback(&pnr_ent, NULL, NULL, change_pnr, NULL);
//	myent_set_callback(&pnr_ent, NULL, check_pnr, change_pnr, NULL);

	///////////////////////////////////////////////////////////
	re.x = dnm_x;
	re.y = dnm_y;
	re.w = dnm_w;
	re.h = dnm_h;
	myent_new(&det_ent, zcfgw, &re, 0);
	myent_add_caret(&det_ent, RGB_BLACK);
	myent_set_cache(&det_ent, dnstr, 16, 12);
	myent_add_kbar(&det_ent, &zkb);

	re.x = znm_x;
	re.y = znm_y;
	re.w = znm_w;
	re.h = znm_h;
	myent_new(&znm_ent, zcfgw, &re, 0);	
	myent_add_caret(&znm_ent, RGB_BLACK);
	myent_set_cache(&znm_ent, znmstr, 32, 30);
	myent_add_kbar(&znm_ent, &zkb);

	re.x = pnm_x;
	re.y = pnm_y;
	re.w = pnm_w;
	re.h = pnm_h;
	myent_new(&pnm_ent, zcfgw, &re, 0);	
	myent_add_caret(&pnm_ent, RGB_BLACK);
	myent_set_cache(&pnm_ent, pnmstr, 32, 30);
	myent_add_kbar(&pnm_ent, &zkb);

	////////////////////////////////////////////////////////////
	re.x = vip_x;
	re.y = vip_y;
	re.w = vip_w;
	re.h = vip_h;
	myent_new(&vip_ent, zcfgw, &re, 0);	
	myent_add_caret(&vip_ent, RGB_BLACK);
	myent_set_cache(&vip_ent, vipstr, 16, 15);
	myent_add_kbar(&vip_ent, &zke);

	vtbn = toggle_button_new(zcfgw, BN_SEL, &r0, (void *)select_video, (void*)0);

	r0.w = psel_w;
	r0.h = psel_h;
	r0.x = prot_x;
	r0.y = prot_y0;
	prt[0] = toggle_button_new(zcfgw, BN_SEL, &r0, (void *)select_proto, (void*)1);
	r0.y = prot_y1;
	prt[1] = toggle_button_new(zcfgw, BN_SEL, &r0, (void *)select_proto, (void*)2);
	r0.y = prot_y2;
	prt[2] = toggle_button_new(zcfgw, BN_SEL, &r0, (void *)select_proto, (void*)3);
	
	////////////////////////////////////////////////////////////
	button_new(zcfgw, BN_RET160G, &r1, go_back, (void*)0);

	r1.w = zcfg_bar_w1;
	r1.x = zcfg_bar_x1;
	button_new(zcfgw, BN_PWDSV, &r1, zcfg_save, (void*)0);

	r1.w = zcfg_bar_w2;
	r1.x = zcfg_bar_x2;
	button_new(zcfgw, BN_PWDCL, &r1, zcfg_cancel, (void*)0);
}

/* id: based from 1 */
void
show_zcfg_win(void *z, int id)
{
	if (utk_window_top() != zcfgw) {
		if (z) {
			memcpy(&g_zone, z, sizeof(zone_t));
			znew = 0;
		} else {
			memset(&g_zone, 0, sizeof(zone_t));
			znew = 1;
		}
		zid = id;
		CLEARA(idtxt);
		snprintf(idtxt, sizeof(idtxt), "%04d", id);
		DBG("--- BEFORE SHOW ZCFG WINDOW ---\n");
		utk_window_show(zcfgw);
	}
}


