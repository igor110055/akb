#include "ulib.h"
#include "utk.h"
#include "utksignal.h"
#include "utkwindow.h"
#include "utkmap.h"
#include "utklabel.h"
#include "utkclock.h"
#include "utkmstlbl.h"
#include "utkvisitem.h"
#include "utklistbox.h"
#include "layout.h"
#include "gui.h"
#include "stb.h"
#include "config.h"
#include "pzone.h"
#include "zopt.h"
#include "qtimer.h"

static UtkWindow   *zmgrw;
static UtkListBox  *slb;
static id_col_t     zidc;
static char         zbuf[GLB_ROWS][4];
static char        *idbuf[GLB_ROWS];

static UtkWindow   *ddialog;
static tip_t        prterr;

static int          selectedid = -1;
static int          svflag = 0;

zone_t zonedb[MAXZ];
int    zarray[MAXZ];
int    zarcnt = 0;

static int
sort_iarray(int *arr, int n)
{
	int  i;
	int  v;

	if (n <= 1||arr[n-1] > arr[n-2]) return 0;
	v = arr[n-1];
	DBG("---- NEW INSERT: %d at: %d ----\n", v, n-1);
	for (i = 0; i < n-1; ++i) {
		if (v <= arr[i]) {
			DBG("------- arr[i] = %d, at: %d -------\n", arr[i], i);
			memmove(&arr[i+1], &arr[i], (n-1-i)*sizeof(int));
			arr[i] = v;
			return 1;
		}
	}
	return 0;
}

#ifdef DEBUG
static void
dump_zinfo(zone_t *z)
{
	char *ztname[] = {"instant zone", "delayed zone", "24hour zone"};
	printf("zone-%d: name:%s,detector:%s,type:%s,pzone:%d ---\n", z->znr, z->zname, z->dname, ztname[z->ztp], z->pnr);
}
#endif

static void
load_zonedb(void)
{
	int     i;
	zone_t *z;

	zarcnt = 0;
	printf("--- sizeof(zone_t) = %d ---\n", sizeof(zone_t));
	for (i = 0; i < MAXZ; ++i) {
		z = &zonedb[i];
		if (z->znr == i+1) {
			z->idx = zarcnt;
			zarray[zarcnt++] = i;
		}
	}
	zarray[zarcnt] = -1;
	zstate_init(zarcnt);
	DBG("--- ZONE COUNT: %d ---\n", zarcnt);
}

/* DONE: called when boot or finished syncfg. */
void
load_zone(void)
{
	FILE   *fp;
	int     n;

	zarcnt = 0;
	CLEARA(zonedb);
	if ((fp = fopen(ZONEDB, "r")) == NULL) {
		DBG("--- open %s failed ---\n", ZONEDB);
		return;
	}
	
	n = fread(&zonedb[0], sizeof(zone_t), MAXZ, fp);
	if (n == MAXZ) {
		load_zonedb();
	} else {
		DBG("--- read zoneinfo error: n = %d ---\n", n);
	}
	fclose(fp);
}

/* DONE */
static void
reload_zmgr_ui(void)
{
	utk_list_box_reload_item(slb, zarcnt);
	id_colum_reset_base(&zidc);
	id_colum_set_maxid(&zidc, zarcnt);
	id_colum_update(&zidc);
}

/* DONE */
static void
store_zone_file(void)
{
	FILE  *fp;
	int    x;

	if (zarcnt == 0) {
		CLEARA(zonedb);
		CLEARA(zarray);
		zstate_init(0);
		fi_clear(ZoneDbId);
		return;
	}

	if ((fp = fopen(ZONEDB, "w")) != NULL) {
		x = fwrite(&zonedb[0], sizeof(zone_t), MAXZ, fp);
		fflush(fp);
		fclose(fp);
		fi_update(ZoneDbId);
	}
	load_zonedb();
	reload_zmgr_ui();
}

