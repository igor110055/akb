#include "ulib.h"
#include "utk.h"
#include "utkresource.h"
#include "utksignal.h"
#include "utkcaret.h"
#include "utkwindow.h"
#include "utkcontainer.h"
#include "utkbutton.h"
#include "utklabel.h"
#include "utkclock.h"
#include "utkmstlbl.h"
#include "utkvisitem.h"
#include "utklistbox.h"
#include "utkpoint.h"
#include "utkline.h"
#include "utktimeout.h"
#include "layout.h"
#include "gui.h"
#include "config.h"
#include "zopt.h"
#include "qtimer.h"

#define LB_H       36
#define LB_W       60

#define LB_XOFF    ((LB_W - 4*12)>>1)
#define LB_YOFF    ((LB_H - 24)>>1)

#define DRAW_W     900
#define DRAW_H     500

#define DRAW_X     (1024-DRAW_W)/2
#define DRAW_Y     (530-DRAW_H)/2

#define DA_CLR     RGB_GREEN
#define AM_CLR     RGB_RED
#define ILB_CLR    RGB_GRAY
#define OLB_CLR    RGB565(230,213,136)


#define PNT_SZ     5
#define LIN_SZ     3

#define CARET_CLR  RGB_BLACK
#define MAXZLEN    8

#define MAXLINMAP  32

#define MBOTTOM     520

typedef void (*PVD)(void*);
typedef void (*PVI)(int);
typedef void (*FBK)(UtkWindow *);

typedef struct _zmap {
	unsigned short  zid;     //zoneid:[1, MAXZ]
	unsigned short  mapid;
	unsigned short  t;       //1: inner zone, >=2: outer zone, 0: nodefined.
	unsigned short  pos;     //from which the outer zones point., index of position in ozpos[] array.
	unsigned short  pidx;    //index of position in ptr array of zmap of outer zones.
	UPoint  zpt;   //coordinate of the zone number. or inner zone.
} zmap_t;

/* zone number string stored here */
typedef struct _ztxt {
	char text[MAXZLEN];
} ztxt_t;

typedef struct _zme_ops {
	void  (*dozme) (void);
	void  (*undo) (void);
	void  (*znest) (void); //start editing znr.
	void  (*zneok) (void);
	void  (*znecl) (void);
	void  (*touch) (void *arg);
	void* (*zmnew) (zmap_t *zm);
} zme_ops_t;

typedef struct _znr_node {
	UListHdr   list;
	UtkWindow *w;
	int        mapid;
	int        alert;
	int        zid;
} znr_node_t;

typedef void (*FBN)(znr_node_t *);

static UtkWindow  *mapvw = NULL;
static UtkWindow  *bnret;
static UtkWindow  *bndisp;
static UtkWindow  *bnok;
static UtkWindow  *bnpu;
static UtkWindow  *bnpd;
static UtkWindow  *cn_edt;
static UtkWindow  *bnoz;
static UtkWindow  *bniz;
static UtkWindow  *bnmz;
static UtkWindow  *zdel;
static UtkWindow  *zchg;

static tip_t     nozone;
static tip_t     undefz;
static tip_t     redefz;

static keybar_t  mkbar;

static int       fromw = 0;
static unsigned int   timerid = 0;

/* when zone state changed, operate on these objects. like blink. */
static UListHdr   *znrarr;  //list head znr label or inner zone label per each map. for show or hide the zone number or inner zones.
static UListHdr   *zalist;

/* 用于有报警时的翻页显示 */
/* 标记地图是否有警情 */
static unsigned int     *zmapa = NULL;
static int               zmbmp_cnt = 0;
#define zmapa_set(i)     zmapa[(i)>>5] |= (1<<((i)&0x1f))
#define zmapa_clr(i)     zmapa[(i)>>5] &= ~(1<<((i)&0x1f))

static void     **resrc = NULL;
static int        mapcnt = 0;
static int        mapid = 0;

static zmap_t     zmapdb[MAXZ];
static ztxt_t     ztxt[MAXZ];  /* zone number string stored here */
static znr_node_t znode[MAXZ];
static UtkWindow *zplw[MAXZ] = {NULL,};  //array of ptr of utkline or utkpoint

static int     ezt = -1;

static UPoint  znrpt;
static int     currznr = 0;
static char    znrbuf[MAXZLEN] = {0};
static int     znrlen = 0;
static int     deltax = 0;
static int     inpy = 0;

#define  cleanup_znrbuf()  \
	do {  \
		memset(znrbuf, 0, sizeof(znrbuf));  \
		znrlen = 0; \
	} while (0)


static UImage  bimg;
static unsigned char    baddr[LB_W*LB_W*MXC_BPP];
static UImage  pimg[MAXLINMAP];
static unsigned char    paddr[MAXLINMAP][PNT_SZ*PNT_SZ*MXC_BPP];

///////////////////////////////////////////////////////////////
static UPoint  opt[MAXLINMAP];
static int     ocnt = 0;
static int     ozcnt = 0;  //关系此次周界防区编辑是否连接上一次的问题。

#define  cleanup_optbuf()  \
	do {  \
		memset(opt, 0, sizeof(opt));  \
		ocnt = 0; \
	} while (0)

static UPoint  ozpos[(MAXZ>>1)+1];
static int     ozpcnt = 0;

/* only for outer zones, should be saved into file. */
/* 按zmap->pos的升序排列 ,实际上也是按编辑顺序排列的 */
static unsigned short  ozmapidx[256] = {0}; //保存index of position in zmapdb[] array.
static int             ozmcnt = 0;
///////////////////////////////////////////////////////////////

static int     edst = 0;

static unsigned short gclr[] = {DA_CLR, AM_CLR};
static unsigned short rclr[3] = {ILB_CLR, OLB_CLR, ILB_CLR};

/* new add on 2014.05.04 */
/* called when edit the map. */
static unsigned int    pzmapall = 0;
static unsigned short  pzmapcnt[MAXPZ] = {0};

#define pzmapall_set(p)  pzmapall |= (1 << (p))
#define pzmapall_clr(p)  pzmapall &= ~(1 << (p))

/* z: [0, MAXZ) */
static inline void 
pzmapall_setz(int z)
{
	int  i = pnrofz(z);
	pzmapall |= (1 << i);
}

/* z: [0, MAXZ) */
static inline void 
pzmapall_clrz(int z)
{
	int  i = pnrofz(z);
	if (--pzmapcnt[i] <= 0) {
		pzmapall &= ~(1 << i);
		pzmapcnt[i] = 0;
	}
}

#define pzmapall_isset(p)    (pzmapall & (1 << (p)))


static void  do_izme(void);
static void  undo_izme(void);
static void  start_izne(void);
static void  izne_ok(void);
static void  izne_cl(void);
static void  izme_touch(void *arg);
static void *izm_new(zmap_t *zm);

static void  do_ozme(void);
static void  undo_ozme(void);
static void  start_ozne(void);
static void  ozne_ok(void);
static void  ozne_cl(void);
static void  ozme_touch(void *arg);
static void *ozm_new(zmap_t *zm);

static void  mznr_ok(void);
static void  mznr_cl(void);
static void  mznr_touch(void *arg);

