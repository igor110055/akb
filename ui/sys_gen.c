#include <arpa/inet.h>
#include <string.h>
#include <time.h>
#include "mxc_sdk.h"
#include "ulib.h"
#include "utk.h"
#include "utksignal.h"
#include "utkwindow.h"
#include "utktimeout.h"
#include "utkbutton.h"
#include "utklabel.h"
#include "utkmap.h"
#include "utkentry.h"
#include "layout.h"
#include "stb.h"
#include "gui.h"
#include "config.h"
#include "rs485.h"
#include "qtimer.h"
#include "zopt.h"
#include "recver.h"
#include "xfer.h"


static UtkWindow  *gwin;
static UtkWindow  *gcbx[3];
static UtkWindow  *tsdialog;

static int         tsd = 0;

static void       *stb;

static drop_menu_t adm;
static drop_menu_t edm;

static kbar_ent_t  gke;
static kbar_ent_t  gkb;

static myent_t     aent;
static myent_t     vent;
static myent_t     nent;
static myent_t     tent[6];
static myent_t     usrent;

static char        pa[6] = {0};
static char        g_pa[6] = {0};

static char        pv[4] = {0};
static char        g_pv[4] = {0};

static char        pn[4] = {0};
static char        g_pn[4] = {0};

static char        yr[6] = {0};
static char        mon[4] = {0};
static char        day[4] = {0};
static char        hur[4] = {0};
static char        min[4] = {0};
static char        sec[4] = {0};

static char        g_pusr[48] = {0};

static char       *defusr = "上海安人电子有限公司";

static struct tm   g_tm;
///////////////////////////////

static unsigned char atp;
static unsigned char etp;
static PVOID  sg_cb[] = {show_sysnet_win, show_sysuart_win, show_systmr_win, show_sysalr_win};
static int    g_vol[] = {0, 128, 200, 220, 235, 240, 245, 250, 255};
static gencfg_t    gencfg;

static int   tflag = 0;

#define tflag_set(i)     tflag |= (1 << (i))
#define tflag_isset(i)  (tflag & (1 << (i)))
#define tflag_clr(i)     tflag &= ~(1 << (i))

//////////////////////////////////
//new add on 2014.02.27
//////////////////////////////////
unsigned short  g_devid = 0;
unsigned short  masterid = 0;
static int      masterh = 1;
static int      standalone = 0;

static tip_t    usrt;


/* DONE */
int
is_master(void)
{
	return (masterh != 0);
}

/* DONE */
int
is_standalone(void)
{
	return (standalone != 0);
}

static void
master_service_start(void)
{
	zopt_start();
	recver_init();
	recver_prepare();
	recver_start();
	if (standalone == 0) {
		start_file_server();
		start_master();
		grp_start();
	}
}

static void
slave_service_start(void)
{
	start_file_client();
	start_slave();
}

static void
master_service_stop(void)
{
	grp_end();
	end_master();
	stop_ft_loop();
	recver_exit();
	recver_finish();
	recver_uninit();
	zopt_exit();
}

static void
slave_service_stop(void)
{
	end_slave();
	stop_ft_loop();
}

static void
master_net_start(void)
{
	if (standalone == 0) {
		if (miface == 2) {
			recver_start();
		}
		start_file_server();
		start_master();
		grp_start();
	}
}

static void
master_net_stop(void)
{
	grp_end();
	end_master();
	stop_ft_loop();
	if (miface == 2) {
		recver_exit();
	}
}

static PVOID  role_start[] = {slave_service_start, master_service_start};
static PVOID  role_stop[] = {slave_service_stop, master_service_stop};

static PVOID  net_start[] = {slave_service_start, master_net_start};
static PVOID  net_stop[] = {slave_service_stop, master_net_stop};

void
start_service(void)
{
	(*role_start[masterh])();
}

#if 0
void
stop_service(void)
{
	(*role_stop[masterh])();
}
#endif

void
start_net(void)
{
	(*net_start[masterh])();
}

