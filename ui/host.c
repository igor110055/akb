#include "ulib.h"
#include "utk.h"
#include "utksignal.h"
#include "utkwindow.h"
#include "utkbutton.h"
#include "utklabel.h"
#include "utkmap.h"
#include "config.h"
#include "layout.h"
#include "hashtbl.h"
#include "gui.h"
#include "stb.h"
#include "zopt.h"
#include "recver.h"

#define xstoff   ((40-24)/2)
#define ystoff   ((42-29)/2)
#define yseoff   ((42-24)/2)

typedef struct _hip {
	int   id;
	int   idx;
	int   ol;
	unsigned int m;
	char  ip[16];
} hip_t;

static UtkWindow    *hostw = NULL;
static UtkWindow    *selbl[MAXI] = {NULL};
static UtkWindow    *stlbl[MAXI] = {NULL};
static UtkWindow    *iplbl[MAXI] = {NULL};

static myent_t       ident[MAXI];
static char          hidstr[MAXI][4];
static char         _hidstr[MAXI][4];

static kbar_ent_t    ike;

static hip_t         hinf[MAXHOST];
static int           hcnt = 0;
static int           pgoff = 0;

static char   harray[MAXHOST] = {0}; //element:[1,32]

static unsigned int    hmap = 0;
#define hmap_set(i)    hmap |= (1 << (i))
#define hmap_clr(i)    hmap &= ~(1 << (i))
#define hmap_isset(i)  (hmap & (1 << (i)))

static unsigned int    hmapc = 0;
#define hmapc_set(i)   hmapc |= (1 << (i))
#define hmapc_clr(i)   hmapc &= ~(1 << (i))
#define hmapc_isset(i) (hmapc & (1 << (i)))

static int    hchanged = 0;

/* host selected flag */
static unsigned int    selm = 0;
#define selmap_set(i)     selm |= (1 << (i))
#define selmap_clr(i)     selm &= ~(1 << (i))
#define selmap_isset(i)   (selm & (1 << (i)))

static int   g_ht = -1;  //net type and flags of host loaded.
static int   hadd = 0;

static int
ipmap_isset(unsigned int  m)
{
	int  r = 0;
	int  i;
	unsigned int  map = hmap;
	while (map) {
		i = __builtin_ctz(map);
		if (hinf[i].m == m) {
			r = 1;
			break;
		}
		map &= ~(1 << i);
	}
	return r;
}

/* DONE */
static void
build_st_label(void)
{
	int  i;
	int  x[] = {86+xstoff, 370+xstoff, 654+xstoff};
	int  y[] = {227+ystoff, 281+ystoff, 335+ystoff, 389+ystoff, 443+ystoff};

	for (i = 0; i < MAXI; ++i) {
		stlbl[i] = utk_label_new_fp();
		utk_widget_set_size(stlbl[i], 24, 29);
		utk_widget_add_cntl(hostw, stlbl[i],  x[i%3], y[i/3]);
		utk_label_set_bg(stlbl[i], PTARM_SEL);
	}
}

/* DONE */
static void
build_sel_label(void)
{
	int  i;
	int  x[] = {86+xstoff, 370+xstoff, 654+xstoff};
	int  y[] = {227+yseoff, 281+yseoff, 335+yseoff, 389+yseoff, 443+yseoff};

	for (i = 0; i < MAXI; ++i) {
		selbl[i] = utk_label_new_fp();
		utk_widget_set_size(selbl[i], 23, 24);
		utk_widget_add_cntl(hostw, selbl[i],  x[i%3], y[i/3]);
		utk_label_set_bg(selbl[i], ICO_ZSEL0);
	}
}

static void
hashtbl_set_all(int t)
{
	int  i;
	int  id;

	hashtable_set_type(t);
	for (i = 0; i < hcnt; ++i) {
		id = harray[i] - 1;
		DBG("--- hostidx = %d ---\n", id);
		if (hinf[id].id > 0) {
			DBG("--- hostid = %d ---\n", hinf[id].id);
			hashtbl_save(hinf[id].m, hinf[id].id);
		}
	}
}

static void
clear_host_ui(void)
{
	int  i;

	for (i = 0; i < MAXI; ++i) {
		memset(_hidstr[i], 0, 4);
		memset(hidstr[i], 0, 4);
		myent_update(&ident[i]);
		utk_label_set_text((UtkLabel*)iplbl[i], NULL);
		utk_label_update(iplbl[i]);
		utk_widget_update_state(stlbl[i], 0);
		utk_widget_set_state(selbl[i], 0);
	}
}