static zme_ops_t  zmeops[] = {
	[0] = {
		.dozme = do_izme,
		.undo  = undo_izme,
		.znest = start_izne,
		.zneok = izne_ok,
		.znecl = izne_cl,
		.touch = izme_touch,
		.zmnew = izm_new,
	},
	[1] = {
		.dozme = do_ozme,
		.undo  = undo_ozme,
		.znest = start_ozne,
		.zneok = ozne_ok,
		.znecl = ozne_cl,
		.touch = ozme_touch,
		.zmnew = ozm_new,
	},
	[2] = {
		.dozme = NULL,
		.undo  = NULL,
		.znest = NULL,
		.zneok = mznr_ok,
		.znecl = mznr_cl,
		.touch = mznr_touch,
		.zmnew = NULL,
	},
};

static int  zblnk = 0;

/* should cleanup the coordinate info. */
static void
cleanup_zmap(zmap_t *zm)
{
	int  pidx = zm->pidx;
	int  d, s;
	int  ndel = 0;
	int  i;
	int  p;
	zmap_t *zmx = NULL;

	p = pnrofz(zm->zid - 1);
	if (zm->t > 1) {
		DBG("--- cleanup the zmap: zone[%d], pidx = %d ---\n", zm->zid, pidx);
		d = zm->pos;
		ndel = zm->t;
		s = d + ndel;
		if (ozpcnt > s) {
			memmove(&ozpos[d], &ozpos[s], (ozpcnt - s)*sizeof(ozpos[0]));
		}
		DBG("--- delete the zmap pos info: from %d to %d ---\n", d, s);
		ozpcnt -= ndel;

		if (pidx < ozmcnt-1) {
			memmove(&ozmapidx[pidx], &ozmapidx[pidx+1], (ozmcnt - pidx - 1)*sizeof(ozmapidx[0]));
		}
		--ozmcnt;
		
		for (i = pidx; i < ozmcnt; ++i) {
			zmx = &zmapdb[ozmapidx[i]];
			zmx->pos -= ndel;
			utk_line_set_endpoint(zplw[zmx->zid-1], &ozpos[zmx->pos], zmx->t);
			zmx->pidx--;
		}
	}

	--pzmapcnt[p];
	if (pzmapcnt[p] == 0) {
		pzmapall_clr(p);
	}

	DBG("--- ozmcnt = %d, ozpcnt = %d ---\n", ozmcnt, ozpcnt);
	memset(zm, 0, sizeof(zmap_t));
}

/* called at last in deletion. */
/* should think about the alarmed zone. */
static void
free_znarr_list(int mid)
{
	UListHdr *head;
	UListHdr *list;
	znr_node_t *zn;
	
	head = &znrarr[mid];
	while (!u_list_empty(&znrarr[mid])) {
		list = head->next;
		u_list_del(list);
		zn = (znr_node_t*)list;
		cleanup_zmap(&zmapdb[zn->zid-1]);
		memset(zn, 0, sizeof(znr_node_t));
		U_INIT_LIST_HEAD(list);
	}
	U_INIT_LIST_HEAD(&znrarr[mid]);
}

static int
first_alert_mapid(void)
{
	int  r;
	int  i;
	int  j;

	for (i = 0; i < zmbmp_cnt; ++i) {
		DBG("--- zmapa[%d] = 0x%08x ---\n", i, zmapa[i]);
		if (zmapa[i] > 0) {
			j = __builtin_ctz(zmapa[i]);
			r = (i << 5) + j;
			DBG("--- first alarm mapid = %d ---\n", r);
			return r;
		}
	}

	return -1;
}

static void
set_current_map(int z)
{
	int  id;
	if (z < 0) {
		id = first_alert_mapid();
		mapid = (id >= 0 ? id : 0);
	} else {
		if (zmapdb[z].t > 0) {
			mapid = zmapdb[z].mapid;
		} else {
			id = first_alert_mapid();
			mapid = id >= 0 ? id : 0;
		}
	}
	utk_window_set_current_page(mapvw, mapid, 0);
}

static void
update_iz(int z, int st)
{
	utk_label_set_text_color((UtkLabel*)znode[z].w, gclr[st]);
	utk_label_update(znode[z].w);
}

static void
update_oz(int z, int st)
{
	utk_widget_set_color(zplw[z], gclr[st]);
	utk_label_set_text_color((UtkLabel*)znode[z].w, gclr[st]);
	utk_label_update(znode[z].w);
	utk_widget_update(zplw[z]);
}

typedef void (*FV2INT)(int, int);

static FV2INT update_z[] = {update_iz, update_oz};

#if 0
static inline int
zone_iotype(int z)
{
	return ((zmapdb[z].t >> 1) > 0);
}
#else
#define zone_iotype(z)   (zmapdb[z].t >> 1)
#endif

static int
pzmap_disarm(int p, int z)
{
	int  t = zone_iotype(z);

	if (zmapdb[z].t > 0) {
		(*update_z[!!t])(z, 0);
	}
	return 0;
}

static void
update_zdisarmall(void)
{
	int  i, j, id;
	unsigned int zar = pzmapall;
	znr_node_t  *zn;
	UListHdr  *list;
	UListHdr  *head;

	while (zar) {
		i = __builtin_ctz(zar);
		pzopt_zones(i, pzmap_disarm);
		zar &= ~(1 << i);
	}

	for (i = 0; i < zmbmp_cnt; ++i) {
		while (zmapa[i]) {
			j = __builtin_ctz(zmapa[i]);
			zmapa[i] &= ~(1 << j);
			id = (i << 5) + j;
			head = &zalist[id];
			while (!u_list_empty(head)) {
				list = head->next;
				zn = (znr_node_t*)list;
				u_list_del(&zn->list);
				u_list_append(&zn->list, &znrarr[zn->mapid]);
				zn->alert = 0;
			}
		}
	}
}

static int
pzmap_arm(int p, int z)
{
	int  i = zoneidx(z);
	int  t = zone_iotype(z);

	if (zmapdb[z].t > 0&&(zstat[i] >= ZST_ARM)) {
		(*update_z[!!t])(z, 1);
	}
	return 0;
}

static void
update_zarmall(void)
{
	int  i;
	unsigned int zar = pzmapall;

	while (zar) {
		i = __builtin_ctz(zar);
		pzopt_zones(i, pzmap_arm);
		zar &= ~(1 << i);
	}
}

static int
pzmap_allset(int p, int z)
{
	int  i = zoneidx(z);
	int  t = zone_iotype(z);

	if (zmapdb[z].t > 0) {
		if (zstat[i] >= ZST_ARM)
			(*update_z[!!t])(z, 1);
		else
			(*update_z[!!t])(z, 0);
	}
	return 0;
}

static void
update_zmapall(void)
{
	int  i;
	unsigned int zar = pzmapall;

	while (zar) {
		i = __builtin_ctz(zar);
		pzopt_zones(i, pzmap_allset);
		zar &= ~(1 << i);
	}
}

static PVOID  zallopr[] = {update_zdisarmall, update_zarmall, update_zmapall};

static int
pzmap_pdisarm(int p, int z)
{
	int  t = zone_iotype(z);
	znr_node_t	*zn;

	if (zmapdb[z].t > 0) {
		zn = &znode[z];
		if (zn->alert) {
			u_list_del(&zn->list);
			u_list_append(&zn->list, &znrarr[zn->mapid]);
			zn->alert = 0;
			if (u_list_empty(&zalist[zn->mapid])) {
				zmapa_clr(zn->mapid);
			}
		}
		(*update_z[!!t])(z, 0);
	}
	return 0;
}

