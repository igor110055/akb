#include "mxc_sdk.h"
#include "config.h"
#include "zopt.h"
#include "pzone.h"
#include "pztimer.h"
#include "znode.h"
#include "gui.h"
#include "zdelay.h"

typedef struct _pzst {
	int      nunknown;
	int      nalert;
	int      nalertr;
	int      nready;
	int      narm;
	int      ndisarm;
	int      bpcnt;
	int      n24hr;
	int      nzofp;
} pzst_t;

typedef struct _pzinf {
	int     *zarr;
	int      zcnt;
	pzst_t  *pzst;
} pzinf_t;

static unsigned int   pzbmp = 0;
#define  pzbmp_set(i)     pzbmp |= (1 << (i))
#define  pzbmp_clr(i)     pzbmp &= ~(1 << (i))
#define  pzbmp_isset(i)  (pzbmp & (1 << (i)))

/* 旁路恢复标记 */
unsigned int    bparmflag = 0;
unsigned int    frcpzmap = 0;       //need to be saved.

static int      total_nready = 0;

static pzst_t    pzstat[MAXPZ];
static pzinf_t   pzinfo[MAXPZ];

pzone_t  pzone[MAXPZ];
int      pzarr[MAXPZ] = {0};
int      pzcnt = 0;

/* 标记各分区是否有警情 */
unsigned short  pzaltmap = 0;

/* pzone arm flag */
static unsigned int  pzarm = 0;
#define pzarm_set(p)    pzarm |= (1<<(p))
#define pzarm_clr(p)    pzarm &= ~(1<<(p))

/* pzone disarm flag */
static unsigned int  pzdm = 0;
#define pzdm_set(p)     pzdm |= (1<<(p))
#define pzdm_clr(p)     pzdm &= ~(1<<(p))

char   pstat[MAXPZ] = {0};
char  *zstat = NULL;
static int  zsize = 0;

void
zstate_init(int sz)
{
	if (sz != zsize) {
		if (sz == 0) {
			if (zstat != NULL) {
				free(zstat);
				zstat = NULL;
				zsize = 0;
			}
		} else {
			zstat  = realloc(zstat, sz);
			zsize  = sz;
			memset(zstat, 0, sz);
		}
	}
}

/* DONE */
int
pzone_exist(int pnr)
{
	return pzbmp_isset(pnr-1);
}

/* called once only when boot. */
void
pzinfo_init(void)
{
	int  i;

	CLEARA(pzinfo);
	CLEARA(pzstat);
	for (i = 0; i < MAXPZ; ++i) {
		pzinfo[i].pzst = &pzstat[i];
	}
}

static inline void
system_clr(void)
{
	if (zstat != NULL&&zsize > 0) {
		memset(zstat, 0, zsize);
	}
	CLEARA(pstat);
	CLEARA(pzstat); //should not clear pzinfo,because pzinfo->pzst should not be cleared.

	pzbmp = 0;
	bparmflag = 0;
	total_nready = 0;
	tmrpzmap = 0;
	frcpzmap = 0;
	pztmrflg = 0;
	pzaltmap = 0;

	pzarm = 0;
	pzdm =  0;
	nready_clear();
	alarmst_clear();
	clear_sysfst(); //clear all message in main UI[start.c]
	//should clean the map info here.
}

////////////////////////////////////////////////////////////////
/* DONE: called in zmgr.c when exit the system programming. */
void
load_pzonedb(void)
{
	int      i;
	pzone_t *pz;

	pzcnt = 0;
	pzbmp = 0;
	CLEARA(pzarr);
	for (i = 0; i < MAXPZ; ++i) {
		pz = &pzone[i];
		if (pz->pnr == i+1) {
			pzbmp_set(i);
			pz->idx = pzcnt;
			pzarr[pzcnt++] = i;
		}
	}
	prepare_pztip(pzarr, pzcnt);
}

/* DONE */
int
load_pzone(void)
{
	FILE  *fp;
	int    n;
	int    r = 0;

	pzcnt = 0;
	CLEARA(pzone);
	if (zarcnt == 0) {
		DBG("--- no zone info defined ---\n");
		unlink(PZONEDB);
		return 0;
	}

	if ((fp = fopen(PZONEDB, "r")) == NULL) {
		DBG("--- open %s failed ---\n", PZONEDB);
//		zone_clear();
		return 0;
	}

	n = fread(&pzone[0], sizeof(pzone_t), MAXPZ, fp);
	if (n == MAXPZ) {
		load_pzonedb();
		r = 1;
	} else {
		DBG("--- read pzone error: n = %d ---\n", n);
	}

	fclose(fp);
	return r;
}

/* DONE: called by slave in syncfg.c */
void
reload_pzonedb(void)
{
	if (load_pzone()) {
		init_pzinfo();
	}
	clear_pzst_label();
	pzent_enable(pzcnt);
}

//////////////////////////////////////////////////////////////////

/* DONE */
void
store_pzone_file(void)
{
	FILE  *fp;

	if (zarcnt == 0) {
		fi_clear(PzneDbId);
		memset(pzone, 0, sizeof(pzone_t)*MAXPZ);
		pzcnt = 0;
	}
	if ((fp = fopen(PZONEDB, "w")) != NULL) {
		if (fwrite(&pzone[0], sizeof(pzone_t), MAXPZ, fp) != MAXPZ) {
			DBG("--- fwrite error: %s ---\n", strerror(errno));
		}
		fflush(fp);
		fclose(fp);
		fi_update(PzneDbId);
		DBG("--- pzone file stored ---\n");
	}
}

/* called in zcfg.c */
void
store_pzone(int p, char *name)
{
	pzone_t  *pz;

	pz = &pzone[p-1];
	if (pz->pnr != p||strcmp(pz->pname, name)) {//modified on 2014.04.29
		memset(pz->pstr, 0 , 4);
		snprintf(pz->pstr, 4, "%d", p);
		pz->pnr = p;
		memset(pz->pname, 0, MAXNAME);
		strncpy(pz->pname, name, MAXNAME - 1);
		DBG("--- pzone-%s: pzname = %s ---\n", pz->pstr, pz->pname);
	}
}

int *
pzinfo_get(int p, int *n)
{
	*n = pzinfo[p].zcnt;
	return pzinfo[p].zarr;
}


////////////////////////////////////////////////////////////////////////
/* should be called after zone and pzone loaded. */
void
init_pzinfo(void)
{
	int  i;
	zone_t *z;
	pzinf_t *pzi;
	
	system_clr();
	if (zarcnt == 0) {
		return;
	}

	for (i = 0; i < zarcnt; ++i) {
		z = &zonedb[zarray[i]];
		pzstat[z->pnr-1].nzofp++;
	}

	for (i = 0; i < MAXPZ; ++i) {
		pzi = &pzinfo[i];
		if (pzstat[i].nzofp == 0) {
			pzbmp_clr(i);
			if (pzi->zarr) {
				free(pzi->zarr);
				pzi->zarr = NULL;
				pzi->zcnt = 0;
			}
		} else {
			if (pzstat[i].nzofp != pzi->zcnt) {
				pzi->zarr = realloc(pzi->zarr, pzstat[i].nzofp*sizeof(int));
				if (pzi->zarr != NULL) {
					pzi->zcnt = pzstat[i].nzofp;
				} else {
					DBG("--- realloc failed for pzinfo and exit ---\n");
					exit(1);
				}
			}
			memset(pzi->zarr, 0, sizeof(int)*pzi->zcnt);
			pzstat[i].nzofp = 0;
			pzbmp_set(i);
		}
	}

	for (i = 0; i < zarcnt; ++i) {
		z = &zonedb[zarray[i]];
		pzi = &pzinfo[z->pnr-1];
		z->pidx = pzi->pzst->nzofp++;
		pzi->zarr[z->pidx] = zarray[i];
		if (z->ztp == 2) {
			pzi->pzst->n24hr++;
		}
	}

	for (i = 0; i < MAXPZ; ++i) {
		if (pzbmp_isset(i)) {
			pzstat[i].nunknown = pzstat[i].nzofp - pzstat[i].n24hr;
		}
	}
}

