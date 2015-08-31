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
#include "utktimeout.h"
#include "layout.h"
#include "config.h"
#include "gui.h"
#include "stb.h"
#include "zopt.h"
#include "xfer.h"


#define  ZALB_ROWS       6

//////////////////////////////////////////////////
static UtkWindow      *from = NULL;
static UtkWindow      *zalrmw;
static UtkListBox     *zalb;
static UtkWindow      *almlbl;

static int             selectedid = -1;

static unsigned short  zacnt = 0;
static zainf_t         zainf[MAXZA];
static int             za_last = -1;

static unsigned int    said = 0;

static char   almbuf[128] = {0};

//static unsigned short  g_aidx = 0;

static char *zainfstr[] = {
	[ZALR] = "报警恢复",
	[ZALT] = "防区报警",
	[ZFLR] = "故障恢复",
	[ZFLT] = "防区故障",
	[ZARM] = "防区布防",
	[ZDAM] = "防区撤防",
	[ZBPA] = "防区旁路",
	[ZBPR] = "旁路恢复",
};

static char *painfstr[] = {
	[AARM] = "全部布防",
	[FARM] = "强制布防",
	[ADAM] = "全部撤防",	
	[PARM] = "分区布防",
	[PFAM] = "强制布防",
	[PDAM] = "分区撤防",
	[PBPA] = "分区旁路",
	[PBPR] = "旁路恢复",
	[PTAM] = "定时布防",
	[PTDM] = "定时撤防",
};

/* z: [1, MAXZ] */
static void
update_alarm_label(int z, int op)
{
	int  h;
	int  zz;

	--z;
	h = z/99+1;
	zz = z%99+1;
	DBG("--- zidx = %d, host = %d, zone = %d ---\n", z, h, zz);
	memset(almbuf, 0, sizeof(almbuf));
	snprintf(almbuf, sizeof(almbuf)-1, "%d主机%d防区[%s]%s", h, zz, zonedb[z].zname, op?"报警":"报警恢复");
	utk_label_update(almlbl);
}

int
get_alarm_count(void)
{
	return zacnt;
}

void
load_alarm(void)
{
	FILE  *fp;
	unsigned short  cnt;

	if ((fp = fopen(ALRMDB, "r")) == NULL) return;
	if (fread(&cnt, sizeof(cnt), 1, fp) != 1) {
		fclose(fp);
		return;
	}

	cnt = (cnt > MAXZA) ? MAXZA : cnt;
	if (fread(&zainf[0], sizeof(zainf_t), cnt, fp) == cnt) {
		zacnt = cnt;
	}
	DBG("--- LOAD UNHANDLED ALARM: %d ---\n", zacnt);
	fclose(fp);
}

static void
store_alarm_file(void)
{
	FILE  *fp;
	int    x;

	if ((fp = fopen(ALRMDB, "w")) != NULL) {
		x = fwrite(&zacnt, sizeof(zacnt), 1, fp);
		x = fwrite(&zainf[0], sizeof(zainf_t), zacnt, fp);
		fflush(fp);
		fclose(fp);
	}
}

static void
save_alarm(void *arg)
{
	if (zacnt > 0) {
		DBG("--- time to save alarm ---\n");
		store_alarm_file();
	} else {
		remove(ALRMDB);
	}
}

/* znr: 1~512 */
int
mk_zainf(zainf_t *zi, int opt, unsigned short znr)
{
	zone_t  *z;

	if (znr == 0||znr > MAXZ) return 0;
	z = (zonedb[znr-1].znr == znr) ? &zonedb[znr-1] : NULL;
	if (z == NULL) return 0;

	memset(zi, 0, sizeof(zainf_t));
	zi->opcode = (unsigned short)opt;
	memcpy(zi->zstr, z->zstr, 6);
	memcpy(zi->zname, z->zname, MAXNAME);
	snprintf(zi->pstr, 4, "%02d", z->pnr);
	zi->znr = znr;
	zi->pnr = z->pnr;
	get_timestr(&zi->etm, zi->tmstr, 20);
	strncpy(zi->inf, zainfstr[opt], 16);
	DBG("--- alarm: zone:%s, name:%s, pznr:%s: %s ---\n", zi->zstr, zi->zname, zi->pstr, zi->inf);

	return 1;
}