static void
update_pzdisarm(int p)
{
	pzopt_zones(p, pzmap_pdisarm);
}

static void
update_pzarm(int p)
{
	pzopt_zones(p, pzmap_arm);
}

static FVINT  pzallopr[] = {update_pzdisarm, update_pzarm};

////////////////////////////////////////////////////////
static void
zmap_array_init(void)
{
	int  i;

	memset(&zmapdb[0], 0, sizeof(zmap_t)*MAXZ);
	memset(&ztxt[0], 0, sizeof(ztxt_t)*MAXZ);
	memset(&zplw[0], 0, sizeof(UtkWindow*)*MAXZ);
	memset(&znode[0], 0, sizeof(znr_node_t)*MAXZ);
	for (i = 0; i < MAXZ; ++i) {
		sprintf(ztxt[i].text, "%s", zonedb[i].zstr);
		U_INIT_LIST_HEAD(&znode[i].list);
		znode[i].zid = i + 1;
	}
}

/* DONE */
static void
zmap_init(void)
{
	int  i;

	zmap_array_init();
	znrarr = calloc(mapcnt, sizeof(UListHdr));
	zalist = calloc(mapcnt, sizeof(UListHdr));
	for (i = 0; i < mapcnt; ++i) {
		U_INIT_LIST_HEAD(&znrarr[i]);
		U_INIT_LIST_HEAD(&zalist[i]);
	}
	/* 报警地图的位标志 */
	zmbmp_cnt = ((mapcnt-1) >> 5) + 1;
	zmapa = calloc(zmbmp_cnt, sizeof(unsigned int));
}

//////////////////////////////////////////////////////////////////
static void
clear_zmapinfo(void)
{
	zmap_array_init();
	ozpcnt = 0;
	ozmcnt = 0;
	fi_clear(ZmapDbId);
	fi_clear(ZmPosId);
	pzmapall = 0;
}

/* DONE */
static int
save_zmap(void)
{
	FILE *fp;

	fp = fopen(ZMAPDB, "w");
	if (fp == NULL) {
		fi_clear(ZmapDbId);
		return -1;
	}
	ignore_result(fwrite(zmapdb, sizeof(zmap_t), MAXZ, fp));
	fflush(fp);
	fclose(fp);
	fi_update(ZmapDbId);
	return 0;
}

/* DONE: maybe no outer zones.  */
static int
save_ozmapos(void)
{
	FILE *fp;

	if (ozpcnt == 0||ozmcnt == 0) {
		DBG("--- remove the file: ozpcnt = %d, ozmcnt = %d ---\n", ozpcnt, ozmcnt);
		return -1;
	}

	fp = fopen(ZMPOS, "w");
	if (fp == NULL) {
		fi_clear(ZmPosId);
		return -1;
	}
	ignore_result(fwrite(&ozpcnt, sizeof(ozpcnt), 1, fp));
	ignore_result(fwrite(ozpos, sizeof(ozpos[0]), ozpcnt, fp));
	ignore_result(fwrite(&ozmcnt, sizeof(ozmcnt), 1, fp));
	ignore_result(fwrite(ozmapidx, sizeof(ozmapidx[0]), ozmcnt, fp));
	fflush(fp);
	fclose(fp);
	fi_update(ZmPosId);
	return 0;
}

/* should be called when quit the system programming UI. */
void
save_mapinfo(void)
{
	if (pzmapall != 0) {
		if (save_zmap() < 0) {
			clear_zmapinfo();
			return;
		}
		if (save_ozmapos() < 0) {
			clear_zmapinfo();
			return;
		}
	} else {
		clear_zmapinfo();
	}
}

//////////////////////////////////////////////////////////////
/* DONE */
static void
load_zmap(void)
{
	FILE *fp;
	int   i;
	int   p;

	pzmapall = 0;
	memset(zmapdb, 0, MAXZ*sizeof(zmap_t));

	fp = fopen(ZMAPDB, "r");
	if (fp == NULL) return;
	ignore_result(fread(zmapdb, sizeof(zmap_t), MAXZ, fp));
	fclose(fp);

	for (i = 0; i < MAXZ; ++i) {
		if (zmapdb[i].t > 0&&zmapdb[i].zid == i+1) {
			p = pnrofz(i);
			pzmapcnt[p]++;
			pzmapall_set(p);
		}
	}
}

/* DONE */
static void
load_zmpos(void)
{
	FILE *fp;
	int   cnt;

	fp = fopen(ZMPOS, "r");
	if (fp == NULL) {
		clear_zmapinfo();
		return;
	}

	if (fread(&cnt, sizeof(cnt), 1, fp) != 1) {
		goto err_zmpos;
	}

	if (cnt == 0) {
		goto err_zmpos;
	}

	if (fread(ozpos, sizeof(ozpos[0]), cnt, fp) != cnt) {
		goto err_zmpos;
	}
	ozpcnt = cnt;

	if (fread(&cnt, sizeof(cnt), 1, fp) != 1) {
		goto err_zmpos;
	}

	if (fread(ozmapidx, sizeof(ozmapidx[0]), cnt, fp) != cnt) {
		goto err_zmpos;
	}
	ozmcnt = cnt;

	fclose(fp);
	return;

err_zmpos:
	fclose(fp);
	clear_zmapinfo();
}

static void
load_mapinfo(void)
{
	if (zarcnt > 0&&mapcnt > 0) {
		load_zmap();
		if (pzmapall > 0)
			load_zmpos();
	} else {
		clear_zmapinfo();
	}
}

//////////////////////////////////////////////////////

/* DONE */
static void*
izm_new(zmap_t *zm)
{
	UtkWindow *w;
	unsigned short clr;
	char  *ptr;
	int    t;

	if (get_zone_state_direct(zm->zid-1) < ZST_ARM) {
		clr = DA_CLR;
	} else {
		clr = AM_CLR;
	}

	t = (zm->t > 1) ? 1 : 0;
	w = utk_label_new();
	utk_widget_set_color(w, rclr[t]);
	utk_widget_set_size(w, LB_W, LB_H);
	utk_label_set_fnt((UtkLabel*)w, 1);
	utk_label_set_text_pos((UtkLabel*)w, LB_XOFF, LB_YOFF);
	utk_label_set_text_size((UtkLabel*)w, 4);
	utk_label_set_text_color((UtkLabel*)w, clr);
	ptr = ztxt[zm->zid-1].text;
	ptr[4] = 0;
	utk_label_set_text((UtkLabel*)w, ptr);
	utk_window_add_cntl(mapvw, w, zm->zpt.x, zm->zpt.y);
	DBG("--- new inner zone[%d] in %d map ---\n", zm->zid, zm->mapid);

	return (void*)w;
}

/* DONE */
static void*
ozm_new(zmap_t *zm)
{
	UtkWindow *w;
	unsigned short clr;

	if (get_zone_state_direct(zm->zid-1) < ZST_ARM) {
		clr = DA_CLR;
	} else {
		clr = AM_CLR;
	}
	w = utk_line_new();
	utk_line_set_size((UtkLine *)w, 3);
	utk_line_set_ept_size((UtkLine *)w, 4);
	utk_line_set_color(w, clr);
	utk_line_set_endpoint(w, &ozpos[zm->pos], zm->t);

	utk_window_add_cntl(mapvw, w, -1, -1);
	DBG("--- new outer zone[%d] in %d map ---\n", zm->zid, zm->mapid);

	return (void*)w;
}