#if 0
#ifdef DEBUG
static char *pzst[] = {
	"Unknown",
	"Disarmed",
	"Partial Bypassed",
	"Bypassed",
	"Not Ready",
	"Partial Armed",
	"Armed",
	"Forced Armed",
	"Timer Armed",
	"Alarmed",
	"Alarm restored",
};

static void
dump_pzstate(int p)
{
	printf("--- pzone-%d: has %d zones, st = %d:%s ---\n", p+1, nzofp[p], pstat[p], pzst[(int)pstat[p]]);
	printf("--- n24hr[%d] = %d ---\n", p, n24hr[p]);
	printf("--- nunknown[%d] = %d ---\n", p, nunknown[p]);
	printf("--- narm[%d]     = %d ---\n", p, narm[p]);
	printf("--- ndisarm[%d]  = %d ---\n", p, ndisarm[p]);
	printf("--- bpcnt[%d]    = %d ---\n", p, bpcnt[p]);
	printf("--- nready[%d]   = %d ---\n", p, nready[p]);
	printf("--- nalert[%d]   = %d ---\n", p, nalert[p]);
	printf("--- nalertr[%d]  = %d ---\n", p, nalertr[p]);
	printf("--- total_nready = %d ---\n", total_nready);
}
#endif
#endif

static void
pzinfo_calc(int p)
{
	pzst_t  *ps = &pzstat[p];

	if (ps->nalert > 0) {
		pstat[p] = PST_ALARM;
		pzaltmap_set(p);
		pzdm_clr(p);
	} else if (ps->nalertr > 0) {
		pstat[p] = PST_ALARMR;
		pzaltmap_set(p);
		pzdm_clr(p);
	} else if (ps->nready > 0) {
		pstat[p] = PST_NRDY;
		pzaltmap_clr(p);
		pzarm_clr(p);
		pzdm_set(p);
	} else if (ps->narm > 0) {
		if (tmrpzmap & (1<<p)) {
			pstat[p] = PST_TMRARM;
		} else if (frcpzmap & (1<<p)) {
			pstat[p] = PST_FRCARM;
		} else if (ps->narm == ps->nzofp) {
			pstat[p] = PST_ARM;
		} else {
			pstat[p] = PST_PTARM;
		}
		pzaltmap_clr(p);
		pzarm_set(p);
		pzdm_clr(p);
	} else if (ps->bpcnt > 0) {
		if (ps->bpcnt == ps->nzofp) {
			pstat[p] = PST_BYPASS;
		} else {
			pstat[p] = PST_PTBPASS;
		}
		pzaltmap_clr(p);
		pzarm_clr(p);
		pzdm_set(p);
	} else if (ps->ndisarm + ps->n24hr > 0) {//modified on 2013.11.03
		pstat[p] = PST_DISARM;
		pzaltmap_clr(p);
		pzarm_clr(p);
		pzdm_set(p);
	} else {
		pstat[p] = PST_UNKNOWN;
		pzaltmap_clr(p);
		pzarm_clr(p);
		pzdm_set(p);
	}

	if (ps->ndisarm + ps->nunknown + ps->nready > 0) {
		pzdm_set(p);
	} else {
		pzdm_clr(p);
	}

#if 0
#ifdef DEBUG
	dump_pzstate(p);
#endif
#endif
}

/* p: 0~MAXPZ-1 */
void
update_pzone_state(int p)
{
	pzinfo_calc(p);
	update_pzoneui_state(p, pstat[p]);
	update_tmrpz_ui(p, pstat[p]);
#if ENABLE_SCRSVR
	if (_syst == 0&&pzaltmap == 0&&nready_yet() == 0) {
		enable_scrsvr();
	}
	DBG("--- enable_scrsvr ---\n");
#endif
}

/* DONE */
static void
unknown_out(int z)
{
	int  p = pnrofz(z);
	if (zone_type(z) != 2) {
		pzstat[p].nunknown--;
	}
}

/* DONE */
static void
unknown_in(int z)
{
	return;
}

static void
disarm_out(int z)
{
	int  p = pnrofz(z);
	if (zone_type(z) != 2) {
		pzstat[p].ndisarm--;
	}
}

static void
disarm_in(int z)
{
	int  p = pnrofz(z);
	int  i = zoneidx(z);

	if (zone_type(z) != 2) {
		++pzstat[p].ndisarm;
	}
	zstat[i] = ZST_DISARM;
	frcpzmap &= ~(1 << p);
	update_zoneui_state(z, ZST_DISARM);
}

/* DONE */
static void
nready_out(int z)
{
	int  p = pnrofz(z);

	--pzstat[p].nready;
	--total_nready;
	nready_del(z);
	if (total_nready <= 0) {
		update_pzst_label(0);
		total_nready = 0;
	}

	if (zone_type(z) != 2) {
		send_ainfo(ZFLR, z + 1);
	}
}

/* DONE */
static void
nready_in(int z)
{
	int  p = pnrofz(z);
	int  i = zoneidx(z);

	++pzstat[p].nready;
	++total_nready;
	zstat[i] = ZST_NRDY;
	nready_put(z);
	frcpzmap &= ~(1 << p);
	update_zoneui_state(z, ZST_NRDY);
	if (total_nready == 1) {
		update_pzst_label(1);
	}

	send_ainfo(ZFLT, z + 1);
#if ENABLE_SCRSVR
	disable_scrsvr();
#endif
}

/* DONE */
static void
bypass_out(int z)
{
	int  p = pnrofz(z);
	--pzstat[p].bpcnt;
}

/* DONE */
static void
bypass_in(int z)
{
	int  p = pnrofz(z);	
	int  i = zoneidx(z);

	++pzstat[p].bpcnt;
	zstat[i] = ZST_BYPASS;
	update_zoneui_state(z, ZST_BYPASS);
}

/* called when from arm to disarm */
static void
arm_out(int z)
{
	int  p = pnrofz(z);
	int  i = zoneidx(z);

	if (zonedb[z].ztp != 2) {
		--pzstat[p].narm;
	}

	if (zstat[i] == ZST_ALARM)
		--pzstat[p].nalert;
	else if (zstat[i] == ZST_ALARMR)
		--pzstat[p].nalertr;
	frcpzmap &= ~(1 << p);
}

/* can be called by pzone operation. */
static void
arm_in(int z)
{
	int  p = pnrofz(z);
	int  i = zoneidx(z);

	if (zonedb[z].ztp != 2) {
		++pzstat[p].narm;
		zstat[i] = ZST_ARM;
		update_zoneui_state(z, ZST_ARM);
	}
}

/* DONE */
static void
alarm_out(int z)
{
	int  p = pnrofz(z);
	--pzstat[p].nalert;
	alarmst_del(z);
}

