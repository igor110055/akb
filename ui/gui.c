#include "utksignal.h"
#include "utkwindow.h"
#include "utktimeout.h"
#include "utkcontainer.h"
#include "utkentry.h"
#include "utkmap.h"
#include "utklabel.h"
#include "utkled.h"
#include "utkbutton.h"
#include "utkmstlbl.h"
#include "utkvisitem.h"
#include "utklistbox.h"
#include "utkdialog.h"
#include "layout.h"
#include "gui.h"
#include "pinyin.h"


#define  fkb_y0     43
#define  fkb_y1     101
#define  fkb_y2     159
#define  fkb_y3     217
#define  fkb_y4     275

#define  fkb_x0     12

#define   MAXCARR   512
#if 0
static hzcandidate_t  *carr = NULL;
#else
static hzcandidate_t   carr[MAXCARR];
#endif

static char  inbuf[16] = {0};
static char  ibuf[16] = {0};

static int   imt  = 0;
static int   caps = 0;
static int   sft  = 0;

static char  digx[] = {')', '!', '@', '#', '$', '%', '^', '&', '*', '('};

//////////////////////////////////////////////////////////////////////////
typedef UtkWindow* (*BN_NEWF)(void);

static BN_NEWF   new_bn0[] = {utk_button_new, utk_toggle_button_new};
static BN_NEWF   new_bn1[] = {utk_button_new_fp, utk_toggle_button_new_fp};

void*
led_new(void *parent, char *str, int n, rect_t *r)
{
	UtkWindow *led;
	UtkWindow *lbl;
	int        i;

	led = utk_led_new();
	utk_widget_set_size(led, r->w*n, r->h);
	utk_led_set_fg((UtkLed*)led, LED_0, 0);
	utk_led_set_fg((UtkLed*)led, LED_1, 1);
	utk_led_set_fg((UtkLed*)led, LED_2, 2);
	utk_led_set_fg((UtkLed*)led, LED_3, 3);
	utk_led_set_fg((UtkLed*)led, LED_4, 4);
	utk_led_set_fg((UtkLed*)led, LED_5, 5);
	utk_led_set_fg((UtkLed*)led, LED_6, 6);
	utk_led_set_fg((UtkLed*)led, LED_7, 7);
	utk_led_set_fg((UtkLed*)led, LED_8, 8);
	utk_led_set_fg((UtkLed*)led, LED_9, 9);
	utk_led_set_text((UtkLed*)led, str);
	utk_led_set_text_max((UtkLed*)led, n);
	utk_widget_add_cntl(parent, led, r->x, r->y);

	for (i = n-1; i >= 0; --i)
	{
		lbl = utk_label_new_fp();
		utk_widget_set_size(lbl, r->w, r->h);
		utk_led_add_icon((UtkLed*)led, lbl, r->x+i*r->w, r->y);
	}

	return led;
}

void
led_update(void *arg)
{
	utk_led_update((UtkWindow*)arg);
}

static void *
new_button(void *parent, char *bg[], int x, int y, void *f, void *arg, int t)
{
	UtkWindow *bn;

	bn = (*new_bn0[t])();
	utk_button_set_bg(bn, bg[0]);
	utk_button_set_bg(bn, bg[1]);
	utk_signal_connect(bn, "clicked", f, (void*)arg);
	utk_widget_add_cntl((UtkWindow*)parent, bn, x, y);

	return bn;
}

void*
button_new0(void *parent, char *bg[], int x, int y, void *f, void *arg)
{
	return new_button(parent, bg, x, y, f, arg, 0);
}

void*
toggle_button_new0(void *parent, char *bg[], int x, int y, void *f, void *arg)
{
	return new_button(parent, bg, x, y, f, arg, 1);
}

static void *
new_button_fp(void *parent, char *bg, rect_t *r, void *f, void *arg, int t)
{
	UtkWindow *bn;

	bn = (*new_bn1[t])();
	utk_widget_set_size(bn, r->w, r->h);
	utk_signal_connect(bn, "clicked", f, (void*)arg);
	utk_widget_add_cntl((UtkWindow*)parent, bn, r->x, r->y);
	utk_button_set_bg(bn, bg);

	return bn;
}

void*
button_new(void *parent, char *bg, rect_t *r, void *f, void *arg)
{
	return new_button_fp(parent, bg, r, f, arg, 0);
}

void*
toggle_button_new(void *parent, char *bg, rect_t *r, void *f, void *arg)
{
	return new_button_fp(parent, bg, r, f, arg, 1);
}

void*
label_new(void *parent, char *bg, char *text, COLOR clr, rect_t *r)
{
	UtkWindow *lbl;

	lbl = utk_label_new_fp();
	utk_widget_set_size(lbl, r->w, r->h);
	utk_label_set_text_size((UtkLabel*)lbl, ((r->w)>>3));
	utk_label_set_text_color((UtkLabel*)lbl, clr);

	if (text) {
		utk_label_set_text((UtkLabel*)lbl, text);
	}

	utk_widget_add_cntl(parent, lbl, r->x, r->y);

	if (bg) {
		utk_label_set_bg(lbl, bg);
	}

	return lbl;
}

void*
label_txt_new(void *parent, COLOR clr, rect_t *r, int t, int size)
{
	UtkWindow *lbl;

	lbl = utk_label_new_fp();
	utk_widget_set_size(lbl, r->w, r->h);
	utk_label_set_fnt((UtkLabel *)lbl, t);
	utk_label_set_text_size((UtkLabel*)lbl, size);
	utk_label_set_text_color((UtkLabel*)lbl, clr);
	utk_widget_add_cntl(parent, lbl, r->x, r->y);

	return lbl;
}

void*
map_new(void *parent, rect_t *r, void *p, void *f, void *arg)
{
	UtkWindow  *bn;

	bn = (UtkWindow*)utk_map_new();
	utk_widget_set_size(bn, r->w, r->h);
	utk_widget_add_cntl((UtkWindow*)parent, bn, r->x, r->y);
	if (p) {
		utk_signal_connect(bn, "pressed", p, arg);
	}
	if (f) {
		utk_signal_connect(bn, "clicked", f, arg);
	}

	return bn;
}

/* called by system function ui sys_*.c */
void
build_savecancel_buttons(void *parent, void *fsv, void *fcl)
{
	rect_t   r = {.x = bn_sv_x, .y = bn_sys_y, .w = bn_sys_w, .h = bn_sys_h};

	button_new(parent, BN_SV150, &r, (void *)fsv, (void *)0);

	r.x = bn_cancel_x;
	button_new(parent, BN_CL150, &r, (void *)fcl, (void *)0);
}

/* diferent background in one line */
void
build_button_array(void *parent, int n, int bh, int by, int w[], int x, void *f, char *bg[])
{
	int  i = 0;
	int  rx = 0;
	rect_t  r;

	r.h = bh;
	r.y = by;
	do {
		r.w = w[i];
		r.x = x + rx;
		button_new(parent, bg[i], &r, (void *)f, (void *)i);
		rx += w[i];
		++i;
	} while (i < n);
}