int
store_zone(zone_t *z, int add)
{
	zone_t  *zb;
	int   srt = 0;

	zb = &zonedb[z->znr-1];
	if (add) { //new add zone info
		if (zb->znr != z->znr) {
			memcpy(zb, z, sizeof(zone_t));
			zarray[zarcnt] = z->znr - 1;
			++zarcnt;
			srt = sort_iarray(zarray, zarcnt);
		} else {
			printf("--- zonedb exists,won't add it. ---\n");
			return -1;
		}
		DBG("--- new zone add: zarcnt = %d ---\n", zarcnt);
	} else { //modify zone info,znr won't be changed.
		memcpy(zb, z, sizeof(zone_t));
		DBG("--- modify zone info: znr = %s ---\n", z->zstr);
	}

	++svflag;
	DBG("--- store zone ---\n");

	if (add) {
		if (srt == 0) {
			utk_list_box_add_item(slb, 1);
		} else {
			utk_list_box_reload_item(slb, zarcnt);
			id_colum_reset_base(&zidc);
		}
	}
	id_colum_set_maxid(&zidc, zarcnt);
	id_colum_update(&zidc);

	return 0;
}

static void
reload_zoneinfo(void)
{
	if (!is_master()||svflag == 0) return;

	DBG("-------- save the zone info to file -------\n");
	init_pztimer(0);
	store_zone_file();
	store_pzone_file();
	load_pzonedb();
	init_pzinfo();
	unlink(SYSTDB);

	clear_pzst_label();
	pzent_enable(pzcnt); //sys_tmr.c
	load_zinf(); //zinfo.c

	sync_zone_ui();
	sync_pzone_ui();
	svflag = 0;
}

/* DONE: called by slave in syncfg.c */
void
reload_zonedb(void)
{
	load_zone();
	reload_zmgr_ui();
	load_zinf();
}

static void
zone_add(void)
{
	if (!is_master()) return;
	if (zarcnt >= MAXZ) {
		DBG("--- zonedb number up to limit ---\n");
	} else {
		show_zcfg_win(NULL, zarcnt + 1);
	}
}

static void
zone_mod(void)
{
	int  id;

	if (!is_master()) return;
	if (selectedid < 0) return;
	id = utk_list_box_get_id(slb, selectedid);
	DBG("--- modify zonedb-%d ---\n", id);
	show_zcfg_win(&zonedb[zarray[id]], id + 1);
	if (selectedid >= 0) {
		utk_list_box_unselect(slb, selectedid);
		selectedid = -1;
	}
}

static void
del_opt(void *data)
{
	int id;
	int z;

	hide_model_dialog(ddialog);

	id = utk_list_box_get_id(slb, selectedid);
	z = zarray[id];
	DBG("--- delete zone-%d ---\n", z+1);
	zmap_del(z); //delete zone in map.

	if (utk_list_box_del_item(slb, selectedid) >= 0) {
		--zarcnt;
		++svflag;
		DBG("--- zone-%d deleted ---\n", z+1);
		id_colum_set_maxid(&zidc, zarcnt);
		id_colum_update(&zidc);
	}
	if (selectedid >= 0) {
		utk_list_box_unselect(slb, selectedid);
		selectedid = -1;
	}
}

static void
cancel_del(void *data)
{
	hide_model_dialog(ddialog);
	if (selectedid >= 0) {
		utk_list_box_unselect(slb, selectedid);
		selectedid = -1;
	}
}

static void 
zone_del(void)
{
	if (!is_master()) return;
	if (selectedid < 0) return;
	if (priority_get(PRI_DEL) == 0) {
		show_tip(&prterr);
		return;
	}

	show_model_dialog(ddialog);
}

static void 
zmgr_pageup(void)
{
	if (utk_list_box_scroll_pageup(slb)) {
		id_colum_pageup(&zidc);
	}
}

static void
zmgr_pagedn(void)
{
	if (utk_list_box_scroll_pagedn(slb)) {
		id_colum_pagedn(&zidc);
	}
}

static void
go_back(void)
{
	reload_zoneinfo();
	show_sysp_win();
}

static PVOID  bn_cb[] = {go_back, zone_add, zone_mod, zone_del, zmgr_pageup, zmgr_pagedn};

static void
do_zmgr_opt(void *data)
{
	int i = (int)data;
	bn_cb[i]();
}

static void
selected_handle(void *data)
{
	int i = (int)data;

	if ((i>>8)&1) {
		selectedid = i&0xff;
		DBG("--- selected = %d ---\n", i&0xff);
	} else {
		selectedid = -1;
		DBG("--- unselected = %d ---\n", i&0xff);
	}
}

