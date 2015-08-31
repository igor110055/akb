#include <arpa/inet.h>
#include <string.h>
#include "ulib.h"
#include "unotifier.h"
#include "utk.h"
#include "utksignal.h"
#include "utkwindow.h"
#include "utkbutton.h"
#include "utklabel.h"
#include "utkmap.h"
#include "utkentry.h"
#include "layout.h"
#include "stb.h"
#include "gui.h"
#include "config.h"
#include "zopt.h"
#include "qtimer.h"
#include "pztimer.h"

#define  PSZ             4
#define  TSZ             6

#ifndef  SECS_PER_DAY
#define  SECS_PER_DAY   (24*3600)
#endif

/* timer changed event notifier */
static struct u_notifier_head  te_ntf_head;

////////////////////////////////////////////
static UtkWindow  *twin;
static UtkWindow  *cbx[12];
static UtkWindow  *wex;
static UtkWindow  *pztip;
static UtkWindow  *tmap[4];
static tip_t       terr;
static tip_t       pinv;

static kbar_ent_t  tke;
static myent_t     pzent[MAXTPZ];
static myent_t     ptent[4];

////////////////////////////////////////////
static pztm_t   g_pztm;  //load from conf file, and not changed until the configuration being changed and saved. respond to configuration file.
static pztm_t   pztm;    //copy from g_pztm. changed when user changed the configuration before saving.respond to UI.

/* hh:mm, timeslice string */
static char  g_pt[4][TSZ] = {{0}};
static char  pt[4][TSZ]   = {{0}};
static int   ptval[4] = {-1};
int   g_tflag = 0;

/* part zone number string */
static char  g_pz[MAXTPZ][PSZ] = {{0}};
static char  pz[MAXTPZ][PSZ]   = {{0}};

/* used for sort buffer */
static unsigned char  pzarray[MAXPZ] = {0};     //pnr
static unsigned char  ptcbx[MAXPZ] = {0};  //ptmflag
static int            pztcnt = 0;

/////////////////////////////////////////////////
static char  pztxt[32] = {0};
static char *nopz = "请先在系统编程中配置有效分区!";

////////////////////////////////////////
static void
clear_pzstr(void)
{
	int  i;

	for (i = 0; i < MAXTPZ; ++i) {
		memset(g_pz[i], 0, PSZ);
		memset(pz[i], 0, PSZ);
	}
}

static inline void
clear_pzarr(void)
{
	CLEARA(pzarray);
	CLEARA(ptcbx);
}
///////////////////////////////////////////

////////////////////////////////////////////////////
/* DONE: called when loading or storing pzone info in zcfg.c */
/* ui: 0-this window not created yet, 1-this window has created. */
void
prepare_pztip(int *arr, int cnt)
{
	int   i;
	int   len;
	char *ptr;

	CLEARA(pztxt);
	if (cnt == 0) {
		strncpy(pztxt, nopz, sizeof(pztxt));
	} else {
		ptr = pztxt;
		len = sprintf(ptr, "有效分区:");
		ptr += len;
		for (i = 0; i < cnt-1; ++i) {
			len = sprintf(ptr, "%d,", arr[i]+1);
			ptr += len;
		}
		sprintf(ptr, "%d", arr[cnt-1]+1);
	}
}

void
pzent_enable(int v)
{
	int  i;

	if (v) {
		for (i = 0; i < ARR_SZ(pzent); ++i) {
			myent_enable(&pzent[i]);
		}
	} else {
		for (i = 0; i < ARR_SZ(pzent); ++i) {
			myent_disable(&pzent[i]);
		}
	}
}

//////////////////////////////////////////////
/* DONE, called only when loading the configuration. */
static void
set_tmrui(int i)
{
	int  j = i << 1;

	snprintf(g_pt[j], TSZ, "%02d:%02d", pztm.tm[j].hur, pztm.tm[j].min);
	memcpy(pt[j], g_pt[j], TSZ);

	snprintf(g_pt[j+1], TSZ, "%02d:%02d", pztm.tm[j+1].hur, pztm.tm[j+1].min);
	memcpy(pt[j+1], g_pt[j+1], TSZ);

	tmslice[j]   = (pztm.tm[j].hur * 60   + pztm.tm[j].min) * 60;
	tmslice[j+1] = (pztm.tm[j+1].hur * 60 + pztm.tm[j+1].min) * 60;
	if (tmslice[j] > tmslice[j+1])
		tmslice[j+1] += SECS_PER_DAY;
	
	ptval[j]   = 1;
	ptval[j+1] = 1;
}

