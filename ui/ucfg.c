#include "ulib.h"
#include "utk.h"
#include "utksignal.h"
#include "utkwindow.h"
#include "utkbutton.h"
#include "utklabel.h"
#include "utkentry.h"
#include "utkmap.h"
#include "layout.h"
#include "gui.h"
#include "stb.h"
#include "config.h"
#include "qtimer.h"

static UtkWindow   *ucfgw;
static UtkWindow   *upriw[6];
static UtkWindow   *upzw[10];

static UtkWindow   *idlbl;
static char         idstr[4] = {0};

static kbar_ent_t   uke;
static myent_t      pwd_ent;
static char         pwdstr[MAXPLEN+2] = {0};

static myent_t      usr_ent;
static char         ustr[24] = {0};

static usr_t        usr;
static int          uid = 0;
static unsigned short  upri = 0;
static unsigned short  upz = 0;

static tip_t        utip;

static char *ustrs[] = {
	"请输入4~6位密码",
	"请输入用户名",
	"用户名重复",
	"请选择权限",
};

static int  ulen[4];

static void
clear_ucfg(void)
{
	int  i;

	CLEARA(pwdstr);
	CLEARA(ustr);
	upri = 0;
	upz = 0;
	for (i = 0; i < ARR_SZ(upriw); ++i) {
		utk_widget_update_state(upriw[i], 0);
		utk_widget_update_state(upzw[i], 0);
	}
	
	for (; i < ARR_SZ(upzw); ++i) {
		utk_widget_update_state(upzw[i], 0);
	}
}

static void
clear_ucfg_ui(void)
{
	clear_ucfg();
	myent_update(&pwd_ent);
	myent_update(&usr_ent);
}

static int
is_valid_ucfg(void)
{
	int  plen;
	int  i;

	plen = strlen(pwdstr);
	if (plen > MAXPLEN||plen < MINPLEN) {
		DBG("--- password invalid ---\n");
		myent_clear(&pwd_ent);
		show_strtip(&utip, ustrs[0], ulen[0], 2, tip_w, qry_tip_y);
		return 0;
	}

	if (strlen(ustr) == 0) {
		DBG("--- user name empty ---\n");
		show_strtip(&utip, ustrs[1], ulen[1], 2, tip_w, qry_tip_y);
		return 0;
	}
	
	for (i = 0; i < uid-1; ++i) {
		if (strcmp(ustr, g_usr[i].name) == 0) {
			myent_clear(&usr_ent);
			show_strtip(&utip, ustrs[2], ulen[2], 2, tip_w, qry_tip_y);
			return 0;
		}
	}

	if (upri == 0) {
		DBG("--- no priority selected ---\n");
		show_strtip(&utip, ustrs[3], ulen[3], 2, tip_w, qry_tip_y);
		return 0;
	}
	return 1;
}


static void 
go_back(void *data)
{
	show_usrm_win();
}

static void 
user_save(void *data)
{
	if (kbar_entry_save_prepared(&uke) == 0) return;
	if (!is_valid_ucfg()) return;
	disable_syncfg();
	memcpy(usr.name, ustr, 24);
	memcpy(usr.passwd, pwdstr, sizeof(pwdstr));
	usr.pri = upri;
	usr.pz = upz;

	add_usr(&usr, uid);

	memset(&usr, 0, sizeof(usr_t));
	clear_ucfg_ui();
	++uid;
	snprintf(idstr, 2, "%d", uid);
	utk_label_update(idlbl);
}

static void
user_cancel(void *data)
{
	kbar_entry_cancel(&uke);
	clear_ucfg_ui();
}

static void
proriority_select(void *data)
{
	int  i = (int)data;
	int  st;
	
	st = utk_widget_state(upriw[i]);
	if (st) {
		upri |= (1<<i);
	} else {
		upri &= ~(1<<i);
	}
}