/* DONE */
static void
alarm_in(int z)
{
	int  p = pnrofz(z);
	int  i = zoneidx(z);

	++pzstat[p].nalert;
	zstat[i] = ZST_ALARM;
#if ENABLE_SCRSVR
	disable_scrsvr();
#endif
	alarmst_put(z, 1);
	update_zoneui_state(z, ZST_ALARM);
	update_pzone_state(p);
	update_zmapst(z, 2);
	send_alarm(ZALT, z + 1);
	relay_on();
}

/* DONE */
static void
alarmr_out(int z)
{
	int  p = pnrofz(z);
	--pzstat[p].nalertr;
	alarmst_del(z);
}

/* DONE */
static void
alarmr_in(int z)
{
	int  p = pnrofz(z);	
	int  i = zoneidx(z);

	++pzstat[p].nalertr;
	zstat[i] = ZST_ALARMR;
#if ENABLE_SCRSVR
	disable_scrsvr();
#endif
	alarmst_put(z, 0);
	update_zoneui_state(z, ZST_ALARMR);
	update_pzone_state(p);
	send_alarm(ZALR, z + 1);
	relay_on();
}

FVINT st_out[] = {
	[ZST_UNKNOWN] = unknown_out,
	[ZST_DISARM]  = disarm_out,
	[ZST_NRDY]    = nready_out,	
	[ZST_BYPASS]  = bypass_out,
	[ZST_ARM]     = arm_out,
	[ZST_ALARM]   = alarm_out,
	[ZST_ALARMR]  = alarmr_out,
};

FVINT st_in[] = {
	[ZST_UNKNOWN] = unknown_in,
	[ZST_DISARM]  = disarm_in,
	[ZST_NRDY]    = nready_in,
	[ZST_BYPASS]  = bypass_in,
	[ZST_ARM]     = arm_in,
	[ZST_ALARM]   = alarm_in,
	[ZST_ALARMR]  = alarmr_in,
};

int
calc_allflag(void)
{
	int  i;
	int  a = 0;
	int  d = 0;

	for (i = 0; i < pzcnt; ++i) {
		if (pstat[pzarr[i]] < PST_ARM) {
			++a;
		}
		if (pstat[pzarr[i]] > PST_DISARM) {
			++d;
		}
	}
	if (a == 0) {
		return 1;
	} else if (d == 0) {
		return 0;
	}
	return 2;
}

static void
syst_calc(void)
{
	int   i;
	int   p;
	int   z;

	for (i = 0; i < zarcnt; ++i) {
		z = zarray[i];
		p = pnrofz(z);
		if (zstat[i] >= ZST_ARM) {
			zstat[i] = ZST_ARM;
			++pzstat[p].narm;
//			zmapst_set(z);
//			DBG("--- zone-%d[pzone-%d]: st = ZST_ARM ---\n", z+1, p+1);
		} else if (zstat[i] == ZST_BYPASS) {
			++pzstat[p].bpcnt;
//			zmapst_clr(z);
//			DBG("--- zone-%d[pzone-%d]: st = ZST_BYPASS ---\n", z+1, p+1);
		} else {
			zstat[i] = ZST_DISARM;
			++pzstat[p].ndisarm;
//			zmapst_clr(z);
//			DBG("--- zone-%d[pzone-%d]: st = ZST_DISARM ---\n", z+1, p+1);
		}
	}

	for (i = 0; i < pzcnt; ++i) {
		p = pzarr[i];
		pzstat[p].nunknown = 0;
		pzstat[p].ndisarm -= pzstat[p].n24hr;
		pzinfo_calc(p);
	}
}

////////////////////////////////////////
/* called when system start. */
void
syst_load(void)
{
	FILE   *fp;
	int     cnt = 0;

	if (zarcnt == 0) {
		unlink(SYSTDB);
		return;
	}

	if ((fp = fopen(SYSTDB, "r")) == NULL) return;
	ignore_result(fread(&cnt, sizeof(int), 1, fp));
	if (cnt > 0) {
		if (cnt != zarcnt) {
			zstate_init(cnt);
		}
		ignore_result(fread(zstat, sizeof(unsigned char), cnt, fp));
		ignore_result(fread(&pztmrflg, sizeof(pztmrflg), 1, fp));
		ignore_result(fread(&frcpzmap, sizeof(frcpzmap), 1, fp));
	}
	fclose(fp);
	zarcnt = cnt;
	DBG("--- syst_load: zones = %d ---\n", zarcnt);
	if (cnt > 0) {
		syst_calc();
	}
}

/* called when system status changed. */
void
syst_save(void)
{
	FILE    *fp;

	if ((fp = fopen(SYSTDB, "w")) != NULL) {
		ignore_result(fwrite(&zarcnt, sizeof(int), 1, fp));
		ignore_result(fwrite(zstat, sizeof(unsigned char), zarcnt, fp));
		ignore_result(fwrite(&pztmrflg, sizeof(pztmrflg), 1, fp));
		ignore_result(fwrite(&frcpzmap, sizeof(frcpzmap), 1, fp));
		fflush(fp);
		fclose(fp);
	}
}

int
pzopt_zones(int p, int (*cb)(int, int))
{
	pzinf_t  *pzi = &pzinfo[p];
	pzst_t   *ps = pzi->pzst;
	unsigned short  z;
	int  i;
	int  cnt = 0;

	for (i = 0; i < ps->nzofp; ++i) {
		z = pzi->zarr[i];
		cnt += (*cb)(p, z);
	}

	return cnt;
}

void
pzopt_zones_st(int p, xbuf_t *x)
{
	pzinf_t  *pzi = &pzinfo[p];
	pzst_t   *ps = pzi->pzst;	
	unsigned short	z;
	int   i;
	int   n = 0;
	
	for (i = 0; i < ps->nzofp; ++i) {
		z = pzi->zarr[i];
		n = sprintf(x->buf + x->n, "|%d:%d", z, zstat[zoneidx(z)]);
		x->n += n;
	}
}

/* DONE */
int
cannot_tmrarm(unsigned short p)
{
	if (pstat[p] >= PST_ALARM) { //alarmed
		DBG("--- pzone-%d alarmed ---\n", p+1);
		return PZ_ALRT;
	}

	if (pzstat[p].n24hr == pzstat[p].nzofp) { //24 hour pzone
		return PZ_24HUR;
	}

	return 0;
}

int
cannot_pzdisarm(int p)
{
	return ((pstat[p] < PST_PTARM)||pztimer_enabled(p));
}

int
cannot_pzbp(int p)
{
	return (pztimer_enabled(p)||(pstat[p] != PST_NRDY&&pstat[p] > PST_PTBPASS));
}

/* DONE */
int
cannot_fpzarm(unsigned short p)
{
	if (pstat[p] >= PST_ALARM) { //alarmed
		DBG("--- pzone-%d alarmed ---\n", p+1);
		return PZ_ALRT;
	}

	if (pztimer_enabled(p)) { //timer pzone
		DBG("--- pzone-%d is timer zone ---\n", p+1);
		return PZ_TMRED;
	}

	if (pzstat[p].n24hr == pzstat[p].nzofp) { //24 hour pzone
		return PZ_24HUR;
	}

	if (pzstat[p].narm > 0&&pzstat[p].narm + pzstat[p].n24hr + pzstat[p].bpcnt == pzstat[p].nzofp) { //armed
		DBG("--- pzone-%d armed ---\n", p+1);
		return PZ_ARMED;
	}

	if (pzstat[p].ndisarm + pzstat[p].nunknown == 0) { //no zone to be armed
		DBG("--- pzone-%d: no zone can be armed ---\n", p+1);
		return PZ_NARM;
	}

	return 0;
}