void
stop_net(void)
{
	(*net_stop[masterh])();
}

/* DONE */
static void
set_devid(unsigned short id)
{
	if (id == 0) { //standalone.
		standalone = 1;
		masterh = 1;
	} else { //connected to LAN.
		standalone = 0;
		if (id < 100) { //hhh
			masterh = 1;
			DBG("--- localhost is master ---\n");
		} else { //shhh
			masterh = 0;
			DBG("--- localhost is slave ---\n");
		}
	}

	masterid = id % 100;
	g_devid = id;
}

/* should be called first by system_init() int init.c */
/* DONE */
void
devid_conf(void)
{
	FILE *fp;
	unsigned short  id = 0;

	set_usrptr(g_pusr);
	if ((fp = fopen(DEVID, "r")) != NULL) {
		ignore_result(fread(&id, sizeof(id), 1, fp));
		fclose(fp);
	}

	sprintf(g_pa, "%d", id);
	sprintf(pa,   "%d", id);

	set_devid(id);
}

static void
devid_save(unsigned short id)
{
	FILE *fp;
	int   _masterh;

	if ((fp = fopen(DEVID, "w")) != NULL) {
		if (fwrite(&id, sizeof(unsigned short), 1, fp) != 1) {
			DBG("--- fwrite error ---\n");
		}
		fflush(fp);
		fclose(fp);
	}

	_masterh = masterh;

	(*role_stop[masterh])();

	set_devid(id);

	if (_masterh != masterh) {
		DBG("--- stb_display_slave(%d) ---\n", masterh);
		stb_display_slave(masterh);
	}

	(*role_start[masterh])();
}

//////////////////////////////////////////////

static void
gencfg_def(void)
{
	memset(&gencfg, 0, sizeof(gencfg_t));
	gencfg.vol  = 6;
	gencfg.nrh  = 1;
	gencfg.rprt = 1;
	strcpy(gencfg.user, defusr);
}

void
general_conf(void)
{
	FILE *fp;
	int   r = 0;

	if ((fp = fopen(GENCONF, "r")) != NULL) {
		r = fread(&gencfg, sizeof(gencfg_t), 1, fp);
		fclose(fp);
	}

	if (r == 0) {
		gencfg_def();
	}

	sprintf(g_pv, "%d", gencfg.vol);
	sprintf(pv,   "%d", gencfg.vol);
	sprintf(g_pn, "%d", gencfg.nrh);
	sprintf(pn,   "%d", gencfg.nrh);
	memcpy(g_pusr, gencfg.user, sizeof(g_pusr));

	gencfg.vol = (gencfg.vol > 8) ? 8 : ((gencfg.vol < 0) ? 0 : gencfg.vol);
	mxc_set_vol(g_vol[gencfg.vol]);
}

static void
general_conf_save(void)
{
	FILE *fp;

	if ((fp = fopen(GENCONF, "w")) != NULL) {
		if (fwrite(&gencfg, sizeof(gencfg_t), 1, fp) != 1) {
			DBG("--- fwrite error ---\n");
		}
		fflush(fp);
		fclose(fp);
	}
}

void
update_username(char *buf)
{
	strcpy(g_pusr, buf);
	strcpy(gencfg.user, buf);
	general_conf_save();
}

static void
store_time(void)
{
	time_t tm;

	CLEARV(g_tm);
	g_tm.tm_year = atoi(yr)-1900;
	g_tm.tm_mon = atoi(mon)-1;
	g_tm.tm_mday = atoi(day);	
	g_tm.tm_hour = atoi(hur);
	g_tm.tm_min = atoi(min);
	g_tm.tm_sec = atoi(sec);
	tm = mktime(&g_tm);
	mxc_set_rtc(&g_tm);
	stime(&tm);
}

static void
set_time(void)
{
	if (tflag)
	{
		store_time();
		stb_update_clock(stb);
		tflag = 0;
	}
	stb_start_clock(stb);
}

