#include <unistd.h>
#include <string.h>
#include "ulib.h"
#include "mxc_sdk.h"
#include "mouse.h"
#include "utkwindow.h"
#include "utksignal.h"
#include "utklabel.h"
#include "layout.h"
#include "config.h"
#include "gui.h"

static UtkWindow  *cwin;
static UtkWindow  *clbl[5];
static UtkWindow  *ctlb;

static char *ctip[6] = {
	"请点击左上角的白色准心",
	"请点击右上角的白色准心",
	"请点击右下角的白色准心",
	"请点击左下角的白色准心",
	"请点击中间的白色准心",
	"校准完成"
	};


static void
prepare_calibrate(void *data)
{
	int  i;

	utk_widget_enable(clbl[0]);
	for (i = 1; i < 5; i++)
	{
		utk_widget_disable(clbl[i]);
	}
	utk_label_set_text((UtkLabel *)ctlb, ctip[0]);
	utk_calibrate_prepare();
	DBG("---- prepare show tscal window ----\n");
}

static void
finish_calibrate(void *data)
{
	utk_calibrate_finish();
}

static void
handle_touch(UtkWindow *win, int  x, int y)
{
	if (win != cwin) return;

	utk_widget_hide(clbl[x]);
	utk_label_set_text((UtkLabel *)ctlb, ctip[x+1]);
	utk_label_update(ctlb);
	if (x < 4)
	{
		utk_widget_show(clbl[x+1]);
	}
	else if (x == 4)
	{
		show_sysgen_win();
	}
}

void
build_tscal_ui(void)
{
	int   i;
	int   ref_x[5] = {50, 1024-50, 1024-50, 50, 512};
	int   ref_y[5] = {50, 50, 550, 550, 512};
	rect_t  r = {.x = (1024-200)/2, .y = 150, .w = 200, .h = 16};

	utk_calibrate_xy(ref_x, ref_y);

	cwin = utk_window_derive();

	utk_widget_show_before(cwin, prepare_calibrate, (void*)0);
	utk_widget_hide_after(cwin, finish_calibrate, (void*)0);
	utk_widget_async_evt_handler(cwin, handle_touch);

	ctlb = label_new(cwin, NULL, ctip[0], RGB_WHITE, &r);

	for (i = 0; i < 5; i++)
	{
		clbl[i] = utk_label_new();
		utk_label_set_bg(clbl[i], ICO_CROSS);
		utk_widget_disable(clbl[i]);
		utk_widget_add_cntl(cwin, clbl[i], ref_x[i]-(cross_w/2), ref_y[i]-(cross_h/2));
	}
}

/* called by other thread */
void
show_tscal(void)
{
	if (utk_window_top() != cwin)
	{
		utk_window_show(cwin);
		DBG("---- tscal window showed ----\n");
	}
}