static void
id_col_sel(void *data)
{
	int i = (int)data;
	utk_list_box_toggle_item((UtkWindow*)slb, i);
}

static char *gztp[] = {
	"即时防区",
	"延时防区",
	"24小时防区"
};

int
show_ztp(void *p, char *buf, int n)
{
	zone_t  *z = p;
	return snprintf(buf, n-1, "%s", gztp[z->ztp]);
}

static void
hide_before(void *arg)
{
	if (selectedid > 0) {
		utk_list_box_unselect_all(slb);
		selectedid = 0;
	}
	reload_zoneinfo();
}

void
build_zmgr_gui(void)
{
	int   i;
	int   bw[] = {zmbar_w0, zmbar_w1, zmbar_w2, zmbar_w3, zmbar_w4, zmbar_w5};
	char *bg[] = {BN_RET160G, BN_ZADD, BN_ZMOD, BN_DEL156G, BN_ZPU, BN_ZPD};
	char *bgok[] = {BN_DELOK0, BN_DELOK1};
	char *bgcl[] = {BN_DELCL0, BN_DELCL1};

	rect_t  r = {.x = id_col_x, .y = id_col_y, .w = id_col_w, .h = row_h};
	rect_t  fr = {.x = ZLB_X, .y = ZLB_Y, .w = ZLB_W, .h = row_h};
	int   cx[5] = {0};
	int   co[5] = {0};
	int   cw[] = {100, 100, 364, 119, 119};
	int   chr[] = {4, 2, 28, 10, 8};
	long  off[] = {U_STRUCT_OFFSET(zone_t, zstr), U_STRUCT_OFFSET(zone_t, pstr), U_STRUCT_OFFSET(zone_t, zname), 0, U_STRUCT_OFFSET(zone_t, dname)};
	int   fw[] = {8, 12};
	int   dw = 0;
	int   ft = ZLBX_FT;

	zmgrw = utk_window_derive();
#ifndef MAPDLL
	utk_widget_resident(zmgrw);
#endif

	utk_window_set_bg(zmgrw, BG_ZMGR);
	utk_widget_hide_before(zmgrw, hide_before, (void*)0);

	register_qtimer((void*)zmgrw);

	stb_new(zmgrw, sysp_exit);
	build_tip_lbl(&prterr, zmgrw, (LCD_W-tip_w)/2, (LCD_H-tip_h)/2, LBL_PRTERR, 1);
	ddialog = build_model_dialog(zmgrw, BG_DELDIAG, bgok, bgcl, del_opt, cancel_del);

	for (i = 0; i < GLB_ROWS; ++i) {
		idbuf[i] = &zbuf[i][0];
	}

	id_colum_init(&zidc, GLB_ROWS, zarcnt, 4);
	id_colum_new(&zidc, zmgrw, idbuf, &r, id_col_sel);

again:
	for (i = 0; i < 5; ++i) {
		co[i] = cw[i] - chr[i]*fw[ft];
		if (co[i] < 0) {
			if (ft > 0) {
				--ft;
				goto again;
			}
		}
		co[i] >>= 1;
	}

	for (i = 0; i < 5; ++i) {
		cx[i] = dw + co[i];
		dw += cw[i] + 2;
	}

	slb = listbox_new(zmgrw, &fr, GLB_ROWS);
	listbox_set_font(slb, ft);
	listbox_set_cols(slb, ARR_SZ(cx), cx, chr, off);
	listbox_set_content(slb, zonedb, sizeof(zone_t), MAXZ, 0);
	listbox_add_rows(slb, selected_handle, GLB_ROWS);
	utk_list_box_set_hash_array(slb, zarray, zarcnt);
	utk_list_box_set_text_clr(slb, LBX_TXT_CLR);
	utk_list_box_set_col_cb(slb, 3, show_ztp);

	build_button_array(zmgrw, ARR_SZ(bw), zmbar_h, zmbar_y, bw, zmbar_x0, do_zmgr_opt, bg);
}

void
show_zmgr_win(void)
{
	if (utk_window_top() != zmgrw) {
		utk_window_show(zmgrw);
	}
}