static void
general_save(void *data)
{
	int  r = 0;
	int  st;
	int  rprt = 0;
	int  d = 0;
	unsigned short id = 0;

	hide_menu(&adm);
	hide_menu(&edm);
	if (kbar_entry_save_prepared(&gke) == 0) return;
	set_time();
	if (strcmp(g_pa, pa)) {
		memcpy(g_pa, pa, 6);
		id = atoi(pa);
		++d;
	}

	if (strcmp(g_pv, pv)) {
		memcpy(g_pv, pv, 4);
		gencfg.vol = atoi(pv);
		mxc_set_vol(g_vol[gencfg.vol]);
		++r;
	}

	if (strcmp(g_pn, pn)) {
		memcpy(g_pn, pn, 4);
		gencfg.nrh = atoi(pn);
		update_hostnr(gencfg.nrh);
		++r;
	}

	st = utk_widget_state(gcbx[0]);
	if (st != gencfg.cid) {
		gencfg.cid = st;
		++r;
	}

	st = utk_widget_state(gcbx[1]);
	if (st != gencfg.aud) {
		gencfg.aud = st;
		++r;
	}

	st = utk_widget_state(gcbx[2]);
	if (st != gencfg.rprt) {
		gencfg.rprt = st;
		rprt = 1;
		rs485_exit();
		++r;
	}

	if (gencfg.atp != atp) {
		gencfg.atp = atp;
		++r;
	}

	if (gencfg.etp != etp) {
		gencfg.etp = etp;
		++r;
	}

	if (memcmp(g_pusr, gencfg.user, sizeof(g_pusr))) {
		memcpy(g_pusr, gencfg.user, sizeof(g_pusr));
		++r;
	}

	if (r) {
		general_conf_save();
	}

	if (rprt) {
		rs485_start();
	}

	if (d) {
		devid_save(id);
	}
}

static inline void
restore_year(void)
{
	CLEARA(yr);
	sprintf(yr, "%04d", g_tm.tm_year+1900);
	utk_entry_update((UtkWindow*)tent[0].ent);
	tflag_clr(0);
}

static inline void
restore_mon(void)
{
	CLEARA(mon);
	sprintf(mon, "%02d", g_tm.tm_mon+1);
	utk_entry_update((UtkWindow*)tent[1].ent);
	tflag_clr(1);
}

static inline void
restore_mday(void)
{
	CLEARA(day);
	sprintf(day, "%02d", g_tm.tm_mday);
	utk_entry_update((UtkWindow*)tent[2].ent);
	tflag_clr(2);
}

static inline void
restore_hour(void)
{
	CLEARA(hur);
	sprintf(hur, "%02d", g_tm.tm_hour);
	utk_entry_update((UtkWindow*)tent[3].ent);
	tflag_clr(3);
}

static inline void
restore_min(void)
{
	CLEARA(min);
	sprintf(min, "%02d", g_tm.tm_min);
	utk_entry_update((UtkWindow*)tent[4].ent);
	tflag_clr(4);
}

static inline void
restore_sec(void)
{
	CLEARA(sec);
	sprintf(sec, "%02d", g_tm.tm_sec);
	utk_entry_update((UtkWindow*)tent[5].ent);
	tflag_clr(5);
}

static void
restore_time()
{
	get_tm(&g_tm, NULL);
	if (tflag_isset(0))
	{
		restore_year();
	}

	if (tflag_isset(1))
	{
		restore_mon();
	}

	if (tflag_isset(2))
	{
		restore_mday();
	}

	if (tflag_isset(3))
	{
		restore_hour();
	}

	if (tflag_isset(4))
	{
		restore_min();
	}

	if (tflag_isset(5))
	{
		restore_sec();
	}
}

static void
_gencfg_cancel(void)
{
	kbar_entry_cancel(&gke);
	restore_time();
	if (strcmp(gencfg.user, g_pusr)) {
		memcpy(gencfg.user, g_pusr, sizeof(g_pusr));
		myent_update(&usrent);
	}
	tflag = 0;
	utk_widget_update_state(gcbx[0], gencfg.cid > 0 ? 1 : 0);
	utk_widget_update_state(gcbx[1], gencfg.aud > 0 ? 1 : 0);
	utk_widget_update_state(gcbx[2], gencfg.rprt > 0 ? 1 : 0);
}


