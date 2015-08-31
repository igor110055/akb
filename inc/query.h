#ifndef _QUERY_H_
#define _QUERY_H_

#include <time.h>
#include "gui.h"
#include "qtimer.h"

#define AINFT     0
#define EINFT     1

typedef struct _qry_ui {
	int         type;
	UtkWindow  *parent;
	UtkWindow  *qryw;
	tip_t       qryt;
	tip_t       terr;

	kbar_ent_t  qke;
	myent_t     te[2][6];
	myent_t     zinp;
	drop_menu_t dm;
	UtkWindow  *evid;

	void       *data;
	int         icnt;
	int        *qres;
	int         qcnt;
	void    (*clr_result)(struct _qry_ui*);
	void    (*set_result)(struct _qry_ui*, int);
} qry_ui_t;

#define MAXPRT     32
#define MAXDB      10240

extern int    qresult[MAXDB];
extern int    prtarr[MAXPRT];

/* defined in query.c */
extern void  start_query_thread(void);
extern void  qry_ui_new(qry_ui_t *qu, void *parent, char *bg);

extern void  load_info_file(qry_ui_t *q);
extern void  store_info_file(qry_ui_t *q);
extern void  show_qry_ui(qry_ui_t *q);


#endif