/* DONE */
static void
widget_new_from_zmap(zmap_t *zm)
{
	znr_node_t  *zn;
	int   t;

	if (zm->t == 0) return;

	utk_window_set_current_page(mapvw, zm->mapid, 0);
	t = !!(zm->t >> 1);
	zplw[zm->zid-1] = (*zmeops[t].zmnew)(zm);
	zn = &znode[zm->zid-1];

	if (zm->t == 1) { // for inner zones,zone number label represents the inner zone.
		zn->w = zplw[zm->zid-1];
	} else if (zm->t > 1) {//create the zone number label for outer zone.
		zn->w = (*zmeops[0].zmnew)(zm);
	}

	zn->alert = 0;
	zn->mapid = zm->mapid;
	zn->zid = zm->zid;
	u_list_append(&zn->list, &znrarr[zm->mapid]);
	DBG("--- new %s zone[%d] widget in %d map ---\n", (zm->t > 1)?"outer":"inner", zm->zid, zm->mapid);
}

/* DONE: z > 0 */
static zmap_t*
init_zmap(int z)
{
	zmap_t  *zmap;

	zmap = &zmapdb[z-1];
	zmap->mapid = mapid;
	zmap->zpt = znrpt;
	zmap->zid = z;

	return zmap;
}

/* DONE */
static void
save_izmap(void)
{
	zmap_t  *zmap;

	if (currznr == 0) return;
	disable_syncfg();
	zmap = init_zmap(currznr);
	zmap->t = 1;
	pzmapall_setz(zmap->zid-1);
	widget_new_from_zmap(zmap);
}

/* DONE */
static void
save_ozmap(void)
{
	zmap_t  *zmap;

	if (currznr == 0) return;
	disable_syncfg();
	zmap = init_zmap(currznr);
	if (ozcnt == 0) { //from scratch
		zmap->t = ocnt;
		zmap->pos = ozpcnt;
		DBG("--- pos = %d, pcnt = %d ---\n", ozpcnt, ocnt);
	} else { //from the last point.
		memcpy(&ozpos[ozpcnt], &ozpos[ozpcnt-1], sizeof(UPoint));
		zmap->t = ocnt + 1;
		zmap->pos = ozpcnt++;
		DBG("--- pos = %d, pcnt = %d ---\n", zmap->pos, zmap->t);
	}
	memcpy(&ozpos[ozpcnt], &opt[0], ocnt*sizeof(UPoint));
	ozpcnt += ocnt;
	ocnt = 0;
	memset(opt, 0, MAXLINMAP*sizeof(UPoint));
	++ozcnt;

	zmap->pidx = ozmcnt;
	ozmapidx[ozmcnt++] = zmap->zid-1;
	DBG("--- pidx = %d ---\n", zmap->pidx);

	pzmapall_setz(zmap->zid-1);
	widget_new_from_zmap(zmap);
}

static void
bimage_init(void)
{
	int  i;

	bimg.UIMG_W = LB_W;
	bimg.UIMG_H = LB_H;
	bimg.UIMG_RBYTES = LB_W*MXC_BPP;
	bimg.img = baddr;

#if 0
	pimg.UIMG_W = PNT_SZ;
	pimg.UIMG_H = PNT_SZ;
	pimg.UIMG_RBYTES = PNT_SZ*MXC_BPP;
	pimg.img = paddr;
#else
	for (i = 0; i < MAXLINMAP; ++i) {
		pimg[i].UIMG_W = PNT_SZ;
		pimg[i].UIMG_H = PNT_SZ;
		pimg[i].UIMG_RBYTES = PNT_SZ*MXC_BPP;
		pimg[i].img = paddr[i];
	}
#endif
}

static int
pz_widget_new(int p, int z)
{
	widget_new_from_zmap(&zmapdb[z]);
	return 0;
}

static void
new_widget_from_zmap(void)
{
	unsigned int  pm = pzmapall;
	int   p;

	while (pm) {
		p = __builtin_ctz(pm);
		pzopt_zones(p, pz_widget_new);
		pm &= ~(1 << p);
	}
}

/////////////////////////////////////////////////////////////
/* DONE */
static void
go_mapsel(void *arg)
{
	cleanup_optbuf();
	cleanup_znrbuf();
	if (ezt == 1||utk_widget_state(bnoz) != 0) {
		utk_widget_update_state(bnoz, 0);
	}

	if (ezt == 0||utk_widget_state(bniz) != 0) {
		utk_widget_update_state(bniz, 0);
	}
	if (ezt == 2||utk_widget_state(bnmz) != 0) {
		utk_widget_update_state(bnmz, 0);
	}
	ezt = -1;
	utk_widget_undrawable(mapvw);
	utk_widget_detach(cn_edt);
	utk_widget_attach(bnret);
	utk_widget_attach(bndisp);
	utk_widget_attach(bnok);
	utk_widget_attach(bnpu);
	utk_widget_attach(bnpd);
	utk_window_reflush(mapvw);
	edst = 0;
}

////////////////////////////////////////////
/* DONE */
static void 
oz_edit(void *arg)
{
	if (utk_widget_state(bnoz) > 0) {
		if (ezt == 0) {
			utk_widget_update_state(bniz, 0);
		} else if (ezt == 2) {
			utk_widget_update_state(bnmz, 0);
		}
		edst = 1;
		ezt = 1;
		ocnt = 0;
	} else {
		edst = 0;
		ezt = -1;
	}
	ozcnt = 0;
}

/* DONE */
static void 
iz_edit(void *arg)
{
	if (utk_widget_state(bniz) > 0) {
		if (ezt == 1) {
			utk_widget_update_state(bnoz, 0);
		} else if (ezt == 2) {
			utk_widget_update_state(bnmz, 0);
		}
		edst = 1;
		ezt = 0;
	} else {
		if (edst == 2) {
			DBG("--- edst = 2, when quit. ---\n");
			dav_fb_draw(znrpt.x, znrpt.y, &bimg);
		}
		edst = 0;
		ezt = -1;
	}
}

/* DONE */
static inline void
del_point(UPoint *pt)
{
	dav_fb_draw(pt->x - (PNT_SZ>>1), pt->y - (PNT_SZ>>1), &pimg[ocnt]);
}

/* DONE */
static inline void
draw_point(UPoint *pt)
{
	int  sz = PNT_SZ >> 1;
	dav_fb_get_scr(pt->x - sz, pt->y - sz, PNT_SZ, PNT_SZ, &pimg[ocnt]);
	dav_fb_fill_rect(pt->x - sz, pt->y - sz, PNT_SZ, PNT_SZ, DA_CLR);
}

/* DONE */
static void
undo_izme(void)
{
	UtkResource *res;

	switch (edst) {
	case 1:
		edst = 0;
		utk_widget_update_state(bniz, 0);
		ezt = -1;
		break;
	case 2:
		res = resrc[mapid];
		dav_fb_draw(znrpt.x, znrpt.y, &bimg);
		edst = 0;
		utk_widget_update_state(bniz, 0);
		ezt = -1;
		break;
	default:
		break;
	}
}

