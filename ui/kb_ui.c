#include <stdlib.h>
#include "ulib.h"
#include "utksignal.h"
#include "utkwindow.h"
#include "utkbutton.h"
#include "layout.h"
#include "kb_ui.h"

static int   key_w[3] = {110, 111, 109};
static int   key_h[2][4] = {{55, 55, 55, 55}, {55, 56, 55, 56}};

static int   key_x[3] = {0, 110, 221};
static int   key_y[2][4] = {{0, 55, 110, 165}, {0, 55, 111, 166}};

static char *bg_kb[2][12] = {
	{KB_NUM0, KB_NUM1, KB_NUM2, KB_NUM3, KB_NUM4, KB_NUM5, KB_NUM6, KB_NUM7, KB_NUM8, KB_NUM9, KB_NUM10, KB_NUM11},
	{KP_NUM0, KP_NUM1, KP_NUM2, KP_NUM3, KP_NUM4, KP_NUM5, KP_NUM6, KP_NUM7, KP_NUM8, KP_NUM9, KP_NUM10, KP_NUM11}
};

void
kb_para_init(kb_para_t *par, void *f, int x, int y, int t)
{
	par->clicked = f;
	par->t = t;
	par->x = x;
	par->y = y;
}

my_kb_t*
kb_new(void *parent, kb_para_t *para)
{
	my_kb_t*  kb;
	int  i;
	int  kx[12] = {1, 0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 2}; //posision-x in keyboard grid
	int  ky[12] = {3, 0, 0, 0, 1, 1, 1, 2, 2, 2, 3, 3}; //posision-y in keyboard grid
	int  x, y;

	kb = malloc(sizeof(my_kb_t));
	if (!kb) return NULL;

	for (i=0; i<12; i++)
	{
		kb->bn[i] = utk_button_new_fp();
		utk_signal_connect(kb->bn[i], "clicked", (UtkSignalFunc)para->clicked, (void*)i);
		x = key_w[kx[i]];
		y = key_h[para->t][ky[i]];
		utk_widget_set_size(kb->bn[i], x, y);
		
		x = para->x + key_x[kx[i]];
		y = para->y + key_y[para->t][ky[i]];
		utk_widget_add_cntl((UtkWindow*)parent, kb->bn[i], x, y);
		utk_button_set_bg(kb->bn[i], bg_kb[para->t][i]);
	}
	return kb;
}

void
disable_kb(my_kb_t *kb, int j)
{
	int  i;

	for (i=0; i<12; i++)
	{
		utk_widget_disable(kb->bn[i]);
	}
	if (j >= 0)
		utk_widget_enable(kb->bn[j]);
}

void
enable_kb(my_kb_t *kb)
{
	int  i;
	for (i=0; i<12; i++)
	{
		utk_widget_enable(kb->bn[i]);
	}
}

void
kb_free(my_kb_t *kb)
{
	free(kb);
}