/* same background on diferent positions */
void
build_button_array1(void *parent, UtkWindow *wid[], int n, int w, int h, int x[], int y[], char *bg, void *f)
{
	int  i;
	UtkWindow  *bn;
	rect_t  r;

	r.w = w;
	r.h = h;
	for (i = 0; i < n; ++i) {
		r.x = x[i];
		r.y = y[i];
		bn = toggle_button_new(parent, bg, &r, f, (void *)i);
//		if (wid) {
			wid[i] = bn;
//		}
	}
}

void
build_button_array2(void *parent, int n, int bh, int by, int w[], int x[], void *f, char *bg[])
{
	int     i;
	rect_t  r;

	r.h = bh;
	r.y = by;
	for (i = 0; i < n; ++i) {
		r.w = w[i];
		r.x = x[i];
		button_new(parent, bg[i], &r, (void *)f, (void *)i);
	}
}

/* same size in one line */
void
build_map_array(void *parent, int n, int bw[], int bx[], int y, int h, void *f)
{
	int  i;
	rect_t  r;

	r.h = h;
	r.y = y;
	for (i = 0; i < n; i++) {
		r.x = bx[i];
		r.w = bw[i];
		map_new(parent, &r, NULL, f, (void*)i);
	}
}

/* same size in one line */
void
build_map_array1(UtkWindow **wg, void *parent, int n, int bw[], int bx[], int y, int h, void *f)
{
	int  i;
	rect_t  r;

	r.h = h;
	r.y = y;
	for (i = 0; i < n; i++) {
		r.x = bx[i];
		r.w = bw[i];
		wg[i] = map_new(parent, &r, NULL, f, (void*)i);
	}
}

void
build_map_col(void *parent, rect_t *r, int n, void *f)
{
	int  i;
	rect_t  r0;

	r0 = *r;
	for (i = 0; i < n; i++) {
		r0.y = r->y + i*r->h;
		map_new(parent, &r0, NULL, f, (void*)i);
	}
}