static void
general_cancel(void *data)
{
	hide_menu(&adm);
	hide_menu(&edm);
	_gencfg_cancel();
	stb_start_clock(stb);
}

static void
if_tscal(void *data)
{
	if (tsd == 0) {
		show_model_dialog(tsdialog);
		tsd = 1;
	}
}

static void
sysf_gnav(void *data)
{
	sg_cb[(int)data]();
}

static void
build_addr_entry(void *parent)
{
	rect_t re = {.x = aent_x, .y = gent_y0, .w = aent_w, .h = gent_h};
	myent_new(&aent, parent, &re, 0);
	myent_add_caret(&aent, RGB_BLACK);
	myent_set_cache(&aent, pa, 6, 4);
	myent_set_buffer(&aent, g_pa, 6);
	myent_add_kbar(&aent, &gke);
}

static char  *usrstr = "分机不用设置使用者";
static int    usrlen = 0;

static int
user_edit(void *arg)
{
	if (is_master()) return 0;
	show_strtip(&usrt, usrstr, usrlen, 2, tip_w, qry_tip_y);
	return 1;
}

static void
build_user_entry(void *parent)
{
	rect_t re = {.x = 326, .y = 447, .w = 616, .h = 40};

	myent_new(&usrent, parent, &re, 1);
	myent_add_caret(&usrent, RGB_WHITE);
	myent_set_text_color(&usrent, RGB_WHITE);
	myent_set_cache(&usrent, gencfg.user, 48, 46);
	myent_set_buffer(&usrent, g_pusr, 48);
	myent_add_kbar(&usrent, &gkb);
	myent_set_callback(&usrent, user_edit, NULL, NULL, NULL);
}

static void
fin_vol(void)
{
	int   v;
	char *p = NULL;

	v = strtol(pv, &p, 10);
	if (v > 8) {
		CLEARA(pv);
		pv[0] = '8';
		myent_update(&vent);
	} else if (v == 0&&p == pv) {
		CLEARA(pv);
		sprintf(pv, "%d", gencfg.vol);
		myent_update(&vent);
	}
}

static void
build_vol_entry(void *parent)
{
	rect_t re = {.x = vent_x, .y = gent_y0, .w = aent_w, .h = gent_h};
	myent_new(&vent, parent, &re, 0);
	myent_add_caret(&vent, RGB_BLACK);
	myent_set_cache(&vent, pv, 4, 2);
	myent_set_buffer(&vent, g_pv, 4);
	myent_add_kbar(&vent, &gke);
	myent_set_callback(&vent, NULL, fin_vol, NULL, NULL);
}

static void
build_nrofhost_entry(void *parent)
{
	rect_t re = {.x = nent_x, .y = gent_y1, .w = aent_w, .h = gent_h};
	myent_new(&nent, parent, &re, 0);
	myent_add_caret(&nent, RGB_BLACK);
	myent_set_cache(&nent, pn, 4, 3);
	myent_set_buffer(&nent, g_pn, 4);
	myent_add_kbar(&nent, &gke);
}

