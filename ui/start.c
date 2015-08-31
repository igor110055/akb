#include "ulib.h"
#include "mxc_sdk.h"
#include "utksignal.h"
#include "utkbutton.h"
#include "utkmstbutton.h"
#include "utkspinbutton.h"
#include "utktextarea.h"
#include "utklabel.h"
#include "utkclock.h"
#include "utktimeout.h"
#include "layout.h"
#include "stb.h"
#include "gui.h"
#include "config.h"
#include "recver.h"
#include "zopt.h"
#include "rs485.h"
#include "xfer.h"
#include "znode.h"


#define HOSTMSK     ((1<<6)-1)

static UtkWindow *mnwin;
static void      *tbar;

static UtkWindow *hled;
static UtkWindow *mled;
static UtkWindow *clbl;

static UtkWindow *msglbl;

static tip_t     prterr;
static tip_t     nomap;
static tip_t     nzone;
static tip_t     nsysp0;
static tip_t     nsysp1;

static char  hur[4] = {0};
static char  min[4] = {0};
static char  hour;
static char  minu;
static int   hide_clk = 0;
static pwd_diag_t  sys_diag;
static unsigned int    mtid;

static char *systf[] = {
	"系统故障:网络故障",
	"系统故障:RS485故障",
	"系统故障:1号主机交流断电",    /* EV_APF/EV_APR */
	"系统故障:1号主机电池故障",    /* EV_DPF/EV_DPR */
	"系统故障:1号主机电池电压低",  /* EV_BL/EV_BLR */
	"系统故障:1号主机总线故障",        /* EV_HBUSE/EV_HBUSR */
	"系统故障:1号主机防拆故障",        /* EV_A120DP/EV_A120DPR */
	"系统故障:1号主机通讯故障",
	"系统故障:2号主机交流断电",
	"系统故障:2号主机电池故障",
	"系统故障:2号主机电池电压低",
	"系统故障:2号主机总线故障",
	"系统故障:2号主机防拆故障",
	"系统故障:2号主机通讯故障",
};

static char  *syststr[] = {"系统正常", "系统故障"};

static char   dispbuf[36];

static volatile unsigned int  sysfmap = 0;
static unsigned int  sysfmsk = 0;
static int           disp_pz = 0;

static char *allst[] = {
	"系统正常:撤防状态",
	"系统正常:全部布防",
};

static int    npid = 0;   //index of pzone in normal state.
static int    fpid = 0;   //index of pzone in fault state.

static char  *alrstr[] = {"报警恢复", "报警"};

void
clear_sysfst(void)
{
	sysfmsk = ~0;
}

static int
sysalrm_disp(void)
{
	int  z;
	int  a;
	int  h;
	int  zz;

	z = alarmst_get(NULL);
	if (z < 0) return 0;
	a = (z >> 16) & 1;
	z &= 0xffff;
	h = z/99+1;
	zz = z%99+1;
	memset(dispbuf, 0, sizeof(dispbuf));
//	DBG("--- host = %d, zone = %d, a = %d ---\n", h, zz, a);
	snprintf(dispbuf, sizeof(dispbuf)-1, "系统报警:%d主机%d防区%s", h, zz, alrstr[a]);
	utk_label_set_text((UtkLabel *)msglbl, dispbuf);
	utk_label_update(msglbl);

	return 1;
}

static char  *pzstr[] = {
	[PST_UNKNOWN] = "分区撤防",
	[PST_DISARM]  = "分区撤防",
	[PST_PTBPASS] = "分区局部旁路",
	[PST_BYPASS]  = "分区旁路",
	[PST_NRDY]    = "分区未准备",
	[PST_PTARM]   = "分区局部布防",
	[PST_ARM]     = "分区布防",
	[PST_FRCARM]  = "分区强制布防",
	[PST_TMRARM]  = "分区定时布防",
};

static void
disp_pzone_state(int p, int st, int t)
{
	if (st < PST_ALARM) {
		snprintf(dispbuf, sizeof(dispbuf), "%s: %d %s", syststr[t], p, pzstr[st]);
		utk_label_set_text((UtkLabel *)msglbl, dispbuf);
		utk_label_update(msglbl);
	}
}