/* DONE */
static void
undo_ozme(void)
{
	if (edst == 1) {
		if (ocnt > 0) {
			del_point(&opt[--ocnt]);
		} else {
			edst = 0;
			utk_widget_update_state(bnoz, 0);
			ezt = -1;
			ozcnt = 0;
		}
	} else if (edst == 2) {//all points have been linked.
	}
}

/* DONE */
static void 
edit_cancel(void *arg)
{
	if (ezt >= 0&&ezt < 2) {
		(*zmeops[ezt].undo)();
	}
}

////////////////////////////////////////////////////

static void
do_izme(void)
{
}

/* DONE */
static void
do_ozme(void)
{
	int  i;
	int  ext;

	ext = (ozcnt > 0) ? 1 : 0;
	if ((edst == 1)&&(ocnt + ext >= 2)) {
		if (ozcnt > 0) {
			dav_fb_draw_line(ozpos[ozpcnt-1].x, ozpos[ozpcnt-1].y, opt[0].x, opt[0].y, DA_CLR, 0);
		}
		for (i = 0; i < ocnt - 1; ++i) {
			dav_fb_draw_line(opt[i].x, opt[i].y, opt[i+1].x, opt[i+1].y, DA_CLR, 0);
		}
		edst = 2;
	}
}

/* DONE */
static void 
edit_ok(void *arg)
{
	if (ezt >= 0&&ezt < 2) {
		(*zmeops[ezt].dozme)();
	}
}

////////////////////////////////////////////////////////
/* DONE */
static void
start_izne(void)
{
	int  x;

	if (edst == 2) {
		cleanup_znrbuf();
		DBG("--- znr will be displayed here: %d, %d ---\n", znrpt.x, znrpt.y);
		x = znrpt.x + LB_XOFF;
		inpy = znrpt.y + LB_YOFF;
		utk_widget_show_caret(mapvw, x - 1-DRAW_X, inpy-DRAW_Y);
		deltax = x;
		show_keybar(&mkbar);
		edst = 3;
	}
}

/* DONE */
static void
start_ozne(void)
{
	if (edst == 2) {
		DBG("--- editing outer znr ---\n");
		cleanup_znrbuf();
		edst = 3;
	}
}

/* DONE */
static void 
edit_znr(void *arg)
{
	if (ezt >= 0&&ezt < 2) {
		(*zmeops[ezt].znest)();
	}
}

////////////////////////////////////////////////////////////

static void 
modify_zinf(void *arg)
{
	if (utk_widget_state(bnmz) > 0) {
		if (ezt != -1) {
			utk_widget_update_state(bnmz, 0);
			return;
		}
		ezt = 2;
	} else {
		ezt = -1;
	}
}

static void
show_znr_label(void)
{
	UListHdr *head = &znrarr[mapid];
	UListHdr *list;
	znr_node_t  *node;

	list = head->next;
	while (list != head) {
		node = (znr_node_t*)list;
		utk_widget_show(node->w);
		list = list->next;
	}
}

static void
hide_znr_label(void)
{
	UListHdr *head = &znrarr[mapid];
	UListHdr *list;
	znr_node_t  *node;

	list = head->next;
	while (list != head) {
		node = (znr_node_t*)list;
		utk_widget_hide(node->w);
		list = list->next;
	}

	list = head->next;
	while (list != head) {
		node = (znr_node_t*)list;
		if (zmapdb[node->zid-1].t > 1) {
			utk_widget_update(zplw[node->zid-1]);
		}
		list = list->next;
	}
}

static void 
show_znr(void *arg)
{
	UtkWindow *w = (UtkWindow*)arg;

	if (utk_widget_state(w) == 0) {
		show_znr_label();
	} else {
		hide_znr_label();
	}
}

///////////////////////////////////////////////////////
/* DONE */
static int
znr_del(void)
{
	utk_widget_hide_caret(mapvw);
	if (znrlen == 0) return 0;
	if (znrlen > 0) {
		znrbuf[--znrlen] = 0;
		deltax -= 12;
		dav_fb_fill_rect(deltax, inpy, 12, 24, rclr[ezt]);
	}
	utk_widget_show_caret(mapvw, deltax+1-DRAW_X, inpy-DRAW_Y);

	return 1;
}

/* DONE */
static void
znr_inp(int v)
{
	DBG("--- INPUT %c ---\n", v);
	if (znrlen >= 4) return;
	znrbuf[znrlen++] = (char)v;
	znrbuf[znrlen] = 0;
	utk_widget_hide_caret(mapvw);
	utk_draw_ascii_char(v, deltax, inpy, DA_CLR, 1);
	deltax += 12;
	utk_widget_show_caret(mapvw, deltax+1-DRAW_X, inpy-DRAW_Y);
}

/* DONE */
static void
handle_keybar_show(int i)
{
	if (i == 0)
		keybar_ungrab(&mkbar);
	else
		keybar_grab(&mkbar);
}

/* DONE */
static void
znr_undo(void)
{
	if (znrlen > 0) {
		dav_fb_fill_rect(znrpt.x, znrpt.y, LB_W, LB_H, rclr[ezt]);
	}
}

static int
get_znr(char *s)
{
	int  z;
	int  h;
	int  zz;

	z = strtol(s, NULL, 10);
	h = z / 100;
	zz = z % 100;
	if ((zz == 0)||(h == 0)||(h > 32)) {
		DBG("--- invalid znr ---\n");
		return 0;
	}

	z = (h - 1) * 99 + zz;
	if (zonedb[z-1].znr != z) {
		DBG("--- ZONE-%d not exists ---\n", z);
		return 0;
	}
	DBG("--- znr = %d: h = %d, z = %d ---\n", z, h, zz);

	return z;
}

/* DONE */
static int
check_zone(void)
{
	int z;

	z = get_znr(znrbuf);
	currznr = z;
	if (z == 0) {
		znr_undo();
		show_tip(&undefz);
		return 0;
	}

	if (zmapdb[z-1].t > 0) {
		znr_undo();
		show_tip(&redefz);
		return 0;
	}

	return 1;
}

/* DONE */
static void
izne_ok(void)
{
	DBG("--- znr input finished, please press ok. ---\n");
	if (znrlen > 0) {
		if (check_zone() == 1) {
			save_izmap();
			edst = 1;
		} else {
			cleanup_znrbuf();
			edst = 2;
		}
	} else {
		cleanup_znrbuf();
		edst = 2;
	}
}

/* DONE */
static void
izne_cl(void)
{
	DBG("--- znr input canceled, you can relocate the znr display area. ---\n");
	edst = 2;
	cleanup_znrbuf();
}

/* DONE */
static void
ozne_cl(void)
{
	DBG("--- znr input canceled, you can relocate the znr display area. edst = %d ---\n", edst);
	cleanup_znrbuf();
	dav_fb_draw(znrpt.x, znrpt.y, &bimg);
	edst = 3;
}

/* DONE */
static void
ozne_ok(void)
{
	DBG("--- znr input finished, please press ok. edst = %d ---\n", edst);
	if (znrlen > 0) {
		if (check_zone() == 1) {
			save_ozmap();
			edst = 1; //prepare the next loop.
		} else {
			dav_fb_draw(znrpt.x, znrpt.y, &bimg);
			cleanup_znrbuf();
		}
	} else {
		cleanup_znrbuf();
		dav_fb_draw(znrpt.x, znrpt.y, &bimg);
	}
}