static void
reload_host_ui(void)
{
	int  i;
	int  j;
	int  max;

	pgoff = 0;
	if (hcnt == 0) return;
	max = (hcnt > MAXI) ? MAXI : hcnt;
	for (i = 0; i < max; ++i) {
		j = (int)harray[i];
		--j;
		memset(_hidstr[i], 0, 4);
		memset(hidstr[i], 0, 4);
		if (hinf[j].id > 0) {
			sprintf(hidstr[i], "%02d", hinf[j].id);
			memcpy(_hidstr[i], hidstr[i], 4);
		}
		myent_update(&ident[i]);
		utk_label_set_text((UtkLabel*)iplbl[i], hinf[j].ip);
		utk_label_update(iplbl[i]);
	}

	for (i = max; i < MAXI; ++i) {
		memset(_hidstr[i], 0, 4);
		memset(hidstr[i], 0, 4);
		myent_update(&ident[i]);
		utk_label_set_text((UtkLabel*)iplbl[i], NULL);
		utk_label_update(iplbl[i]);
	}
}

static void
host_clear(void)
{
	hcnt = 0;
	pgoff = 0;
	hmap = 0;
	hmapc = 0;
	CLEARA(hinf);
	CLEARA(harray);
}

/* DONE
 * format: id = x.x.x.x\n
 */
static void
host_conf(void)
{
	FILE *fp;
	char  buf[160];
	char  ip[128];
	char *addr = NULL;
	char *ptr;
	int   id;
	int   nodef = 0;
	unsigned int  map;
	unsigned int  m;
	int   ht = 0;

	/* the following statement will lead to a problem when device is changed from slave into master. */
	if (!is_master()) return;
	host_clear();
	if ((fp = fopen(HOSTCONF, "r")) == NULL) return;
	/* load the configured host info. */
	while(!feof(fp))
	{
		CLEARA(buf);
		CLEARA(ip);
		if (fgets(buf, sizeof(buf), fp) == NULL) continue;
		if ((ptr = u_trim(buf)) == NULL) continue;		
		addr = strchr(ptr, '=');
		if (addr == NULL) continue;
		*addr = ' ';
		if (sscanf(ptr, "%d %s", &id, ip) != 2) continue;
		DBG("--- id = %d, value = %s ---\n", id, ip);
		if (is_valid_unip(ip)) {
			if (id > 0&&id <= MAXHOST) {
				--id;
				m = get_net_hostp(ip);
				hinf[id].m = m;
				hinf[id].id = id + 1;
				strcpy(hinf[id].ip, ip);
				hmapc_set(id);
				ht += (m >> 8);
			} else if (id == 0) {
				++nodef;
			}
		}
	}

	hmap = hmapc;
	map = hmapc;
	while (map) {
		id = __builtin_ctz(map);
		hinf[id].idx = hcnt + 1;
		harray[hcnt++] = id + 1;
		map &= ~(1 << id);
	}

	DBG("--- total configured alarm host: %d ---\n", hcnt);
	
	/* load the unconfigured host info. */
	map = ~hmap;
	if (nodef > 0&&map > 0) {
		int   k;

		fseek(fp, 0, SEEK_SET);
		while(!feof(fp)&&(map > 0))
		{
			CLEARA(buf);
			CLEARA(ip);
			if (fgets(buf, sizeof(buf), fp) == NULL) continue;
			if ((ptr = u_trim(buf)) == NULL) continue;		
			addr = strchr(ptr, '=');
			if (addr == NULL) continue;
			*addr = ' ';
			if (sscanf(ptr, "%d %s", &id, ip) != 2) continue;
			DBG("--- id = %d, value = %s ---\n", id, ip);
			if (id == 0) {
				k = __builtin_ctz(map);
				m = get_net_hostp(ip);
				hinf[k].m = m;
				hinf[k].id = 0;
				strcpy(hinf[k].ip, ip);
				hinf[k].idx = hcnt + 1;
				harray[hcnt++] = k + 1;
				hmap_set(k);
				map &= ~(1 << k);
			}
		}
	}
	fclose(fp);
	DBG("--- total alarm host: %d ---\n", hcnt);

	hashtbl_set_all(ht);
	g_ht = ht;
}

void
host_load(void)
{
	if (g_ht == -1) {
		host_conf();
		reload_host_ui();
	}
}

void
host_unload(void)
{
	host_clear();
	clear_host_ui();
	g_ht = -1;
}