/* DONE */
/* judge whether the pzone can be armed. */
int
cannot_pzarm(unsigned short p)
{
	int  r;
	
	r = cannot_fpzarm(p);
	if (r > 0) return r;
	
	if (_syst != 0) { //can force arm,system fault
		DBG("--- SYS FAULT ---\n");
		return PZ_SYSF;
	}

	if (pstat[p] == PST_NRDY&&pzstat[p].nready < pzstat[p].nzofp) { //can force arm,not ready
		DBG("--- pzone-%d not ready ---\n", p+1);
		return PZ_NRDY;
	}

	return 0;
}

/* slave only: called by client thread. */
int
cannot_pzbpr(int p)
{
	return (pstat[p] != PST_BYPASS&&pstat[p] != PST_PTBPASS);
}

/* DONE: arg: 0-511  */
int
cannot_zarm(int z)
{
	int  i = zoneidx(z);
	if (z >= MAXZ) {
		return Z_UNKNOWN;
	}

	if (zonedb[z].ztp == 2) { //24 hour zones cannot be armed.
		return Z_24HUR;
	}

	if (pztimer_enabled(pnrofz(z))) { //zones in timer pzone cannot be armed.
		return Z_TMRED;
	}

	if (zstat[i] > ZST_DISARM) { //zones in not-disarmed state cannot be armed.
		return Z_STERR;
	}

	return 0;
}

/* DONE: arg: 0~511 */
int
cannot_zbpass(int z)
{
	int  i = zoneidx(z);
	if (z >= MAXZ) {
		return Z_UNKNOWN;
	}
	
	if (zonedb[z].ztp == 2) { //24 hour zones cannot be bypassed.
		return Z_24HUR;
	}

	if (pztimer_enabled(pnrofz(z))) { //zones in timer pzone cannot be bypassed.
		return Z_TMRED;
	}

	if (zstat[i] > ZST_NRDY) { //armed or alarmed zones cannot be bypassed.
		return Z_STERR;
	}

	return 0;
}

/* slave only: called by client thread. */
int
cannot_zbpassr(int z)
{
	return (zstat[zoneidx(z)] != ZST_BYPASS);
}

/* 单独防区撤防不需要自动旁路恢复被旁路分区. */
int
cannot_zdisarm(int z)
{
	int  i = zoneidx(z);
	if (z >= MAXZ) {
		DBG("--- zone-%d is unknown ---\n", z + 1);
		return Z_UNKNOWN;
	}

	if (zonedb[z].ztp == 2&&zstat[i] < ZST_ALARM) {
		return Z_24HUR;
	}

	if (pztimer_enabled(pnrofz(z))) {
		DBG("--- zone-%d belongs to timerzone. ---\n", z + 1);
		return Z_TMRED;
	}

	if (zstat[i] < ZST_ARM) {
		DBG("--- zone-%d not armed. ---\n", z + 1);
		return Z_STERR;
	}

	return 0;
}

int
try_pzdisarm(int p)
{
	if (p == 0) return 0;
	return cannot_pzdisarm(p-1);
}

static int
is_allarmed(void)
{
	int  i;

	for (i = 0; i < pzcnt; ++i) {
		if (pstat[pzarr[i]] < PST_ARM&&pstat[pzarr[i]] != PST_BYPASS) {
			return 0;
		}
	}
	return 1;
}

static int
_cannot_allarm(void)
{
	int  r = 0;

	if (pzaltmap > 0) {
		r = PZ_ALRT;
	} else if (is_allarmed()) {
		r = ALLARMED;
	} else if (nready_yet()||_syst > 0) {
		r = PZ_SYSF;
	}

	return r;
}

/* DONE */
/*
 * when pstat == PST_PTBPASS or PST_PTARM, it is judged by master.
 */
static int
_cannot_fpzarm(unsigned short p)
{
	if (pstat[p] >= PST_ALARM) { //alarmed
		DBG("--- pzone-%d alarmed ---\n", p+1);
		return PZ_ALRT;
	}

	if (pztimer_enabled(p)) { //timer pzone
		DBG("--- pzone-%d is timer zone ---\n", p+1);
		return PZ_TMRED;
	}

	if (pzstat[p].n24hr == pzstat[p].nzofp) { //24 hour pzone
		return PZ_24HUR;
	}

	if (pstat[p] >= PST_ARM) { //armed
		DBG("--- pzone-%d armed ---\n", p+1);
		return PZ_ARMED;
	}

	if (pstat[p] == PST_BYPASS) { //no zone to be armed
		DBG("--- pzone-%d: no zone can be armed ---\n", p+1);
		return PZ_NARM;
	}

	return 0;
}

static int
_cannot_fallarm(void)
{
	int   i;
	int   k = 0;

	for (i = 0; i < pzcnt; ++i) {
		if (_cannot_fpzarm(pzarr[i]) > 0) {
			++k;
		}
	}

	return (k == pzcnt);
}

int
try_fpzarm(int p)
{
	if (p == 0) {
		return _cannot_fallarm();
	} else {
		return _cannot_fpzarm(p-1);
	}
}

/* DONE */
/* judge whether the pzone can be armed. */
static int
_cannot_pzarm(unsigned short p)
{
	int  r;
	
	r = _cannot_fpzarm(p);
	if (r > 0) return r;
	
	if (_syst != 0) { //can force arm,system fault
		DBG("--- SYS FAULT ---\n");
		return PZ_SYSF;
	}

	if (pstat[p] == PST_NRDY) { //can force arm,not ready
		DBG("--- pzone-%d not ready ---\n", p+1);
		return PZ_NRDY;
	}

	return 0;
}

int
try_pzarm(int arg)
{
	if (arg == 0)
		return _cannot_allarm();
	else
		return _cannot_pzarm(arg-1);
}

/* DONE */
int
disarmz(int p, int z)
{
	int  i = zoneidx(z);

	if (zstat[i] >= ZST_ARM) {
		(*st_out[ZST_ARM])(z);
		if (zalert_isset(z)) {
			(*st_in[ZST_NRDY])(z);
		} else {
			(*st_in[ZST_DISARM])(z);
		}
		finish_zidelay(z);
		zodelay_clr(z);
		return 1;
	} else if (zstat[i] == ZST_BYPASS) {
		if (bparmflag_isset(p)) {
			(*st_out[ZST_BYPASS])(z);
			if (zalert_isset(z)) {
				(*st_in[ZST_NRDY])(z);
			} else {
				(*st_in[ZST_DISARM])(z);
			}
			return 1;
		}
	}
	return 0;
}