/* DONE, called only when loading the configuration. */
static void
clear_tmrui(int i)
{
	int  j = i << 1;

	memset(g_pt[j],   0, TSZ);
	memset(pt[j],     0, TSZ);
	memset(g_pt[j+1], 0, TSZ);
	memset(pt[j+1],   0, TSZ);
	memset(&pztm.tm[j], 0xff, 2*sizeof(tclk_t));

	tmslice[j]   = 0;
	tmslice[j+1] = 0;
	ptval[j]   = 0;
	ptval[j+1] = 0;
}

/* called when input invalid time slice value. */
static void
restore_tmrui(int i)
{
	int j = i << 1;
	
	memcpy(&pztm.tm[j], &g_pztm.tm[j], 2*sizeof(tclk_t));
	if ((pztm.tm[j].hur < 24)&&(pztm.tm[j].min < 60)
	  &&(pztm.tm[j+1].hur < 24)&&(pztm.tm[j+1].min < 60)) {
	  	set_tmrui(i);
	} else {
		clear_tmrui(i);
	}
}

///////////////////////////////////////////////
/* DONE: pzarray[]->pzmt.pz[]->pzent[]  */
static void
get_pzarr(int t)
{
	int  i;

	pztcnt = 0;
	for (i = 0; i < MAXPZ&&pztcnt < MAXTPZ; ++i) {
		if (pzarray[i] == i+1) {
			snprintf(g_pz[pztcnt], PSZ, "%d", i+1);
			memcpy(pz[pztcnt], g_pz[pztcnt], PSZ);
			pztm.pz[pztcnt] = pzarray[i];
			if ((t & 1) == 0) {
				ptcbx[i] &= 0xfe; //~1
			}

			if ((t & 2) == 0) {
				ptcbx[i] &= 0xfd; //~2
			}
			pztm.pt[pztcnt] = ptcbx[i];
			++pztcnt;
		}
	}

	pztm.npzt = pztcnt;

	for (i = pztcnt; i < MAXTPZ; ++i) {
		memset(g_pz[i], 0, PSZ);
		memset(pz[i], 0, PSZ);
		pztm.pz[i] = 0;
		pztm.pt[i] = 0;
	}
}

/* DONE */
static void
put_pzarr(void)
{
	int  i;
	unsigned char nr;

	clear_pzarr();
	for (i = 0; i < ARR_SZ(pzent); ++i) {
		nr = pztm.pz[i];
		if ((nr > 0)&&(nr <= MAXPZ)&&(pztm.pt[i] > 0)) {
			pzarray[nr-1] = nr;
			ptcbx[nr-1] = pztm.pt[i];
		}
	}
}

/* DONE */
static void
clear_pztm(void)
{
	CLEARV(pztm);
	CLEARV(g_pztm);
	memset(&pztm.tm[0], 0xff, 4*sizeof(tclk_t));
	memset(&g_pztm.tm[0], 0xff, 4*sizeof(tclk_t));
}

/* should be called after load_pzone() being called. */
void
load_pztmr(int ud)
{
	FILE *fp;
	int   t0 = 0;
	int   t1 = 0;

	u_notifier_head_init(&te_ntf_head);
	if (pzcnt == 0) {// no valid pzone.
		clear_pztm();
		remove(PZTMRDB);
		tmrui_clr_content(ud);
		return;
	}

	if ((fp = fopen(PZTMRDB, "r")) == NULL) {
		clear_pztm();
		tmrui_clr_content(ud);
		return;
	}

	if (fread(&g_pztm, sizeof(pztm_t), 1, fp) == 1) {
		memcpy(&pztm, &g_pztm, sizeof(pztm_t));
		if ((pztm.tm[0].hur < 24)&&(pztm.tm[0].min < 60)
		  &&(pztm.tm[1].hur < 24)&&(pztm.tm[1].min < 60)) {
			set_tmrui(0);
			t0 = 1;
		} else {
			clear_tmrui(0);
			t0 = 0;
		}

		if ((pztm.tm[2].hur < 24)&&(pztm.tm[2].min < 60)
		  &&(pztm.tm[3].hur < 24)&&(pztm.tm[3].min < 60)) {
			set_tmrui(1);
			t1 = 1;
		} else {
			clear_tmrui(1);
			t1 = 0;
		}
		
		if (t0 == 0&&t1 == 0) { // no timeslice being configured.
			fclose(fp);
			remove(PZTMRDB);
			clear_pztm();
			tmrui_clr_content(ud);
			return;
		}
		put_pzarr();
		g_tflag = t0|(t1<<1);
		get_pzarr(g_tflag);

		/* sorted and sync to source. */
		memcpy(&g_pztm, &pztm, sizeof(pztm_t));
		tmrui_set_content(&g_pztm, ud); //defined in tmr.c
	}

	fclose(fp);
}

