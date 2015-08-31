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

#define  TMSTRLEN       4

static UtkWindow   *genw;
static UtkWindow   *cbx[7];

genchr_t     genchr;
static int   geninit = 0;

static tip_t nomap;


static kbar_ent_t   gke;
static myent_t      itm_ent[2];
static char         itmstr[2][TMSTRLEN] = {{0},};
static char         g_itmstr[2][TMSTRLEN] = {{0},};

static myent_t      otm_ent;
static char         otmstr[TMSTRLEN] = {0};
static char         g_otmstr[TMSTRLEN] = {0};

static unsigned char xfer = 0;

void
load_genchr(void)
{
	FILE  *fp;

	memset(&genchr, 0, sizeof(genchr_t));

	if ((fp = fopen(GENDB, "r")) == NULL) return;
	if (fread(&genchr, sizeof(genchr_t), 1, fp) != 1) {
		DBG("--- fread error: %s ---\n", strerror(errno));
	} else {
		xfer = genchr.xfer;
		snprintf(itmstr[0], TMSTRLEN, "%d", genchr.itm[0]);
		memcpy(g_itmstr[0], itmstr[0], TMSTRLEN);
		snprintf(itmstr[1], TMSTRLEN, "%d", genchr.itm[1]);
		memcpy(g_itmstr[1], itmstr[1], TMSTRLEN);
		snprintf(otmstr, TMSTRLEN, "%d", genchr.otm);
		memcpy(g_otmstr, otmstr, TMSTRLEN);
		geninit = 1;
	}
	fclose(fp);
}

void
reload_gendb(void)
{
	int  i;

	geninit = 0;
	load_genchr();

	myent_update(&itm_ent[0]);
	myent_update(&otm_ent);
	for (i = 0; i < 7; ++i) {
		utk_widget_update_state(cbx[i], (xfer>>i)&1);
	}
}

static void
store_genchr_file(void)
{
	FILE  *fp;

	if ((fp = fopen(GENDB, "w")) == NULL) {
		fi_clear(GenDbId);
		return;
	}
	if (fwrite(&genchr, sizeof(genchr_t), 1, fp) != 1) {
		DBG("--- fwrite error: %s ---\n", strerror(errno));
	}
	fflush(fp);
	fclose(fp);
	fi_update(GenDbId);
}

static void 
go_back(void *data)
{
	show_usrm_win();
}

static void
gencfg_cancel(void *data)
{
	int  i;
	kbar_entry_cancel(&gke);
	if (genchr.xfer != xfer) {
		xfer = genchr.xfer;
		for (i = 0; i < 7; ++i) {
			utk_widget_update_state(cbx[i], (xfer>>i)&1);
		}
	}
}

static void
gencfg_save(void *data)
{
	if (kbar_entry_save_prepared(&gke) == 0) return;
	if (!is_master()) {
		gencfg_cancel(NULL);
		return;
	}
	if (geninit == 0) goto save;
	if (memcmp(itmstr[0], g_itmstr[0], TMSTRLEN)) goto save;
	if (memcmp(itmstr[1], g_itmstr[1], TMSTRLEN)) goto save;
	if (memcmp(otmstr, g_otmstr, TMSTRLEN)) goto save;
	if (xfer == genchr.xfer) return;
save:
	disable_syncfg();
	memcpy(g_itmstr[0], itmstr[0], TMSTRLEN);
	memcpy(g_itmstr[1], itmstr[1], TMSTRLEN);
	memcpy(g_otmstr, otmstr, TMSTRLEN);
	genchr.itm[0] = atoi(itmstr[0]);
	genchr.itm[1] = atoi(itmstr[1]);
	genchr.otm = atoi(otmstr);
	genchr.xfer = xfer;
	store_genchr_file();
	geninit = 1;
	sysp_ref();
}