//////////////////////////////////////////////////////////
static void
clear_lbl(int i, int id)
{
	utk_label_set_text((UtkLabel*)iplbl[i], NULL);
}

static void
set_lbl(int i, int id)
{
	utk_label_set_text((UtkLabel*)iplbl[i], hinf[id-1].ip);
}

static VINT2  do_lbl[] = {clear_lbl, set_lbl};

/* called when page up/down */
/* i: index of iplabel. [0, MAXI) */
static void
_update_iplbl(int i)
{
	int    id;

	id = (int)harray[i + pgoff];
	(*do_lbl[!!id])(i, id);
	utk_label_update(iplbl[i]);
}

/* called when page up/down */
/* i: index of iplabel. [0, MAXI) */
static inline void
_clear_iplbl(int i)
{
	utk_label_set_text((UtkLabel*)iplbl[i], NULL);
	utk_label_update(iplbl[i]);
}

////////////////////////////////////////////////////
static void
clr_hid(hip_t *h, int i)
{
	utk_widget_update_state(stlbl[i], 0);
}

static void
set_hid(hip_t *h, int i)
{
	sprintf(hidstr[i], "%02d", h->id);
	memcpy(_hidstr[i], hidstr[i], 4);
	if (!selmap_isset(h->idx-1)) {
		utk_widget_update_state(stlbl[i], h->ol);
	}
}

static void (*do_hid[])(hip_t *, int) = {clr_hid, set_hid};
/* DONE */
/* i: index of ident. [0, MAXI) */
static inline void
_clear_ident(int i)
{
	memset(hidstr[i], 0, 4);
	memset(_hidstr[i], 0, 4);
	myent_update(&ident[i]);
	utk_widget_update_state(stlbl[i], 0);
}

static void
clr_ident(int i, int id)
{
	_clear_ident(i);
}

static void
set_ident(int i, int id)
{
	hip_t  *h = &hinf[id-1];

	memset(hidstr[i], 0, 4);
	memset(_hidstr[i], 0, 4);
	(*do_hid[!!h->id])(h, i);
	myent_update(&ident[i]);
}

static VINT2  do_ident[] = {clr_ident, set_ident};

/* called when page up/down */
/* i: index of ident. [0, MAXI) */
static void
_update_ident(int i)
{
	int     id;

	id = (int)harray[i + pgoff];
	(*do_ident[!!id])(i, id);
}

////////////////////////////////////////////////////////////////

/* DONE, only called in save_hip() */
static void
update_ident(hip_t *h)
{
	int  idx;

	idx = h->idx - 1 - pgoff;
	if (idx >= 0&&idx < MAXI) {
		memset(hidstr[idx], 0, 4);
		memset(_hidstr[idx], 0, 4);
		sprintf(hidstr[idx], "%02d", h->id);
		memcpy(_hidstr[idx], hidstr[idx], 4);
		utk_widget_update_state(stlbl[idx], 0);
		myent_update(&ident[idx]);
	}
}

/* DONE */
static inline void
update_iplbl(hip_t *h)
{
	int   idx;

	idx = h->idx - 1 - pgoff;
	if (idx >= 0&&idx < MAXI) {
		utk_label_set_text((UtkLabel*)iplbl[idx], h->ip);
		utk_label_update(iplbl[idx]);
	}
}

/* DONE: at this time, pgoff = 0. before being displayed. */
static void
prepare_host_ui(void)
{
	int  i;
	int  j;
	int  max;

	if (hcnt == 0) return;
	max = (hcnt > MAXI) ? MAXI : hcnt;
	for (i = 0; i < max; ++i) {
		j = (int)harray[i];
		--j;
		if (hinf[j].id > 0) {
			sprintf(hidstr[i], "%02d", hinf[j].id);
			memcpy(_hidstr[i], hidstr[i], 4);
		}
		utk_label_set_text((UtkLabel*)iplbl[i], hinf[j].ip);
	}
}

static inline void
unsel_all_hui(void)
{
	int  i;
	for (i = 0; i < MAXI; ++i) {
		utk_widget_update_state(selbl[i], 0);
	}
}

static void
unselect_all_host(void)
{
	int  i;

	if (selm > 0) {
		for (i = 0; i < MAXI; ++i) {
			utk_widget_update_state(selbl[i], 0);
		}
		selm = 0;
	}
}

static int
prepare_hid(void *arg)
{
	int  i = (int)arg;

	if (pgoff + i >= hcnt) { //should prompt the user.
		return 1;
	}

	return 0;
}

