#include <ctype.h>
#include "ulib.h"
#include "utk.h"
#include "utksignal.h"
#include "utkwindow.h"
#include "utklabel.h"
#include "utkmap.h"
#include "utkvisitem.h"
#include "utklistbox.h"
#include "layout.h"
#include "gui.h"
#include "stb.h"
#include "config.h"
#include "qtimer.h"

#define ULB_SELCLR   RGB_BLUE
#define ULB_COLS     2

static UtkWindow   *umw;
static UtkListBox  *ulb;
static id_col_t     uidc;
static char         ubuf[8][4];
static char        *idptr[10];

static UtkWindow   *ddialog;
static tip_t        prterr;


usr_t   g_usr[MAXUSR];
int     g_ucnt = 0;
static int     selecteduid = -1;
static int     changed = 0;

static char  pwdmd5v[MAXUSR][36];
static char  sumd5v[36] = {0};

int
verify_user(char *name, char *md5str)
{
	int  i;
	usr_t  *suser = NULL;

	suser = get_superuser();
	if ((strcmp(name, "admin") == 0||strcmp(name, suser->name) == 0)
	   &&strcmp(md5str, sumd5v) == 0)
	{
		return 0;
	}

	for (i = 0; i < g_ucnt; ++i)
	{
		if (strcmp(name, g_usr[i].name) == 0) {
			break;
		}
	}

	if (i >= g_ucnt||strcmp(md5str, pwdmd5v[i]) != 0)
	{
		DBG("--- user name not exists or password not matched ---\n");
		return 2;
	}

	if (usr_priority(i+1, PRI_PG))
		return 0;
	DBG("--- the user don't have the priority ---\n");
	return 1;
}

void
load_usr(void)
{
	FILE  *fp;
	int    cnt;
	int    i;
	int    j;
	int    n;
	char   md5[36] = {0};

	memset(&pwdmd5v[0][0], 0, MAXUSR*36);
	memset(&g_usr[0], 0, sizeof(usr_t)*MAXUSR);
	u_md5_data(SUPASSWD, strlen(SUPASSWD), md5);
	for (j = 0; j < strlen(md5); ++j) {
		sumd5v[j] = toupper(md5[j]);
	}
	DBG("--- su password: %s ---\n", sumd5v);

	if ((fp = fopen(USRDB, "r")) == NULL) return;
	if (fread(&cnt, sizeof(cnt), 1, fp) != 1) {
		fclose(fp);
		return;
	}

	if (fread(&g_usr[0], sizeof(usr_t), cnt, fp) != cnt) {
		DBG("--- fread error: %s ---\n", strerror(errno));
		fclose(fp);
		return;
	}
	fclose(fp);
	g_ucnt = cnt;

	for (i = 0; i < cnt; ++i) {
		n = strlen(g_usr[i].passwd);
		memset(md5, 0, sizeof(md5));
		u_md5_data((unsigned char*)g_usr[i].passwd, n, md5);
		for (j = 0; j < strlen(md5); ++j) {
			pwdmd5v[i][j] = toupper(md5[j]);
		}
		DBG("--- password-%d: %s ---\n", i, pwdmd5v[i]);
	}
}

static inline void
reload_usr_listbox(void)
{
	utk_list_box_reload_item(ulb, g_ucnt);
	id_colum_reset_base(&uidc);
	id_colum_set_maxid(&uidc, g_ucnt);
	id_colum_update(&uidc);
}

void
reload_usrdb(void)
{
	load_usr();
	reload_usr_listbox();
}

static void
usr_save(void)
{
	FILE  *fp;
	int    i;

	if (!is_master()) return;
	for (i = 0; i < g_ucnt; ++i) {
		g_usr[i].id = i + 1;
	}
	if ((fp = fopen(USRDB, "w")) == NULL) return;
	ignore_result(fwrite(&g_ucnt, sizeof(g_ucnt), 1, fp));
	if (fwrite(&g_usr[0], sizeof(usr_t), g_ucnt, fp) != g_ucnt) {
		DBG("--- fwrite error: %s ---\n", strerror(errno));
	}
	fflush(fp);
	fclose(fp);
	fi_update(UsrDbId);
	changed = 0;
}

static void
usr_add(void)
{
	if (!is_master()) return;
	if (g_ucnt >= MAXUSR) {
		DBG("--- user number up to limit ---\n");
	} else {
		show_ucfg_win(g_ucnt + 1);
	}
}

static void
del_opt(void *data)
{
	hide_model_dialog(ddialog);
	utk_list_box_del_item(ulb, selecteduid);
	--g_ucnt;
	if (g_ucnt < 0)
		g_ucnt = 0;
	memset(&g_usr[g_ucnt], 0, sizeof(usr_t));
	id_colum_set_maxid(&uidc, g_ucnt);
	id_colum_update(&uidc);
	++changed;

	if (g_ucnt == 0) {
		fi_clear(UsrDbId);
	}
}