static void
update_time(void *arg)
{
	struct tm  *tm = arg;

	if (tm->tm_sec != g_tm.tm_sec)
	{
		g_tm.tm_sec = tm->tm_sec;
		memset(sec, '\0', 4);
		sprintf(sec, "%02d", tm->tm_sec);
//		utk_entry_update((UtkWindow*)tent[5].ent);
		myent_update(&tent[5]);
	}

	if (tm->tm_min != g_tm.tm_min)
	{
		g_tm.tm_min = tm->tm_min;
		memset(min, '\0', 4);
		sprintf(min, "%02d", tm->tm_min);
//		utk_entry_update((UtkWindow*)tent[4].ent);
		myent_update(&tent[4]);
	}

	if (tm->tm_hour != g_tm.tm_hour)
	{
		g_tm.tm_hour = tm->tm_hour;
		memset(hur, '\0', 4);
		sprintf(hur, "%02d", tm->tm_hour);
//		utk_entry_update((UtkWindow*)tent[3].ent);
		myent_update(&tent[3]);
	}

	if (tm->tm_mday != g_tm.tm_mday)
	{
		g_tm.tm_mday = tm->tm_mday;
		memset(day, '\0', 4);
		sprintf(day, "%02d", tm->tm_mday);
//		utk_entry_update((UtkWindow*)tent[2].ent);
		myent_update(&tent[2]);
	}

	if (tm->tm_mon != g_tm.tm_mon)
	{
		g_tm.tm_mon = tm->tm_mon;
		memset(mon, '\0', 4);
		sprintf(mon, "%02d", tm->tm_mon+1);
//		utk_entry_update((UtkWindow*)tent[1].ent);
		myent_update(&tent[1]);
	}

	if (tm->tm_year != g_tm.tm_year)
	{
		g_tm.tm_year = tm->tm_year;
		memset(yr, '\0', 6);
		sprintf(yr, "%4d", tm->tm_year+1900);
//		utk_entry_update((UtkWindow*)tent[0].ent);
		myent_update(&tent[0]);
	}
}


static void
load_time(void *arg)
{	
	struct tm  *tm = arg;

	g_tm = *tm;
	memset(yr, '\0', 6);
	sprintf(yr, "%4d", tm->tm_year+1900);

	memset(mon, '\0', 4);
	sprintf(mon, "%02d", tm->tm_mon+1);

	memset(day, '\0', 4);
	sprintf(day, "%02d", tm->tm_mday);

	memset(hur, '\0', 4);
	sprintf(hur, "%02d", tm->tm_hour);

	memset(min, '\0', 4);
	sprintf(min, "%02d", tm->tm_min);

	memset(sec, '\0', 4);
	sprintf(sec, "%02d", tm->tm_sec);

	DBG("--- time: %s.%s.%s-%s:%s:%s ---\n", yr, mon, day, hur, min, sec);

	tflag = 0;
}

////////////////////////////////////

static void
fin_yr(void)
{
	int   m;
	char *p = NULL;

	m = strtol(yr, &p, 10);
	if ((m < 2013)||(m > 2063)) {
		DBG("--- invalid year value ---\n");
		restore_year();
	} else if (m == g_tm.tm_year+1900) {
		tflag_clr(0);
	} else {
		tflag_set(0);
	}
}

static void
fin_mon(void)
{
	int   m;
	char *p;

	m = strtol(mon, &p, 10);
	if ((m < 1)||(m > 12)) {
		DBG("--- invalid month value ---\n");
		restore_mon();
	} else if (m == g_tm.tm_mon+1) {
		tflag_clr(1);
	} else {
		tflag_set(1);
	}
}

static void
fin_mday(void)
{
	int   m;
	char *p;

	m = strtol(day, &p, 10);
	if ((m < 0)||(m > 31)||(m == 0&&p == day)) {
		DBG("--- invalid day value ---\n");
		restore_mday();
	} else if (m == g_tm.tm_mday) {
		tflag_clr(2);
	} else {
		tflag_set(2);
	}
}

static void
fin_hour(void)
{
	int   m;
	char *p;

	m = strtol(hur, &p, 10);
	if ((m < 0)||(m > 23)||(m == 0&&p == hur)) {
		DBG("--- invalid hour value ---\n");
		restore_hour();
	} else if (m == g_tm.tm_hour) {
		tflag_clr(3);
	} else {
		tflag_set(3);
	}
}

static void
fin_min(void)
{
	int   m;
	char *p;

	m = strtol(min, &p, 10);
	if ((m < 0)||(m > 59)||(m == 0&&p == min)) {
		DBG("--- invalid miniute value ---\n");
		restore_min();
	} else if (m == g_tm.tm_min) {
		tflag_clr(4);
	} else {
		tflag_set(4);
	}
}