static void
fin_hid(void *arg)
{
	int   i = (int)arg;
	int   j;
	int   id;
	int   idx;
	hip_t *hd;
	hip_t *hs;
	char   tmp[16] = {0};
	int    tidx;
	unsigned int  m;

	id = strtol(hidstr[i], NULL, 10);
	DBG("--- input: id = %d ---\n", id);
	if (id > MAXHOST||id == 0) { //should prompt the user.
		DBG("--- alarm host id invalid ---\n");
		return;
	}

	j = pgoff + i;
	idx = (int)harray[j]; //idx > 0 because prepare_hid();
	DBG("--- input: idx = %d ---\n", idx);
	if (id == idx) {
		hd = &hinf[idx-1];
		hd->id = id;
	} else {
		hs = &hinf[idx - 1];
		hd = &hinf[id - 1];
		if (!hmap_isset(id-1)) {
			memset(hd, 0, sizeof(hip_t));
			hd->m = hs->m;
			memcpy(hd->ip, hs->ip, 16);
			memset(hs, 0, sizeof(hip_t));
			hmapc_clr(idx - 1);
			hmap_clr(idx - 1);
		} else {
			tidx = hd->idx;
			memcpy(tmp, hs->ip, 16);
			m = hs->m;
			memcpy(hs->ip, hd->ip, 16);
			hs->m = hd->m;
			memcpy(hd->ip, tmp, 16);
			hd->m = m;

			hs->id = 0;
			hs->idx = tidx;
			hs->ol = 0;
			harray[tidx-1] = idx;
//			hmapc_clr(idx-1);
//			hmap_set(idx-1);			
			hd->ol = 0;
		}
		hd->idx = j + 1;
		hd->id = id;
		harray[j] = id;
	}
	hmapc_set(id-1);
	hmap_set(id-1);
	++hchanged;
}

/* DONE */
static void
build_hid_entries(UtkWindow *parent, int *x, int *y)
{
	int  i;
	rect_t r = {.w = 48, .h = 24};

	for (i = 0; i < ARR_SZ(ident); ++i)
	{
		r.x = x[i%3];
		r.y = y[i/3] + YOFF;
		myent_new(&ident[i], parent, &r, 1);
		myent_set_text_color(&ident[i], RGB_WHITE);
		myent_add_caret(&ident[i], RGB_WHITE);
		myent_set_cache(&ident[i], hidstr[i], 4, 2);
		myent_set_buffer(&ident[i], _hidstr[i], 4);
		myent_add_kbar(&ident[i], &ike);
		myent_set_callback(&ident[i], prepare_hid, fin_hid, NULL, (void*)i);
	}
}

/* DONE */
static void
make_hostip(unsigned int h, char *buf)
{
	struct in_addr  ia;
	memset(buf, 0, 16);
	ia.s_addr = htonl(hnet|h);
	inet_ntop(AF_INET, &ia, buf, 16);
	DBG("--- hostip = %s ---\n", buf);
}

/* DONE */
static void
update_host(UtkWindow *win, int  i, int st)
{
	unsigned int  m = (unsigned int)st;
	unsigned int  map;
	hip_t   *h;
	int      id;

	if (i == 0) { //display undefined host ip.
		if (ipmap_isset(m)) return;
		map = ~hmap;
		if (map == 0) return; //no space for hostinfo.
		id = __builtin_ctz(map);
		h = &hinf[id];
		h->ol = 0;
		h->id = 0;
		h->idx = hcnt + 1;
		h->m = m;
		make_hostip(m, h->ip);
		harray[hcnt++] = id + 1;
		hmap_set(id);
		update_iplbl(h);
	} else if (i > 0&&i <= MAXHOST) {//maybe should check the hmap.
		h = &hinf[i-1];
		h->ol = !!st;
		id = h->idx - 1 - pgoff;
		if (id >= 0&&id < MAXI) {
			utk_widget_update_state(stlbl[id], h->ol);
		}
	}
}

static void 
go_back(void)
{
	show_sysnet_win();
}

static void
prepare_leave(void *arg)
{
	kbar_entry_cancel(&ike);
	unselect_all_host();
	if (hadd == 0) {
		enable_netdev(1);
	}
}

static void
clear_all_host(void)
{
	unlink(HOSTCONF);
	hmapc = 0;
	hmap = 0;
	hchanged = 0;
	pgoff = 0;
	hcnt = 0;
	selm = 0;
	CLEARA(harray);
	CLEARA(hinf);
	hashtbl_clear();
	clear_host_ui();
}