static void
sysfpz_disp(void)
{
	int  st;
	int  p = pzarr[fpid]+1;

	st = get_pzone_state_direct(p);
	disp_pzone_state(p, st, 1);
	++fpid;
	if (fpid == pzcnt) {
		fpid = 0;
		disp_pz = 0;
		sysfmsk = ~0;
	}
}

static int
sysf_disp(void)
{
	int  z;
	int  last = 0;
	int  h;
	int  zz;

	z = nready_get(&last);
	if (z < 0&&sysfmap == 0) {//no fault
		fpid = 0;
		disp_pz = 0;
		sysfmsk = ~0;
		return 0;
	}

	sysfmsk &= sysfmap;
	if (sysfmsk) {
		z = __builtin_ctz(sysfmsk);
		sysfmsk &= ~(1 << z);
		utk_label_set_text((UtkLabel *)msglbl, systf[z]);
		utk_label_update(msglbl);
		disp_pz = 0;
		fpid = 0;
		return 1;
	}

	if (z >= 0&&disp_pz == 0) { //no not-ready zones
		h = z/99+1;
		zz = z%99+1;
		snprintf(dispbuf, 31, "系统故障:%d主机%d防区未准备", h, zz);
		utk_label_set_text((UtkLabel *)msglbl, dispbuf);
		utk_label_update(msglbl);
		if (last) {
			disp_pz = 1;
			fpid = 0;  //finish displaying not-ready zones info.
		}
		return 1;
	}

	sysfpz_disp();
	return 1;
}

/* DONE */
static void
sysnormal_disp(void)
{
	int  st;
	int  p = pzarr[npid]+1;

	st = get_pzone_state_direct(0);
	if (st < 2) {
		utk_label_set_text((UtkLabel *)msglbl, allst[st]);
		utk_label_update(msglbl);
		return;
	}

	st = get_pzone_state_direct(p);
	disp_pzone_state(p, st, 0);
	++npid;
	npid = (npid >= pzcnt) ? 0 : npid;
	disp_pz = 0;
}

/* DONE */
static int
zone_none(void)
{
	char *nztip = "系统没有配置任何防区";

	if (zarcnt == 0) {
		DBG("--- no zone configured ---\n");
		utk_label_set_text((UtkLabel *)msglbl, nztip);
		utk_label_update(msglbl);
		return 1;
	}
	return 0;
}

static void
update_sys_st(void *arg)
{
	if (sysalrm_disp()) return;
	if (sysf_disp()) return;
	sysnormal_disp();
}

static void
goto_bypass(void *data)
{
	if (zarcnt == 0) {
		show_tip(&nzone);
		return;
	}
	if (priority_get(PRI_BP)) {
		show_bpent_win();
	} else {
		show_tip(&prterr);
	}
}

static void
goto_arm(void *data)
{
	if (zarcnt == 0) {
		show_tip(&nzone);
		return;
	}
	if (priority_get(PRI_ARM)) {
		show_arment_win();
	} else {
		show_tip(&prterr);
	}
}

/* DONE */
static void
goto_mapview(void *data)
{
	if (show_map_win(1, -1) < 0) {
		show_tip(&nomap);
	}
}

/////////// bottom menubar callbacks ///////////
static void
goto_sysf(void)
{
	if (priority_get(PRI_FN)) {
		show_dialog(&sys_diag, 0);
	} else {
		show_tip(&prterr);
	}
}

static void
goto_sysp(void)
{
	int  r;
	tip_t  *tip[] = {&nsysp0, &nsysp1};

	if (priority_get(PRI_PG)) {
		r = get_syst();
		if (r > 0) {
			show_tip(tip[r-1]);
			return;
		}
		show_dialog(&sys_diag, 1);
	} else {
		show_tip(&prterr);
	}
}

static void
sysf_passwd_ck(char *passwd)
{
	if (passwd_verify(passwd)) {
		cfg_map_clr();
		show_sysnet_win();
		DBG("--- enter the system function UI ---\n");
	} else {
		pwd_diag_error(&sys_diag);
	}
}

