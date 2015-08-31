#include "ulib.h"
#include "utk.h"
#include "utksignal.h"
#include "utkwindow.h"
#include "utkbutton.h"
#include "utklabel.h"
#include "utkclock.h"
#include "utkmstlbl.h"
#include "utkvisitem.h"
#include "utklistbox.h"
#include "utktimeout.h"
#include "layout.h"
#include "gui.h"
#include "stb.h"
#include "config.h"
#include "zopt.h"

#define SYSFKEEPTM    (5*60)

typedef struct syst_s {
	char   evn[20];
	char   ststr[8];
} syst_t;

static UtkWindow   *systw;
static UtkListBox  *slb;
static id_col_t     zidc;

static syst_t  syst[MAXST];
static int     nrh = 1;

static char *systev[] = {
	"网络工作状态",
	"RS485工作状态",
	"1号主机交流供电",    /* EV_APF/EV_APR */
	"1号主机电池供电",    /* EV_DPF/EV_DPR */
	"1号主机电池电压低",  /* EV_BL/EV_BLR */
	"1号主机总线",        /* EV_HBUSE/EV_HBUSR */
	"1号主机防拆",        /* EV_A120DP/EV_A120DPR */
	"1号主机通讯",
	"2号主机交流供电",
	"2号主机电池供电",
	"2号主机电池电压低",
	"2号主机总线",
	"2号主机防拆",
	"2号主机通讯",
	};

static char *ststr[] = {"正常", "故障"};

static char         zbuf[GLB_ROWS][4];
static char        *idbuf[GLB_ROWS];

/* defined in start.c for display system info. */
extern void  sysfmap_set(int id);
extern void  sysfmap_clr(int id);
extern int   clear_sysfmap(int h);

static int
syst_init(void)
{
	int  i;

	nrh = get_host_nr();
	memset(syst, 0, sizeof(syst_t)*MAXST);
	for (i = 0; i < MAXST; ++i) {
		strncpy(syst[i].evn, systev[i], 18);
		strncpy(syst[i].ststr, ststr[0], 8);
	}
	return (2 + HSTNR*nrh);
}

static void
syst_pageup(void)
{
	if (utk_list_box_scroll_pageup(slb)) {
		id_colum_pageup(&zidc);
	}
}

static void
syst_pagedn(void)
{
	if (utk_list_box_scroll_pagedn(slb)) {
		id_colum_pagedn(&zidc);
	}
}

static PVOID  bn_cb[] = {show_stent_win, syst_pageup, syst_pagedn};

static void
do_syst_opt(void *data)
{
	int i = (int)data;
	bn_cb[i]();
}

static FVINT  fmset[] = {sysfmap_clr, sysfmap_set};

/* x: evid; y: state. */
static void
handle_syst(UtkWindow *win, int  x, int y)
{
	int st;
	int base;
	int id;
	int id1;
	int vis;

	if (win != systw) return;

	DBG("--- evid = %d, st = %d ---\n", x, y);
	vis  = utk_widget_is_visible(win);
	base = utk_list_box_get_id(slb, 0);
	
	id = x - base;
	if (x >= 2&&x < 8) {
		if (x != 7) {
			id1 = 7 - base;
			strncpy(syst[7].ststr, ststr[0], 8);
			if (vis &&(id1 >= 0)&&(id1 < GLB_ROWS)) {
				utk_list_box_updatex((UtkWindow*)slb, id1);
			}
			(*fmset[0])(7);
		}
	} else if (x >= 8) {
		if (x != 13) {
			id1 = 13 - base;
			strncpy(syst[13].ststr, ststr[0], 8);
			if (vis &&(id1 >= 0)&&(id1 < GLB_ROWS)) {
				utk_list_box_updatex((UtkWindow*)slb, id1);
			}
			(*fmset[0])(13);
		}
	}

	st = y & 1;
	strncpy(syst[x].ststr, ststr[st], 8);
	if (vis &&(id >= 0)&&(id < GLB_ROWS)) {
		utk_list_box_updatex((UtkWindow*)slb, id);
	}
	(*fmset[st])(x); //refresh the state flag in start.c

	/* state of label in stent.c */
	st = (y >> 1) ? 1 : 0;
	update_syst_label(st);
}

void
build_syst_gui(void)
{
	int   i;
	int   bw[] = {pzst_bar_w0, pzst_bar_w1, pzst_bar_w2};
	char *bg[] = {BN_RET184, BN_PU182, BN_PD182};

	rect_t	rid = {.x = 56, .y = 107, .w = 100, .h = row_h};
	rect_t	fr = {.x = 158, .y = 107, .w = (1024-(158+56)), .h = row_h};
	int   cx[] = {(500-120)/2, 502+(308-32)/2};
	int   chr[] = {16, 4};
	long  off[] = {U_STRUCT_OFFSET(syst_t, evn), U_STRUCT_OFFSET(syst_t, ststr)};
	int   sysn;

	sysn = syst_init();

	systw = utk_window_derive();
#ifndef MAPDLL
	utk_widget_resident(systw);
#endif
	utk_window_set_bg(systw, BG_SYST);
	utk_widget_async_evt_handler(systw, handle_syst);
	stb_new(systw, show_main_win);

	for (i = 0; i < GLB_ROWS; ++i) {
		idbuf[i] = &zbuf[i][0];
	}

	id_colum_init(&zidc, GLB_ROWS, sysn, 4);
	id_colum_new(&zidc, systw, idbuf, &rid, NULL);

	slb = listbox_new(systw, &fr, GLB_ROWS);
	listbox_set_font(slb, 1);
	listbox_set_cols(slb, ARR_SZ(cx), cx, chr, off);
	listbox_set_content(slb, syst, sizeof(syst_t), MAXST, sysn);
	listbox_add_rows(slb, NULL, GLB_ROWS);
	utk_list_box_set_text_clr(slb, LBX_TXT_CLR);
	utk_widget_disable((UtkWindow*)slb);

	build_button_array(systw, ARR_SZ(bw), pzst_bar_h, pzst_bar_y, bw, pzst_bar_x0, do_syst_opt, bg);
}

void
show_syst_win(void)
{
	if (utk_window_top() != systw) {
		utk_window_show(systw);
	}
}

/* st: 0-normal, 1-fault. */
void
update_syst(int id, int st)
{
	utk_widget_send_async_event(systw, id, st);
}

/* DONE: called in sys_gen.c */
void
update_hostnr(int n)
{
	if (n != nrh&&n <= 2&&n > 0) {
		utk_list_box_reload_item(slb, 2+HSTNR*n);
		id_colum_reset_base(&zidc);
		id_colum_set_maxid(&zidc, 2+HSTNR*n);
		id_colum_update(&zidc);
		nrh = n;
	}
}



