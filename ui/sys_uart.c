#include <arpa/inet.h>
#include <string.h>
#include "ulib.h"
#include "utk.h"
#include "utksignal.h"
#include "utkwindow.h"
#include "utkbutton.h"
#include "utklabel.h"
#include "utkmap.h"
#include "utkentry.h"
#include "layout.h"
#include "stb.h"
#include "gui.h"
#include "config.h"
#include "qtimer.h"
#include "rs485.h"

static UtkWindow  *uwin;
static kbar_ent_t  uke;
static myent_t     uent[4];
static UtkWindow  *cbx[4];

static char  g_rbuf[4] = {0};
static char  g_sbuf[4] = {0};
static char  g_buf0[10] = {0};
static char  g_buf1[10] = {0};

static char  rbuf[4] = {0};
static char  sbuf[4] = {0};
static char  buf0[10] = {0};
static char  buf1[10] = {0};

static PVOID  su_cb[] = {show_sysnet_win, show_sysgen_win, show_systmr_win, show_sysalr_win};

static ucnf_t   ucnf;
static ucnf_t   gucnf;

static tip_t   uterr;

void
uart_conf(void)
{
	FILE *fp;

	CLEARV(ucnf);
	CLEARV(gucnf);
	memset(&ucnf.tm[0], 0xff, 6);
	memset(&gucnf.tm[0], 0xff, 6);
	if ((fp = fopen(UARTCONF, "r")) == NULL) return;
	if (fread(&gucnf, sizeof(ucnf_t), 1, fp) == 1) {
		if (gucnf.relay > 0) {
			snprintf(g_rbuf, sizeof(g_rbuf), "%d", gucnf.relay);
			memcpy(rbuf, g_rbuf, sizeof(rbuf));
		}

		if (gucnf.siom > 0) {
			snprintf(g_sbuf, sizeof(g_sbuf), "%d", gucnf.siom);
			memcpy(sbuf, g_sbuf, sizeof(sbuf));
		}

		if (gucnf.tm[0] < 24&&gucnf.tm[1] < 60&&gucnf.tm[2] < 60) {
			snprintf(g_buf0, sizeof(g_buf0), "%02d:%02d:%02d", gucnf.tm[0], gucnf.tm[1], gucnf.tm[2]);
			memcpy(buf0, g_buf0, sizeof(buf0));
		}

		if (gucnf.tm[3] < 24&&gucnf.tm[4] < 60&&gucnf.tm[5] < 60) {
			snprintf(g_buf1, sizeof(g_buf1), "%02d:%02d:%02d", gucnf.tm[3], gucnf.tm[4], gucnf.tm[5]);
			memcpy(buf1, g_buf1, sizeof(buf1));
		}

		memcpy(&ucnf, &gucnf, sizeof(ucnf_t));
	}
	fclose(fp);
}

static void
uart_file_save(void)
{
	FILE  *fp = NULL;

	if ((fp = fopen(UARTCONF, "w")) == NULL) return;

	ignore_result(fwrite(&gucnf, sizeof(gucnf), 1, fp));

	fflush(fp);
	fclose(fp);
}

static void
build_uart_entries(UtkWindow *parent)
{
	int  i;
	int  uy[2] = {uent_y0, uent_y1};
	int  ux[4] = {uent_x0, uent_x1, uent_x2, uent_x3};
	int  uw[] = {uent_w0, uent_w1};
	int  sz[] = {4, 4, 10, 10};
	int  vsz[] = {1, 2, 8, 8};
	rect_t  re;
	char   *g_pbuf[4];
	char   *pbuf[4];

	g_pbuf[0] = g_rbuf;
	g_pbuf[1] = g_sbuf;
	g_pbuf[2] = g_buf0;
	g_pbuf[3] = g_buf1;

	pbuf[0] = rbuf;
	pbuf[1] = sbuf;
	pbuf[2] = buf0;
	pbuf[3] = buf1;

	re.h = uent_h;
	for (i = 0; i < ARR_SZ(uent); ++i) {
		re.w = uw[i>>1];
		re.x = ux[i];
		re.y = uy[i>>1];
		myent_new(&uent[i], parent, &re, 0);
		myent_add_caret(&uent[i], RGB_BLACK);
		myent_set_cache(&uent[i], pbuf[i], sz[i], vsz[i]);
		myent_set_buffer(&uent[i], g_pbuf[i], sz[i]);
		myent_set_text_max(&uent[i], vsz[i]);
		myent_add_kbar(&uent[i], &uke);
	}
}

static void
get_tmval(unsigned char *t, char *tmstr)
{
	char *ptr = tmstr;
	char *p = NULL;

	t[0] = (unsigned char)strtol(ptr, &p, 10);
	if ((t[0] == 0&&p == ptr)||t[0] >= 24) {
		t[0] = 0xff;
	} else {
		ptr = ++p;
		t[1] = strtol(ptr, &p, 10);
		if ((t[1] == 0&&p == ptr)||t[1] >= 60) {
			t[0] = 0xff;
			t[1] = 0xff;
		} else {
			ptr = ++p;
			t[2] = strtol(ptr, NULL, 10);
			if (t[2] >= 60) {
				t[2] = 0;
			}
		}
	}
}

