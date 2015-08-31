#ifndef _GUI_H_
#define _GUI_H_

#include "ulib.h"
#include "mxc_sdk.h"
#include "utkwindow.h"
#include "utkentry.h"
#include "utklabel.h"
#include "qtimer.h"

#define  TIP_SPLASH_TIME       2

//typedef void (*PVOID)(void);

#ifndef ARR_SZ
#define ARR_SZ(x)     (sizeof(x)/sizeof(x[0]))
#endif

#ifndef CLEARV
#define CLEARV(x)     memset(&x, 0, sizeof(x))
#endif

#ifndef CLEARA
#define CLEARA(x)     memset(&x[0], 0, sizeof(x))
#endif

#define  GLB_SELCLR     RGB_BLUE
#define  GLB_ROWS       10

#define LBX_TXT_CLR     RGB565(66, 66, 66)

/* listbox text font */
#define ZLBX_FT     1

typedef struct rect {
	int x;
	int y;
	int w;
	int h;
} rect_t;

typedef struct _tip {
	UtkWindow    *parent;
	UtkWindow    *w;
	unsigned int  tid;
	void  (*hide)(void* arg);
	void  *arg;
	int    sec;  //splash time
} tip_t;

#define PASSMAXLEN     15
typedef struct _keybar_ops {
	void (*fw)(int);
	int  (*bp)(void);
	void (*show)(int);
	void (*act)(int);
} keybar_ops_t;

typedef struct _keybar {
	UtkWindow *parent;
	UtkWindow *cn;
	UtkWindow *imt;   //as for qwerty keyboard
	UtkWindow *ent;   //as for qwerty keyboard
	UtkWindow *lbx;   //as for qwerty keyboard
	void    *owner;
	int      flag;
	int      type;
	keybar_ops_t *kops;
} keybar_t;

#define keybar_showed(kbar)     ((kbar)->flag)

typedef struct kbar_ent {
	UListHdr   head;
	void      *curent;
	void      *ent_ok;
	void      (*show)(int);
	keybar_t   kbar;
} kbar_ent_t;

typedef struct myentry {
	UListHdr    list;
	kbar_ent_t *ke;
	void       *ent;
	char       *buf;
	int         size;
	int       (*inp)(void*);
	void      (*fin)(void*);
	void      (*chgd)(void*);
	void       *arg;
	int         ncf;
} myent_t;

typedef struct drop_menu {
	UtkWindow *parent;
	UtkWindow *cn;
	void      *mst;
	int        flag;
	void     (*select)(int);
	int        grab;
} drop_menu_t;

typedef struct _dialog {
	UtkWindow *cn;
	void  (*ok)(void);
	void  (*cl)(void);
} dialog_t;

typedef struct _id_col {
	UtkWindow   **idw;
	UtkWindow   **map;
	int          rows;
	int          base;
	int          maxid;
	int          len;
} id_col_t;

typedef struct _bn_col {
	UtkWindow   **idw;
	void        *lb;
	int          rows;
	int          base;
	int          maxid;
	int          idx;
} bn_col_t;

#define MAXCB    3

typedef struct _pwd_diag {
	UtkWindow   *parent;
	void *diag;
	void *ent;
	tip_t pwderr;
	int   cb;
	void (*ok[MAXCB]) (char *);
} pwd_diag_t;

//#define pwd_dialog_cancel_event(diag, func)  (diag)->cancel = (func)
//#define pwd_dialog_ok_event(diag, func)   (diag)->ok = (func)

static inline void
id_colum_set_maxid(id_col_t *idc, int maxid)
{
	idc->maxid = maxid;
}

#if 1
#define id_colum_reset_base(idc)    (idc)->base = 0
#define id_colum_set_base(idc, b)   (idc)->base = (b)
#else
static inline void
id_colum_reset_base(id_col_t *idc)
{
	idc->base = 0;
}
#endif

static inline void
bn_colum_set_maxid(bn_col_t *idc, int maxid)
{
	idc->maxid = maxid;
}

#if 1
#define bn_colum_reset_base(idc)   (idc)->base = 0
#else
static inline void
bn_colum_reset_base(bn_col_t *idc)
{
	idc->base = 0;
}
#endif

static inline void
bn_colum_set_base(bn_col_t *idc, int i)
{
	idc->base = i;
}

static inline int
bn_colum_state(bn_col_t *idc, int i)
{
	return utk_widget_state(idc->idw[i]);
}

