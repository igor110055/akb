#ifndef _IMXKB_UI_H
#define _IMXKB_UI_H

#include "utkwindow.h"

typedef struct _kb_para {
	void *clicked;
	void *clr;
	int     x;
	int     y;
	int     type;
} imxkb_para_t;

typedef struct _my_kb {
	UtkWindow  *bn[11];
	UtkWindow  *bn_clr;
} imx_kb_t;

#ifdef __cplusplus
extern "C" {
#endif

void    imxkb_para_init(imxkb_para_t *par, void *f1, void *f2, int x, int y, int t);
imx_kb_t   *imxkb_new(void *parent, imxkb_para_t *para);
void    disable_imxkb(imx_kb_t *kb, int j);
void    enable_imxkb(imx_kb_t *kb);
void    imxkb_free(imx_kb_t *kb);

#ifdef __cplusplus
}
#endif

#endif