/////////////////////////////////////////////////
/*    DONE: return whether overlay or being contained.   */
static int
tmslice_invalid(unsigned int s0, unsigned int e0, unsigned int s1, unsigned int e1)
{
	return ((s0 <= e1&&s0 >= s1)||(e0 <= e1&&e0 >= s1)||(s1 <= e0&&s1 >= s0)||(e1 <= e0&&e1 >= s0));
}

/* DONE */
/* 0: deleted, 1: normal, -1: error. */
static int
prepare_save_timeslice(int i, unsigned int *s, unsigned int *e)
{
	int j = i<<1;

	if (ptval[j] == 1&&ptval[j+1] == 1) {
		DBG("--- time slice%d: %d:%d--%d:%d ---\n", i, pztm.tm[j].hur, pztm.tm[j].min, pztm.tm[j+1].hur, pztm.tm[j+1].min);
		if (pztm.tm[j].hur == pztm.tm[j+1].hur&&pztm.tm[j].min == pztm.tm[j+1].min) {
			restore_tmrui(i); //timeslice invalid
			myent_update(&ptent[j]);
			myent_update(&ptent[j+1]);
			return -1;
		}
		*s = pztm.tm[j].hur * 3600 + pztm.tm[j].min*60;
		*e = pztm.tm[j+1].hur * 3600 + pztm.tm[j+1].min*60;
		if (*s > *e) {
			*e += SECS_PER_DAY;
		}
		return 1;
	} else if (ptval[j] == 0&&ptval[j+1] == 0) {//delete the timeslice.
		DBG("--- delete the timeslice-%d ---\n", i);
		clear_tmrui(i);
		myent_update(&ptent[j]);
		myent_update(&ptent[j+1]);
		return 0;
	}
	restore_tmrui(i); //timeslice invalid
	myent_update(&ptent[j]);
	myent_update(&ptent[j+1]);
	return -1;
}

/* DONE */
static int
prepare_save_tmr(void)
{
	unsigned int  s0 = 0;
	unsigned int  e0 = 0;
	unsigned int  s1 = 0;
	unsigned int  e1 = 0;
	int  t0;
	int  t1;

	t0 = prepare_save_timeslice(0, &s0, &e0);
	if (t0 < 0) {
		show_tip(&terr);
		return -1;
	}

	t1 = prepare_save_timeslice(1, &s1, &e1);
	if (t1 < 0) {
		show_tip(&terr);
		return -1;
	}

	if (t0 == 0&&t1 == 0) { //both timeslice1 and timeslice2 are cleared.
		DBG("--- both time slice0 and time slice1 are deleted. ---\n");
		return 0;
	}

	if (t0 == 1&&t1 == 1) {
		if (tmslice_invalid(s0, e0, s1, e1)) {
			DBG("--- time slice overlayed ---\n");
			show_tip(&terr);
			clear_tmrui(0);
			clear_tmrui(1);
			myent_update(&ptent[0]);
			myent_update(&ptent[1]);
			myent_update(&ptent[2]);
			myent_update(&ptent[3]);
			return -1;
		}
		if (s0 > s1) {
			unsigned char ch;
			U_SWAP(s0,s1);
			U_SWAP(e0,e1);
			ch = pztm.tm[0].hur;
			pztm.tm[0].hur = pztm.tm[2].hur;
			pztm.tm[2].hur = ch;
			ch = pztm.tm[0].min;
			pztm.tm[0].min = pztm.tm[2].min;
			pztm.tm[2].min = ch;

			ch = pztm.tm[1].hur;
			pztm.tm[1].hur = pztm.tm[3].hur;
			pztm.tm[3].hur = ch;
			ch = pztm.tm[1].min;
			pztm.tm[1].min = pztm.tm[3].min;
			pztm.tm[3].min = ch;
		}
	}

	if (t0) {
		tmslice[0] = s0;
		tmslice[1] = e0;
	} else {
		tmslice[0] = 0;
		tmslice[1] = 0;
	}

	if (t1) {
		tmslice[2] = s1;
		tmslice[3] = e1;
	} else {
		tmslice[2] = 0;
		tmslice[3] = 0;
	}
	
	return (t0|(t1<<1));
}