static void
znr_act(int i)
{
	utk_widget_hide_caret(mapvw);
	if (i == 1) { //ok pressed.
		(*zmeops[ezt].zneok)();
	} else if (i == 0) { //cancel pressed and clear znr buffer
		(*zmeops[ezt].znecl)();
	}
}

static keybar_ops_t  mkop = {
	.fw = znr_inp,
	.bp = znr_del,
	.show = handle_keybar_show,
	.act = znr_act,
};

/* DONE */
static void
draw_rect(UPoint *pt, int w, int h, unsigned short clr)
{
	if (pt->x + w > DRAW_X+DRAW_W) {
		pt->x = DRAW_X+DRAW_W - w;
	}

	if (pt->y + h > DRAW_Y+DRAW_H) {
		pt->y = DRAW_Y+DRAW_H - h;
	}
	dav_fb_get_scr(pt->x, pt->y, w, h, &bimg);
	dav_fb_fill_rect(pt->x, pt->y, w, h, clr);
}

///////////////////////////////////////////////////////////
#define RECTL(r)    ((r)->left)
#define RECTT(r)    ((r)->top)
#define RECTW(r)    ((r)->right - (r)->left + 1)
#define RECTH(r)    ((r)->bottom - (r)->top + 1)

static znr_node_t  *mnode = NULL;

static znr_node_t*
change_znr(znr_node_t *zn, int replace)
{
	int  zs = zn->zid - 1;
	int  zd;
	zmap_t  *zms = &zmapdb[zs];
	zmap_t  *zmd;
	znr_node_t *znd;

	if (currznr == 0||currznr == zms->zid) return zn;
	zd = currznr-1;
	zmd = &zmapdb[zd];
	znd = &znode[zd];
	if (!u_list_empty(&zn->list)) {
		u_list_del(&zn->list);
	}

	if (!u_list_empty(&znd->list)) {
		u_list_del(&znd->list);
	}

	if (replace > 0) {
		if (zmd->t > 0) {
			utk_widget_destroy(zplw[zd]);
			if (zmd->t > 1) {
				utk_widget_destroy(znd->w);
			}
		}
		cleanup_zmap(zmd);
	}

	memcpy(zmd, zms, sizeof(zmap_t));
	zmd->zid = zd + 1;
	pzmapall_clrz(zms->zid-1);
	memset(zms, 0, sizeof(zmap_t));

	memcpy(znd, zn, sizeof(znr_node_t));
	U_INIT_LIST_HEAD(&znd->list);
	znd->zid = zd + 1;
	znd->alert = 0;
	u_list_append(&znd->list, &znrarr[mapid]);

	zplw[zd] = zplw[zs];
	zplw[zs] = NULL;
	utk_label_set_text((UtkLabel*)znd->w, ztxt[zd].text);

	pzmapall_setz(zmd->zid-1);
	save_mapinfo();

	return znd;
}

static void
znr_change(void *data)
{
	hide_model_dialog(zchg);
	change_znr(mnode, 1);
	utk_window_reflush(mapvw);
}

static void
cancel_mdf(void *data)
{
	hide_model_dialog(zchg);
	utk_label_update(mnode->w);
}

/* DONE */
static int
check_zonenr(void)
{
	int z;

	z = get_znr(znrbuf);
	currznr = z;
	if (z == 0) {
		show_tip(&undefz);
		return -1;
	}

	if (zmapdb[z-1].t > 0) {
		return 1;
	}

	return 0;
}

static void
mznr_ok(void)
{
	int  z;
	znr_node_t  *zn = mnode;

	DBG("--- znr input finished, please press ok. ---\n");
	if (znrlen > 0) {
		z = check_zonenr();
		if (z == 0) {
			zn = change_znr(mnode, 0);
			utk_label_update(zn->w);
		} else if (z < 0) {
			cleanup_znrbuf();
			utk_label_update(zn->w);
		} else if (z > 0) {
			show_model_dialog(zchg);
			cleanup_znrbuf();
		}
	} else {
		cleanup_znrbuf();
		utk_label_update(zn->w);
	}
}

static void
mznr_cl(void)
{
	cleanup_znrbuf();
	utk_label_update(mnode->w);
}

static void
do_mdfznr(URect *r)
{
	int  x;

	cleanup_znrbuf();
	znrpt.x = RECTL(r);
	znrpt.y = RECTT(r);
	dav_fb_fill_rect(RECTL(r), RECTT(r), RECTW(r), RECTH(r), rclr[2]);
	x = znrpt.x + LB_XOFF;
	inpy = znrpt.y + LB_YOFF;
	utk_widget_show_caret(mapvw, x - 1 - DRAW_X, inpy - DRAW_Y);
	deltax = x;
	show_keybar(&mkbar);
}

static void
mznr_touch(void *arg)
{
	UListHdr *head = &znrarr[mapid];
	UListHdr *list;
	znr_node_t  *node;
	URect    *r = NULL;
	UPoint   *pnt = (UPoint*)arg;
	int      t;

	list = head->next;
	while (list != head) {
		node = (znr_node_t*)list;
		r = utk_widget_get_rect(node->w);
		if (u_pt_in_rect(r, pnt->x, pnt->y)) {
			mnode = node;
			t = (zmapdb[node->zid-1].t > 1) ? 1 : 0;
			rclr[2] = rclr[t];
			do_mdfznr(r);
			return;
		}
		list = list->next;
	}
}

///////////////////////////////////////////////////////////

/* DONE */
static void
izme_touch(void *arg)
{
	UPoint *pnt = (UPoint*)arg;

	DBG("---- touch: x = %d, y = %d ---\n", pnt->x, pnt->y);
	switch (edst) {
	case 1:
		draw_rect(pnt, LB_W, LB_H, ILB_CLR);
		znrpt = *pnt;
		edst = 2;
		break;
	case 2:
		dav_fb_draw(znrpt.x, znrpt.y, &bimg);
		draw_rect(pnt, LB_W, LB_H, ILB_CLR);
		znrpt = *pnt;
		break;
	default:
		break;
	}
}

/* DONE */
static void
ozme_touch(void *arg)
{
	UPoint *pnt = (UPoint*)arg;
	int   x;

	DBG("---- touch: x = %d, y = %d ---\n", pnt->x, pnt->y);
	switch (edst) {
	case 1: //start draw point,DONE
		if (ocnt < MAXLINMAP) {
			draw_point(pnt);
			opt[ocnt++] = *pnt;
		}
		break;
	case 3: //input znr,DONE
		draw_rect(pnt, LB_W, LB_H, OLB_CLR);
		znrpt = *pnt;
		x = znrpt.x + LB_XOFF;
		inpy = znrpt.y + LB_YOFF;
		utk_widget_show_caret(mapvw, x - 1 - DRAW_X, inpy - DRAW_Y);
		deltax = x;
		show_keybar(&mkbar);
		break;
	default:
		break;
	}
}

/* DONE */
static void
handle_touch(void *arg)
{
	if (ezt >= 0&&ezt < 3) {
		(*zmeops[ezt].touch)(arg);
	}
}

