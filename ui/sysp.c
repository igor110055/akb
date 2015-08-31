#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <string.h>
#include <errno.h>
#include "ulib.h"
#include "utk.h"
#include "utksignal.h"
#include "utkwindow.h"
#include "utkbutton.h"
#include "layout.h"
#include "stb.h"
#include "gui.h"
#include "config.h"
#include "recver.h"
#include "zopt.h"
#include "qtimer.h"
#include "rs485.h"
#include "xfer.h"


static UtkWindow *syspw;
static UtkWindow *ifsel[12];

static unsigned char   ift = 0;
static unsigned char  _ift = 0;

static int  sysp  = 0;
int  mdev  = -1;
int  miface = -1;

extern void  if_save_sysp(void);

void
sysp_ref(void)
{
	++sysp;
}

static void
sysp_finish(void)
{
	if (sysp > 0) {
		send_event(EV_PRG, 0);
		sysp = 0;
	}
}

void
load_iftype(void)
{
	int   fd;
	unsigned char d[2] = {0};
	
	if ((fd = open(IFTCONF, O_RDONLY)) < 0) return;
	if (read(fd, d, 2) == 2) {
		if (d[0] == 0xff) {
			ift = d[1];
			_ift = ift;
			DBG("--- load: ift = %d ---\n", ift);
			if (ift > 0) {
				mdev = (ift - 1) / 3;
				miface = (ift - 1) % 3;
			}
		}
	}
	close(fd);
}

static void
store_iftype(void)
{
	int fd;
	unsigned char d[2];

	if ((fd = open(IFTCONF, O_WRONLY|O_CREAT|O_TRUNC, S_IRUSR|S_IWUSR)) < 0) {
		DBG("--- open %s error ---\n", IFTCONF);
		fi_clear(IfConfId);
		return;
	}

	d[0] = 0xff;
	d[1] = ift;
	DBG("--- store: ift = %d ---\n", ift);
	if (write(fd, d, 2) != 2) {
		DBG("--- write error: errno = %d ---\n", errno);
	}
	close(fd);

	mdev = (ift - 1) / 3;
	miface = (ift - 1) % 3;

	fi_update(IfConfId);
}

static void
select_iface(void *data)
{
	int  i = (int)data;

	if (i % 3 == 0) {
		utk_widget_update_state(ifsel[i], 0);
		return;
	}

	if (utk_widget_state(ifsel[i]) == 1) {
		if (_ift > 0&&_ift <= 12) {
			utk_widget_update_state(ifsel[_ift - 1], 0);
		}
		_ift = (unsigned char)i+1;
	} else {
		_ift = 0;
	}
}

static void
sysp_cancel(void)
{
	if (_ift != ift) {
		if (_ift > 0) {
			utk_widget_update_state(ifsel[_ift-1], 0);
		}

		if (ift > 0) {
			utk_widget_update_state(ifsel[ift-1], 1);
		}
		_ift = ift;
	}
}

static void
sysp_save(void)
{
	if (!is_master()) {
		sysp_cancel();
		return;
	}

	if (_ift != ift) {
		disable_syncfg();
		ift = _ift;
		if (ift > 0) {
			store_iftype();
		} else {
			fi_clear(IfConfId);
			mdev = -1;
			miface = -1;
		}
		++sysp;
	}
}

static PVOID  sysp_cb[] = {sysp_save, sysp_cancel, sysp_exit, show_zmgr_win, show_usrm_win};

static void
do_sysp_opt(void *data)
{
	int i = (int)data;
	sysp_cb[i]();
}

static void
sysp_quit(void *data)
{
	sysp_cancel();
}

void
build_sysp_gui(void)
{
	int   i;
	int   x[] = {481, 606, 711};
	int   y[] = {149, 222, 296, 369};
	int   bw[] = {sysp_bar_w0, sysp_bar_w1, sysp_bar_w2, sysp_bar_w3, sysp_bar_w4};
	char *bg[] = {BN_PWDSV, BN_PWDCL, BN_RET160G, BN_ZPRG, BN_CHRS};
	rect_t   r = {.x = 0, .y = 0, .w = psel_w, .h = psel_h};

	syspw = utk_window_derive();
#ifndef MAPDLL
	utk_widget_resident(syspw);
#endif
	utk_window_set_bg(syspw, BG_SYSP);
	utk_widget_hide_before(syspw, sysp_quit, NULL);

	register_qtimer((void*)syspw);

	stb_new(syspw, sysp_exit);

	for (i = 0; i < 12; ++i) {
		r.x = x[i%3];
		r.y = y[i/3];
		ifsel[i] = toggle_button_new(syspw, BN_SEL, &r, (void *)select_iface, (void*)i);
		if (i % 3 == 0) {
			utk_widget_disable(ifsel[i]);
		} else {
			if (i+1 == ift) {
				utk_widget_set_state(ifsel[i], 1);
			}
		}
	}

	build_button_array(syspw, ARR_SZ(bw), sysp_bar_h, sysp_bar_y, bw, sysp_bar_x0, do_sysp_opt, bg);
}

/* DONE */
void
show_sysp_win(void)
{
	if (utk_window_top() != syspw) {
		utk_window_show(syspw);
	}
}

/*
 * wrong sequence will lead to alarm window auto exit
 * because of qtimer.
 */
void
sysp_exit(void)
{
	++sysp;
	show_main_win();
	sysp_finish();
	xfer_cfgcmd();
	recver_prepare();
	recver_start();
}

/* DONE: called by slave in syncfg.c */
void
reload_ifconf(void)
{
	int  i;
	unsigned int  v = 0;

	load_iftype();
	if (ift > 0) {
		v |= (1 << (ift-1));
	}

	for (i = 0; i < 12; ++i) {
		utk_widget_update_state(ifsel[i], (v >> i)&1);
	}
}