static void
cancel_del(void *data)
{
	hide_model_dialog(ddialog);
}


static void
usr_del(void)
{
	if (!is_master()) return;
	if (selecteduid < 0) return;
	if (priority_get(PRI_DEL) == 0) {
		show_tip(&prterr);
		return;
	}

	show_model_dialog(ddialog);
}

void
add_usr(usr_t *u, int id)
{
	if (!is_master()) return;
	if (id > MAXUSR||id != g_ucnt + 1) return;

	memcpy(&g_usr[id-1], u, sizeof(usr_t));
	g_usr[id-1].id = id;
	++g_ucnt;

	utk_list_box_add_item(ulb, 1);
	id_colum_set_maxid(&uidc, g_ucnt);
	id_colum_update(&uidc);
	++changed;
}

static PVOID  bn_cb[] = {show_sysp_win, usr_add, usr_del, usr_save};

static void
do_usrm_opt(void *data)
{
	int i = (int)data;
	bn_cb[i]();
}

static void
go_gencfg(void *data)
{
	show_gencfg_win();
}

static void
selected_handle(void *data)
{
	int i = (int)data;

	if ((i>>8)&1) {
		selecteduid = i&0xff;
		DBG("--- selected = %d ---\n", i&0xff);
	} else {
		selecteduid = -1;
		DBG("--- unselected = %d ---\n", i&0xff);
	}
}

static void
id_col_touch(void *data)
{
	int i = (int)data;
	utk_list_box_toggle_item((UtkWindow*)ulb, i);
}

static void
if_save(void *data)
{
	if (!is_master()) return;
	if (changed > 0&&g_ucnt > 0) {
		usr_save();
		sysp_ref();
	}
	changed = 0;
	if (selecteduid > 0) {
		utk_list_box_unselect_all(ulb);
		selecteduid = 0;
	}
}

void
build_usrm_gui(void)
{
	UtkWindow *cmap;
	int   i;
	int   bw[] = {um_bar_w0, um_bar_w1, um_bar_w2, um_bar_w3};
	char *bg[] = {BN_RET160G, BN_UADD, BN_DEL156G, BN_USAVE};
	char *bgok[] = {BN_DELOK0, BN_DELOK1};
	char *bgcl[] = {BN_DELCL0, BN_DELCL1};

	rect_t  r = {.x = uid_col_x, .y = uid_col_y, .w = uid_col_w, .h = row_h};
	rect_t  fr = {.x = ULB_X, .y = ULB_Y, .w = ULB_W, .h = row_h};
	int   cx[] = {ulb_col_x0, ulb_col_x1};
	int   chr[] = {18, 20};
	long  off[] = {U_STRUCT_OFFSET(usr_t, passwd), U_STRUCT_OFFSET(usr_t, name)};

	umw = utk_window_derive();
#ifndef MAPDLL
	utk_widget_resident(umw);
#endif
	utk_window_set_bg(umw, BG_USRM);
	utk_widget_hide_before(umw, if_save, NULL);

	register_qtimer((void*)umw);

	for (i = 0; i < MAXUSR; ++i) {
		idptr[i] = &ubuf[i][0];
	}

	stb_new(umw, sysp_exit);
	build_tip_lbl(&prterr, umw, (LCD_W-tip_w)/2, (LCD_H-tip_h)/2, LBL_PRTERR, 1);
	ddialog = build_model_dialog(umw, BG_DELDIAG, bgok, bgcl, del_opt, cancel_del);

	cmap = (UtkWindow*)utk_map_new();
	utk_widget_set_size(cmap, tmap_w, tmap_h);
	utk_widget_add_cntl(umw, cmap, tmap_x1, tmap_y);
	utk_signal_connect(cmap, "clicked", go_gencfg, 0);

	id_colum_init(&uidc, MAXUSR, g_ucnt, 4);
	id_colum_new(&uidc, umw, idptr, &r, id_col_touch);

	ulb = listbox_new(umw, &fr, MAXUSR);
	listbox_set_font(ulb, 1);
	listbox_set_cols(ulb, ULB_COLS, cx, chr, off);
	listbox_set_content(ulb, g_usr, sizeof(usr_t), MAXUSR, g_ucnt);
	listbox_add_rows(ulb, selected_handle, MAXUSR);
	utk_list_box_set_text_clr(ulb, LBX_TXT_CLR);

	////////////////////////////////////////////////////////////
	build_button_array(umw, ARR_SZ(bw), um_bar_h, um_bar_y, bw, um_bar_x0, do_usrm_opt, bg);
}

void
show_usrm_win(void)
{
	if (utk_window_top() != umw) {
		utk_window_show(umw);
	}
}



