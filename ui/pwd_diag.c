#include "ulib.h"
#include "mxc_sdk.h"
#include "utksignal.h"
#include "utkwindow.h"
#include "utkbutton.h"
#include "utkentry.h"
#include "utkdialog.h"
#include "layout.h"
#include "imxkb_ui.h"
#include "gui.h"

static char       pbuf[16] = {0};
static char       _val[11] = {'0','1','2','3','4','5','6','7','8','9','*'};
static pwd_diag_t *curr = NULL;

static void
cancel_check(void *data)
{
	pwd_diag_t *dialog = (pwd_diag_t *)data;
	utk_window_show(dialog->parent);
}

static void
pwd_check(void *data)
{
	pwd_diag_t *dialog = (pwd_diag_t *)data;

	if (dialog->cb >= MAXCB) return;
	if (dialog->ok[dialog->cb])
		dialog->ok[dialog->cb](pbuf);
}

static void
input_pwd(void *data)
{
	int  i;
	i = (int)data;
	if (curr)
		utk_entry_add_char((UtkEntry*)curr->ent, _val[i]);
}

static void
uninput_pwd(void *data)
{
	if (curr)
		utk_entry_del_char((UtkEntry*)curr->ent);
}

static void
prepare_input(void *data)
{
	if (curr) {
		utk_entry_prepare_input((UtkEntry*)curr->ent);
	}
}

static void
hide_event(void *data)
{
	if (curr) {
		utk_entry_clear_text((UtkEntry*)curr->ent);
	}
}

static void
quit_dialog(void *arg)
{
	pwd_diag_t *d = arg;
	utk_window_show(d->parent);
}

void
build_dialog(void* parent, pwd_diag_t *diag, int tm)
{
	imxkb_para_t  par;
	rect_t  rc = {.x = imxpwd_ent_x, .y = imxpwd_ent_y, .w = imxpwd_ent_w, .h = imxpwd_ent_h};
	char *bgok[] = {YES_BN0, YES_BN1};
	char *bgcl[] = {NO_BN0, NO_BN1};
	int   bx[] = {imx_ok_x, imx_cl_x};

	memset(diag, 0, sizeof(pwd_diag_t));

	diag->parent = parent;
	diag->diag = dialog_new(parent, CDIALOG_BG, bx, imx_bn_y, bgok, bgcl);
	dialog_ok_event(diag->diag, pwd_check, diag);
	dialog_cancel_event(diag->diag, cancel_check, diag);

	utk_widget_show_after((UtkWindow*)diag->diag, prepare_input, (void*)0);
	utk_widget_hide_before((UtkWindow*)diag->diag, hide_event, (void*)0);

//	register_qtimer((void*)diag->diag);

	build_tip_lbl(&diag->pwderr, diag->diag, (LCD_W-tip_w)/2, (LCD_H-tip_h)/2, LBL_PWDERR, 2);

	CLEARA(pbuf);
	diag->ent = build_passwd_entry(diag->diag, 0, &rc, pbuf, 16, RGB_BLACK);
	imxkb_para_init(&par, input_pwd, uninput_pwd, pwd_kb_x, pwd_kb_y, 0);
	imxkb_new(diag->diag, &par);
}

/* called by other thread */
void
show_dialog(pwd_diag_t *diag, int cb)
{
	curr = diag;
	diag->cb = cb;
	utk_dialog_show_up((UtkWindow*)curr->diag);
}

void
pwd_diag_error(pwd_diag_t *diag)
{
	utk_entry_clear_text(diag->ent);
	utk_entry_update((UtkWindow *)diag->ent);
	show_tip(&diag->pwderr);
}

void
pwd_dialog_ok_event(pwd_diag_t *d, void *func, int cb)
{
	if (cb < 0||cb >= MAXCB) return;
	d->ok[cb] = func;
}


