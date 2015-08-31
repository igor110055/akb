#include "ulib.h"
#include "unotifier.h"
#include "utk.h"
#include "utksignal.h"
#include "utkwindow.h"
#include "utkbutton.h"
#include "utklabel.h"
#include "utkclock.h"
#include "utkmap.h"
#include "utkvisitem.h"
#include "utklistbox.h"
#include "config.h"
#include "layout.h"
#include "gui.h"
#include "config.h"
#include "stb.h"
#include "zopt.h"
#include "pztimer.h"

static UtkWindow  *tckw;
static UtkWindow  *tlbl[2];
static UtkListBox *plb;

static UtkWindow  *tico0[MAXTPZ];
static UtkWindow  *tico1[MAXTPZ];

static char tmstr0[16] = {0};
static char tmstr1[16] = {0};

static pztm_t  *pzt = NULL;

static struct u_notifier_node  node;

static void 
go_back(void *data)
{
	show_tmr_win();
}

static void
load_timeslice(void)
{
	memset(tmstr0, 0, 16);
	memset(tmstr1, 0, 16);

	if (pzt->tm[0].hur < 24&&pzt->tm[0].min < 60&&pzt->tm[1].hur < 24&&pzt->tm[1].min < 60) {
		sprintf(tmstr0, "%02d:%02d--%02d:%02d", pzt->tm[0].hur, pzt->tm[0].min, pzt->tm[1].hur, pzt->tm[1].min);
	}

	if (pzt->tm[2].hur < 24&&pzt->tm[2].min < 60&&pzt->tm[3].hur < 24&&pzt->tm[3].min < 60) {
		sprintf(tmstr1, "%02d:%02d--%02d:%02d", pzt->tm[2].hur, pzt->tm[2].min, pzt->tm[3].hur, pzt->tm[3].min);
	}
}

static void
update_tmlbl(struct u_notifier_node* node, unsigned long v, void *data)
{
	load_timeslice();
	DBG("---- update time lable ---\n");
}

static void
set_time_ico(pztm_t *pzt)
{
	int  i;

	for (i = 0; i < MAXTPZ; ++i) {
		if (i >= tpzcnt) {
			utk_widget_set_state(tico0[i], -1);
			utk_widget_set_state(tico1[i], -1);
		} else {
			utk_widget_set_state(tico0[i], pzt->pt[i]&1);
			utk_widget_set_state(tico1[i], (pzt->pt[i]>>1)&1);
		}
	}
}

void
build_tmrck_gui(void)
{
	rect_t  r0 = {.x = bn_tcret_x, .y = bn_tbar_y, .w = bn_tcret_w, .h = bn_tbar_h};
	rect_t  r1 = {.x = tmr_lbl_x0, .y = tmr_lbl_y, .w = tmr_lbl_w, .h = tmr_lbl_h};
	rect_t  r2 = {.x = tmr_lbl_x1, .y = tmr_lbl_y, .w = tmr_lbl_w, .h = tmr_lbl_h};

	rect_t  fr = {.x = 56, .y = 107, .w = 492, .h = row_h};
	int   cx[] = {((120-20)/2), ((370-300)/2+122)};
	int   chr[] = {2, 30};
	long  off[] = {U_STRUCT_OFFSET(pzone_t, pstr), U_STRUCT_OFFSET(pzone_t, pname)};
	char  *bgx[] = {PTARM_UNSEL, PTARM_SEL};

	tckw = utk_window_derive();
#ifndef MAPDLL
	utk_widget_resident(tckw);
#endif
	utk_window_set_bg(tckw, BG_TMRCK);

	pzt = get_pztm_conf();
	stb_new(tckw, show_main_win);

	////////////////////////////////////////////////////////////	
	button_new(tckw, BN_RET136, &r0, (void *)go_back, (void*)0);

	plb = listbox_new(tckw, &fr, MAXTPZ);
	listbox_set_font(plb, 1);
	listbox_set_cols(plb, ARR_SZ(cx), cx, chr, off);
	listbox_set_content(plb, pzone, sizeof(pzone_t), MAXTPZ, 0);
	listbox_add_rows(plb, NULL, MAXTPZ);
	utk_list_box_set_hash_array(plb, tpzarr, tpzcnt);
	utk_list_box_set_text_clr(plb, LBX_TXT_CLR);
	utk_widget_disable((UtkWindow*)plb);

	build_label_col((void**)tico0, tckw, (208-24)/2+550, 107+(row_h-29)/2, row_h, MAXTPZ, bgx, ARR_SZ(bgx));
	build_label_col((void**)tico1, tckw, (208-24)/2+760, 107+(row_h-29)/2, row_h, MAXTPZ, bgx, ARR_SZ(bgx));

	set_time_ico(pzt);

	load_timeslice();
	tlbl[0] = label_new(tckw, NULL, tmstr0, RGB_BLACK, &r1);
	tlbl[1] = label_new(tckw, NULL, tmstr1, RGB_BLACK, &r2);

	u_notifier_node_init(&node, update_tmlbl, 0, NULL);
	register_time_change_notifier(&node);
}

void
show_tmrck_win(void)
{
	if (utk_window_top() != tckw) {
		utk_window_show(tckw);
	}
}

void
reload_tmrck_ui(void)
{
	if (plb) {
		utk_list_box_reload_item(plb, tpzcnt);
		if (!pzt) {
			pzt = get_pztm_conf();
		}

		set_time_ico(pzt);
	}
}