/* p: 1~MAXPZ */
int
mk_painf(zainf_t *zi, int opt, unsigned short p)
{
	pzone_t *z;

	if (pzone[p-1].pnr != p) return 0;

	z = &pzone[p-1];
	memset(zi, 0, sizeof(zainf_t));
	zi->opcode = (unsigned short)opt;
	sprintf(zi->zstr, "0000");
	memcpy(zi->zname, z->pname, MAXNAME);
	snprintf(zi->pstr, 4, "%02d", z->pnr);
	zi->znr = 0;
	zi->pnr = p;
	get_timestr(&zi->etm, zi->tmstr, 20);
	opt -= MAXZCMD;

	strncpy(zi->inf, painfstr[opt], 16);
	DBG("--- alarm: pzone:%s, name:%s: %s ---\n", zi->pstr, zi->zname, zi->inf);

	return 1;
}

void
mk_aainf(zainf_t *zi, int opt)
{
	memset(zi, 0, sizeof(zainf_t));
	zi->opcode = (unsigned short)opt;
	sprintf(zi->zstr, "0000");
	sprintf(zi->zname, "所有防区");
	sprintf(zi->pstr, "00");
	zi->znr = 0;
	zi->pnr = 0;
	get_timestr(&zi->etm, zi->tmstr, 20);
	opt -= MAXZCMD;
	strncpy(zi->inf, painfstr[opt], 16);
	DBG("--- alarm: pzone:%s, name:%s: %s ---\n", zi->pstr, zi->zname, zi->inf);
}

static void
add_alarm(int op, int arg)
{
	int  wrap = 0;

	if (zacnt == MAXZA) {
		memmove(&zainf[0], &zainf[1], (zacnt-1)*sizeof(zainf_t));
		--zacnt;
		wrap = 1;
	}

	if (mk_zainf(&zainf[zacnt], op, arg) == 0) return;
	utk_timeout_remove(said);

	zainf[zacnt].idx = zacnt;
	memset(zainf[zacnt].idstr, 0, 8);
	snprintf(zainf[zacnt].idstr, 8, "%d", zacnt);
	update_alarm_label(arg, op);

	play_alarm();

	add_alarm_info(&zainf[zacnt]);

	++zacnt;

	if (wrap == 0) {
		utk_list_box_add_item(zalb, 1);
	} else {
		utk_list_box_reload_item(zalb, zacnt);
	}
	utk_timeout_add_once(300, said);
}

static void 
go_back(void)
{
	if (from&&from != desktop_window())
		utk_window_show(from);
	else
		show_main_win();
}

static void
reindex_ainfo1(int fr)
{
	int i;

	for (i = fr; i < zacnt; ++i) {
		zainf[i].idx = i;
		memset(zainf[i].idstr, 0, 8);
		snprintf(zainf[i].idstr, 8, "%d", i);
	}
}

static void
clear_ainf(int id)
{
	--zacnt;
	if (zacnt == 0) {
		relay_off();
		stop_alarm_all();
		go_back();
	} else {
		reindex_ainfo1(id);
		utk_list_box_update((UtkWindow*)zalb);
	}
	utk_timeout_add_once(300, said);
}

static void
_handle_ainf(int id)
{
	int  vid;

	if (zacnt == 0) return;
	utk_timeout_remove(said);
	vid = utk_list_box_get_id(zalb, 0);
	if (id >= vid&&id < ZALB_ROWS + vid) {
		utk_list_box_del_item(zalb, id - vid);
	}

	clear_ainf(id);
}


static void 
handle_ainf(void)
{
	int  id;

	if (selectedid < 0) return;

	utk_timeout_remove(said);
	id = utk_list_box_get_id(zalb, selectedid);
	utk_list_box_del_item(zalb, selectedid);

	clear_ainf(id);

	if (is_master()) {
		netxfer_alarmclr(id);
	} else {
		netxfer_alarmclr_slave(id);
	}
}

static void
_handle_ainf_all(void)
{
	if (zacnt == 0) return;
	utk_timeout_remove(said);

	utk_list_box_clear_all((UtkWindow*)zalb);
	stop_alarm_all();
	relay_off();
	zacnt = 0;
	remove(ALRMDB);
	go_back();
}

static void 
handle_ainf_all(void)
{
	if (is_master()) {
		netxfer_alarmclr(-1);
	} else {
		netxfer_alarmclr_slave(-1);
	}
	_handle_ainf_all();
}

static void 
amap_check(void)
{
	int  id;

	if (selectedid < 0) {
		show_map_win(2, za_last-1);
	} else {
		id = utk_list_box_get_id(zalb, selectedid);
		DBG("--- zone-%d alarmed ---\n", zainf[id].znr);
		show_map_win(2, zainf[id].znr-1);
	}
}

static void 
zalrm_pageup(void)
{
	utk_list_box_scroll_pageup(zalb);
}

static void
zalrm_pagedn(void)
{
	utk_list_box_scroll_pagedn(zalb);
}