/////////////////////////////////////////////////////////////

static void
reload_mapinfo(void)
{
	int  i;

	for (i = 0; i < mapcnt; ++i) {
		utk_window_page_destroy(mapvw, i);
		free_znarr_list(i);
	}
	load_mapinfo();
	new_widget_from_zmap();
}

static void
del_zmap(void *data)
{
	hide_model_dialog(zdel);
	utk_window_page_destroy(mapvw, mapid);
	free_znarr_list(mapid);
	save_mapinfo();
	utk_window_reflush(mapvw);
}

static void
cancel_del(void *data)
{
	hide_model_dialog(zdel);
}

static void
undo_all(void *arg)
{
	if (ezt >= 0) return;
	show_model_dialog(zdel);
}

static void*
build_edit_buttons(void *parent, int x, int y)
{
	UtkWindow  *cn;
	UtkWindow  *bn;

	cn = utk_container_new();
	utk_container_set_bg(cn, BG_MAP);
	utk_widget_add_cntl((UtkWindow*)parent, cn, x, y);

	bn = utk_button_new();
	utk_button_set_bg(bn, BN_RET1180);
	utk_button_set_bg(bn, BN_RET118G);
	utk_signal_connect(bn, "clicked", go_mapsel, (void*)0);
	utk_container_add_widget(cn, bn, 0, 0);

	bnoz = utk_toggle_button_new();
	utk_button_set_bg(bnoz, BN_OZT0);
	utk_button_set_bg(bnoz, BN_OZT1);
	utk_signal_connect(bnoz, "clicked", oz_edit, (void*)0);
	utk_container_add_widget(cn, bnoz, 174-56, 0);

	bniz = utk_toggle_button_new();
	utk_button_set_bg(bniz, BN_IZT0);
	utk_button_set_bg(bniz, BN_IZT1);
	utk_signal_connect(bniz, "clicked", iz_edit, (void*)0);
	utk_container_add_widget(cn, bniz, 336-56, 0);

	bn = utk_button_new();
	utk_button_set_bg(bn, BN_MCL0);
	utk_button_set_bg(bn, BN_MCL1);
	utk_signal_connect(bn, "clicked", edit_cancel, (void*)0);
	utk_container_add_widget(cn, bn, 498-56, 0);

	bn = utk_button_new();
	utk_button_set_bg(bn, BN_MOK0);
	utk_button_set_bg(bn, BN_MOK1);
	utk_signal_connect(bn, "clicked", edit_ok, (void*)0);
	utk_container_add_widget(cn, bn, 579-56, 0);

	bn = utk_button_new();
	utk_button_set_bg(bn, BN_MZNR0);
	utk_button_set_bg(bn, BN_MZNR1);
	utk_signal_connect(bn, "clicked", edit_znr, (void*)0);
	utk_container_add_widget(cn, bn, 660-56, 0);

	bnmz = utk_toggle_button_new();
	utk_button_set_bg(bnmz, BN_MMOD0);
	utk_button_set_bg(bnmz, BN_MMOD1);
	utk_signal_connect(bnmz, "clicked", modify_zinf, (void*)0);
	utk_container_add_widget(cn, bnmz, 806-56, 0);

	bn = utk_button_new();
	utk_button_set_bg(bn, BN_MDEL0);
	utk_button_set_bg(bn, BN_MDEL1);
	utk_signal_connect(bn, "clicked", undo_all, (void*)0);
	utk_container_add_widget(cn, bn, 887-56, 0);

	return (void*)cn;
}