/* DONE */
int
farmz(int p, int z)
{
	int  i = zoneidx(z);

	if (zonedb[z].ztp == 2) return 0;
	if (zstat[i] == ZST_NRDY) {
		(*st_out[ZST_NRDY])(z);
		(*st_in[ZST_BYPASS])(z);
		bparmflag_set(p);
		return 1;
	}
	
	switch (zonedb[z].ztp) {
	case 0:
		if (zstat[i] <= ZST_DISARM) {
			(*st_out[(int)zstat[i]])(z);
			(*st_in[ZST_ARM])(z);
			DBG("--- zone-%d in pzone-%d armed ---\n", z, p+1);
			return 1;
		}
		break;
	case 1:
		if (zstat[i] <= ZST_DISARM) {
			(*st_out[(int)zstat[i]])(z);
			(*st_in[ZST_ARM])(z);
			DBG("--- zone-%d in pzone-%d armed ---\n", z, p+1);
			if (odelay > 0) {
				start_zodelay(z);
			}
			zidelay_clr(z);
			return 1;
		}
	default:
		break;
	}
	return 0;
}

///////////////////////////////////////////////////////////////////

#define if_allarmed()  ((pzdm == 0)&&(pzarm > 0))

/* DONE */
static int
armz(int p, int z)
{
	int  i = zoneidx(z);
	switch (zone_type(z)) {
	case 0: //instant zone
		if (zstat[i] <= ZST_DISARM) {//ignore bypassed zones
			(*st_out[(int)zstat[i]])(z);
			(*st_in[ZST_ARM])(z);
			DBG("--- zone-%d in pzone-%d armed ---\n", z, p+1);
			return 1;
		}
		break;
	case 1: //delayed zone
		if (zstat[i] <= ZST_DISARM) {
			(*st_out[(int)zstat[i]])(z);
			(*st_in[ZST_ARM])(z);
			if (odelay > 0) {
				start_zodelay(z);
			}
			zidelay_clr(z);
			return 1;
		}
	default: //24 hour zone
		break;
	}
	return 0;
}

/* DONE */
static int
_pz_arm(unsigned short arg)
{
	int r = 0;

	DBG("--- pzone-%d arm opt ---\n", arg + 1);
	if (pstat[arg] != PST_BYPASS) {
		pzopt_zones(arg, armz);
		frcpzmap &= ~(1 << arg);
		update_pzone_state(arg);
		r = 1;
	}

	return r;
}

////////////////////////////////////////////////////////////////////////

/* DONE */
static int
_all_arm(int id)
{
	int  i;
	int  r = 0;
	int  err = 0;
	int  p;
	unsigned short pzerrmap = 0;

	DBG("--- pzdm = 0x%08x, pzarm = 0x%08x ---\n", pzdm, pzarm);
	if (pzaltmap > 0) {
		r = PZ_ALRT;
	} else if (if_allarmed()) {
		r = ALLARMED;
	} else if (nready_yet() > 0||_syst > 0) {
		r = PZ_SYSF;
	}

	if (r) {
		if (id > 0)
			netxfer_result(OPZAM, 0, r, id);
		else
			allopr_return(1, r);
		return r;
	}

	for (i = 0; i < pzcnt; ++i) {
		p = pzarr[i];
		if (pzstat[p].bpcnt > 0) {
			bparmflag_set(p);
		}

		err = cannot_pzarm(p);
		if (err == 0) {
			r += _pz_arm(p);
		} else {
			pzerrmap |= (1 << (err - 1));
		}
	}

	if (r > 0) {
		send_ainfo(AARM+MAXZCMD, 0);
		update_zmapst(MAXZ, 1); //update zone map.
		netxfer_allarm(1, 0, id);
		if (id == 0) {
			DBG("--- local all arm success ---\n");
			allopr_return(1, 0);
		}
		return 0;
	} else if (pzerrmap > 0) {
		err = __builtin_ctz(pzerrmap)+1;
		if (id > 0)
			netxfer_result(OPZAM, 0, err, id);
		else
			allopr_return(1, err);
	}
	return 1;
}

//////////////////////////////////////////////////////////

/* slave only: called by client thread. */
void
pzarm_ack(int arg, int r)
{
	if (arg == 0) {
		allopr_return(1, r);
	} else {
		pzopr_return(0, arg - 1, r);
	}
}

/* DONE: arg: 0~MAXPZ
 * 0:  all
 * [1, MAXPZ]: pzone
 */
int
pzarm_opt(unsigned int var)
{
	unsigned short arg = var & ARG_MASK;
	int  id = var >> DEVID_SFT;  //slave id
	int  r;

	if (arg == 0) {
		return _all_arm(id);
	}
	--arg;

	if (pzstat[arg].bpcnt > 0) {
		bparmflag_set(arg);
	}

	r = cannot_pzarm(arg);
	if (r) {
		if (id > 0) {
			netxfer_result(OPZAM, arg + 1, r, id);
		} else
			pzopr_return(0, arg, r);
		return 1;
	}
	_pz_arm(arg);
	send_ainfo(PARM + MAXZCMD, arg + 1);
	update_zmapst(-1, (arg << 1)|1); //update zone map.
	netxfer_pzopt(arg, 1, 0);

	return 0;
}

////////////////////////////////////////////////////////

/* DONE */
static int
pz_farm(unsigned short arg)
{
	DBG("--- pzone-%d force arm opt ---\n", arg + 1);
	if (pstat[arg] != PST_BYPASS) {
		if (pzopt_zones(arg, farmz) > 0) {
			frcpzmap |= (1 << arg);
			update_pzone_state(arg);
			return 1;
		}
	}

	return 0;
}

/* DONE */
static int
_all_farm(int id)
{
	int  i;
	int  k = 0;
	int  r = 0;
	unsigned short p;

	for (i = 0; i < pzcnt; ++i) {
		p = pzarr[i];
		if (pzstat[p].bpcnt > 0) {
			bparmflag_set(p);
		}
		r = cannot_fpzarm(p);
		if (r > 0) {
			++k;
		}
	}

	if (k == pzcnt) {
		DBG("--- no pzone can be armed ---\n");
		if (id > 0)
			netxfer_result(OPZFA, 0, 1, id);
		else
			allopr_return(2, 1);
		return 1;
	}

	DBG("--- ALL forced arm opt ---\n");
	k = 0;
	for (i = 0; i < pzcnt; ++i) {
		p = pzarr[i];
		if (!pztimer_enabled(p)) { //ignore the tiimer pzone.
			if (pz_farm(p) > 0) {
				++k;
			}
		}
	}

	if (k > 0) {
		send_ainfo(FARM + MAXZCMD, 0);
		update_zmapst(MAXZ, 1);
		netxfer_allarm(1, 1, id);
		if (id == 0) {
			DBG("--- local all forced arm success ---\n");
			allopr_return(2, 0);
		}
		return 0;
	} else {
		if (id > 0)
			netxfer_result(OPZFA, 0, 1, id);
		else
			allopr_return(2, 1);
	}
	return 1;
}

//////////////////////////////////////////////////////////
/* slave only: called by client thread. */
void
pzfarm_ack(int arg, int r)
{
	if (arg == 0) {
		allopr_return(2, r);
	} else {
		pzopr_return(1, arg - 1, r);
	}
}

/* DONE: arg: 0~MAXPZ
 * 0:  all
 */
