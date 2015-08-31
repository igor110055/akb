#include <stdlib.h>
#include "ulib.h"
#include "utksignal.h"
#include "utkwindow.h"
#include "utkbutton.h"
#include "layout.h"
#include "imxkb_ui.h"

void
imxkb_para_init(imxkb_para_t *par, void *f1, void *f2, int x, int y, int t)
{
	par->clicked = f1;
	par->clr = f2;
	par->type = t;
	par->x = x;
	par->y = y;
}

imx_kb_t*
imxkb_new(void *parent, imxkb_para_t *para)
{
	imx_kb_t*  kb;
	int  i;
	int  x0, x1, x2;
	int  y0, y1, y2, y3;

	kb = malloc(sizeof(imx_kb_t));
	if (!kb) return NULL;
	
	x0 = para->x;
	x1 = x0 + w_1;
	x2 = x1 + w_2 - 2;

	y0 = para->y;
	y1 = y0 + h_1;
	y2 = y1 + h_2;
	y3 = y2 + h_2 + 1;

	for (i=0; i<11; i++)
	{
		kb->bn[i] = utk_button_new_fp();
		utk_signal_connect(kb->bn[i], "clicked", (UtkSignalFunc)para->clicked, (void*)i);
	}

	kb->bn_clr = utk_button_new_fp();
	utk_signal_connect(kb->bn_clr, "clicked", (UtkSignalFunc)para->clr, (void*)NULL);
	utk_widget_set_size(kb->bn[0], w_2, h_1);
	utk_widget_set_size(kb->bn[1], w_1, h_1);
	utk_widget_set_size(kb->bn[2], w_2, h_1);
	utk_widget_set_size(kb->bn[3], w_3, h_1);
	utk_widget_set_size(kb->bn[4], w_1, h_1);
	utk_widget_set_size(kb->bn[5], w_2, h_1);
	utk_widget_set_size(kb->bn[6], w_3, h_1);
	utk_widget_set_size(kb->bn[7], w_1, h_2);
	utk_widget_set_size(kb->bn[8], w_2, h_2);
	utk_widget_set_size(kb->bn[9], w_3, h_2);
	utk_widget_set_size(kb->bn[10], w_1, h_1);
	utk_widget_set_size(kb->bn_clr, w_3, h_1);

	utk_widget_add_cntl((UtkWindow*)parent, kb->bn[1], x0, y0);
	utk_widget_add_cntl((UtkWindow*)parent, kb->bn[2], x1, y0);
	utk_widget_add_cntl((UtkWindow*)parent, kb->bn[3], x2, y0);
	utk_widget_add_cntl((UtkWindow*)parent, kb->bn[4], x0, y1);
	utk_widget_add_cntl((UtkWindow*)parent, kb->bn[5], x1, y1);
	utk_widget_add_cntl((UtkWindow*)parent, kb->bn[6], x2, y1);
	utk_widget_add_cntl((UtkWindow*)parent, kb->bn[7], x0, y2);
	utk_widget_add_cntl((UtkWindow*)parent, kb->bn[8], x1, y2);
	utk_widget_add_cntl((UtkWindow*)parent, kb->bn[9], x2, y2);
	utk_widget_add_cntl((UtkWindow*)parent, kb->bn[10], x0, y3);
	utk_widget_add_cntl((UtkWindow*)parent, kb->bn[0], x1, y3);
	utk_widget_add_cntl((UtkWindow*)parent, kb->bn_clr, x2, y3);

	utk_button_set_bg(kb->bn[0], IMXKB_NUM0);
	utk_button_set_bg(kb->bn[1], IMXKB_NUM1);
	utk_button_set_bg(kb->bn[2], IMXKB_NUM2);
	utk_button_set_bg(kb->bn[3], IMXKB_NUM3);
	utk_button_set_bg(kb->bn[4], IMXKB_NUM4);
	utk_button_set_bg(kb->bn[5], IMXKB_NUM5);
	utk_button_set_bg(kb->bn[6], IMXKB_NUM6);
	utk_button_set_bg(kb->bn[7], IMXKB_NUM7);
	utk_button_set_bg(kb->bn[8], IMXKB_NUM8);
	utk_button_set_bg(kb->bn[9], IMXKB_NUM9);
	if (para->type == 0)
		utk_button_set_bg(kb->bn[10], IMXKB_STAR);
	else
		utk_button_set_bg(kb->bn[10], IMXKB_DOT);
	utk_button_set_bg(kb->bn_clr, IMXKB_CLR);

	return kb;
}

/*
void
disable_imxkb(imx_kb_t *kb, int j)
{
	int  i;
	utk_widget_disable(kb->bn_clr);
	for (i=0; i<11; i++)
	{
		utk_widget_disable(kb->bn[i]);
	}
	if (j >= 0)
		utk_widget_enable(kb->bn[j]);
}

void
enable_imxkb(imx_kb_t *kb)
{
	int  i;
	utk_widget_enable(kb->bn_clr);
	for (i=0; i<11; i++)
	{
		utk_widget_enable(kb->bn[i]);
	}
}

void
imxkb_free(imx_kb_t *kb)
{
	free(kb);
}
*/

