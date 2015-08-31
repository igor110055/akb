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
#include "config.h"
#include "layout.h"
#include "gui.h"
#include "stb.h"
#include "zopt.h"

typedef struct slv_info {
	char  idstr[6];
	int   id;
	int   idx;
	char *addr;
} slv_info_t;

static UtkWindow   *slvw = NULL;
static UtkWindow   *idlbl[MAXI] = {NULL};
static UtkWindow   *iplbl[MAXI] = {NULL};

static slv_info_t    slv_inf[99];
static unsigned char slv_id[99] = {0};
static unsigned int  slv_map[4] = {0};
static int           slvcnt = 0;
static int           pgoff  = 0;

#define  slvmap_set(i)     slv_map[(i) >> 5] |= (1 << ((i)&0x1f))
#define  slvmap_clr(i)     slv_map[(i) >> 5] &= ~(1 << ((i)&0x1f))

/* DONE */
static void
slave_info_init(void)
{
	int  i;

	for (i = 0; i < ARR_SZ(slv_inf); ++i) {
		memset(&slv_inf[i], 0, sizeof(slv_info_t));
		slv_inf[i].id = i;
		sprintf(slv_inf[i].idstr, "%02d%02d", i+1, g_devid);
	}
}

/* DONE */
static void
clear_slave_label(int i)
{
	utk_label_set_text((UtkLabel*)idlbl[i], NULL);
	utk_label_update(idlbl[i]);
	utk_label_set_text((UtkLabel*)iplbl[i], NULL);
	utk_label_update(iplbl[i]);
}

static void
update_slave_ui(int i, int id)
{
	slv_info_t  *si = &slv_inf[id];

	utk_label_set_text((UtkLabel*)idlbl[i], si->idstr);
	utk_label_update(idlbl[i]);
	utk_label_set_text((UtkLabel*)iplbl[i], si->addr);
	utk_label_update(iplbl[i]);
}

static void
reload_slave_ui(void)
{
	unsigned int  map;
	int   i, j;
	int   id;
	int   max;

	pgoff = 0;
	slvcnt = 0;
	for (i = 0; i < ARR_SZ(slv_map); ++i) {
		map = slv_map[i];
		while (map) {
			j = __builtin_ctz(map);
			id = (i << 5) + j;
			slv_id[slvcnt++] = id;
			map &= ~(1 << j);
		}
	}

	max = slvcnt > MAXI ? MAXI : slvcnt;
	for (i = 0; i < max; ++i) {
		update_slave_ui(i, slv_id[i]);
	}

	for (i = max; i < MAXI; ++i) {
		clear_slave_label(i);
	}
}

/* called by master in net_tcp.c */
void
set_slave_ui(int id, char *ip)
{
	utk_widget_send_async_event(slvw, id, (int)ip);
}

/* called by master in net_tcp.c */
void
clr_slave_ui(int id)
{
	utk_widget_send_async_event(slvw, id, 0);
}

/* DONE */
static void
slave_clr(int id, int st)
{
	int  i;

	if (st != 0) return;
	if (id == 0) { //cleanup all
		CLEARA(slv_map);
		CLEARA(slv_id);
		CLEARA(slv_inf);
		for (i = 0; i < MAXI; ++i) {
			clear_slave_label(i);
		}
		slvcnt = 0;
		pgoff = 0;
	} else {
		--id;
		slvmap_clr(id);
		reload_slave_ui();
	}
}

static void
slave_set(int id, int st)
{
	--id;
	slvmap_set(id);
	slv_inf[id].addr = (char*)st;
	DBG("--- slave-%d: %s ---\n", id, (char*)st);
	reload_slave_ui();
}

static FVINT2  slave_op[] = {slave_clr, slave_set};

static void
update_slave(UtkWindow *win, int  i, int st)
{
	slave_op[!!st](i, st);
}

static void 
go_back(void *data)
{
	show_sysnet_win();
}

static void
prev_page(void *data)
{
	int  i;

	if (pgoff - MAXI < 0) return;
	pgoff -= MAXI;
	for (i = 0; i < MAXI; ++i) {
		update_slave_ui(i, slv_id[pgoff+i]);
	}
}

static void
next_page(void *data)
{
	int  i;
	int  max;

	if (pgoff + MAXI > slvcnt) return;
	pgoff += MAXI;
	max = slvcnt - pgoff;
	max = max > MAXI ? MAXI : max;
	for (i = 0; i < max; ++i) {
		update_slave_ui(i, slv_id[pgoff+i]);
	}

	for (i = max; i < MAXI; ++i) {
		clear_slave_label(i);
	}
}

void
build_slave_gui(void)
{
	int     i;
	int     x0[] = {86+XOFF0, 370+XOFF0, 654+XOFF0};
	int     x1[] = {185+XOFF1, 469+XOFF1, 753+XOFF1};
	int     y[] = {227+YOFF, 281+YOFF, 335+YOFF, 389+YOFF, 443+YOFF};
	rect_t  r0 = {.w = 48, .h = 24};
	rect_t  r1 = {.w = 180, .h = 24};
	rect_t  r = {.x = 422, .y = 600-67, .w = 182, .h = 42};

	slave_info_init();
	slvw = utk_window_derive();
#ifndef MAPDLL
	utk_widget_resident(slvw);
#endif
	utk_window_set_bg(slvw, BG_SLV);
	utk_widget_async_evt_handler(slvw, update_slave);

	stb_new(slvw, show_sysnet_win);
	for (i = 0; i < MAXI; ++i) {
		r0.x = x0[i%3];
		r0.y = y[i/3];
		idlbl[i] = label_txt_new(slvw, RGB_WHITE, &r0, 1, 4);
		r1.x = x1[i%3];
		r1.y = y[i/3];
		iplbl[i] = label_txt_new(slvw, RGB_WHITE, &r1, 1, 15);
	}

	////////////////////////////////////////////////////////////
	button_new(slvw, BN_RET182, &r, (void *)go_back, (void*)0);
	r.x = 422 + 182;
	button_new(slvw, BN_PU182, &r, (void *)prev_page, (void*)0);
	r.x = 422 + 182 + 182;
	button_new(slvw, BN_PD182, &r, (void *)next_page, (void*)0);
}

void
show_slave_win(void)
{
	if (utk_window_top() != slvw) {
		utk_window_show(slvw);
	}
}