int
pzarm_fopt(unsigned int var)
{
	unsigned short arg = var & ARG_MASK;
	int  id = var >> DEVID_SFT; //slave id
	int  r;

	if (arg == 0) {
		return _all_farm(id);
	}

	--arg;
	if (pzstat[arg].bpcnt > 0) {
		bparmflag_set(arg);
	}
	r = cannot_fpzarm(arg);
	if (r) {
		if (id > 0) {
			netxfer_result(OPZFA, arg+1, r, id);
		} else {
			pzopr_return(1, arg, r);
		}
		return 1;
	}

	if (!pztimer_enabled(pzarr[arg])) { //timer pzone cannot be force armed.
		if (pz_farm(arg) > 0) {
			send_ainfo(PFAM+MAXZCMD, arg + 1);
			update_zmapst(-1, (arg << 1)|1);  //update zone map.
			netxfer_pzopt(arg, 1, 2);
			return 0;
		} else {
			if (id > 0) {
				netxfer_result(OPZFA, arg+1, PZ_NARM, id);
			} else {
				pzopr_return(1, arg, PZ_NARM);
			}
		}
	} else {
		if (id > 0) {
			netxfer_result(OPZFA, arg+1, PZ_TMRED, id);
		} else
			pzopr_return(1, arg, PZ_TMRED);
	}

	return 1;
}

//////////////////////////////////////////////////

static int   bpassz_r(int p, int z);

/* DONE: arg: [0, MAXPZ) */
static int
pzbpr_auto(unsigned short arg)
{
	DBG("--- pzone-%d bypass restore auto ---\n", arg + 1);
	pzopt_zones(arg, bpassz_r);
	bparmflag_clr(arg);
	update_pzone_state(arg);
	return 0;
}

/* DONE: [0, MAXPZ)  */
/* when 24hour zone being triggered,narm still be zero, so cannot disarm, it's a bug. */
static int
_pz_disarm(unsigned short arg)
{
	if (pstat[arg] < PST_PTARM) {//bug? 
		if (pstat[arg] == PST_BYPASS||pstat[arg] == PST_PTBPASS) {
			if (bparmflag_isset(arg)) {
				DBG("--- pzone-%d: auto bprestore. ---\n", arg+1);
				pzbpr_auto(arg);
			}
		}
		return 15;
	} else if (pztimer_enabled(arg)) {
		DBG("--- pzone-%d is timer zone ---\n", arg+1);
		return 1;
	}

	DBG("--- pzone-%d disarm opt ---\n", arg + 1);
	alarmst_clearpz(arg);
	pzopt_zones(arg, disarmz);
	bparmflag_clr(arg);
	frcpzmap &= ~(1 << arg);
	update_pzone_state(arg);

	return 15;
}

/* DONE */
static int
_all_disarm(int id)
{
	int  i;
	int  r = 0;

	for (i = 0; i < pzcnt; ++i) {
		r += _pz_disarm(pzarr[i]);
	}

	if (r >= 15) {
		send_ainfo(ADAM + MAXZCMD, 0);
		update_zmapst(MAXZ, 0);
		netxfer_allarm(0, 0, id);
		if (id == 0) {
			DBG("--- local all disarm ---\n");
			allopr_return(0, 0);
		}
		return 0;
	} else  if (r == 0) {
		if (id > 0)
			netxfer_result(OPZDA, 0, 1, id);
		else
			allopr_return(0, 1);
	} else {
		if (id > 0)
			netxfer_result(OPZDA, 0, 2, id);
		else
			allopr_return(0, 2);
	}
	return 1;
}

/* slave only: called by client thread. */
void
pzdisarm_ack(int arg, int r)
{
	if (arg == 0) {
		allopr_return(0, r);
	} else {
		pzopr_return(2, arg - 1, r);
	}
}
/////////////////////////////////////////////////////

/* DONE: arg: 0~MAXPZ
 * 0:  all
 * [1, MAXPZ]: pzone
 */
int
pzdisarm_opt(unsigned int var)
{
	unsigned short arg = var & ARG_MASK;
	int  id = var >> DEVID_SFT; //slave id
	int  r;

	if (arg == 0) {
		return _all_disarm(id);
	}

	--arg;
	r = _pz_disarm(arg);
	if (r == 15) {
		DBG("--- before send_ainfo ---\n");
		send_ainfo(PDAM+MAXZCMD, arg + 1);
		DBG("--- before update_zmapst ---\n");
		update_zmapst(-1, (arg << 1));
		DBG("--- after update_zmapst ---\n");
		netxfer_pzopt(arg, 0, 0);
		return 0;
	} else if (r == 0) {
		if (id > 0)
			netxfer_result(OPZDA, arg+1, 1, id);
		else
			pzopr_return(2, arg, 1);
	} else {
		if (id > 0)
			netxfer_result(OPZDA, arg+1, 2, id);
		else
			pzopr_return(2, arg, 2);
	}

	return 1;
}



/* DONE: z: [0, MAXZ) */
static int
bpassz(int p, int z)
{
	int  i = zoneidx(z);

	if (zonedb[z].ztp == 2) return 0; //24 hour zone cannot be bypassed.
	switch (zstat[i]) {
	case ZST_DISARM:
	case ZST_NRDY:
	case ZST_UNKNOWN:
		(*st_out[(int)zstat[i]])(z);
		(*st_in[ZST_BYPASS])(z);
		return 1;
	default:
		break;
	}
	return 0;
}

/* DONE: arg: [0, MAXPZ) */
/*
  * 考虑含有24小时防区的分区，其中24小时防区报警后撤防，
  * 此时转为未准备状态。那么此时分区是未准备状态，
  * 但不能旁路此未准备分区。
  */
int
pzbp_opt(unsigned int var)
{
	unsigned short arg = var & ARG_MASK;
	int  id = var >> DEVID_SFT; //slave id

	if ((pstat[arg] == PST_NRDY||pstat[arg] <= PST_PTBPASS)&&!pztimer_enabled(arg)) {
		if (pzopt_zones(arg, bpassz) > 0) {
			bparmflag_clr(arg);
			frcpzmap &= ~(1 << arg);
			update_pzone_state(arg);
			send_ainfo(PBPA + MAXZCMD, arg + 1);
			netxfer_pzopt(arg, 3, 0);
			return 0;
		}
	} else {
		if (id > 0)
			netxfer_result(OPZBP, arg, 1, id);
		else
			pzopr_return(3, arg, 1);
	}

	return 1;
}

////////////////////////////////////////////////////

void
pzbp_ack(int arg, int r)
{
	pzopr_return(3, arg, r);
}

///////////////////////////////////////////////////////

/* DONE: z: [0, MAXZ) */
static int
bpassz_r(int p, int z)
{
	int  i = zoneidx(z);

	if (zstat[i] == ZST_BYPASS) {
		(*st_out[ZST_BYPASS])(z);
		if (zalert_isset(z)) {
			(*st_in[ZST_NRDY])(z);
		} else {
			(*st_in[ZST_DISARM])(z);
		}
		return 1;
	}
	return 0;
}

/* DONE: arg: [0, MAXPZ) */
int
pzbpr_opt(unsigned int var)
{
	unsigned short arg = var & ARG_MASK;
	int  id = var >> DEVID_SFT; //slave id

	if (pstat[arg] == PST_BYPASS||pstat[arg] == PST_PTBPASS) {
		DBG("--- pzone-%d bypass restore opt ---\n", arg + 1);
		pzopt_zones(arg, bpassz_r);
		bparmflag_clr(arg);
		update_pzone_state(arg);
		send_ainfo(PBPR + MAXZCMD, arg + 1);
		netxfer_pzopt(arg, 2, 0);
		return 0;
	} else {
		if (id > 0)
			netxfer_result(OPZBPR, arg, 1, id);
		else
			pzopr_return(4, arg, 1);
	}

	return 1;
}

/* slave only: called by client thread. */
void
pzbpr_ack(int arg, int r)
{
	pzopr_return(4, arg, r);
}