///////////////////////////////////////////////////
static inline void
disable_cbx(int i)
{
	utk_widget_update_state(cbx[i], 0);
	utk_widget_disable(cbx[i]);
	DBG("--- disable cbx[%d] ---\n", i);
}

static inline void
enable_cbx(int i, int st)
{
	utk_widget_update_state(cbx[i], st);
	utk_widget_enable(cbx[i]);
	DBG("--- enable cbx[%d] ---\n", i);
}

/* called when save configuration or cancel the configuring.*/
static void
update_pzui(int pz, int t)
{
	int  i;
	int  j;

	for (i = 0; i < ARR_SZ(pzent); ++i) {
		j = i << 1;
		if (pztm.pz[i] == 0||pztm.pz[i] > MAXPZ) {
			disable_cbx(j);
			disable_cbx(j+1);
		} else {
			if (t & 1) {
				enable_cbx(j, pztm.pt[i]&1);
			} else {
				disable_cbx(j);
			}

			if ((t>>1) & 1) {
				enable_cbx(j+1, (pztm.pt[i]>>1)&1);
			} else {
				disable_cbx(j+1);
			}
		}
		if (pz) {
			myent_update(&pzent[i]);
		}
	}
}

void
reload_pztmr(void)
{
	load_pztmr(1);
	update_pzui(1, g_tflag);
	myent_update(&ptent[0]);
	myent_update(&ptent[1]);
	myent_update(&ptent[2]);
	myent_update(&ptent[3]);
	utk_widget_update_state(wex, pztm.we);
	u_notifier_call_chain(&te_ntf_head, 0, NULL);
}

static void
clear_pzui(void)
{
	int  i;
	int  j;

	for (i = 0; i < ARR_SZ(pzent); ++i) {
		j = i << 1;
		disable_cbx(j);
		disable_cbx(j+1);
		myent_update(&pzent[i]);
	}
}

static void
tmrcfg_cancel(void *data)
{
	if (memcmp(&pztm, &g_pztm, sizeof(pztm_t))) {
		memcpy(&pztm, &g_pztm, sizeof(pztm_t));
		kbar_entry_cancel(&tke);
		update_pzui(0, g_tflag);
		if (utk_widget_state(wex) != pztm.we) {
			utk_widget_update_state(wex, pztm.we);
		}
	} else {
		kbar_entry_cancel(&tke);
	}
}

/* DONE */
static void
tmrcfg_save(void *data)
{
	int   t;
	int   i;
	FILE *fp;

	if (kbar_entry_save_prepared(&tke) == 0) return;
	if (!is_master()) {
		tmrcfg_cancel(NULL);
		return;
	}

	if (memcmp(&g_pztm, &pztm, sizeof(pztm_t)) == 0) {
		DBG("--- timer zone configuration not changed ---\n");
		return;
	}

	init_pztimer(0);
	if ((t = prepare_save_tmr()) < 0) {
		init_pztimer(1);
		return;
	}
	if (t == 0) { //should remove the tiimer.
		g_tflag = 0;
		clear_pztm();
		clear_pzstr();
		clear_pzarr();
		clear_pzui();
		fi_clear(PzTmDbId);
		return;
	}

	g_tflag = t;
	pztm.we = utk_widget_state(wex);

	/* sort pztm.pz[] using pzarray[] */
	put_pzarr();
	get_pzarr(t);

	if (memcmp(&g_pztm, &pztm, sizeof(pztm_t)) == 0) {
		DBG("--- timer zone configuration not changed ---\n");
		init_pztimer(1);
		return;
	}

	disable_syncfg();
	memcpy(&g_pztm, &pztm, sizeof(pztm_t));
	u_notifier_call_chain(&te_ntf_head, 0, NULL);

	for (i = 0; i < 4; ++i) {
		memcpy(g_pt[i], pt[i], TSZ);
	}

	update_pzui(1, t);
	tmrui_set_content(&g_pztm, 1);  //defined in tmr.c

	/* think about the pzones in timer list which are removed now. */
	if ((fp = fopen(PZTMRDB, "w")) != NULL) {
		if (fwrite(&g_pztm, sizeof(pztm_t), 1, fp) != 1) {
			DBG("--- fwrite error ---\n");
		}
		fflush(fp);
		fclose(fp);
		fi_update(PzTmDbId);
	}
	init_pztimer(1);
}