static inline void
tip_set_timeout(tip_t *t, int tmeo)
{
	t->sec = tmeo;
}

static inline void
tip_set_hide_cb(tip_t *t, void *cb, void *arg)
{
	t->hide = cb;
	t->arg = arg;
}

/* Note: me->ncf == 1 means not call me->fin callback, when canceled. */
static inline void
myent_set_ncf(myent_t *me)
{
	me->ncf = 1;
}

static inline void
myent_update(myent_t *me)
{
	utk_entry_update((UtkWindow*)me->ent);
}

static inline void
myent_clear(myent_t *me)
{
	UtkEntry *e = (UtkEntry*)me->ent;

	utk_entry_clear_text(e);
	utk_entry_update((UtkWindow*)e);
}

static inline int
myent_text_length(myent_t *me)
{
	return utk_entry_text_length((UtkEntry*)me->ent);
}

static inline void
myent_set_text_max(myent_t *me, int sz)
{
	utk_entry_set_text_max((UtkEntry*)me->ent, sz);
}

static inline void
myent_set_text(myent_t *me, char *text)
{
	utk_entry_set_text((UtkEntry *)me->ent, text);
}

static inline void
myent_disable(myent_t *me)
{
	UtkWindow *w = me->ent;
	utk_widget_disable(w);
}

static inline void
myent_enable(myent_t *me)
{
	UtkWindow *w = me->ent;
	utk_widget_enable(w);
}

static inline void
myent_set_buffer(myent_t *me, char *buf, int sz)
{
	me->buf = buf;
	me->size = sz;
}

static inline void
myent_set_callback(myent_t *me, void *inp, void *fin, void *chgd, void *arg)
{
	me->inp = inp;
	me->fin = fin;
	me->chgd = chgd;
	me->arg = arg;
}

static inline void
myent_set_text_color(myent_t *me, unsigned short clr)
{
	utk_entry_set_text_color((UtkEntry*)me->ent, clr);
}

/* should be called after keybar created. */
static inline void
myent_add_kbar(myent_t *me, kbar_ent_t *ke)
{
	u_list_append(&me->list, &ke->head);
	me->ke = ke;
}

static inline void
keybar_grab(keybar_t *kbar)
{
	utk_widget_grab_focus(kbar->cn);
}

static inline void
keybar_ungrab(keybar_t *kbar)
{
	utk_widget_ungrab_focus(kbar->cn);
}

static inline int
kbar_ent_showed(kbar_ent_t *ke)
{
	return ke->kbar.flag;
}

static inline int
menu_showed(drop_menu_t *dm)
{
	return dm->flag;
}

static inline void
menu_set_grab(drop_menu_t *dm)
{
	dm->grab = 1;
}

static inline void
menu_set_ungrab(drop_menu_t *dm)
{
	dm->grab = 0;
}