static void
gen_select(void *data)
{
	int  i = (int)data;
	int  st;
	
	st = utk_widget_state(cbx[i]);
	if (st) {
		xfer |= (1<<i);
	} else {
		xfer &= ~(1<<i);
	}

	if (i == 3) {
		if (st) {
			utk_widget_update_state(cbx[4], 0);
			utk_widget_update_state(cbx[5], 0);
			utk_widget_disable(cbx[4]);
			utk_widget_disable(cbx[5]);
			xfer &= ~(1 << 4);
			xfer &= ~(1 << 5);
		} else {
			utk_widget_enable(cbx[4]);
			utk_widget_enable(cbx[5]);
		}
	}
}

static void
go_map(void *data)
{
	if (show_map_win(0, -1) < 0) {
		show_tip(&nomap);
	}
}

void
build_gencfg_gui(void)
{
	int  x[] = {gsel_x0, gsel_x1, gsel_x2, gsel_x3, gsel_x4, gsel_x5, gsel_x6};
	int  y[] = {gsel_y0, gsel_y0, gsel_y0, gsel_y1, gsel_y1, gsel_y1, gsel_y1};
	int  i;
	rect_t r = {.x = tomap_x, .y = tomap_y, .w = tomap_w, .h = tomap_h};
	rect_t r0 = {.x = gen_bar_x0, .y = gen_bar_y, .w = gen_bar_w0, .h = gen_bar_h};
	rect_t re = {.x = gcfg_ent_x0, .y = gcfg_ent_y, .w = gcfg_ent_w, .h = gcfg_ent_h};

	genw = utk_window_derive();
#ifndef MAPDLL
	utk_widget_resident(genw);
#endif
	utk_window_set_bg(genw, BG_GEN);
	utk_widget_hide_before(genw, gencfg_cancel, NULL);

	register_qtimer((void*)genw);

	stb_new(genw, sysp_exit);

	build_tip_lbl(&nomap, genw, (LCD_W-tip_w)/2, (LCD_H-tip_h)/2, LBL_NOMAP, 2);

	build_entry_kbar(&gke, genw, gcfg_kbar_y, 0);

	myent_new(&itm_ent[0], genw, &re, 0);
	myent_add_caret(&itm_ent[0], RGB_BLACK);
	myent_set_cache(&itm_ent[0], itmstr[0], TMSTRLEN, 3);
	myent_set_buffer(&itm_ent[0], g_itmstr[0], TMSTRLEN);
	myent_set_text_max(&itm_ent[0], 2);
	myent_add_kbar(&itm_ent[0], &gke);

	re.x = 	gcfg_ent_x1;
	myent_new(&itm_ent[1], genw, &re, 0);
	myent_add_caret(&itm_ent[1], RGB_BLACK);
	myent_set_cache(&itm_ent[1], itmstr[1], TMSTRLEN, 3);
	myent_set_buffer(&itm_ent[1], g_itmstr[1], TMSTRLEN);
	myent_set_text_max(&itm_ent[1], 2);
	myent_add_kbar(&itm_ent[1], &gke);

	re.x = 	gcfg_ent_x2;
	myent_new(&otm_ent, genw, &re, 0);
	myent_add_caret(&otm_ent, RGB_BLACK);
	myent_set_cache(&otm_ent, otmstr, TMSTRLEN, 3);
	myent_set_buffer(&otm_ent, g_otmstr, TMSTRLEN);
	myent_set_text_max(&otm_ent, 2);
	myent_add_kbar(&otm_ent, &gke);

	build_button_array1(genw, cbx, ARR_SZ(x), psel_w, psel_h, x, y, BN_SEL, gen_select);

	for (i = 0; i < 7; ++i) {
		utk_widget_set_state(cbx[i], (xfer>>i)&1);
	}

	button_new(genw, BN_TOMAP, &r, go_map, (void*)0);
	
	////////////////////////////////////////////////////////////
	button_new(genw, BN_RET160G, &r0, go_back, (void*)0);

	r0.x = gen_bar_x1;
	r0.w = gen_bar_w1;
	button_new(genw, BN_GENSV, &r0, gencfg_save, (void*)0);

	r0.x = gen_bar_x2;
	r0.w = gen_bar_w2;
	button_new(genw, BN_GENCL, &r0, gencfg_cancel, (void*)0);
}

void
show_gencfg_win(void)
{
	if (utk_window_top() != genw) {
		utk_window_show(genw);
	}
}