static PVOID  bn_cb[] = {handle_ainf, handle_ainf_all, amap_check, zalrm_pageup, zalrm_pagedn};

static void
do_zalrm_opt(void *data)
{
	int i = (int)data;
	bn_cb[i]();
}

static void
selected_handle(void *data)
{
	int i = (int)data;
	if ((i >> 8)&1) {
		selectedid = i&0xff;
		DBG("--- selected = %d ---\n", i&0xff);
	} else {
		selectedid = -1;
		DBG("--- unselected = %d ---\n", i&0xff);
	}
}

static void
handle_async_evt(UtkWindow *win, int op, int arg)
{
	if (arg == 0) {
		if (op < 0) {
			_handle_ainf_all();
		} else {
			_handle_ainf(op);
		}
	} else {
		add_alarm(op, arg);
		za_last = arg;
	}
}

#define alm_inf_w       (672)
#define alm_inf_h       32
#define alm_inf_x       177
#define alm_inf_y       (80+(92-alm_inf_h)/2)

static void
build_alarm_label(UtkWindow *parent)
{
	almlbl = utk_label_new_fp();
	utk_label_set_fnt((UtkLabel*)almlbl, 2);
	utk_widget_set_size(almlbl, alm_inf_w, alm_inf_h);
	utk_label_set_text_size((UtkLabel*)almlbl, 54);
	utk_label_set_text_color((UtkLabel*)almlbl, RGB_WHITE);
	utk_label_set_text((UtkLabel *)almlbl, almbuf);
	utk_widget_add_cntl(parent, almlbl, alm_inf_x, alm_inf_y);
}

void
build_zalrm_gui(void)
{
	int   bw[] = {/*ainfck_bar_w0,*/ ainfck_bar_w1, ainfck_bar_w2, ainfck_bar_w3, ainfck_bar_w4, ainfck_bar_w5};
	char *bg[] = {/*BN_RET160G,*/ BN_AIHDL, BN_AIHDLA, BN_MAPCK, BN_AICKPU1, BN_AICKPD1};
	rect_t  fr  = {.x = ZALB_X, .y = ZALB_Y, .w = ZALB_W, .h = row_h};
	int   cx[]  = {zalb_col_x0, zalb_col_x1, zalb_col_x2, zalb_col_x3, zalb_col_x4, zalb_col_x5};
	int   chr[] = {6, 4, 30, 2, 12, 19};
	long  off[] = {U_STRUCT_OFFSET(zainf_t, idstr), U_STRUCT_OFFSET(zainf_t, zstr), U_STRUCT_OFFSET(zainf_t, zname), U_STRUCT_OFFSET(zainf_t, pstr),
		           U_STRUCT_OFFSET(zainf_t, inf), U_STRUCT_OFFSET(zainf_t, tmstr)};

	zalrmw = utk_window_derive();
	utk_widget_resident(zalrmw);
	utk_window_set_bg(zalrmw, BG_AINFCK);
	utk_widget_async_evt_handler(zalrmw, handle_async_evt);
	said = utk_widget_timer(zalrmw, save_alarm, (void*)0);

	stb_new(zalrmw, NULL);

	build_alarm_label(zalrmw);

	zalb = listbox_new(zalrmw, &fr, ZALB_ROWS);
	listbox_set_font(zalb, 0);
	listbox_set_cols(zalb, ARR_SZ(cx), cx, chr, off);
	listbox_set_content(zalb, zainf, sizeof(zainf_t), MAXZA, zacnt);
	listbox_add_rows(zalb, selected_handle, ZALB_ROWS);
	utk_list_box_set_text_clr(zalb, LBX_TXT_CLR);

	////////////////////////////////////////////////////////////
	build_button_array(zalrmw, ARR_SZ(bw), ainfck_bar_h, ainfck_bar_y, bw, ainfck_bar_x1, do_zalrm_opt, bg);
}

void
show_zalrm_win(void)
{
	UtkWindow *pWin;

	/* should login first */
	if (curr_usr == NULL) return;
	pWin = utk_window_top();
	if (pWin != zalrmw)
	{
		from = pWin;
		utk_window_show(zalrmw);
	}
}

/* arg: [1, MAXZ] */
void
send_alarm(int opt, int arg)
{
	if (zalrmw) {
		show_zalrm_win();
		utk_widget_send_async_event(zalrmw, opt, arg);
	}
}

void
clear_alarm(int id)
{
//	if (utk_window_top() == zalrmw) {
		utk_widget_send_async_event(zalrmw, id, 0);
//	}
}

int
unhandled_zalarm(void)
{
	return zacnt;
}