////////////////////////////////////////////////////
//                   keybar                       //
////////////////////////////////////////////////////
#if 0
{
#endif

static char *kbg0[] = {KEY00, KEY10, KEY20, KEY30, KEY40, KEY50, KEY60, KEY70, KEY80, KEY90, KEYP0, KEYOK0, KEYCL0};
static char *kbg1[] = {KEY01, KEY11, KEY21, KEY31, KEY41, KEY51, KEY61, KEY71, KEY81, KEY91, KEYP1, KEYOK1, KEYCL1};
static char  kv[2][13] = {
	                      {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '.', 'o', 'c'},
                          {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', ':', 'o', 'c'}
                         };

static keybar_t  *curr_kbar = NULL;

void
show_keybar(keybar_t *kbar)
{
	keybar_ops_t *op = kbar->kops;
	DBG("--- show keybar, parent = %p ---\n", kbar->parent);
	if (utk_widget_is_visible(kbar->parent)) {
		DBG("--- window is visible ---\n");
		if (kbar->flag == 0) {
			if (curr_kbar&&curr_kbar != kbar) {
				hide_keybar(curr_kbar);
			}
			curr_kbar = kbar;
			if (op&&op->show)
				(*op->show)(1);
			DBG("--- show the keybar ---\n");
			utk_widget_show(kbar->cn);
			kbar->flag = 1;
		}
	}
}

void
hide_keybar(keybar_t *kbar)
{
	keybar_ops_t *op = kbar->kops;
	if (kbar->flag == 1&&kbar == curr_kbar) {
		if (utk_widget_is_visible(kbar->parent))
			utk_widget_hide(kbar->cn);
		if (op&&op->show)
			(*op->show)(0);
		curr_kbar = NULL;
		kbar->flag = 0;
	}
}

static void
keypress(void *data)
{
	int i = (int)data;
	keybar_ops_t *op;
	char  v;

	if (curr_kbar == NULL) return;
	op = curr_kbar->kops;
	switch (i) {
	case 12: //cancel/backspace
		if ((*op->bp)() == 0) {
			hide_keybar(curr_kbar);
			if (op->act)
				(*op->act)(0);
		}
		break;
	case 11: //ok
		hide_keybar(curr_kbar);
		if (op->act)
			(*op->act)(1);
		break;
	default:
		v = kv[curr_kbar->type][i];
		(*op->fw)(v);
		break;
	}
}

void
build_keybar(keybar_t *kbar, void *parent, int x, int y, keybar_ops_t *op, int t)
{
	UtkWindow *cn;
	UtkWindow *bn;
	int  i;
	int  dx[] = {key_x0, key_x1, key_x2, key_x3, key_x4, key_x5, key_x6, key_x7, key_x8, key_x9, keyp_x, keyok_x, keycl_x};

	memset(kbar, 0, sizeof(keybar_t));
	cn = utk_container_new();
	utk_container_set_bg(cn, BG_KEYBAR);
	utk_widget_add_cntl((UtkWindow*)parent, cn, x, y);
	for (i = 0; i < ARR_SZ(dx); ++i) {
		bn = utk_button_new();
		utk_button_set_bg(bn, kbg0[i]);
		utk_button_set_bg(bn, kbg1[i]);
		utk_signal_connect(bn, "clicked", keypress, (void*)i);
		utk_container_add_widget(cn, bn, dx[i], 0);
	}
	utk_widget_disable(cn);

	kbar->cn = cn;
	kbar->parent = parent;
	kbar->kops = op;
	kbar->type = ((t > 0) ? 1 : 0);
}

#if 0
}
#endif

////////////////////////////////////////////////////
void*
build_passwd_entry(void *parent, int t, rect_t *r, char *buf, int n, unsigned short clr)
{
	UtkWindow *e;
	int  fh[] = {16, 32};
	int  fw[] = {8, 16};

	t = (t > 2)?0:t;
	if (r->h < fh[t]) return NULL;
	e = (UtkWindow*)utk_entry_new_fp();
	utk_entry_set_fnt((UtkEntry*)e, t);
	utk_widget_set_size((UtkWindow*)e, r->w, r->h);
	utk_entry_set_text_color((UtkEntry*)e, clr);
	utk_entry_set_password_mode((UtkEntry*)e);
	utk_entry_set_password_char((UtkEntry*)e, '*');
	utk_entry_set_text((UtkEntry*)e, buf);
	utk_entry_set_text_size((UtkEntry*)e, n-1);
	utk_entry_set_vis_size((UtkEntry*)e, (r->w - 30)/fw[t]);
	utk_widget_add_cntl((UtkWindow*)parent, e, r->x, r->y);
	utk_entry_set_text_pos((UtkEntry*)e, 15, (r->h - fh[t])>>1);

	return e;
}

///////////////////////////////////////////////////////////////////////////
#if 0
{
#endif
void
hide_tip(void *arg)
{
	tip_t *t = arg;

	utk_widget_hide(t->w);
	utk_widget_enable(t->parent);
	if (t->hide)
		t->hide(t->arg);
}

void
build_tip_lbl(tip_t *t, void *parent, int x, int y, char *bg, int tm)
{
	memset(t, 0, sizeof(tip_t));
	
	t->parent = parent;
	t->w = utk_osd_new();
	utk_osd_set_bg(t->w, bg);
	utk_widget_add_cntl(parent, t->w, x, y);
	utk_osd_prepare(t->w);
	utk_widget_disable(t->w);
	t->sec = tm;
	t->tid = utk_widget_timer(parent, hide_tip, t);
}

void
tip_lbl_set_text(tip_t *t, char *text, int size, COLOR color, int x, int y)
{
	utk_osd_set_text((UtkOsd *)t->w, text);
	utk_osd_set_text_color((UtkOsd *)t->w, color);
	utk_osd_set_text_size((UtkOsd *)t->w, size);
	utk_osd_set_text_pos((UtkOsd *)t->w, x, y);
}

void
tip_update(tip_t *t)
{
	utk_osd_update(t->w);
	if (t->sec > 0) {
		utk_timeout_add_once(t->sec*100, t->tid);
	}
}

void
show_tip(tip_t *t)
{
	if (utk_widget_is_disable(t->w)) {
		utk_widget_disable(t->parent);
		utk_widget_show(t->w);
	} else {
		utk_timeout_remove(t->tid);
		utk_osd_update(t->w);
	}
	if (t->sec > 0) {
		utk_timeout_add_once(t->sec*100, t->tid);
	}
}

void
show_strtip(tip_t *tip, char *err, int len, int tmeo, int w, int y)
{
	tip_lbl_set_text(tip, err, len, RGB_BLACK, (w - len*8)/2, y);
	tip_set_timeout(tip, tmeo);
	show_tip(tip);
}

#if 0
}
#endif

/////////////////////////////////////////////////////////////
//                       kbar_ent                          //
/////////////////////////////////////////////////////////////
#if 0
{
#endif

static kbar_ent_t *curr_ke = NULL;

static int    cache_coherent(myent_t *me);

static void
kbar_fw(int v)
{
	myent_t  *me;

	if (curr_ke&&curr_ke->curent) {
		me = curr_ke->curent;
		if (utk_entry_add_char(me->ent, v) >= 0) {
			if (me->chgd) {
				me->chgd(me->arg);
			}
		}
	}
}

static int
kbar_bp(void)
{
	myent_t  *me;
	int      r = 0;

	if (curr_ke&&curr_ke->curent) {
		me = curr_ke->curent;
		r = utk_entry_del_char(me->ent);
		if (me->chgd) {
			me->chgd(me->arg);
		}
	}
	return r;
}

static void
kbar_act(int ok)
{
	myent_t  *me;

	if (!curr_ke || !curr_ke->curent) return;
	me = curr_ke->curent;
	if (ok)  {//pressed ok button
		curr_ke->ent_ok = me->ent;
	}
	utk_entry_focus_leave((UtkWindow *)me->ent);
}

static void
kbar_show(int i)
{
	if (!curr_ke) return;
	if (curr_ke->show) {
		curr_ke->show(i);
	}
}

static keybar_ops_t  kop = {
	.fw = kbar_fw,
	.bp = kbar_bp,
	.show = kbar_show,
	.act = kbar_act,
};

void
build_entry_kbar(kbar_ent_t *ke, void *parent, int ky, int t)
{
	memset(ke, 0, sizeof(kbar_ent_t));
	build_keybar(&ke->kbar, parent, (LCD_W-keybar_w)>>1, ky, &kop, t);
	ke->kbar.owner = ke;
	U_INIT_LIST_HEAD(&ke->head);
}

void
kbar_set_show(kbar_ent_t *ke, void *f)
{
	ke->show = f;
}

int
kbar_entry_save_prepared(kbar_ent_t *ke)
{
	myent_t  *me = ke->curent;

	if (keybar_showed(&ke->kbar)) return 0;
	if (me)
		utk_entry_focus_leave((UtkWindow *)me->ent);
	return 1;
}

void
kbar_entry_cancel(void *arg)
{
	UListHdr  *list;
	myent_t   *me;
	kbar_ent_t *ke;

#if 0
	if (!curr_ke) return;
	ke = curr_ke;
#else
	ke = (kbar_ent_t*)arg;
	if (!keybar_showed(&ke->kbar)) return;
#endif
	hide_keybar(&ke->kbar);
	me = ke->curent;
	if (me)
		utk_entry_focus_leave((UtkWindow *)me->ent);

	list = ke->head.next;
	while (list != &ke->head)
	{
		me = (myent_t *)list;
		if ((me->buf != NULL)&&!cache_coherent(me)) {
			myent_reload(me);
			// modified on 2013.06.18
			if (me->fin&&me->ncf == 0) {
				(*me->fin)(me->arg);
			}
		}
		list = list->next;
	}
}

/////////////////////////////////////////////////////////////////
static void
prepare_input(void *data)
{
	myent_t  *me = (myent_t*)data;
	UtkEntry *e;
	int       len;
	kbar_ent_t *ke;

	e = (UtkEntry*)me->ent;
	ke = me->ke;

	DBG("--- prepare input ---\n");
	if (me->inp) {
		DBG("--- do prepare function ---\n");
		if ((*me->inp)(me->arg) != 0) {
			curr_ke = NULL;
			return;
		}
	}

	len = utk_entry_text_length(e);
	DBG("--- entry: len:%d,%s ---\n", len, utk_entry_text(e));
	if (len > 0) {
		DBG("--- clear the entry buffer ---\n");
		utk_entry_clear_text(e);
		DBG("--- the entry buffer cleared ---\n");
		utk_entry_update((UtkWindow*)e);
	}

	if (curr_ke != ke) {
		curr_ke = ke;
	}

	curr_ke->curent = me;
	show_keybar(&curr_ke->kbar);
	DBG("--- FOCUS IN ---\n");
}

static int
cache_coherent(myent_t *me)
{
	char *p;
	UtkEntry *e;
	
	e = (UtkEntry*)me->ent;
	p = utk_entry_text(e);
	return (!strcmp(p, me->buf));
}

void
myent_reload(myent_t *me)
{
	char *dst;
	UtkEntry *e;
	int   sz;
	
	e = (UtkEntry*)me->ent;
	sz = utk_entry_text_size(e);
	dst = utk_entry_text(e);
	if (me->buf&&(me->size > 0)) {
		memcpy(dst, me->buf, sz);
		utk_entry_set_text(e, dst);
		utk_entry_update((UtkWindow*)e);
	}
}

static void
finish_input(void *data)
{
	myent_t  *me = (myent_t*)data;
	UtkEntry *e;

	if (curr_ke == NULL) return;
	e = (UtkEntry*)me->ent;
	if (me->buf != NULL) {
		if ((utk_entry_text_length(e) == 0)&&(strlen(me->buf) > 0)&&(e != curr_ke->ent_ok)) {
			myent_reload(me);
		}
	}

	if (me->fin) {
		(*me->fin)(me->arg);
	}

	curr_ke->ent_ok = NULL;
	curr_ke->curent = NULL;
	DBG("--- FOCUS OUT: ipent ---\n");
}

////////////////////////////////////////////////////////
//          should be called by following order.      //
////////////////////////////////////////////////////////
/* type: [font type] 0-8*16, 1-16*32 */
int
myent_new(myent_t *me, void *parent, rect_t *r, int t)
{
	UtkEntry  *ent;

	memset(me, 0, sizeof(myent_t));
	ent = utk_entry_new_fp();
	if (!ent) return -1;
	utk_entry_set_fnt(ent, t);
	utk_widget_set_size((UtkWindow*)ent, r->w, r->h);
	utk_signal_connect((UtkWindow*)ent, "focus_in", prepare_input, (void*)me);
	utk_signal_connect((UtkWindow*)ent, "focus_out", finish_input, (void*)me);
	me->ent = ent;
	U_INIT_LIST_HEAD(&me->list);

	utk_widget_add_cntl((UtkWindow*)parent, (UtkWindow*)ent, r->x, r->y);
	utk_entry_set_text_pos(ent, ent->fh/2, (r->h - ent->fh)/2);

	return 0;
}

void
myent_add_caret(myent_t *me, COLOR clr)
{
	utk_entry_add_caret((UtkEntry*)me->ent, clr);
}

void
myent_set_cache(myent_t *me, char *text, int tsz, int vsz)
{
	UtkEntry *e;

	e = (UtkEntry*)me->ent;
	utk_entry_set_text(e, text);
	utk_entry_set_text_size(e, tsz);
	utk_entry_set_vis_size(e, vsz);
}

#if 0
}
#endif

void*
build_checkbox(void *parent, int x, int y, void *f, void *data)
{
	UtkWindow  *bn;

	bn = utk_toggle_button_new_fp();
	utk_widget_set_size(bn, sys_checkbox_w, sys_checkbox_h);
	if (f) {
		utk_signal_connect(bn, "clicked", f, (void*)data);
	}
	utk_widget_add_cntl(parent, bn, x, y);
	utk_button_set_bg(bn, SYS_CBOX);

	return bn;
}

///////////////////////////////////////////////////////////////
//                     drop menubar                          //
///////////////////////////////////////////////////////////////
#if 0
{
#endif
static drop_menu_t  *curdm = NULL;

static void*
build_mst_label(void *parent, char *bg[], int n, rect_t *r)
{
	UtkWindow   *mst_lbl;
	UtkWindow   *lbl;
	UtkMSTLabel *mst;
	int   i;

	mst_lbl = (UtkWindow*)utk_mst_label_new_fp();
	mst = (UtkMSTLabel*)mst_lbl;
	
	for (i = 0; i < n; ++i) {
		lbl = utk_label_new();
		utk_label_set_bg(lbl, bg[i]);
		utk_mst_label_add_icon(mst, lbl, r->x, r->y, UTK_MST_STAT(i));
	}

	utk_widget_set_size(mst_lbl, r->w, r->h);
	utk_widget_add_cntl((UtkWindow*)parent, mst_lbl, r->x, r->y);
	
	return mst_lbl;
}

static void
menu_select(void *data)
{
	int  i = (int)data;
	UtkWindow *w;

	if (curdm) {
		w = curdm->mst;
		utk_widget_update_state(w, UTK_MST_STAT(i));
		if (curdm->select)
			(*curdm->select)(i);
		utk_widget_hide(curdm->cn);
		curdm->flag = 0;
		if (curdm->grab) {
			utk_widget_ungrab_focus(curdm->cn);
		}
		curdm = NULL;
	}
}

void
show_menu(void *data)
{
	drop_menu_t *dm = data;

	if (utk_widget_is_visible(dm->parent)) {
		if (dm->flag == 0) {
			if (dm->grab) {
				utk_widget_grab_focus(dm->cn);
			}
			utk_widget_show(dm->cn);
			curdm = dm;
			dm->flag = 1;
		} else {
			utk_widget_hide(dm->cn);
			dm->flag = 0;
			if (dm->grab) {
				utk_widget_ungrab_focus(dm->cn);
			}
			curdm = NULL;
		}
	}
}

void
hide_menu(drop_menu_t *dm)
{
	if (dm->flag == 1) {
		utk_widget_hide(dm->cn);
		dm->flag = 0;
		if (dm->grab) {
			utk_widget_ungrab_focus(dm->cn);
		}
		curdm = NULL;
	}
}

void
drop_menu_update_state(drop_menu_t *dm, int st)
{
	UtkWindow *w = dm->mst;
	utk_widget_update_state(w, UTK_MST_STAT(st));
}

void
build_drop_menu(drop_menu_t *dm, void *parent, char *cbg, rect_t *r, char *bg0[], char *bg1[], int n, void *f)
{
	UtkWindow *cn;
	UtkWindow *bn;
	int  i;
	int  dy;

	dy = r->y + r->h;
	memset(dm, 0, sizeof(drop_menu_t));
	cn = utk_container_new();
	utk_container_set_bg(cn, cbg);
	utk_widget_add_cntl((UtkWindow*)parent, cn, r->x, dy);

	for (i = 0; i < n; ++i) {
		bn = utk_button_new();
		utk_button_set_bg(bn, bg0[i]);
		utk_button_set_bg(bn, bg1[i]);
		utk_signal_connect(bn, "clicked", menu_select, (void*)i);
		utk_container_add_widget(cn, bn, 0, i*r->h);
	}
	utk_widget_disable(cn);

	dm->cn = cn;
	dm->parent = parent;
	dm->mst = build_mst_label(parent, bg0, n, r);
	dm->select = f;

	map_new(parent, r, NULL, (void*)show_menu, (void*)dm);
}

/////////////////////////////////////////////////////////////
static void
pmenu_select(void *data)
{
	int  i = (int)data;

	if (curdm) {
		(*curdm->select)(i);
		utk_widget_hide(curdm->cn);
		if (curdm->grab) {
			utk_widget_ungrab_focus(curdm->cn);
		}
		curdm->flag = 0;
		curdm = NULL;
	}
}

void
build_pop_menu(drop_menu_t *dm, void *parent, char *cbg, rect_t *r, int n, void *f)
{
	UtkWindow *cn;
	UtkWindow *bn;
	int  i;

	memset(dm, 0, sizeof(drop_menu_t));
	cn = (UtkWindow *)utk_container_new();
	utk_container_set_bg(cn, cbg);
	utk_widget_add_cntl((UtkWindow*)parent, cn, r->x, r->y);

	dm->cn = cn;
	dm->parent = parent;
	dm->select = f;

	for (i = 0; i < n; ++i) {
		bn = (UtkWindow *)utk_map_new();
		utk_widget_set_size(bn, r->w, r->h);
		utk_signal_connect(bn, "clicked", pmenu_select, (void*)i);
		utk_container_add_widget(cn, bn, 0, i*r->h);
	}
	utk_widget_disable(cn);
}

#if 0
}
#endif

/////////////////////////////////////////////////////////
#if 0
{
#endif

void*
listbox_new(UtkWindow *parent, rect_t *r, int rows)
{
	UtkListBox  *slb;
	
	slb = utk_list_box_new();
	if (slb) {
		utk_list_box_set_style(slb, LB_RADIO);
		utk_widget_set_size((UtkWindow*)slb, r->w, r->h*rows);
		utk_list_box_set_item_size(slb, r->w, r->h);
		utk_list_box_set_vis_item_size(slb, rows);
		
		utk_list_box_set_select_clr(slb, GLB_SELCLR);
		utk_list_box_set_sel_text_clr(slb, RGB_WHITE);
		utk_widget_add_cntl(parent, (UtkWindow*)slb, r->x, r->y);
	}
	return slb;
}

void
listbox_set_cols(void  *lb, int cols, int x[], int chrcnt[], long off[])
{
	int  i;
	UtkListBox  *slb = lb;

	utk_list_box_set_col_num(slb, cols);
	for (i = 0; i < cols; ++i) {
		utk_list_box_set_col_inf(slb, i, x[i], chrcnt[i]);
		utk_list_box_set_col_content(slb, i, off[i]);
	}
}

void
listbox_set_content(void  *lb, void *obj, int objsz, int maxr, int nready)
{
	UtkListBox  *slb = lb;
	utk_list_box_set_content_array(slb, obj, objsz, maxr, nready);
}

void
listbox_add_rows(void  *lb, void *f, int rows)
{
	UtkListBox  *slb = lb;
	UtkVisItem  *vi;
	int  i;

	for (i = 0; i < rows; i++) {
		vi = utk_list_box_add_vis_item(slb, i);
		if (f) {
			utk_signal_connect(vi, "selected", f, (void*)(i|(1<<8)));
			utk_signal_connect(vi, "unselected", f, (void*)i);
		}
	}
}

void
listbox_set_font(void *lb, int t)
{
	utk_list_box_set_fnt((UtkListBox *)lb, t);
}

//////////////////////////////////////////////////////////////////
void
id_colum_update(id_col_t *idc)
{
	int  n;
	int  i;
	char *text;

	for (i = 0; i < idc->rows; ++i) {
		text = utk_label_text((UtkLabel*)idc->idw[i]);
		if ((n = idc->base + i) < idc->maxid) {
			snprintf(text, idc->len, "%d", n+1);
			if (idc->map) {
				utk_widget_enable(idc->map[i]);
			}
		} else {
			memset(text, 0, idc->len);
			if (idc->map) {
				utk_widget_disable(idc->map[i]);
			}
		}
		utk_label_update(idc->idw[i]);
	}
}

void
id_colum_pagedn(id_col_t *idc)
{
	int  b;

	if ((b = idc->base + idc->rows) >= idc->maxid) return;
	idc->base = b;
	id_colum_update(idc);
}

void
id_colum_pageup(id_col_t *idc)
{
	int  b;
	if (idc->base == 0) return;
	b = idc->base - idc->rows;
	idc->base = (b < 0) ? 0 : b;
	id_colum_update(idc);
}

void
id_colum_init(id_col_t *idc, int rows, int maxid, int len)
{
	memset(idc, 0, sizeof(id_col_t));
	idc->rows = rows;
	idc->maxid = maxid;
	idc->len = len;
	idc->idw = calloc(rows, sizeof(UtkWindow*));
}

void
id_colum_new(id_col_t *idc, void *parent, char *buf[], rect_t *r, void *f)
{
	int  i;
	int  xoff;
	int  yoff;
	rect_t  r0;
	rect_t  r1;

	if (idc->idw == NULL) return;
	xoff = (r->w - 30)/2;
	yoff = (r->h - 16)/2;

	r0.w = 32;
	r0.h = 16;
	r0.x = r->x + xoff;
	for (i = 0; i < idc->rows; ++i) {
		r0.y = r->y + i*r->h + yoff;
		idc->idw[i] = label_new(parent, NULL, buf[i], RGB_BLACK, &r0);
		
		if (idc->base + i + 1 > idc->maxid) {
			memset(buf[i], 0, idc->len);
		} else {
			snprintf(buf[i], idc->len, "%d", idc->base + i + 1);
		}
	}
	
	if (f) {
		idc->map = calloc(idc->rows, sizeof(UtkWindow*));
		if (idc->map == NULL) return;

		r1 = *r;
		for (i = 0; i < idc->rows; ++i) {
			r1.y = r->y + i*r->h;
			idc->map[i] = map_new(parent, &r1, NULL, f, (void *)i);
			if (idc->base + i + 1 > idc->maxid) {
				utk_widget_disable(idc->map[i]);
			}
		}
	}
}

////////////////////////////////////////////////////
void
bn_colum_update(bn_col_t *idc)
{
	int  i;

//	DBG("--- idc->base = %d, idc->maxid = %d ---\n", idc->base, idc->maxid);
	for (i = 0; i < idc->rows; ++i) {
		if (idc->base + i < idc->maxid) {
			if (utk_list_box_selected(idc->lb, i)) {
				utk_widget_update_state(idc->idw[i], 1);
//				DBG("--- update to selected: %d ---\n", i);
			} else {
				utk_widget_update_state(idc->idw[i], 0);
//				DBG("--- update to unselected: %d ---\n", i);
			}
		} else {
			utk_widget_hide(idc->idw[i]);
//			DBG("--- real hide: %d ---\n", i);
		}
	}
}

void
bn_colum_select(bn_col_t *idc, int i)
{
	utk_widget_update_state(idc->idw[i], 1);
}

void
bn_colum_unselect(bn_col_t *idc, int i)
{
	utk_widget_update_state(idc->idw[i], 0);
}

/* called by callback of the button of the bn_col_t */
void
bn_colum_pressed(bn_col_t *idc, int i)
{
	if (utk_widget_state(idc->idw[i])) {
		utk_list_box_select((UtkListBox*)idc->lb, i);
	} else {
		utk_list_box_unselect((UtkListBox*)idc->lb, i);
		idc->idx = -1;
	}
	
}

void
bn_colum_pagedn(bn_col_t *idc)
{
	int  b;

	if ((b = idc->base + idc->rows) >= idc->maxid) return;
	idc->base = b;
	bn_colum_update(idc);
}

void
bn_colum_pageup(bn_col_t *idc)
{
	int  b;
	if (idc->base == 0) return;
	b = idc->base - idc->rows;
	idc->base = (b < 0) ? 0 : b;
	bn_colum_update(idc);
}

void
bn_colum_init(bn_col_t *idc, int rows, int maxid, void *lb)
{
	memset(idc, 0, sizeof(bn_col_t));
	idc->rows  = rows;
	idc->maxid = maxid;
	idc->idw = calloc(rows, sizeof(UtkWindow*));
	idc->lb  = lb;
}

void
bn_colum_new(bn_col_t *idc, void *parent, int x, int y, char *bg[], void *f)
{
	int   i;

	for (i = 0; i < idc->rows; ++i) {
		idc->idw[i] = toggle_button_new0(parent, bg, x, y+i*row_h, f, (void*)i);
		if (idc->base + i >= idc->maxid) {
			utk_widget_hide(idc->idw[i]);
		}
	}
}

/////////////////////////////////////////////////////
void*
dialog_new(void* parent, char *bg, int x[], int y, char *bgok[], char *bgcl[])
{
	UtkDialog  *d;

	d = utk_dialog_derive_from(parent);
	utk_dialog_set_bg(d, bg);
	utk_dialog_set_mask(d, DIALOG_MSK);
	utk_dialog_add_parent((UtkWindow*)d, (UtkWindow*)parent, 1);

	utk_dialog_set_confirm_bg(d, bgok[0]);
	utk_dialog_set_confirm_bg(d, bgok[1]);
	utk_dialog_add_confirm(d, x[0], y);

	utk_dialog_set_cancel_bg(d, bgcl[0]);
	utk_dialog_set_cancel_bg(d, bgcl[1]);
	utk_dialog_add_cancel(d, x[1], y);

	return d;
}

void
dialog_ok_event(void *d, void *f, void *arg)
{
	utk_dialog_confirm_event((UtkDialog*)d, f, arg);
}

void
dialog_cancel_event(void *d, void *f, void *arg)
{
	utk_dialog_cancel_event((UtkDialog*)d, f, arg);
}

void
build_label_col(void *g[], void *parent, int x, int y, int h, int rows, char *bg[], int nst)
{
	int   i;
	int   j;

	for (i = 0; i < rows; ++i) {
		g[i] = utk_label_new();
		for (j = 0; j < nst; ++j) {
			utk_label_set_bg(g[i], bg[j]);
		}
		utk_widget_add_cntl(parent, g[i], x, y+i*h);
	}
}

void*
build_model_dialog(void *parent, char *cbg, char *bgok[], char *bgcl[], void *fok, void *fcl)
{
	UtkWindow *cn;
	UtkWindow *bn;
	int  w = 0;
	int  h = 0;

	cn = utk_container_new();
	utk_container_set_bg(cn, cbg);
	utk_widget_get_size(cn, &w, &h);
	if (w == 0||h == 0) return NULL;
	utk_widget_add_cntl((UtkWindow*)parent, cn, (1024-w)/2, (600-h)/2);

	bn = utk_button_new();
	utk_button_set_bg(bn, bgok[0]);
	utk_button_set_bg(bn, bgok[1]);
	utk_signal_connect(bn, "clicked", fok, (void*)0);
	utk_container_add_widget(cn, bn, w/2-116, dialog_bn_y);

	bn = utk_button_new();
	utk_button_set_bg(bn, bgcl[0]);
	utk_button_set_bg(bn, bgcl[1]);
	utk_signal_connect(bn, "clicked", fcl, (void*)0);
	utk_container_add_widget(cn, bn, w/2, dialog_bn_y);

	utk_widget_disable(cn);

	return cn;
}

void
show_model_dialog(void *d)
{
	UtkWindow *cn = d;
	utk_widget_grab_focus(cn);
	utk_widget_show(cn);
}

void
hide_model_dialog(void *d)
{
	UtkWindow *cn = d;
	utk_widget_hide(cn);
	utk_widget_ungrab_focus(cn);
}

#if 0
}
#endif

///////////////////////////////////////////////////////
#if 0
{
#endif

/* DONE */
static void
kb_backspace(void *arg)
{
	kbar_ent_t *ke = arg;
	keybar_t  *kb;
	myent_t   *me;
	int        r = 0;

	if (curr_ke == ke&&ke->curent) {
		me = ke->curent;
		if (imt == 0) {
			r = utk_entry_del_char(me->ent);
			if (r&&me->chgd) {
				me->chgd(me->arg);
			}
		} else {
			kb = &ke->kbar;
			if (utk_entry_text_length((UtkEntry *)kb->ent) > 0) {
				utk_entry_del_char((UtkEntry *)kb->ent);
				r = utk_entry_text_length((UtkEntry *)kb->ent);
				ibuf[r] = 0;
				if (r > 0) {
					r = py_ime(ibuf, carr, MAXCARR);
				}
				utk_list_box_reload_basic((UtkListBox *)kb->lbx, r);
			} else {
				utk_entry_del_char(me->ent);
			}
		}
	}
}

/* DONE */
static void
backspace_new(kbar_ent_t *ke)
{
	UtkWindow *bn;
	keybar_t  *kb = &ke->kbar;
	char  *bg[] = {"/opt/ui/kbd_bsp0.png", "/opt/ui/kbd_bsp1.png"};

	bn = utk_button_new();
	utk_button_set_bg(bn, bg[0]);
	utk_button_set_bg(bn, bg[1]);
	
	utk_signal_connect(bn, "clicked", kb_backspace, (void*)ke);
	utk_container_add_widget(kb->cn, bn, 918, fkb_y0);
}

static void
kb_tab(void *arg)
{
	kbar_ent_t  *ke = arg;
	keybar_t    *kb;

	kb = &ke->kbar;
	DBG("--- input tab key ---\n");
}

/* DONE */
static void
tab_new(kbar_ent_t *ke)
{
	UtkWindow *bn;
	keybar_t  *kb;
	char  *bg[] = {"/opt/ui/kbd_tab0.png", "/opt/ui/kbd_tab1.png"};

	kb = &ke->kbar;
	bn = utk_button_new();
	utk_button_set_bg(bn, bg[0]);
	utk_button_set_bg(bn, bg[1]);
	
	utk_signal_connect(bn, "clicked", kb_tab, (void*)ke);
	utk_container_add_widget(kb->cn, bn, fkb_x0, fkb_y1);
}

static void
kb_enter(void *arg)
{
	kbar_ent_t *ke = arg;
	myent_t   *me;

	DBG("--- input enter ---\n");
	hide_keybar(&ke->kbar);
	me = ke->curent;
	if (me) {
		utk_entry_focus_leave((UtkWindow *)me->ent);
	}
	imt = 0;
	sft = 0;
	caps = 0;
}

/* DONE */
static void
enter_new(kbar_ent_t *ke)
{
	UtkWindow *bn;
	keybar_t  *kb;
	char  *bg[] = {"/opt/ui/kbd_ent0.png", "/opt/ui/kbd_ent1.png"};

	kb = &ke->kbar;
	bn = utk_button_new();
	utk_button_set_bg(bn, bg[0]);
	utk_button_set_bg(bn, bg[1]);

	utk_signal_connect(bn, "clicked", kb_enter, (void*)ke);
	utk_container_add_widget(kb->cn, bn, 880, fkb_y1);
}

/* DONE */
static void
kb_capslock(void *arg)
{
	UtkWindow *w = arg;
	caps = utk_widget_state(w);
}

/* DONE */
static void
capslock_new(UtkWindow *cn)
{
	UtkWindow *bn;
	char  *bg[] = {"/opt/ui/kbd_caps0.png", "/opt/ui/kbd_caps1.png"};

	bn = utk_toggle_button_new();
	utk_button_set_bg(bn, bg[0]);
	utk_button_set_bg(bn, bg[1]);

	utk_signal_connect(bn, "clicked", kb_capslock, (void*)bn);
	utk_container_add_widget(cn, bn, fkb_x0, fkb_y2);
}

static void
kb_shift(void *arg)
{
	UtkWindow *w = arg;

	sft = utk_widget_state(w);
}

/* DONE */
static void
shift_new(UtkWindow *cn)
{
	UtkWindow *bn;
	char  *bg[] = {"/opt/ui/kbd_sft0.png", "/opt/ui/kbd_sft1.png"};

	bn = utk_toggle_button_new();
	utk_button_set_bg(bn, bg[0]);
	utk_button_set_bg(bn, bg[1]);
	
	utk_signal_connect(bn, "clicked", kb_shift, (void*)bn);
	utk_container_add_widget(cn, bn, fkb_x0, fkb_y3);
}

/* DONE */
static void
kb_inpt(void *arg)
{
	keybar_t *kb = arg;
	int  len;

	imt = utk_widget_state(kb->imt);
	DBG("--- %s ---\n", imt ? "中文输入" : "英文输入");
	len = utk_entry_text_length((UtkEntry *)kb->ent);
	utk_entry_clear_text((UtkEntry *)kb->ent);
	if (imt == 0) {
		if (len > 0) {
			utk_entry_update(kb->ent);
		}
	}
	utk_list_box_clear_all(kb->lbx);
}

/* DONE */
static void
inpt_new(keybar_t *kb)
{
	char  *bg[] = {"/opt/ui/kbd_inpt0.png", "/opt/ui/kbd_inpt1.png"};

	kb->imt = utk_toggle_button_new();
	utk_button_set_bg(kb->imt, bg[0]);
	utk_button_set_bg(kb->imt, bg[1]);

	utk_signal_connect(kb->imt, "clicked", kb_inpt, (void*)kb);
	utk_container_add_widget(kb->cn, kb->imt, 918, fkb_y4);
}

/////////////////////////////////////////////////////
/* digit key */
static void
kbnum_input(void *arg)
{
	int  i = (int)arg;
	int  ch;
	myent_t  *me;

	if (curr_ke&&curr_ke->curent) {
		if (sft == 0) {
			ch = '0' + i;
		} else {
			ch = digx[i];
		}
		DBG("--- INPUT: %c ---\n", ch);
		me = curr_ke->curent;
		if (utk_entry_add_char(me->ent, ch) >= 0) {
			if (me->chgd) {
				me->chgd(me->arg);
			}
		}
	}
}

/* DONE */
static void
make_digit_path(char *p0, char *p1, int i)
{
	memset(p0, 0, 32);
	memset(p1, 0, 32);
	snprintf(p0, 31, "/opt/ui/kbd_%d0.png", i);
	snprintf(p1, 31, "/opt/ui/kbd_%d1.png", i);
//	DBG("--- %d: image0: %s ---\n", i, p0);
//	DBG("--- %d: image1: %s ---\n\n", i, p1);
}

/* DONE */
static void
digit_key_new(UtkWindow *cn)
{
	UtkWindow *bn;
	int     i;
	int     x[] = {659, 77, 142, 206, 271, 336, 400, 465, 530, 594};
	char    bg0[32];
	char    bg1[32];

	for (i = 0; i < ARR_SZ(x); ++i) {
		make_digit_path(bg0, bg1, i);
		bn = utk_button_new();
		utk_button_set_bg(bn, bg0);
		utk_button_set_bg(bn, bg1);
		utk_signal_connect(bn, "clicked", kbnum_input, (void*)i);
		utk_container_add_widget(cn, bn, x[i], fkb_y0);
	}
}

static char  symu[] = {'~', '_', '+', '|', '{', '}', ':', 0x22, '<', '>', '?'};
static char  symn[] = {'`', '-', '=', 0x5c, '[', ']', 0x3b, 0x27, ',', 0x2e, 0x2f};

/* DONE */
static void
make_symbol_path(char *p0, char *p1, int i)
{
	memset(p0, 0, 32);
	memset(p1, 0, 32);
	snprintf(p0, 31, "/opt/ui/kbd_sym%d0.png", i);
	snprintf(p1, 31, "/opt/ui/kbd_sym%d1.png", i);
//	DBG("--- %d: image0: %s ---\n", i, p0);
//	DBG("--- %d: image1: %s ---\n\n", i, p1);
}

static void
kbsym_input(void *arg)
{
	int  i = (int)arg;
	int  ch;
	myent_t  *me;

	DBG("--- INPUT: %c ---\n", symn[i]);
	if (curr_ke&&curr_ke->curent) {
		me = curr_ke->curent;
		if (i == ARR_SZ(symn)) {
			if (utk_entry_add_char(me->ent, ' ') >= 0) {
				if (me->chgd) {
					me->chgd(me->arg);
				}
			}
			return;
		}

		if (sft == 0) {
			ch = symn[i];
		} else {
			ch = symu[i];
		}

		if (utk_entry_add_char(me->ent, ch) >= 0) {
			if (me->chgd) {
				me->chgd(me->arg);
			}
		}
	}
}

static void
symbol_key_new(UtkWindow *cn)
{
	UtkWindow *bn;
	int     i;
	int     x[] = {fkb_x0, 723, 788, 853, 750, 815, 712, 774, 608, 673, 738};
	int     y[] = {fkb_y0, fkb_y0, fkb_y0, fkb_y0, fkb_y1, fkb_y1, fkb_y2, fkb_y2, fkb_y3, fkb_y3, fkb_y3};
	char    bg0[32];
	char    bg1[32];
	int     n;
	
	n = ARR_SZ(symn);
	for (i = 0; i < n; ++i) {
		make_symbol_path(bg0, bg1, i);
		bn = utk_button_new();
		utk_button_set_bg(bn, bg0);
		utk_button_set_bg(bn, bg1);
		utk_signal_connect(bn, "clicked", kbsym_input, (void*)i);
		utk_container_add_widget(cn, bn, x[i], y[i]);
	}
	
	bn = utk_button_new();
	utk_button_set_bg(bn, "/opt/ui/kbd_sp0.png");
	utk_button_set_bg(bn, "/opt/ui/kbd_sp1.png");
	utk_signal_connect(bn, "clicked", kbsym_input, (void*)n);
	utk_container_add_widget(cn, bn, 220, fkb_y4);
}

static void
kbchar_input(void *arg)
{
	int  ch = (int)arg;
	myent_t  *me;
	keybar_t *kb;
	int   i;

	DBG("--- input: %c ---\n", ch);
	if (sft^caps) {
		ch -= 32;
	}
	if (imt == 0) {
		if (curr_ke->curent) {
			me = curr_ke->curent;
			if (utk_entry_add_char(me->ent, ch) >= 0) {
				if (me->chgd) {
					me->chgd(me->arg);
				}
			}
		}
	} else {
		kb = &curr_ke->kbar;
		i = utk_entry_text_length((UtkEntry *)kb->ent);
		utk_entry_add_char((UtkEntry *)kb->ent, ch);
		ch |= 0x20; //convert to lowercase
		ibuf[i++] = (char)ch;
		ibuf[i] = 0;
		i = py_ime(ibuf, carr, MAXCARR);
	#if 0
		if (max != MAXCARR) {
			DBG("--- max items changed ---\n");
			utk_list_box_set_max_item((UtkListBox *)kb->lbx, MAXCARR);
		}
	#endif
		utk_list_box_reload_basic((UtkListBox *)kb->lbx, i);
	}
}

/* DONE */
static void
make_alpha_path(char *p0, char *p1, int c)
{
	memset(p0, 0, 32);
	memset(p1, 0, 32);
	snprintf(p0, 31, "/opt/ui/kbd_%c0.png", c);
	snprintf(p1, 31, "/opt/ui/kbd_%c1.png", c);
}

/* DONE */
static void
alpha_key_new(UtkWindow *cn, int *vchar, int sz, int x[], int y)
{
	UtkWindow  *bn;
	char   bg0[32];
	char   bg1[32];
	int    i;

	for (i = 0; i < sz; ++i) {
		make_alpha_path(bg0, bg1, vchar[i]);
		bn = utk_button_new();
		utk_button_set_bg(bn, bg0);
		utk_button_set_bg(bn, bg1);
		utk_signal_connect(bn, "clicked", kbchar_input, (void*)vchar[i]);
		utk_container_add_widget(cn, bn, x[i], y);
	}
}

/* DONE */
static void
char_key_new(UtkWindow *cn)
{
	int    ch0[] = {'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p'};
	int    ch1[] = {'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l'};
	int    ch2[] = {'z', 'x', 'c', 'v', 'b', 'n', 'm'};
	int    x0[] = {104, 169, 233, 298, 363, 427, 492, 557, 621, 686};
	int    x1[] = {130, 195, 259, 324, 389, 453, 518, 583, 647};
	int    x2[] = {156, 221, 285, 350, 415, 479, 544};

	alpha_key_new(cn, ch0, ARR_SZ(ch0), x0, fkb_y1);
	alpha_key_new(cn, ch1, ARR_SZ(ch1), x1, fkb_y2);
	alpha_key_new(cn, ch2, ARR_SZ(ch2), x2, fkb_y3);
}

static void
do_pageup(void *arg)
{
	keybar_t  *kb = arg;
	DBG("--- pageup candidate ---\n");
	utk_list_box_scroll_pageup((UtkListBox *)kb->lbx);
}

static void
pageup_bn_new(keybar_t *kb)
{
	UtkWindow *bn;
	bn = utk_button_new();
	utk_button_set_bg(bn, "/opt/ui/kbd_arl0.png");
	utk_button_set_bg(bn, "/opt/ui/kbd_arl1.png");
	utk_signal_connect(bn, "clicked", do_pageup, (void*)kb);
	utk_container_add_widget(kb->cn, bn, 341, 0);
}

static void
do_pagedn(void *arg)
{
	keybar_t  *kb = arg;
	DBG("--- pagedn candidate ---\n");
	utk_list_box_scroll_pagedn((UtkListBox *)kb->lbx);
}

static void
pagedn_bn_new(keybar_t *kb)
{
	UtkWindow *bn;

	bn = utk_button_new();
	utk_button_set_bg(bn, "/opt/ui/kbd_arr0.png");
	utk_button_set_bg(bn, "/opt/ui/kbd_arr1.png");
	utk_signal_connect(bn, "clicked", do_pagedn, (void*)kb);
	utk_container_add_widget(kb->cn, bn, 1024-41, 0);
}

////////////////////////////////////////////////////////////////

#if 0
void
show_imekb(keybar_t *kb)
{
	if (kb->flag == 0) {
		utk_widget_grab_focus(kb->cn);
		utk_widget_show(kb->cn);
		kb->flag = 1;
	}
}

void
hide_imekb(keybar_t *kb)
{
	if (kb->flag == 1) {
		utk_widget_hide(kb->cn);
		utk_widget_ungrab_focus(kb->cn);
		kb->flag = 0;
	}
}

#else
void
hide_imekb(kbar_ent_t *ke)
{
	kb_enter((void *)ke);
}

#endif

static UtkWindow*
input_entry_new(UtkWindow *cn)
{
	UtkEntry  *e;

	e = utk_entry_new_fp();
	utk_entry_set_fnt(e, 1);
	utk_widget_set_size((UtkWindow*)e, 180, 24);
	utk_entry_set_text_color(e, RGB_BLACK);
	utk_entry_set_text(e, inbuf);
	utk_entry_set_text_size(e, sizeof(inbuf));
	utk_entry_set_vis_size(e, sizeof(inbuf)-1);
	utk_container_add_widget(cn, e, (340-180)/2, 6);
	utk_entry_set_text_pos(e, 0, 0);

	return (UtkWindow*)e;
}

static void
candidate_select(void *arg)
{
	int  i = (int)arg;
	int  id;
	keybar_t *kb;
	myent_t  *me;

	if (!curr_ke) return;
	if ((i>>8)&1) {
		kb = &curr_ke->kbar;
		id = utk_list_box_get_id((UtkListBox*)kb->lbx, i&0xff);
		DBG("--- %d:%s selected ---\n", i&0xff, carr[id].text);
		me = curr_ke->curent;
		utk_entry_add_string((UtkEntry *)me->ent, carr[id].text);
		utk_list_box_clear_all(kb->lbx);
		utk_entry_clear_text((UtkEntry *)kb->ent);
		utk_entry_update(kb->ent);
	}
}

static UtkWindow*
candidate_listbox_new(UtkWindow *cn)
{
	UtkListBox  *slb;
	UtkVisItem  *vi;
	int  i;
	unsigned short clr = RGB565(36, 36, 36);
	
	slb = utk_list_box_new();
	if (slb) {
		utk_list_box_set_style(slb, LB_RADIO);
		utk_list_box_set_horizon(slb);
		utk_widget_set_size((UtkWindow*)slb, 600, 36);
		utk_list_box_set_fnt(slb, 1);
		utk_list_box_set_item_size(slb, 50, 36);
		utk_list_box_set_vis_item_size(slb, 12);
//		utk_list_box_set_select_clr(slb, GLB_SELCLR);
		utk_list_box_set_sel_text_clr(slb, RGB_RED);
		utk_list_box_set_text_clr(slb, clr);
		utk_container_add_widget(cn, (UtkWindow*)slb, 382, 0);
		
		utk_list_box_set_col_num(slb, 1);
		utk_list_box_set_col_inf(slb, 0, (50-24)/2, 2);
		utk_list_box_set_col_content(slb, 0, 0);
		utk_list_box_set_content_array(slb, carr, sizeof(hzcandidate_t), MAXCARR, 0);
		for (i = 0; i < 12; i++) {
			vi = utk_list_box_add_vis_item(slb, i);
			utk_signal_connect(vi, "selected", candidate_select, (void*)(i|(1<<8)));
		}
	}
	return (UtkWindow*)slb;
}

static void
build_imekb(kbar_ent_t *ke, void *parent, int x, int y, keybar_ops_t *op)
{
	UtkWindow *cn;
	keybar_t  *kb = &ke->kbar;

	memset(kb, 0, sizeof(keybar_t));

	cn = utk_container_new();
	utk_container_set_bg(cn, "/opt/ui/qwerty.png");
	utk_widget_add_cntl((UtkWindow*)parent, cn, x, y);

	kb->kops = op;
	kb->owner = ke;
	kb->cn = cn;
	kb->parent = parent;
	kb->ent = input_entry_new(cn);
	kb->lbx = candidate_listbox_new(cn);

	pageup_bn_new(kb);
	pagedn_bn_new(kb);
	inpt_new(kb);     //switch between cn and en. 

	capslock_new(cn);
	shift_new(cn);

	tab_new(ke);
	enter_new(ke);
	backspace_new(ke);

	digit_key_new(cn);  //0~9
	char_key_new(cn);   //a~z
	symbol_key_new(cn);

	utk_widget_disable(cn);
}

static keybar_ops_t  kbop = {
	.show = kbar_show,
};

void
build_entry_keyboard(kbar_ent_t *ke, void *parent, int x, int y)
{
	DBG("--- begin build_entry_keyboard ---\n");
	memset(ke, 0, sizeof(kbar_ent_t));
	build_imekb(ke, parent, x, y, &kbop);
	U_INIT_LIST_HEAD(&ke->head);
	DBG("--- build_entry_keyboard finished ---\n");
}

#if 0
void
carray_init(void)
{
	if (carr == NULL) {
		carr = calloc(MAXCARR, sizeof(struct _hzcandidate));
	}
}

int
carray_reinit(void)
{
	char  *p = carr;
	MAXCARR += 256;
	carr = realloc(p, MAXCARR);
	return MAXCARR;
}
#endif

#if 0
}
#endif