static void
pz_select(void *data)
{
	int  i = (int)data;
	int  st;
	
	st = utk_widget_state(upzw[i]);
	if (st) {
		upz |= (1<<i);
	} else {
		upz &= ~(1<<i);
	}
}

void
build_ucfg_gui(void)
{
	int  x[] = {psel_x0, psel_x1, psel_x2, psel_x3, psel_x4, psel_x5};
	int  y[] = {psel_y0, psel_y0, psel_y0, psel_y0, psel_y1, psel_y1};
	rect_t  lr = {.x = uid_lbl_x, .y = uid_lbl_y, .w = uid_lbl_w, .h = uid_lbl_h};
	rect_t  r = {.x = pwdcfg_bar_x0, .y = pwdcfg_bar_y, .w = pwdcfg_bar_w0, .h = pwdcfg_bar_h};
	rect_t  re = {.x = ucfg_pwd_x, .y = ucfg_ent_y, .w = ucfg_ent_w, .h = ucfg_ent_h};
	rect_t  rz = {.w = psel_w, .h = psel_h};
	int  i;
	int  px[] = {194, 364, 534, 705, 875};
	int  py[] = {385, 446};

	ulen[0] = strlen(ustrs[0]);
	ulen[1] = strlen(ustrs[1]);
	ulen[2] = strlen(ustrs[2]);
	ulen[3] = strlen(ustrs[3]);

	ucfgw = utk_window_derive();
#ifndef MAPDLL
	utk_widget_resident(ucfgw);
#endif

	utk_window_set_bg(ucfgw, BG_PWDCFG);
	utk_widget_hide_before(ucfgw, kbar_entry_cancel, &uke);

	register_qtimer((void*)ucfgw);

	stb_new(ucfgw, sysp_exit);

	idlbl = label_new(ucfgw, NULL, idstr, RGB_BLACK, &lr);

	build_entry_keyboard(&uke, ucfgw, 0, 600-335);

	myent_new(&pwd_ent, ucfgw, &re, 0);
	myent_add_caret(&pwd_ent, RGB_BLACK);
	myent_set_cache(&pwd_ent, pwdstr, sizeof(pwdstr), 6);
	myent_set_text_max(&pwd_ent, 6);
	myent_add_kbar(&pwd_ent, &uke);

	re.x = ucfg_usr_x;
	re.w = 150;
	myent_new(&usr_ent, ucfgw, &re, 0);
	myent_add_caret(&usr_ent, RGB_BLACK);
	myent_set_cache(&usr_ent, ustr, sizeof(ustr), 16);
	myent_add_kbar(&usr_ent, &uke);

	build_button_array1(ucfgw, upriw, ARR_SZ(x), psel_w, psel_h, x, y, BN_SEL, proriority_select);
	build_tip_lbl(&utip, ucfgw, (LCD_W-tip_w)/2, (LCD_H-tip_h)/2, BG_TIP, 0);
	for (i = 0; i < ARR_SZ(upzw); ++i) {
		rz.x = px[i%5];
		rz.y = py[i/5];
		upzw[i] = toggle_button_new(ucfgw, BN_SEL, &rz, pz_select, (void *)i);
	}

	////////////////////////////////////////////////////////////
	button_new(ucfgw, BN_RET160G, &r, (void *)go_back, (void *)0);

	r.w = pwdcfg_bar_w1;
	r.x = pwdcfg_bar_x1;
	button_new(ucfgw, BN_PWDSV, &r, (void *)user_save, (void *)0);

	r.w = pwdcfg_bar_w2;
	r.x = pwdcfg_bar_x2;
	button_new(ucfgw, BN_PWDCL, &r, (void *)user_cancel, (void *)0);
}

void
show_ucfg_win(int id)
{
	if (utk_window_top() != ucfgw) {
		memset(&usr, 0, sizeof(usr_t));
		clear_ucfg();
		uid = id;
		snprintf(idstr, 2, "%d", id);
		utk_window_show(ucfgw);
	}
}