static PVOID  st_cb[] = {show_sysnet_win, show_sysuart_win, show_sysgen_win, show_sysalr_win};

static void
sysf_tnav(void *data)
{
	st_cb[(int)data]();
}

static void
clear_tpzent(void *arg)
{
	int  i = (int)arg;

	myent_clear(&pzent[i]);
	utk_widget_update_state(cbx[i<<1], 0);
	utk_widget_update_state(cbx[(i<<1)+1], 0);
	utk_widget_disable(cbx[i<<1]);
	utk_widget_disable(cbx[(i<<1)+1]);
}

////////////////////////////////////////
/* DONE: pzent[]--->pztm.pz[]  */
static void
end_inpz(void *arg)
{
	int   i = (int)arg;
	int   pnr;
	char *ptr;

	if (myent_text_length(&pzent[i]) > 0) {
		ptr = pz[i];
		pnr = strtol(ptr, NULL, 10);
		if (pzone_exist(pnr)) {
			utk_widget_enable(cbx[i<<1]);
			utk_widget_enable(cbx[(i<<1)+1]);
			pztm.pz[i] = pnr;
		} else {
			pztm.pz[i] = 0;
			pztm.pt[i] = 0;
			tip_set_hide_cb(&pinv, clear_tpzent, arg);
			show_tip(&pinv);
		}
	} else {
		utk_widget_update_state(cbx[i<<1], 0);
		utk_widget_update_state(cbx[(i<<1)+1], 0);
		utk_widget_disable(cbx[i<<1]);
		utk_widget_disable(cbx[(i<<1)+1]);
		pztm.pz[i] = 0;
		pztm.pt[i] = 0;
	}
}

/* DONE */
static void
build_pz_entries(void *parent)
{
	int i;
	int y[] = {pzent_y0, pzent_y1, pzent_y2, pzent_y3, pzent_y4, pzent_y5};
	rect_t re = {.x = pzent_x, .y = 0, .w = pzent_w, .h = pzent_h};
	
	for (i=0; i<ARR_SZ(pzent); ++i) {
		re.y = y[i];
		myent_new(&pzent[i], parent, &re, 0);
		myent_add_caret(&pzent[i], RGB_BLACK);
		myent_set_cache(&pzent[i], pz[i], PSZ, PSZ-2);
		myent_set_buffer(&pzent[i], g_pz[i], PSZ);
		myent_set_callback(&pzent[i], NULL, end_inpz, NULL, (void *)i);
		myent_add_kbar(&pzent[i], &tke);
		myent_set_ncf(&pzent[i]); //when canceled, not to call fin callback:end_inpz().
		if (pzcnt == 0) {
			myent_disable(&pzent[i]);
		}
	}
}

//////////////////////////////////////////
/* DONE: ptent[]->pztm.tm[] */
static void
end_inpt(void *arg)
{
	int    i = (int)arg; //0-3
	char  *p = NULL;
	char  *ptr;
	unsigned char  h = 0xff;
	unsigned char  m = 0xff;

	DBG("--- judging whether the timevalue is valid. ---\n");
	if (myent_text_length(&ptent[i]) > 0) {
		ptr = pt[i];
		h = (unsigned char)strtol(ptr, &p, 10);
		if (h == 0&&p == ptr) {
			ptval[i] = -1; //time invalid input by user.
			show_tip(&terr);
			return;
		}
		ptr = ++p;
		m = (unsigned char)strtol(ptr, &p, 10);
		if (m == 0&&p == ptr) {
			ptval[i] = -1; //time invalid input by user.
			show_tip(&terr);
			return;
		}
		if (h >= 24||m >= 60) {
			ptval[i] = -1; //time invalid input by user.
			show_tip(&terr);
			return;
		} else {
			ptval[i] = 1;  //time valid.
		}
	} else {
		ptval[i] = 0;          //time deleted by user.
	}

	pztm.tm[i].hur = h;
	pztm.tm[i].min = m;
	DBG("--- time-%d: %d:%d ---\n", i, h, m);
}