static void
sysp_passwd_ck(char *passwd)
{
	if (passwd_verify(passwd)) {
		cfg_map_clr();
		recver_exit();
		show_sysp_win();
		DBG("--- enter the system program UI ---\n");
	} else {
		pwd_diag_error(&sys_diag);
	}
}

static void
exit_passwd_ck(char *passwd)
{
	if (passwd_verify(passwd)) {
		send_event(EV_LOGOUT, 0);
		show_login();
	} else {
		pwd_diag_error(&sys_diag);
	}
}

static PVOID  main_cb[] = {show_einfo_win, show_ainfo_win, show_stent_win, show_zinfo_win, goto_sysf, goto_sysp};

static void
do_main_opt(void *data)
{
	int i = (int)data;
	main_cb[i]();
}

static void
load_time(void *arg)
{	
	struct tm  *tm = arg;

	memset(hur, '\0', 4);
	sprintf(hur, "%02d", tm->tm_hour);
	hour = tm->tm_hour;

	memset(min, '\0', 4);
	sprintf(min, "%02d", tm->tm_min);
	minu = tm->tm_min;
/*
	memset(sec, '\0', 4);
	sprintf(sec, "%02d", tm->tm_sec);
*/
}

static void
update_time(void *arg)
{
	struct tm  *tm = arg;

	if (hide_clk == 1) return;
	if (tm->tm_hour != hour) {
		hour = tm->tm_hour;
		memset(hur, '\0', 4);
		sprintf(hur, "%02d", tm->tm_hour);
		led_update(hled);
	}

	if (tm->tm_min != minu) {
		minu = tm->tm_min;
		memset(min, '\0', 4);
		sprintf(min, "%02d", tm->tm_min);
		led_update(mled);
	}
}

/* reserved for future */
/*
static void
hide_wall_clock(void)
{
	stb_stop_clock(tbar);
	hide_clk = 1;
	CLEARA(hur);
	CLEARA(min);
	led_update(hled);
	led_update(mled);
	utk_widget_hide(clbl);
	stb_start_clock(tbar);
}

static void
show_wall_clock(void)
{
	struct tm ntm;
	stb_stop_clock(tbar);
	hide_clk = 0;
	get_tm(&ntm, NULL);
	load_time(&ntm);
	led_update(hled);
	led_update(mled);
	utk_widget_show(clbl);
	stb_start_clock(tbar);
}
*/

static void
start_mtimer(void *arg)
{
	if (zone_none() == 0) {
		utk_timeout_add(100, mtid);
	}
}

static void
stop_mtimer(void *arg)
{
	utk_timeout_remove(mtid);
	utk_label_set_text((UtkLabel *)msglbl, NULL);
	utk_label_update(msglbl);
}

#define inf_w       500
#define inf_h       32
#define inf_x       ((542-inf_w)/2+113)
#define inf_y       (121+(74-inf_h)/2)

static void
build_msg_label(UtkWindow *parent)
{
	msglbl = utk_label_new_fp();
	utk_label_set_fnt((UtkLabel*)msglbl, 2);
	utk_widget_set_size(msglbl, inf_w, inf_h);
	utk_label_set_text_size((UtkLabel*)msglbl, 54);
	utk_label_set_text_color((UtkLabel*)msglbl, RGB_WHITE);
	utk_label_set_text((UtkLabel *)msglbl, NULL);
	utk_widget_add_cntl(parent, msglbl, inf_x, inf_y);
}

static void
build_owner_label(UtkWindow *parent)
{
	UtkWindow *owner;
	char *p;

	p = get_owner_ptr();
	owner = utk_label_new_fp();
	utk_label_set_fnt((UtkLabel*)owner, 1);
	utk_widget_set_size(owner, 432, 24);
	utk_label_set_text_size((UtkLabel*)owner, 36);
	utk_label_set_text_color((UtkLabel*)owner, RGB_WHITE);
	utk_label_set_text_pos((UtkLabel*)owner, 1, 0);
	utk_label_set_text((UtkLabel*)owner, p);
	utk_widget_add_cntl(parent, owner, 325, 437);
}