/////////////////////////////////////////////////////////
#if 0
{
#endif

static void
ret_cb(void *data)
{
	if (fromw == 0) {
		save_mapinfo();
		show_gencfg_win();
	} else if (fromw == 1) {
		show_main_win();
	} else if (fromw == 2) {
		if (unhandled_zalarm() > 0) {
			show_zalrm_win();
		} else {
			show_main_win();
		}
	}
}

static void
map_edit(void *data)
{
	if (!is_master()) return;
	if (zarcnt > 0) {
		utk_widget_detach(bnret);
		utk_widget_detach(bndisp);
		utk_widget_detach(bnok);
		utk_widget_detach(bnpu);
		utk_widget_detach(bnpd);
		utk_widget_attach(cn_edt);
		utk_window_reflush(mapvw);
		utk_widget_drawable(mapvw);
	} else {
		show_tip(&nozone);
	}
	cleanup_optbuf();
	cleanup_znrbuf();
	edst = 0;
	ezt = -1;
}

static void
map_switch(void *arg)
{
	int i = (int)arg;

	utk_timeout_remove(timerid);
	zblnk = 0;
	if (i == 0) {
		utk_window_prev_page(mapvw);
		mapid = (mapid - 1 + mapcnt) % mapcnt;
	} else {
		utk_window_next_page(mapvw);
		mapid = (mapid + 1) % mapcnt;
	}

	if (!u_list_empty(&zalist[mapid])) {
		utk_timeout_add(30, timerid);
	}
}

/* disarmed */
static void
zmst_opclr(int z)
{
	znr_node_t  *zn = &znode[z];
	int  t = zone_iotype(z);

	if (zn->alert == 1) {
		u_list_del(&zn->list);
		u_list_append(&zn->list, &znrarr[zn->mapid]);
		zn->alert = 0;
		if (u_list_empty(&zalist[zn->mapid])) {
			zmapa_clr(zn->mapid);
		}
	}

	(*update_z[!!t])(z, 0);
}

/* armed */
static void
zmst_opset(int z)
{
	int  t = zone_iotype(z);
	(*update_z[!!t])(z, 1);
}

/* alarmed */
static void
zones_alarmed(int z)
{
	znr_node_t  *zn = &znode[z];

	u_list_del(&zn->list);
	u_list_append(&zn->list, &zalist[zn->mapid]);
	zn->alert = 1;
	zmapa_set(zn->mapid);
	DBG("--- zmapa[%d] = 0x%08x ---\n", (zn->mapid>>5), zmapa[zn->mapid>>5]);
}

static void (*zmst_opr[])(int) = {zmst_opclr, zmst_opset, zones_alarmed};

////////////////////////////////////////////////
static void
handle_zst(UtkWindow *win, int  z, int st)
{
	int  p;

	if (win != mapvw) return;
	if (z >= 0&&z < MAXZ) { //single zone operation
		(*zmst_opr[st])(z); //synchronize with zmapst in zopt.c
	} else if (z == MAXZ) {//all arm/disarm
		(*zallopr[st])();
	} else if (z < 0) { //pzone operation
		p = st >> 1;
		st &= 1;
		(*pzallopr[st])(p);
		DBG("--- pzone-%d: %s ---\n", p+1, (st == 1)?"arm":"disarm");
	} else if (z > MAXZ) {
		DBG("--- reload map ---\n");
		reload_mapinfo();
	}
}

static void
hide_widget(znr_node_t *zn)
{
	zmap_t  *zm;

	zm = &zmapdb[zn->zid-1];
	utk_label_clear_text(zn->w);
	if (zm->t > 1) {
		utk_widget_set_color(zplw[zn->zid-1], gclr[zblnk]);
		utk_widget_update(zplw[zn->zid-1]);
	}
}

static void
show_widget(znr_node_t *zn)
{
	zmap_t  *zm;

	zm = &zmapdb[zn->zid-1];
	utk_label_set_text_color((UtkLabel*)zn->w, AM_CLR);
	utk_label_update(zn->w);

	if (zm->t > 1) {
		utk_widget_set_color(zplw[zn->zid-1], gclr[zblnk]);
		utk_widget_update(zplw[zn->zid-1]);
	}
}

static FBN  znr_blink[] = {hide_widget, show_widget};

static void
blink_zones(void *arg)
{
	UListHdr *head = &zalist[mapid];
	UListHdr *list;
	znr_node_t  *zn;

	list = head->next;
	while (list != head) {
		zn = (znr_node_t*)list;
		znr_blink[zblnk](zn);
		list = list->next;
	}
	zblnk ^= 1;
}

static void
start_blink(void *arg)
{
	if (!u_list_empty(&zalist[mapid])) {
		utk_timeout_add(30, timerid);
	}
}

static void
stop_blink(void *arg)
{
	utk_timeout_remove(timerid);
	zblnk = 0;
}

void
build_map_gui(void)
{
	UtkResource *res;
	int   i;
	char *bgok[] = {BN_DELOK0, BN_DELOK1};
	char *bgcl[] = {BN_DELCL0, BN_DELCL1};

	res = load_resource(MAP_PATH, MAP_RES_FILE, &mapcnt);
	if (!res||mapcnt == 0) return;

	bimage_init();
	zmap_init();
	DBG("--- before loading mapinfo ---\n");
	load_mapinfo();
	DBG("--- after loading mapinfo ---\n");

	mapvw = utk_window_derive();
	utk_widget_set_resource_listn(mapvw, res, mapcnt);
	utk_window_set_page_mode(mapvw);
	utk_widget_unresident(mapvw);
	utk_window_set_drawable_area(mapvw, DRAW_X, DRAW_Y, DRAW_W, DRAW_H);
	utk_signal_connect(mapvw, "touch", handle_touch, 0);
	utk_widget_undrawable(mapvw);
	utk_caret_sys_new((void*)mapvw, 2, 24, CARET_CLR);
	utk_widget_async_evt_handler(mapvw, handle_zst);
	timerid = utk_widget_timer(mapvw, blink_zones, (void*)0);
	utk_widget_show_after(mapvw, start_blink, (void*)0);
	utk_widget_hide_before(mapvw, stop_blink, (void*)0);

	resrc = calloc(mapcnt, sizeof(void*));
	for (i = 0; i < mapcnt; ++i) {
		resrc[i] = utk_window_get_resource(mapvw, i);
	}

	build_tip_lbl(&nozone, mapvw, (LCD_W-tip_w)/2, (LCD_H-tip_h)/2, LBL_NOZONE, 2);
	build_tip_lbl(&undefz, mapvw, (LCD_W-tip_w)/2, (LCD_H-tip_h)/2, LBL_UNDEFZ, 2);
	build_tip_lbl(&redefz, mapvw, (LCD_W-tip_w)/2, (LCD_H-tip_h)/2, LBL_REDEFZ, 2);
	build_keybar(&mkbar, mapvw, (LCD_W - keybar_w)>>1, 530, &mkop, 0);

	zdel = build_model_dialog(mapvw, BG_DELDIAG, bgok, bgcl, del_zmap, cancel_del);
	zchg = build_model_dialog(mapvw, BG_CHGZNR, bgok, bgcl, znr_change, cancel_mdf);

	bnret = utk_button_new();
	utk_button_set_bg(bnret, BN_RET1180);
	utk_button_set_bg(bnret, BN_RET118G);
	utk_signal_connect(bnret, "clicked", ret_cb, (void*)0);
	utk_widget_add_cntl(mapvw, bnret, 56, 530);

	bnok = utk_button_new();
	utk_button_set_bg(bnok, BN_MOK0);
	utk_button_set_bg(bnok, BN_MOK1);
	utk_signal_connect(bnok, "clicked", map_edit, (void*)0);
	utk_widget_add_cntl(mapvw, bnok, 602-81, 530);
	
	bndisp = utk_toggle_button_new();
	utk_button_set_bg(bndisp, BN_MDIS0);
	utk_button_set_bg(bndisp, BN_MDIS1);
	utk_signal_connect(bndisp, "clicked", show_znr, (void*)bndisp);
	utk_widget_add_cntl(mapvw, bndisp, 683-81, 530);

	bnpu = utk_button_new();
	utk_button_set_bg(bnpu, BN_AICKPU0);
	utk_button_set_bg(bnpu, BN_AICKPU1);
	utk_signal_connect(bnpu, "clicked", map_switch, (void*)0);
	utk_widget_add_cntl(mapvw, bnpu, 683, 530);

	bnpd = utk_button_new();
	utk_button_set_bg(bnpd, BN_AICKPD0);
	utk_button_set_bg(bnpd, BN_AICKPD1);
	utk_signal_connect(bnpd, "clicked", map_switch, (void*)1);
	utk_widget_add_cntl(mapvw, bnpd, 825, 530);

	cn_edt = build_edit_buttons(mapvw, 56, 530);
	utk_widget_detach(cn_edt);

	new_widget_from_zmap();
}

/* DONE */
/*
 i: 0-config,1-view,2-alarm.
 */
int
show_map_win(int i, int z)
{
	int  res = -1;

	if (i == 2) {
		if (pzmapall == 0) return res;
		
	}
	if (mapvw&&utk_window_top() != mapvw) {
		if (i != 0) {
			utk_widget_detach(bnok);
		} else {
			utk_widget_attach(bnok);
		}
		fromw = i;
		set_current_map(z);
		utk_window_show(mapvw);
		res = 0;
	}
	return res;
}

/* called when zone state changed. */
/* st: 0-disarmed, 1-armed, 2-alarmed. */
void
update_zmapst(int z, int st)
{
	int  s = 0;
	int  p;

	if (mapvw == NULL) {
		return;
	}

	if (z >= 0&&z < MAXZ) { //single zone operation
		if (zmapdb[z].t > 0) {
			s = 1;
		}
	} else if (z == MAXZ) {//all arm/disarm
		if (pzmapall) {
			s = 1;
		}
	} else if (z < 0) { //pzone operation
		p = st >> 1;
		if (pzmapall_isset(p)) {
			DBG("--- pzone-%d: %s ---\n", p+1, (st&1)?"arm":"disarm");
			s = 1;
		}
	}

	if (s) {
		utk_widget_send_async_event(mapvw, z, st);
	}
}

/* called in syncfg.c */
void
reload_mapdb(void)
{
	utk_widget_send_async_event(mapvw, MAXZ+1, 0);
}

/* z: [0, MAXZ) */
void
zmap_del(int z)
{
	zmap_t  *zm = &zmapdb[z];
	znr_node_t *zn = &znode[z];

	if (zm->t > 0) {
		DBG("--- should remove zone-%d from map ---\n", z+1);
		utk_widget_destroy(zplw[z]);
		zplw[z] = NULL;
		if (zm->t > 1) {
			utk_widget_destroy(zn->w);
			zn->w = NULL;
		}
		cleanup_zmap(zm);
		if (!u_list_empty(&zn->list)) {
			u_list_del(&zn->list);
		}
		zn->alert = 0;
		save_mapinfo();
	}
}

#if 0
}
#endif