static void
fin_sec(void)
{
	int   m;
	char *p = NULL;

	m = strtol(sec, &p, 10);
	if ((m < 0)||(m > 59)||(m == 0&&p == sec)) {
		DBG("--- invalid second value ---\n");
		restore_sec();
	} else if (m == g_tm.tm_sec) {
		tflag_clr(5);
	} else {
		tflag_set(5);
	}
}

static PVOID  fin_cb[] = {fin_yr, fin_mon, fin_mday, fin_hour, fin_min, fin_sec};

static int
tentry_focusin(void *data)
{
	stb_stop_clock(stb);
	DBG("---- stop timer ----\n");
	return 0;
}

static void
tentry_fin(void *data)
{
	fin_cb[(int)data]();
}

static void
build_time_entry(void *parent)
{
	int  i;
	int  x[] = {tent_x1, tent_x2, tent_x3, tent_x4, tent_x5};
	rect_t re = {.x = tent_x0, .y = gent_y2, .w = yent_w, .h = gent_h};
	char *ptm[6] = {NULL};

	ptm[0] = yr;
	ptm[1] = mon;
	ptm[2] = day;
	ptm[3] = hur;
	ptm[4] = min;
	ptm[5] = sec;

	myent_new(&tent[0], parent, &re, 0);
	myent_add_caret(&tent[0], RGB_BLACK);
	myent_set_cache(&tent[0], ptm[0], 6, 4);
	myent_set_callback(&tent[0], tentry_focusin, tentry_fin, NULL, (void *)0);
	myent_add_kbar(&tent[0], &gke);

	re.w = tent_w;
	for (i = 1; i < 6; ++i) {
		re.x = x[i-1];
		myent_new(&tent[i], parent, &re, 0);
		myent_add_caret(&tent[i], RGB_BLACK);
		myent_set_cache(&tent[i], ptm[i], 4, 2);
		myent_set_callback(&tent[i], tentry_focusin, tentry_fin, NULL, (void *)i);
		myent_add_kbar(&tent[i], &gke);
	}
}

static void
select_atip(int i)
{
	DBG("--- adm: %d ---\n", i);
	atp = i;
}

static void
select_etip(int i)
{
	DBG("--- edm: %d ---\n", i);
	etp = i;
}

static void
sysgen_quit(void *data)
{
	_gencfg_cancel();
	if (tsd == 1) {
		hide_model_dialog(tsdialog);
		tsd = 0;
	}
	hide_menu(&adm);
	hide_menu(&edm);
	hide_imekb(&gkb);
}

static void
prepare_kbar(int i)
{
	if (i == 0) {
		keybar_ungrab(&gkb.kbar);
		stb_start_clock(stb);
	} else {
		stb_stop_clock(stb);
		keybar_grab(&gkb.kbar);
	}
}

static void
go_tscal(void *data)
{
	if (tsd == 1) {
		hide_model_dialog(tsdialog);
		tsd = 0;
	}
	show_tscal();
}

static void
cancel_tscal(void *data)
{
	if (tsd == 1) {
		hide_model_dialog(tsdialog);
		tsd = 0;
	}
}

