#ifndef _KB_UI_H
#define _KB_UI_H

#include "utkwindow.h"

typedef struct _kb_para {
	void   *clicked;
	int     x;
	int     y;
	int     t;
} kb_para_t;

typedef struct _my_kb {
	UtkWindow  *bn[12];
} my_kb_t;

#ifdef __cplusplus
extern "C" {
#endif

void    kb_para_init(kb_para_t *par, void *f, int x, int y, int t);
my_kb_t  *kb_new(void *parent, kb_para_t *para);
void    disable_kb(my_kb_t *kb, int j);
void    enable_kb(my_kb_t *kb);
void    kb_free(my_kb_t *kb);

#ifdef __cplusplus
}
#endif

#endif