/////////////////////////////////////////////////////

/* DONE: arg: 0-511  */
/* Don't have to call update_zoneui_state() because this is called by ui thread. */
int
zdisarm_opt(unsigned int var)
{
	int  r = 0;
	int  id = var >> DEVID_SFT; //slave id.
	unsigned short arg = var & ARG_MASK;

	r = cannot_zdisarm(arg);
	if (r != 0) {
		if (id > 0) {
			netxfer_result(ODAM, arg, r, id);
		} else {
			zopr_return(1, arg, r);
		}
		return 1;
	}

	(*st_out[ZST_ARM])(arg);
	alarmst_del(arg);
	if (zalert_isset(arg)) {
		(*st_in[ZST_NRDY])(arg);
	} else {
		(*st_in[ZST_DISARM])(arg);
	}

	update_pzone_state(pnrofz(arg));
	update_zmapst((int)arg, 0);
	send_ainfo(ZDAM, arg + 1);

	finish_zidelay(arg);
	zodelay_clr(arg);

	netxfer_zopt(arg, ZOP_DA);

	return 0;
}

/* slave only: called by net tcp client thread. */
void
zdisarm_ack(int arg, int r)
{
	zopr_return(1, arg, r);
}

/* DONE: arg: 0-511  */
int
zarm_opt(unsigned int var)
{
	unsigned short arg = var & ARG_MASK;
	int  id = var >> DEVID_SFT; //slave id
	int  r;
	int  z = zoneidx(arg);

	r = cannot_zarm(arg);
	if (r) {
		if (id > 0) {
			netxfer_result(OARM, arg, r, id);
		} else {
			zopr_return(0, arg, r);
		}
		return 1;
	}

	(*st_out[(int)zstat[z]])(arg);
	(*st_in[ZST_ARM])(arg);
	update_pzone_state(pnrofz(arg));
	update_zmapst((int)arg, 1);
	send_ainfo(ZARM, arg+1);
	
	if (zone_type(arg) == 1) {
		if (odelay > 0) {
			start_zodelay(arg);
		}
		zidelay_clr(arg);
	}
	netxfer_zopt(arg, ZOP_AM);

	return 0;
}


/* slave only: called by net tcp client thread. */
void
zarm_ack(int arg, int r)
{
	zopr_return(0, arg, r);
}

/* DONE: arg: 0~511 */
int
zbp_opt(unsigned int var)
{
	unsigned short arg = var & ARG_MASK;
	int  id = var >> DEVID_SFT; //slave id
	int  r;
	int  z = zoneidx(arg);

	r = cannot_zbpass(arg);
	if (r) {
		if (id > 0) {
			netxfer_result(OZBP, arg, r, id);
		} else {
			zopr_return(2, arg, r);
		}
		return 1;
	}

	(*st_out[(int)zstat[z]])(arg);
	(*st_in[ZST_BYPASS])(arg);
	update_pzone_state(pnrofz(arg));
	send_ainfo(ZBPA, arg + 1);
	netxfer_zopt(arg, ZOP_BP);

	return 0;
}

/* slave only: called by net tcp client thread. */
void
zbp_ack(int arg, int r)
{
	zopr_return(2, arg, r);
}

/* DONE: arg: 0~511 */
int
zbpr_opt(unsigned int  var)
{
	unsigned short arg = var & ARG_MASK;
	int  id = var >> DEVID_SFT;  //slave id
	int  z = zoneidx(arg);
	int  st = (int)zstat[z];

	bpassz_r(pnrofz(arg), arg);
	if (st == ZST_BYPASS) {
		update_pzone_state(pnrofz(arg));
		send_ainfo(ZBPR, arg + 1);
		netxfer_zopt(arg, ZOP_BPR);
		return 0;
	} else {
		if (id > 0) {
			netxfer_result(OZBPR, arg, 1, id);
		} else {
			zopr_return(3, arg, 1);
		}
	}

	return 1;
}


/* slave only: called by net tcp client thread. */
void
zbpr_ack(int arg, int r)
{
	zopr_return(3, arg, 1);
}

/* DONE: arg: 0-511 */
int
za_opt(unsigned int var)
{
	unsigned short arg = var & ARG_MASK;
	int  r = 1;
	int  p;
	int  z = zoneidx(arg);

	if (zonedb[arg].znr != arg+1) return -1;
	if (zstat[z] == ZST_BYPASS) return -1;

	zalert_set(arg);

	if (zonedb[arg].ztp == 2) {
		if (zstat[z] <= ZST_DISARM||zstat[z] == ZST_ALARMR) {
			(*st_out[(int)zstat[z]])(arg);
			(*st_in[ZST_ALARM])(arg);
			netxfer_zopt(arg, ZOP_ALM);
		}
		return 0;
	}

	p = pnrofz(arg);
	switch (zstat[z]) {
	case ZST_ARM:
		if (zone_type(arg) == 1) {
			if (zodelay_isset(arg)) {
				DBG("--- received alert: zone-%d[pzone-%d]: in exit delay mode, not alarm ---\n", arg + 1, p+1);
				return 1;
			}

			if (idelay > 0) {
				start_zidelay(arg);
				return 1;
			} else {
				DBG("--- received alert: zone-%d[pzone-%d]: from arm to alarm ---\n", arg + 1, p+1);
				(*st_in[ZST_ALARM])(arg);
				netxfer_zopt(arg, ZOP_ALM);
				return 0;
			}
		} else {
			(*st_in[ZST_ALARM])(arg);
			netxfer_zopt(arg, ZOP_ALM);
		}
		break;
	case ZST_ALARMR: //???
		DBG("--- received alert: zone-%d[pzone-%d]: from alarm restore to alarm ---\n", arg + 1, p+1);
		(*st_out[ZST_ALARMR])(arg);
		(*st_in[ZST_ALARM])(arg);
		netxfer_zopt(arg, ZOP_ALM);
		r = 0;
		break;
	case ZST_UNKNOWN:
	case ZST_DISARM:
		DBG("--- received alert: zone-%d[pzone-%d]: not ready ---\n", arg + 1, p+1);
		(*st_out[(int)zstat[arg]])(arg);
		(*st_in[ZST_NRDY])(arg);
		update_pzone_state(pnrofz(arg));
		netxfer_zopt(arg, ZOP_FLT);
		r = 0;
		break;
	default:
		DBG("--- received alert: zone-%d[pzone-%d]: unknown status ---\n", arg + 1, p+1);
		break;
	}

	return r;
}