static void
host_save(void)
{
	int    i;
	hip_t *h;
	FILE  *fp;
	int    ht = 0;
	unsigned int  map;

	unselect_all_host();
	if (hchanged == 0) return;
	if ((fp = fopen(HOSTCONF, "w")) == NULL) return;
	hchanged = 0;
	pgoff = 0;
	hcnt = 0;
	CLEARA(harray);
	hashtbl_clear();

	map = hmapc;
	while (map) {
		i = __builtin_ctz(map);
		h = &hinf[i];
		h->idx = hcnt + 1;
		harray[hcnt++] = i + 1;
		fprintf(fp, "%d=%s\n", h->id, h->ip);
		map &= ~(1 << i);
		ht += (h->m >> 8);
	}

	map = hmapc^hmap;
	while (map) {
		i = __builtin_ctz(map);
		h = &hinf[i];
		h->id = 0;
		h->idx = hcnt + 1;
		harray[hcnt++] = i + 1;
		fprintf(fp, "0=%s\n", h->ip);
		map &= ~(1 << i);
	}
	fflush(fp);
	fclose(fp);

	if (hcnt == 0) {
		clear_all_host();
	} else {
		hashtbl_set_all(ht);
		reload_host_ui();
		g_ht = ht;
	}
}

static void
host_add(void)
{
	if (hcnt < MAXHOST) {
		hadd = 1;
		show_hadd_ui();
	}
}

static void
host_del(void)
{
	int  i;
	int  j;
	int  k;
	unsigned int  msk;
	unsigned int  m;
	unsigned int  map;

	if (selm == 0) return;
	msk = ((1 << hcnt) - 1);
	if ((selm & msk) == msk) {
		clear_all_host();
		return;
	}

	map = selm;
	while (map) {
		i = __builtin_clz(selm);
		i = 31 - i;
		j = harray[i];
		m = hinf[j-1].m;
		memset(&hinf[j-1], 0, sizeof(hip_t));
		hmapc_clr(j-1);
		hmap_clr(j-1);	
		for (k = i; k < hcnt-1; ++k) {
			harray[k] = harray[k+1];
		}
		harray[k] = 0;
		--hcnt;
		map &= ~(1 << i);
		++hchanged;
	}
	selm = 0;
	unsel_all_hui();
	host_save();
}

static void
unsel_ui(void)
{
	int  i;
	int  max;
	unsigned int  m;

	max = hcnt - pgoff;
	max = max > MAXI ? MAXI : max;
	max += pgoff;

	m = ((1 << max) - 1) & ~((1 << pgoff) - 1);
	m &= selm;
	while (m) {
		i = __builtin_ctz(m);
		m &= ~(1 << i);
		utk_widget_update_state(selbl[i-pgoff], 0);
	}
}

static void
sel_ui(void)
{
	int  i;
	int  max;
	unsigned int  m;

	max = hcnt - pgoff;
	max = max > MAXI ? MAXI : max;
	max += pgoff;

	m = ((1 << max) - 1) & ~((1 << pgoff) - 1);
	m &= selm;
	while (m) {
		i = __builtin_ctz(m);
		m &= ~(1 << i);
		utk_widget_update_state(stlbl[i-pgoff], 0);
		utk_widget_update_state(selbl[i-pgoff], 1);
	}
}

/* DONE */
static void
prev_page(void)
{
	int  i;

	if (pgoff - MAXI < 0) return;
	unsel_ui();
	pgoff -= MAXI;
	sel_ui();
	for (i = 0; i < MAXI; ++i) {
		_update_iplbl(i);
		_update_ident(i);
	}
}

/* DONE */
static void
next_page(void)
{
	int  i;
	int  max;

	if (pgoff + MAXI >= hcnt) return;
	unsel_ui();
	pgoff += MAXI;
	sel_ui();
	max = hcnt - pgoff;
	max = max > MAXI ? MAXI : max;
	for (i = 0; i < max; ++i) {
		_update_iplbl(i);
		_update_ident(i);
	}

	for (i = max; i < MAXI; ++i) {
		_clear_iplbl(i);
		_clear_ident(i);
	}
}

static PVOID  bn_cb[] = {go_back, host_save, host_add, host_del, prev_page, next_page};

static void
do_host_opt(void *data)
{
	int i = (int)data;
	bn_cb[i]();
}