#ifdef __cplusplus
extern "C" {
#endif

extern void  build_dialog(void* parent, pwd_diag_t *d, int tm);
extern void  show_dialog (pwd_diag_t *d, int cb);
extern void  pwd_diag_error(pwd_diag_t *d);
extern void  pwd_dialog_ok_event(pwd_diag_t *d, void *func, int cb);

///////////////// gui.c ///////////////
extern void*    led_new(void *parent, char *str, int n, rect_t *r);
extern void     led_update(void *arg);
extern void*    button_new0(void *parent, char *bg[], int x, int y, void *f, void *arg);
extern void*    toggle_button_new0(void *parent, char *bg[], int x, int y, void *f, void *arg);
extern void*    button_new(void *parent, char *bg, rect_t *r, void *f, void *arg);
extern void*    toggle_button_new(void *parent, char *bg, rect_t *r, void *f, void *arg);
extern void*    map_new(void *parent, rect_t *r, void *p, void *f, void *arg);
extern void*    label_new(void *parent, char *bg, char *text, COLOR clr, rect_t *r);
extern void*    label_txt_new(void *parent, COLOR clr, rect_t *r, int t, int size);

extern void     build_savecancel_buttons(void *parent, void *fsv, void *fcl);
extern void     build_button_array(void *parent, int n, int bh, int by, int w[], int x, void *f, char *bg[]);
extern void     build_button_array1(void *parent, UtkWindow *bn[], int n, int w, int h, int x[], int y[], char *bg, void *f);
extern void     build_button_array2(void *parent, int n, int bh, int by, int w[], int x[], void *f, char *bg[]);
extern void     build_map_array(void *parent, int n, int bw[], int bx[], int y, int h, void *f);
extern void     build_map_array1(UtkWindow **wg, void *parent, int n, int bw[], int bx[], int y, int h, void *f);
extern void     build_map_col(void *parent, rect_t *r, int n, void *f);

extern void     build_keybar(keybar_t *kbar, void *parent, int x, int y, keybar_ops_t *op, int type);
extern void     show_keybar(keybar_t *kbar);
extern void     hide_keybar(keybar_t *kbar);

extern void*    build_passwd_entry(void *parent, int t, rect_t *r, char *buf, int n, unsigned short clr);

extern void     build_tip_lbl(tip_t *t, void *parent, int x, int y, char *bg, int tm);
extern void     show_tip(tip_t *t);
extern void     hide_tip(void *arg);
extern void     tip_update(tip_t *t);
extern void     tip_lbl_set_text(tip_t *t, char *text, int size, COLOR color, int x, int y);
extern void     show_strtip(tip_t *tip, char *err, int len, int tmeo, int w, int y);

extern void     build_entry_kbar(kbar_ent_t *ke, void *parent, int ky, int t);
extern int      kbar_entry_save_prepared(kbar_ent_t *ke);
extern void     kbar_entry_cancel(void *arg);
extern void     kbar_set_show(kbar_ent_t *ke, void *f);

////////////////////////////////////////////////////////
//          should be called by following order.      //
////////////////////////////////////////////////////////
extern void     myent_reload(myent_t *me);
extern int      myent_new(myent_t *me, void *parent, rect_t *r, int t);
extern void     myent_add_caret(myent_t *me, COLOR clr);
extern void     myent_set_cache(myent_t *me, char *text, int tsz, int vsz);

extern void*    build_checkbox(void *parent, int x, int y, void *f, void *data);
extern void     build_drop_menu(drop_menu_t *dm, void *parent, char *cbg, rect_t *r, char *bg0[], char *bg1[], int n, void *f);
extern void     build_pop_menu(drop_menu_t *dm, void *parent, char *cbg, rect_t *r, int n, void *f);

extern void     hide_menu(drop_menu_t *dm);
extern void     show_menu(void *data);
extern void     drop_menu_update_state(drop_menu_t *dm, int st);

extern void     build_del_dialog(dialog_t *d, void *parent, void *ok, void *cl);

extern void*    dialog_new(void* parent, char *bg, int x[], int y, char *bgok[], char *bgcl[]);
extern void     dialog_ok_event(void *d, void *f, void *arg);
extern void     dialog_cancel_event(void *d, void *f, void *arg);

extern void     show_model_dialog(void *d);
extern void     hide_model_dialog(void *d);
extern void*    build_model_dialog(void *parent, char *cbg, char *bgok[], char *bgcl[], void *fok, void *fcl);
extern void     build_label_col(void *g[], void *parent, int x, int y, int h, int rows, char *bg[], int nst);

//////////////// login.c ////////////////
extern UtkWindow*  desktop_window(void);
extern void     build_login_gui(void);
extern void     show_login(void);

//////////////// ts_cal.c ////////////////
extern void     build_tscal_ui();
extern void     show_tscal();

//////////////// start.c ////////////////
extern void     build_main_gui(void);
extern void     show_main_win(void);
extern void     go_home(void* data);

///////////////////////////////////////////
extern void     id_colum_update(id_col_t *idc);
extern void     id_colum_pageup(id_col_t *idc);
extern void     id_colum_pagedn(id_col_t *idc);
extern void     id_colum_init(id_col_t *idc, int rows, int maxid, int len);
extern void     id_colum_new(id_col_t *idc, void *parent, char *buf[], rect_t *r, void *f);

///////////////////////////////////////////
extern void     bn_colum_update(bn_col_t *idc);
extern void     bn_colum_select(bn_col_t *idc, int i);
extern void     bn_colum_unselect(bn_col_t *idc, int i);
extern void     bn_colum_pressed(bn_col_t *idc, int i);
extern void     bn_colum_pagedn(bn_col_t *idc);
extern void     bn_colum_pageup(bn_col_t *idc);
extern void     bn_colum_init(bn_col_t *idc, int rows, int maxid, void *lb);
extern void     bn_colum_new(bn_col_t *idc, void *parent, int x, int y, char *bg[], void *f);

///////////////////////////////////////////
//               系统功能               //
///////////////////////////////////////////
//////////////// sys_net.c ////////////////
extern void     build_sysnet_gui(void);
extern void     show_sysnet_win(void);

//////////////// sys_uart.c ////////////////
extern void     build_sysuart_gui(void);
extern void     show_sysuart_win(void);

//////////////// sys_gen.c ////////////////
extern void     build_sysgen_gui(void);
extern void     show_sysgen_win(void);

//////////////// sys_tmr.c ////////////////
extern void     build_systmr_gui(void);
extern void     show_systmr_win(void);
extern void     register_time_change_notifier(void *node);
extern void     prepare_pztip(int *arr, int cnt);
extern void     pzent_enable(int v);

//////////////// sys_alr.c ////////////////
extern void     build_sysalr_gui(void);
extern void     show_sysalr_win(void);

////////////////////////////////////////////

extern void     build_zone_gui(void);
extern void     show_zone_gui(int t);
extern void     sync_zone_ui(void);

extern void     build_zinfo_gui(void);
extern void     show_zinfo_win(void);

extern void     build_arment_gui(void);
extern void     show_arment_win(void);

extern void     build_tmrck_gui(void);
extern void     show_tmrck_win(void);
extern void     update_time_itvl(void);
extern void     reload_tmrck_ui(void);

extern void     build_tmr_gui(void);
extern void     show_tmr_win(void);

extern void     build_pzone_gui(void);
extern void     show_pzone_gui(int t);
extern void     sync_pzone_ui(void);
extern void     update_pzoneui_state(int i, int st);

extern void     build_einfo_gui(void);
extern void     show_einfo_win(void);

extern void     build_ainfo_gui(void);
extern void     show_ainfo_win(void);

extern void     build_stent_gui(void);
extern void     show_stent_win(void);
extern void     update_syst_label(int st);
extern void     update_pzst_label(int st);
extern void     clear_pzst_label(void);

extern void     build_syst_gui(void);
extern void     show_syst_win(void);

extern void     build_bpent_gui(void);
extern void     show_bpent_win(void);

///////////////////////////////////////////////////
//                  系统编程                   //
///////////////////////////////////////////////////
extern void     build_sysp_gui(void);
extern void     show_sysp_win(void);

extern void     build_usrm_gui(void);
extern void     show_usrm_win(void);

extern void     build_ucfg_gui(void);
extern void     show_ucfg_win(int id);
extern void     show_ucfg_win1(void);

extern void     build_gencfg_gui(void);
extern void     show_gencfg_win(void);

extern void     build_zmgr_gui(void);
extern void     show_zmgr_win(void);

extern void     build_zcfg_gui(void);
extern void     show_zcfg_win(void *z, int id);

extern void     build_zalrm_gui(void);
extern void     show_zalrm_win(void);

extern void     build_pzck_gui(void);
extern void     show_pzck_gui(int p);
extern void     update_pzck_state(int p, int z, int st);
extern void     update_pzck_pstat(int p);

/////////////////////////////////////////////////////
extern void*    listbox_new(UtkWindow *parent, rect_t *r, int rows);
extern void     listbox_set_cols(void  *lb, int cols, int x[], int chrcnt[], long off[]);
extern void     listbox_set_content(void  *lb, void *obj, int objsz, int maxr, int nready);
extern void     listbox_add_rows(void  *lb, void *f, int rows);
extern void     listbox_set_font(void *lb, int t);

extern void     build_entry_keyboard(kbar_ent_t *ke, void *parent, int x, int y);

extern void     build_map_gui(void);
extern int      show_map_win(int i, int z);
extern void     update_zmapst(int zid, int st);
extern void     zmap_del(int z);

extern int      show_ztp(void *p, char *buf, int n);

//extern int      zmap_defined(int z);
extern int      unhandled_zalarm(void);


extern void     hide_imekb(kbar_ent_t *ke);

extern void     build_mstbn_col(void *g[], void *parent, int x, int y, int rows, void *f);

extern void     build_slave_gui(void);
extern void     show_slave_win(void);

extern void     build_host_gui(void);
extern void     show_host_win(void);

extern void     build_hadd_ui(void *parent);
extern void     show_hadd_ui(void);

#if 0
extern void     carray_init(void);
extern int      carray_reinit(void);
#endif

#ifdef __cplusplus
}
#endif

#endif