static void
logout(void)
{
	show_dialog(&sys_diag, 2);
}

void
build_main_gui(void)
{
	int   bw[] = {menubar_w0, menubar_w1, menubar_w2, menubar_w3, menubar_w4, menubar_w5};
	char *bg[] = {BN_ELOG, BN_ALOG, BN_SYSC, BN_ZINF, BN_SYSF, BN_SYSP};
	rect_t  r = {.x = sbar_x, .y = sbar_y0, .w = sidebar_w, .h = sidebar_h};
	rect_t  tr = {.x = led_x0, .y = led_y, .w = led_w, .h = led_h};

	mnwin = utk_window_derive();
	utk_widget_resident(mnwin);
	utk_window_set_bg(mnwin, BG_MAIN);
	mtid = utk_widget_timer(mnwin, update_sys_st, (void*)0);
	utk_widget_show_after(mnwin, start_mtimer, (void*)0);
	utk_widget_hide_before(mnwin, stop_mtimer, (void*)0);
	
	tbar = stb_new(mnwin, logout);
	stb_add_update_cb(tbar, (void*)update_time, (void*)load_time, 0);

	hled = led_new(mnwin, hur, 2, &tr);
	tr.x = led_x1;
	mled = led_new(mnwin, min, 2, &tr);
	clbl = utk_label_new();
	utk_widget_set_bg(clbl, LED_C, 0);
	utk_widget_add_cntl(mnwin, clbl, colon_x, colon_y);

	build_tip_lbl(&prterr, mnwin, (LCD_W-tip_w)/2, (LCD_H-tip_h)/2, LBL_PRTERR, 2);
	build_tip_lbl(&nomap, mnwin, (LCD_W-tip_w)/2, (LCD_H-tip_h)/2, LBL_NOMAP, 2);
	build_tip_lbl(&nzone, mnwin, (LCD_W-tip_w)/2, (LCD_H-tip_h)/2, LBL_NOZONE, 2);

	build_tip_lbl(&nsysp0, mnwin, (LCD_W-tip_w)/2, (LCD_H-tip_h)/2, LBL_NSYSP0, 2);
	build_tip_lbl(&nsysp1, mnwin, (LCD_W-tip_w)/2, (LCD_H-tip_h)/2, LBL_NSYSP1, 2);

	build_msg_label(mnwin);
	build_owner_label(mnwin);

	button_new(mnwin, SBAR_ARM, &r, goto_arm, (void*)NULL);

	r.y = sbar_y1;
	button_new(mnwin, SBAR_BP, &r, goto_bypass, (void*)NULL);

	r.y = sbar_y2;
	button_new(mnwin, SBAR_MAP, &r, goto_mapview, (void*)NULL);

	build_button_array(mnwin, ARR_SZ(bw), menubar_h, menubar_y, bw, menubar_x0, do_main_opt, bg);

	build_dialog((void*)mnwin, &sys_diag, DQTMEO);
	pwd_dialog_ok_event(&sys_diag, sysf_passwd_ck, 0);
	pwd_dialog_ok_event(&sys_diag, sysp_passwd_ck, 1);
	pwd_dialog_ok_event(&sys_diag, exit_passwd_ck, 2);
}

/* note: reentrance problem */
/* called by other thread */
void
show_main_win(void)
{
	if (utk_window_top() != mnwin)
	{
		utk_window_show(mnwin);
	}
}

void
go_home(void* data)
{
	int  fp = (int)data;

	show_main_win();
	if (fp) {
		DBG("--- quit the system function UI ---\n");
		xfer_cfgcmd();
		handle_netifloop();
	}
}

void
sysf_quit(void)
{
	show_main_win();
	xfer_cfgcmd();
	handle_netifloop();
}

/* uart.c/stb.c--->zopt.c--->syst.c */
/* called in syst.c */
void
sysfmap_set(int id)
{
	sysfmap |= (1 << id);
	DBG("--- sysfmap = %d ---\n", sysfmap);
}

/* called in syst.c */
void
sysfmap_clr(int id)
{
	sysfmap &= ~(1 << id);
	DBG("--- sysfmap = %d ---\n", sysfmap);
}