static void
select_host(void *arg)
{
	int  i = (int)arg;
	int  idx;
	int  j;
	hip_t  *h;

	idx = i + pgoff;
	if (idx >= hcnt) return;
	j = harray[idx];
	h = &hinf[j-1];
	if (selmap_isset(idx)) {
		utk_widget_update_state(selbl[i], 0);
		if (h->ol) {
			utk_widget_update_state(stlbl[i], 1);
		}
		selmap_clr(idx);
	} else {
		if (h->ol) {
			utk_widget_update_state(stlbl[i], 0);
		}
		utk_widget_update_state(selbl[i], 1);
		selmap_set(idx);
	}
}

#define idxoff    ((98-30-48)/2)

/* DONE */
void
build_host_gui(void)
{
	int     i;
	int     bw[] = {160, 142, 156, 156, 142, 142};
	int     x[] = {185, 469, 753};
	int     y[] = {227, 281, 335, 389, 443};
	int     x0[] = {86+30+idxoff, 370+30+idxoff, 654+30+idxoff};
	rect_t  r1 = {.w = 180, .h = 24};
	char   *bg[] = {BN_RET160G, BN_USAVE, BN_ZADD, BN_DEL156G, BN_ZPU, BN_ZPD};
	UtkWindow  *imap = NULL;

	if (miface == 2) {
		host_conf();
	}
	hostw = utk_window_derive();
#ifndef MAPDLL
	utk_widget_resident(hostw);
#endif
	utk_window_set_bg(hostw, BG_HOST);
	utk_widget_async_evt_handler(hostw, update_host);
	utk_widget_hide_before(hostw, prepare_leave, (void*)0);

	stb_new(hostw, show_sysnet_win);
	build_st_label();
	build_sel_label();
	build_entry_kbar(&ike, hostw, 114, 0);
	build_hid_entries(hostw, x0, y);
	
	for (i = 0; i < MAXI; ++i) {
		imap = (UtkWindow*)utk_map_new();
		utk_widget_set_size(imap, 184, 42);
		utk_widget_add_cntl(hostw, imap, x[i%3], y[i/3]);
		utk_signal_connect(imap, "clicked", select_host, (void*)i);
	}

	for (i = 0; i < MAXI; ++i) {
		r1.x = x[i%3] + XOFF1;
		r1.y = y[i/3] + YOFF;
		iplbl[i] = label_txt_new(hostw, RGB_WHITE, &r1, 1, 15);
	}
	prepare_host_ui();
	////////////////////////////////////////////////////////////

	build_button_array(hostw, ARR_SZ(bw), zmbar_h, zmbar_y-1, bw, 70, do_host_opt, bg);
	build_hadd_ui(hostw);
}

/* DONE */
void
show_host_win(void)
{
	if (miface == 2&&utk_window_top() != hostw) {
		if (hadd == 0)
			enable_netdev(0);
		utk_window_show(hostw);
		hadd = 0;
	}
}

/* DONE: called by recever.c */
void
update_hostip(int id, unsigned int hostp)
{
	utk_widget_send_async_event(hostw, id, (int)hostp);
}

/* called in hadd.c */
/* id: [1, MAXHOST] */
int
save_hip(int id, char *ip)
{
	hip_t   *h = &hinf[id-1];
	hip_t   *hd;
	int      idx;
	unsigned int   map = 0;
	unsigned int   m;

	m = get_net_hostp(ip);
	if (ipmap_isset(m)) {
		DBG("--- host ip: %s exists ---\n", ip);
		return 1;
	}

	h->ol = 0;
	if (hmapc_isset(id-1)) { //override the exists item.
		memcpy(h->ip, ip, 16);
		h->m = m;
	} else if (hmap_isset(id-1)) {
		map = ~hmap;
		if (map > 0) {
			idx = __builtin_ctz(map);
			hd = &hinf[idx];
			hd->id = 0;
			memcpy(hd->ip, h->ip, 16);
			hd->idx = hcnt + 1;
			harray[hcnt++] = idx + 1;
			hmap_set(idx);
		}
		h->m = m;
		h->id = id;
		memcpy(h->ip, ip, 16);
		hmapc_set(id-1);
	} else {
		h->m = m;
		h->id = id;
		strcpy(h->ip, ip);
		hmapc_set(id-1);
		hmap_set(id-1);
		h->idx = hcnt + 1;
		harray[hcnt++] = id;
	}

	++hchanged;
	update_iplbl(h);
	update_ident(h);
	return 0;
}



