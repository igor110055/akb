#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "query.h"
#include "config.h"
#include "mxc_sdk.h"
#include "gui.h"
#include "layout.h"

#define BG_HADD     "/opt/ui/bg_hadd.png"

static UtkWindow  *haddw = NULL;
static kbar_ent_t  hke;
static myent_t     hident;
static myent_t     hipent;

static char   hidbuf[4] = {0};
static char   hipbuf[16] = {0};

static inline void
hadd_ui_clear(void)
{
	myent_clear(&hident);
	myent_clear(&hipent);
}

static void
hadd_ok(void *arg)
{
	int  id;
	id = strtol(hidbuf, NULL, 10);
	if (id == 0||id > MAXHOST) {
		return;
	}

	if (!is_valid_unip(hipbuf)) {
		return;
	}

	if (save_hip(id, hipbuf) == 0) {
		hadd_ui_clear();
		show_host_win();
	} else {
	}
}

static void
hadd_cancel(void *arg)
{
	kbar_entry_cancel(&hke);
	hadd_ui_clear();
	show_host_win();
}

/* DONE */
void
build_hadd_ui(void *parent)
{
	int   bx[]  = {eqry_ok_x, eqry_cl_x};
	char *bgcl[] = {BN_RET1500, BN_RET1501};
	char *bgok[] = {BN_GOK0, BN_GOK1};
	rect_t  r0 = {.x = (294+(1024-678)/2)+25, .y = (93+(600-370)/2), .w = 48, .h = 50};
	rect_t  r1 = {.x = (294+(1024-678)/2), .y = (190+(600-370)/2), .w = 180, .h = 50};

	haddw = dialog_new(parent, BG_HADD, bx, eqry_bn_y, bgok, bgcl);
//	utk_widget_async_evt_handler(haddw, handle_hadd);

#if 0
	hqt.sec = DQTMEO;
	init_qtimer(&hqt, (void *)haddw, (void*)hadd_cancel);
#endif

	dialog_ok_event(haddw, hadd_ok, 0);
	dialog_cancel_event(haddw, hadd_cancel, 0);

	build_entry_kbar(&hke, haddw, (600-370)/2+370, 0);
	myent_new(&hident, haddw, &r0, 1);
	myent_add_caret(&hident, RGB_BLACK);
	myent_set_cache(&hident, hidbuf, sizeof(hidbuf), 2);
	myent_set_text_max(&hident, 2);
	myent_add_kbar(&hident, &hke);

	myent_new(&hipent, haddw, &r1, 1);
	myent_add_caret(&hipent, RGB_BLACK);
	myent_set_cache(&hipent, hipbuf, sizeof(hipbuf), 15);
	myent_set_text_max(&hipent, 15);
	myent_add_kbar(&hipent, &hke);
/*
	build_tip_lbl(&qu->terr, qu->qryw, (LCD_W-tip_w)/2, (LCD_H-tip_h)/2, LBL_TINVLD, TIP_SPLASH_TIME);
	tip_set_hide_cb(&qu->terr, (void*)qry_ui_clear, (void *)qu);

	build_tip_lbl(&qu->qryt, qu->qryw, (LCD_W-tip_w)/2, (LCD_H-tip_h)/2, BG_TIP, 0);
	tip_set_hide_cb(&qu->qryt, (void*)qry_ui_clear, (void *)qu);
*/
}

/* DONE */
void
show_hadd_ui(void)
{
	utk_window_show(haddw);
}