/* DONE: arg: 0-511 */
int
zar_opt(unsigned int var)
{
	unsigned short arg = var & ARG_MASK;
	int  z = zoneidx(arg);

	if (zonedb[arg].znr != arg+1) return -1;
	zalert_clr(arg);

	switch (zstat[z]) {
	case ZST_UNKNOWN:
		DBG("--- received alarm restore: zone-%d[pzone-%d]: from unknown to disarm ---\n", arg + 1, pnrofz(arg)+1);
		(*st_out[ZST_UNKNOWN])(arg);
		(*st_in[ZST_DISARM])(arg);
		update_pzone_state(pnrofz(arg));
		break;
	case ZST_NRDY:
		DBG("--- received alarm restore: zone-%d[pzone-%d]: from notready/unknown to disarm ---\n", arg + 1, pnrofz(arg)+1);
		(*st_out[ZST_NRDY])(arg);
		if (zonedb[arg].ztp != 2) {
			(*st_in[ZST_DISARM])(arg);
			update_pzone_state(pnrofz(arg));
			netxfer_zopt(arg, ZOP_FLR); //zone failure restore.
		} else {
			(*st_in[ZST_ALARMR])(arg);
			update_zmapst(arg, 2); //new add on 2013.12.14
			netxfer_zopt(arg, ZOP_ALR);
		}
		break;
	case ZST_ALARM:
		DBG("--- received alarm restore: zone-%d[pzone-%d]: from alarm to alarm restore ---\n", arg + 1, pnrofz(arg)+1);
		(*st_out[ZST_ALARM])(arg);
		(*st_in[ZST_ALARMR])(arg);
		netxfer_zopt(arg, ZOP_ALR);
		break;
	case ZST_ARM:
		DBG("--- received alarm restore: zone-%d[pzone-%d]: in enter delay mode ---\n", arg + 1, pnrofz(arg)+1);
		if (zidelay_isset(arg)) {
			zarflag_set(arg);//for enter delay
		}
		break;
	default:
		DBG("--- received alarm restore: zone-%d[pzone-%d]: unknown status ---\n", arg + 1, pnrofz(arg)+1);
		break;
	}

	return 0;
}

int
syst_return(unsigned int p)
{
	if (pzaltmap > 0) return 1;
	if (pzarm > 0) return 2;
	return 0;
}

static int
nrdymap_chk(int p, int z)
{
	if (zstat[zoneidx(z)] == ZST_NRDY) {
		DBG("--- zone-%d in pzone-%d not ready ---\n", z, p);
		nready_put(z);
	}
	return 0;
}

static void
set_nrdymap(void)
{
	int  i;

	for (i = 0; i < pzcnt; ++i) {
		if (pstat[pzarr[i]] == PST_NRDY) {
			DBG("--- not ready zone-%d ---\n", pzarr[i]+1);
			pzopt_zones(pzarr[i], nrdymap_chk);
		}
	}
}

static int
zfault_chk(int p, int z)
{
	int  i = zoneidx(z);
	if (zstat[i] == ZST_NRDY) {
		DBG("--- zone-%d in pzone-%d not ready ---\n", z, p);
		nready_put(z);
	} else if (zstat[i] == ZST_ALARM) {
		alarmst_put(z, 1);
	} else if (zstat[i] == ZST_ALARMR) {
		alarmst_put(z, 0);
	}
	return 0;
}

/* called by net_tcp.c when zstat synchronized. */
void
calc_zonef(void)
{
	int  i;
	int  p;

	nready_clear();
	alarmst_clear();
	pzaltmap = 0;
	for (i = 0; i < pzcnt; ++i) {
		p = pzarr[i];
		pzopt_zones(p, zfault_chk);
		if (pstat[p] >= PST_ALARM) {
			pzaltmap_set(p);
		}
	}
}

void
handle_allopt(int op, int d)
{
	pzaltmap = 0;
	DBG("--- all arm/disarm opt: op = %d, d = %d ---\n", op, d);
	switch (op) {
	case 0: //all disarm.
		pzarm = 0;
		alarmst_clear();
		nready_clear();
		send_ainfo(ADAM + MAXZCMD, 0);
		update_zmapst(MAXZ, 0);
		if (d)
			allopr_return(0, 0);
		set_nrdymap();
		alarmst_clear();
		break;
	case 1: //all arm.
		send_ainfo(AARM + MAXZCMD, 0);
		update_zmapst(MAXZ, 1);
		if (d)
			allopr_return(1, 0);
		nready_clear();
		break;
	case 2: //all forced arm.
		send_ainfo(FARM + MAXZCMD, 0);
		update_zmapst(MAXZ, 1);
		if (d)
			allopr_return(2, 0);
		nready_clear();
		break;
	default:
		return;
	}

	update_pzck_pstat(0);

	if (_syst == 0&&pzaltmap == 0&&nready_yet() == 0) {
		enable_scrsvr();
	} else {
		disable_scrsvr();
	}
}

/* op: 0-disarm,1-arm,2-bypass restore,3-bypass */
void
pzone_opr(int p, int op, int t)
{
	DBG("--- pzone opt: p = %d, op = %d, t = %d ---\n", p, op, t);
	update_pzoneui_state(p, pstat[p]);
	update_tmrpz_ui(p, pstat[p]);
	sync_zone_ui();
	switch (op) {
	case 0:
		if (t == 0)
			send_ainfo(PDAM + MAXZCMD, p + 1);
		else
			send_ainfo(PTDM + MAXZCMD, p + 1);
		update_zmapst(-1, (p << 1));
		alarmst_clearpz(p);
		break;
	case 1:
		if (t == 0) {
			send_ainfo(PARM + MAXZCMD, p + 1);
		} else if (t == 1) {
			send_ainfo(PTAM + MAXZCMD, p + 1);
		} else if (t == 2) {
			send_ainfo(PFAM + MAXZCMD, p + 1);
		}
		update_zmapst(-1, (p << 1)|1);
		break;
	case 2:
		send_ainfo(PBPR + MAXZCMD, p + 1);
		break;
	case 3:
		send_ainfo(PBPA + MAXZCMD, p + 1);
		break;
	default:
		return;
	}

	if (pstat[p] >= PST_ALARM) { //used for display alarm state in main page.
		pzaltmap_set(p);
	} else {
		pzaltmap_clr(p);
	}

	update_pzck_pstat(p+1);

	if (_syst == 0&&pzaltmap == 0&&nready_yet() == 0) {
		enable_scrsvr();
	} else {
		disable_scrsvr();
	}

	
}

/*
 * op: 0-disarm,1-arm,2-bypassr,3-bypass,4-alarm,
 * 5-alarm restore,6-fault,7-fault restore.
*/
void
zone_opr(int op, int z, int zst, int p, int pst)
{
	zstat[zoneidx(z)] = (char)zst;
	update_zoneui_state(z, zst); //will update the pzck UI.
	if (pstat[p] != (char)pst) {
		pstat[p] = (char)pst;
		update_pzoneui_state(p, pst);
		update_tmrpz_ui(p, pst);
	}

	if (zst == ZST_NRDY) //used for display not ready zones in main page.
		nready_put(z);
	else if (zst == ZST_ALARM)
		alarmst_put(z, 1);
	else if (zst == ZST_ALARMR)
		alarmst_put(z, 0);
	else
		znode_remove(z);

	switch (op) {
	case 0:
		update_zmapst((int)z, 0);
		send_ainfo(ZDAM, z + 1);
		break;
	case 1:
		update_zmapst((int)z, 1);
		send_ainfo(ZARM, z + 1);
		break;
	case 2:
		send_ainfo(ZBPR, z + 1);
		break;
	case 3:
		send_ainfo(ZBPA, z + 1);
		break;
	case 4:
		update_zmapst(z, 2); //update zone map.
		send_alarm(ZALT, z + 1);
		relay_on();
		break;
	case 5:
		send_alarm(ZALR, z + 1);
		relay_on();
		break;
	default:
		break;
	}

	if (pst >= PST_ALARM) {//used for display alarm state in main page.
		pzaltmap_set(p);
	} else {
		pzaltmap_clr(p);
	}

	if (_syst == 0&&pzaltmap == 0&&nready_yet() == 0) {
		enable_scrsvr();
	} else {
		disable_scrsvr();
	}
}