/* DONE */
static void
build_pt_entries(void *parent)
{
	int i;
	int x[] = {ptent_x0, ptent_x1};
	int y[] = {ptent_y0, ptent_y1};
	rect_t re = {.w = ptent_w, .h = ptent_h};
	
	for (i=0; i<ARR_SZ(ptent); ++i) {
		re.x = x[i&1];
		re.y = y[i>>1];
		myent_new(&ptent[i], parent, &re, 0);
		myent_add_caret(&ptent[i], RGB_BLACK);
		myent_set_cache(&ptent[i], pt[i], TSZ, TSZ-1);
		myent_set_buffer(&ptent[i], g_pt[i], TSZ);
		myent_set_callback(&ptent[i], NULL, end_inpt, NULL, (void *)i);
		myent_add_kbar(&ptent[i], &tke);
		myent_set_ncf(&ptent[i]);
	}
}

////////////////////////////////////////////
/* DONE: cbx[]-->pztm.pt[][] */
static void
select_timeslice(void *data)
{
	int i = (int)data;
	int st;

	st = utk_widget_state(cbx[i]);
	if (st) {
		pztm.pt[i>>1] |= (unsigned char)(1<<(i&1));
	} else {
		pztm.pt[i>>1] &= (unsigned char)~(1<<(i&1));
	}
}

static void
weekend_mode(void *data)
{
	pztm.we = utk_widget_state(wex);
}

static void
grab_focus(int o)
{
	int  i;
	if (o == 0) {
		for (i = 0; i < 4; ++i) {
			utk_widget_enable(tmap[i]);
		}
	} else {
		for (i = 0; i < 4; ++i) {
			utk_widget_disable(tmap[i]);
		}
	}
}

/* DONE */
void
build_systmr_gui(void)
{
	int   i, j;
	int   x[] = {systmr_cbox_x0, systmr_cbox_x1};
	int   y[] = {systmr_cbox_y0, systmr_cbox_y1, systmr_cbox_y2, systmr_cbox_y3, systmr_cbox_y4, systmr_cbox_y5};
	int  bw[] = {sys_w0, sys_w1, sys_w2, sys_w4};
	int  bx[] = {sys_x0, sys_x1, sys_x2, sys_x4};
	rect_t  r = {.x = bn_gret_x, .y = bn_sys_y, .w = gret_w, .h = bn_sys_h};
	rect_t  r0 = {.x = 644, .y = (70-16)/2+438, .w = 260, .h = 16};

	twin = utk_window_derive();
#ifndef MAPDLL
	utk_widget_resident(twin);
#endif
	utk_window_set_bg(twin, BG_SYSTMR);
	utk_widget_hide_before(twin, tmrcfg_cancel, NULL);

	register_qtimer((void*)twin);

	stb_new(twin, sysf_quit);
	build_map_array1(tmap, twin, ARR_SZ(bx), bw, bx, sys_y, sys_h, sysf_tnav);
	build_tip_lbl(&terr, twin, (LCD_W-tip_w)/2, (LCD_H-tip_h)/2, LBL_TINVLD, TIP_SPLASH_TIME);
	build_tip_lbl(&pinv, twin, (LCD_W-tip_w)/2, (LCD_H-tip_h)/2, LBL_PINVLD, TIP_SPLASH_TIME);

	build_entry_kbar(&tke, twin, systmr_kbar_y, 1);
	kbar_set_show(&tke, grab_focus);
	build_pz_entries(twin);
	build_pt_entries(twin);

	DBG("--- tmr entry_kbar = %p ---\n", &tke);

	////////////////////////////////////////////////////
	for (i = 0; i < 12; ++i) {
		j = i >> 1;
		cbx[i] = build_checkbox(twin, x[i&1], y[j], select_timeslice, (void *)i);
		if (j >= pztm.npzt) {
			utk_widget_disable(cbx[i]);
		} else {
			utk_widget_set_state(cbx[i], (pztm.pt[j]>>(i&1))&1);
		}
	}

	wex = build_checkbox(twin, 1023-266, 395, weekend_mode, 0);
	utk_widget_set_state(wex, pztm.we);

	pztip = label_new(twin, NULL, pztxt, RGB_WHITE, &r0);
	utk_label_set_text_pos((UtkLabel*)pztip, 1, 1);

	//////////////////////////////////////////////////////
	button_new(twin, BN_RET1501, &r, go_home, (void*)1);
	build_savecancel_buttons((void *)twin, tmrcfg_save, tmrcfg_cancel);
}

/* DONE */
void
show_systmr_win(void)
{
	if (utk_window_top() != twin) {
		utk_window_show(twin);
	}
}

pztm_t*
get_pztm_conf(void)
{
	return &pztm;
}

void
register_time_change_notifier(void *node)
{
	u_notifier_chain_register(&te_ntf_head, node);
}