void
build_sysgen_gui(void)
{
	UtkWindow  *cmap;
	int  st;
	int  bw[] = {sys_w0, sys_w1, sys_w3, sys_w4};
	int  bx[] = {sys_x0, sys_x1, sys_x3, sys_x4};
	char  *bga[] = {ICO_ATIP00, ICO_ATIP10, ICO_ATIP20, ICO_ATIP30};
	char  *bga1[] = {ICO_ATIP01, ICO_ATIP11, ICO_ATIP21, ICO_ATIP31};
	char  *bge[] = {ICO_ETIP00, ICO_ETIP10, ICO_ETIP20};
	char  *bge1[] = {ICO_ETIP01, ICO_ETIP11, ICO_ETIP21};
	rect_t  ar = {.x=sysg_atip_x, .y=sysg_tip_y, .w=sysg_tip_w, .h=sysg_tip_h};
	rect_t  er = {.x=sysg_etip_x, .y=sysg_tip_y, .w=sysg_tip_w, .h=sysg_tip_h};
	rect_t  r = {.x = bn_gret_x, .y = bn_sys_y, .w = gret_w, .h = bn_sys_h};

	char *bgok[] = {BN_DELOK0, BN_DELOK1};
	char *bgcl[] = {BN_DELCL0, BN_DELCL1};

	usrlen = strlen(usrstr);

	gwin = utk_window_derive();
#ifndef MAPDLL
	utk_widget_resident(gwin);
#endif
	utk_window_set_bg(gwin, BG_SYSGEN);
	utk_widget_hide_before(gwin, sysgen_quit, NULL);

	register_qtimer((void*)gwin);

	stb = stb_new(gwin, sysf_quit);
	stb_add_update_cb((void*)stb, (void*)update_time, (void*)load_time, 100);

	build_tip_lbl(&usrt, gwin, (LCD_W-tip_w)/2, (LCD_H-tip_h)/2, BG_TIP, 0);

	build_map_array(gwin, ARR_SZ(bx), bw, bx, sys_y, sys_h, sysf_gnav);

	build_drop_menu(&adm, gwin, BG_ATIP, &ar, bga, bga1, ARR_SZ(bga), select_atip);
	build_drop_menu(&edm, gwin, BG_ETIP, &er, bge, bge1, ARR_SZ(bge), select_etip);
	
	build_entry_kbar(&gke, gwin, sysg_kbar_y, 0);
	build_entry_keyboard(&gkb, gwin, 0, 0);
	kbar_set_show(&gkb, prepare_kbar);

	build_addr_entry(gwin);
	build_vol_entry(gwin);
	build_nrofhost_entry(gwin);
	build_time_entry(gwin);
	build_user_entry(gwin);

	tsdialog = build_model_dialog(gwin, TIP_TSCAL, bgok, bgcl, go_tscal, cancel_tscal);

	///////////////////////////////////////////////////////////
	st = gencfg.cid > 0 ? 1 : 0;	
	gcbx[0] = build_checkbox(gwin, sysgen_cbox_x0, sysgen_cbox_y, NULL, (void*)0);
	utk_widget_set_state(gcbx[0], st);

	st = gencfg.aud > 0 ? 1 : 0;
	gcbx[1] = build_checkbox(gwin, sysgen_cbox_x1, sysgen_cbox_y, NULL, (void*)0);
	utk_widget_set_state(gcbx[1], st);

	st = gencfg.rprt > 0 ? 1 : 0;
	gcbx[2] = build_checkbox(gwin, sysgen_cbox_x2, sysgen_cbox_y, NULL, (void*)0);
	utk_widget_set_state(gcbx[2], st);
	
	cmap = (UtkWindow*)utk_map_new();
	utk_widget_set_size(cmap, sysmap_w, sysmap_h);
	utk_widget_add_cntl(gwin, cmap, sysmap_x, sysmap_y);
	utk_signal_connect(cmap, "clicked", if_tscal, (void*)0);

	///////////////////////////////////////////////////////////
	button_new(gwin, BN_RET1501, &r, go_home, (void*)1);
	build_savecancel_buttons((void *)gwin, general_save, general_cancel);
}

/* DONE */
void
show_sysgen_win(void)
{
	if (utk_window_top() != gwin) {
		utk_window_show(gwin);
	}
}

int
get_host_nr(void)
{
	return ((gencfg.nrh == 0) ? 1 : gencfg.nrh);
}

int
get_real_print(void)
{
	return (int)gencfg.rprt;
}

char*
get_owner_ptr(void)
{
	return gencfg.user;
}

void
handle_netifloop(void)
{
	if (standalone == 1) {
		stop_netif();
	} else {
		start_netif();
	}
}