static void
clear_time(void *arg)
{
	myent_clear(&uent[2]);
	myent_clear(&uent[3]);
}

static void
uart_save(void *data)
{
	int   r = 0;
	int   s = 0;
	unsigned char  t[6];

	if (kbar_entry_save_prepared(&uke) == 0) return;

	r = strtol(rbuf, NULL, 10);
	if (r > 8) {
		myent_clear(&uent[0]);
		//should prompt the user.
		return;
	}

	s = strtol(sbuf, NULL, 10);
	if (s > 64) {
		//should prompt the user.
		myent_clear(&uent[1]);
		return;
	}

	rs485_exit();
	if (strlen(buf0) > 0&&strlen(buf1) > 0)
	{
		get_tmval(&t[0], buf0);
		get_tmval(&t[3], buf1);

		if (t[0] == 0xff||t[3] == 0xff)
		{
			memset(&ucnf.tm[0], 0xff, 6);
//			CLEARA(buf0);
//			CLEARA(buf1);
			show_tip(&uterr);
		}
		else
		{
			memcpy(&ucnf.tm[0], &t[0], 6);
		}
	}

	ucnf.relay = r;
	ucnf.siom = s;
	
	if (memcmp(&gucnf, &ucnf, sizeof(ucnf))) {
		memcpy(&gucnf, &ucnf, sizeof(ucnf));

		memcpy(g_rbuf, rbuf, sizeof(rbuf));
		memcpy(g_sbuf, sbuf, sizeof(sbuf));
		memcpy(g_buf0, buf0, sizeof(buf0));
		memcpy(g_buf1, buf1, sizeof(buf1));
		uart_file_save();
	}
	rs485_start();
}

static void
uart_cancel(void *data)
{
	int  i;

	kbar_entry_cancel(&uke);
	memcpy(&ucnf, &gucnf, sizeof(gucnf));
	for (i = 0; i < 4; i++) {
		utk_widget_update_state(cbx[i], (ucnf.dev >> i)&1);
	}
}

static void
select_device(void *data)
{
	int i = (int)data;
	int st;

	st = utk_widget_state(cbx[i]);
	if (st == 0) {
		ucnf.dev &= ~(1 << i);
	} else {
		ucnf.dev |= (1 << i);
	}
}

static void
sysf_unav(void *data)
{
	su_cb[(int)data]();
}

void
build_sysuart_gui(void)
{
	int   i;
	int   x[] = {sys_cbox_x0, sys_cbox_x1, sys_cbox_x2, sys_cbox_x3};
	int  bw[] = {sys_w0, sys_w2, sys_w3, sys_w4};
	int  bx[] = {sys_x0, sys_x2, sys_x3, sys_x4};
	rect_t  r = {.x = bn_gret_x, .y = bn_sys_y, .w = gret_w, .h = bn_sys_h};

	uwin = utk_window_derive();
#ifndef MAPDLL
	utk_widget_resident(uwin);
#endif
	utk_window_set_bg(uwin, BG_SYSUART);
	utk_widget_hide_before(uwin, uart_cancel, NULL);

	register_qtimer((void*)uwin);

	stb_new(uwin, sysf_quit);
	build_map_array(uwin, ARR_SZ(bx), bw, bx, sys_y, sys_h, sysf_unav);

	build_tip_lbl(&uterr, uwin, (LCD_W-tip_w)/2, (LCD_H-tip_h)/2, LBL_TINVLD, TIP_SPLASH_TIME);
	tip_set_hide_cb(&uterr, (void*)clear_time, (void *)0);

	build_entry_kbar(&uke, uwin, sysu_kbar_y, 1);
	build_uart_entries(uwin);
	DBG("--- uart entry_kbar = %p ---\n", &uke);

	///////////////////////////////////////////////////////////
	for (i = 0; i < 4; i++) {
		cbx[i] = build_checkbox(uwin, x[i], sys_cbox_y, select_device, (void *)i);
		utk_widget_set_state(cbx[i], (ucnf.dev >> i)&1);
	}
	///////////////////////////////////////////////////////////
	button_new(uwin, BN_RET1501, &r, go_home, (void*)1);
	build_savecancel_buttons((void *)uwin, uart_save, uart_cancel);
}

/* DONE */
void
show_sysuart_win()
{
	if (utk_window_top() != uwin) {
		utk_window_show(uwin);
	}
}

ucnf_t*
get_uart_conf(void)
{
	if (gucnf.relay + gucnf.siom + (gucnf.dev&1) == 0) return NULL;
	return &gucnf;
}


