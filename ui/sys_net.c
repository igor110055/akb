#include <arpa/inet.h>
#include <string.h>
#include "ulib.h"
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
#include "qtimer.h"
#include "recver.h"

#define  GEN_MSK     "0.0.0.0"
#define  DEF_DST     "0.0.0.0"

static UtkWindow  *ncwin = NULL;
static UtkWindow  *mcbx = NULL;
static UtkWindow  *ipent1 = NULL;


static kbar_ent_t  nke;
static myent_t     ipent[3];

#if 0
static myent_t     hipent[2];
static char  g_hip[2][16] = {{0}, {0}};
static char  hip[2][16] = {{0}, {0}};
static tip_t htip;
#endif
struct in_addr   gcliaddr[2];


char  g_ipaddr[16] = {0};
static char  g_mask[16] = {0};
static char  g_gw[16] = {0};
static char *g_ipstr[3];

char  g_masterip[16] = {0};

////////////////////////////////////////
static char  ipaddr[16] = {0};
static char  mask[16] = {0};
static char  gw[16] = {0};
static char *ipstr[3];

static char  g_dns[16] = {0};
static char  g_mac[20] = {0};

unsigned int   hmask = 0;
unsigned int   hnet = 0;


static PVOID  sn_cb[] = {show_sysuart_win, show_sysgen_win, show_systmr_win, show_sysalr_win};

int
is_valid_unip(char *val)
{
	struct in_addr ia;
	return (inet_aton(val, &ia)&&!u_bad_addr(ia)&&!u_mcast_addr(ia));
}

static int
is_netmask(char *s)
{
	struct in_addr ia;
	unsigned int  v;

	if (inet_aton(s, &ia) == 0) return 0;
	v = ia.s_addr;
	U_SWAP_32(v);
	return (!(v & 0x07));
}

static inline void
get_haddr(unsigned int *n, unsigned int *m)
{
	struct in_addr  ia;
	unsigned int    hm;
	unsigned int    nm;

	inet_aton(g_mask, &ia);
	hm = ntohl(ia.s_addr);

	inet_aton(g_ipaddr, &ia);
	nm = ntohl(ia.s_addr);

	*n = nm & hm;
	*m = ~hm;
}

/* used for save and display. */
void
netconf_load(void)
{
	FILE *fp;
	char  buf[256];
	char  key[32];
	char  value[128];
	char *addr = NULL;
	char *ptr;
	int   has_ip = 0;
	int   has_msk = 0;

	if ((fp = fopen(NETCONF, "r")) == NULL) {
		goto nqcfg;
	}

	while(!feof(fp))
	{
		CLEARA(buf);
		CLEARA(key);
		CLEARA(value);
		if (fgets(buf, sizeof(buf), fp) == NULL) continue;
		if ((ptr = u_trim(buf)) == NULL) continue;		
		addr = strchr(ptr, '=');
		if (addr == NULL) continue;
		*addr = ' ';
		if (sscanf(ptr, "%s %s", key, value) != 2) continue;

		DBG("--- key = %s, value = %s ---\n", key, value);
		if (strncasecmp("IP", key, 2) == 0)
		{
			if (is_valid_unip(value))
			{
				strncpy(g_ipaddr, value, sizeof(g_ipaddr));
				has_ip = 1;
			}
		}
		else if (strncasecmp("Mask", key, 4) == 0)
		{
			if (is_netmask(value))
			{
				strncpy(g_mask, value, sizeof(g_mask));
				has_msk = 1;
			}
		}
		else if(strncasecmp("Gateway", key, 7) == 0)
		{
			if (is_valid_unip(value))
			{
				strncpy(g_gw, value, sizeof(g_gw));
			}
		}
		else if (strncasecmp("DNS", key, 3) == 0)
		{
			if (is_valid_unip(value))
			{
				strncpy(g_dns, value, sizeof(g_dns));
			}
		}
		else if(strncasecmp("MAC", key, 3) == 0)
		{
			strncpy(g_mac, value, sizeof(g_mac));
		}
	}
	fclose(fp);

nqcfg:
	if (has_ip == 0) {
		u_if_get_ip("eth0", g_ipaddr, sizeof(g_ipaddr), NULL);
	}

	if (has_msk == 0) {
		u_if_get_mask("eth0", g_mask, sizeof(g_mask));
	}

	get_haddr(&hnet, &hmask);
	DBG("--- hnet = 0x%x, hmask = 0x%x ---\n", hnet, hmask);
}

static void
netconf_save(void)
{
	FILE *fp;

	fp = fopen(NETCONF, "w");
	if (fp == NULL) {
		DBG("--- save ipconf failed ---\n");
		return;
	}

	fprintf(fp, "IP=%s\n",  g_ipaddr);
	fprintf(fp, "Mask=%s\n", g_mask);
	fprintf(fp, "Gateway=%s\n", g_gw);
	fprintf(fp, "DNS=%s\n", g_dns);
	fprintf(fp, "MAC=%s\n", g_mac);
	fflush(fp);
	fclose(fp);
}

#if 0
static char *hstr = "分机不需要设置报警主机IP";
static int   hlen = 0;

static int
hip_edit(void *arg)
{
	if (is_master()) return 0;
	show_strtip(&htip, hstr, hlen, 2, tip_w, qry_tip_y);
	return 1;
}
#endif

static void
build_ip_entries(UtkWindow *parent)
{
	int  i;
	int  x[3] = {ipent_x0, ipent_x1, ipent_x2};
	rect_t re = {.x = 0, .y = ipent_y, .w = ipent_w, .h = ipent_h};

	for (i = 0; i < ARR_SZ(ipent); ++i)//ip,netmask,gateway.
	{
		re.x = x[i];
		myent_new(&ipent[i], parent, &re, 0);
		myent_add_caret(&ipent[i], RGB_BLACK);
		myent_set_cache(&ipent[i], ipstr[i], 16, 15);
		myent_set_buffer(&ipent[i], g_ipstr[i], 16);
		myent_add_kbar(&ipent[i], &nke);
	}

#if 0
	re.x = ipent1_x0;
	re.y = ipent1_y0;
	re.w = 188;
	myent_new(&hipent[0], parent, &re, 0);
	myent_add_caret(&hipent[0], RGB_BLACK);
	myent_set_cache(&hipent[0], hip[0], 16, 15);
	myent_set_buffer(&hipent[0], g_hip[0], 16);
	myent_add_kbar(&hipent[0], &nke);
	myent_set_callback(&hipent[0], hip_edit, NULL, NULL, NULL);

	re.y = ipent1_y1;
	myent_new(&hipent[1], parent, &re, 0);
	myent_add_caret(&hipent[1], RGB_BLACK);
	myent_set_cache(&hipent[1], hip[1], 16, 15);
	myent_set_buffer(&hipent[1], g_hip[1], 16);
	myent_add_kbar(&hipent[1], &nke);
	myent_set_callback(&hipent[1], hip_edit, NULL, NULL, NULL);
#endif
}

static int
set_ip(char *ip)
{
	if (is_valid_unip(ip)) {
		u_if_set_ip("eth0", ip);
		return 1;
	}
	return 0;
}

static int
set_netmask(char *ip)
{
	if (is_netmask(ip)) {
		u_if_set_mask("eth0", ip);
		return 1;
	}
	return 0;
}

static int
set_gateway(char *ip)
{
	if (is_valid_unip(ip)) {
		u_route_add(NULL, ip, DEF_DST, GEN_MSK, 1);
		return 1;
	}
	return 0;
}

static int (*fn_set[])(char*) = {set_ip, set_netmask, set_gateway};

static void
set_mac(char *ip)
{
	char   mac[6] = {0};
	int    m1;
	int    m2;
	char  *ptr;
	char   str[16] = {0};
	
	strcpy(str, ip);
	ptr = strrchr(str, '.');
	m2 = strtol(ptr + 1, NULL, 10);
	*ptr = 0;
	ptr = strrchr(str, '.');
	m1 = strtol(ptr + 1, NULL, 10);
	DBG("--- m1 = %d, m2 = %d ---\n", m1, m2);
	u_if_get_mac("eth0", mac);
	if (mac[4] != m1||mac[5] != m2) {
		u_if_down("eth0");
		mac[4] = m1;
		mac[5] = m2;
		u_if_set_mac("eth0", mac);
		u_if_up("eth0");
		memset(g_mac, 0, 20);
		sprintf(g_mac, "%02x:%02x:%02x:%02x:%02x:%02x", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
		DBG("--- set macaddr: %02x:%02x:%02x:%02x:%02x:%02x ---\n", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
	}
}

static void
ncfg_save(void *data)
{
	int  i;
	int  n = 0;
	int  n0 = 0;

	if (kbar_entry_save_prepared(&nke) == 0) return;
	for (i = 0; i < ARR_SZ(ipent); i++)
	{
		if (strcmp(g_ipstr[i], ipstr[i]) != 0) {
			if (strlen(ipstr[i]) == 0) { //delete this ipaddress
				memset(g_ipstr[i], 0, 16);
				++n;
			} else {
				if ((*fn_set[i])(ipstr[i]) == 1) {
					memcpy(g_ipstr[i], ipstr[i], 16);
					++n;
					if (i != 2) {//localhost ipaddress changed.
						++n0;
						stop_net();
					}
				} else {
					myent_reload(&ipent[i]);
				}
			}
		}
	}

	if (n) {
		if (n0) {
			set_mac(g_ipaddr);
			get_haddr(&hnet, &hmask);
			DBG("--- hnet = 0x%x, hmask = 0x%x ---\n", hnet, hmask);
			start_net();
		}
		netconf_save();
	}
}

static void
sysf_nnav(void *data)
{
	int  i = (int)data;
	sn_cb[i]();
}

static void
ncfg_cancel(void *data)
{
	kbar_entry_cancel(&nke);
}

#define BN_SHOST1     "/opt/ui/bn_shnr1.png"
#define BN_SLV1       "/opt/ui/bn_ckslv1.png"

static void
host_set_ui(void *arg)
{
	DBG("--- goto host set ui ---\n");
	if (is_master()) {
		show_host_win();
	}
}

static void
slv_ck_ui(void *arg)
{
	DBG("--- goto slave check ui ---\n");
	if (is_master()) {
		show_slave_win();
	}
}

void
build_sysnet_gui(void)
{
	int  i;
	int  bw[] = {sys_w1, sys_w2, sys_w3, sys_w4};
	int  bx[] = {sys_x1, sys_x2, sys_x3, sys_x4};
	rect_t  r = {.x = bn_gret_x, .y = bn_sys_y, .w = gret_w, .h = bn_sys_h};
	rect_t  rl = {.x = ipent1_x1, .y = 325, .w = sys_checkbox_w, .h = sys_checkbox_h};
	rect_t  ri = {.x = ipent1_x1+16, .y = 394, .w = ipent_w-32, .h = ipent_h};
	rect_t  rb = {.x = 197, .y = 304, .w = 221, .h = 49};

	g_ipstr[0] = g_ipaddr;
	g_ipstr[1] = g_mask;
	g_ipstr[2] = g_gw;
	
	ipstr[0] = ipaddr;
	ipstr[1] = mask;
	ipstr[2] = gw;
	
	for (i = 0; i < 3; ++i) {
		if (strlen(g_ipstr[i]) > 0) {
			memcpy(ipstr[i], g_ipstr[i], 16);
		}
	}

	ncwin = utk_window_derive();
#ifndef MAPDLL
	utk_widget_resident(ncwin);
#endif

	utk_window_set_bg(ncwin, BG_SYSNET);
	utk_widget_hide_before(ncwin, ncfg_cancel, NULL);

	register_qtimer((void*)ncwin);

	stb_new(ncwin, sysf_quit);
#if 0
	build_tip_lbl(&htip, ncwin, (LCD_W-tip_w)/2, (LCD_H-tip_h)/2, BG_TIP, 0);
#endif
	build_map_array(ncwin, ARR_SZ(bx), bw, bx, sys_y, sys_h, sysf_nnav);
	
	build_entry_kbar(&nke, ncwin, sysnet_kbar_y, 0);
	build_ip_entries(ncwin);

	button_new(ncwin, BN_SHOST1, &rb, host_set_ui, (void*)0);
	rb.y = 397;
	button_new(ncwin, BN_SLV1, &rb, slv_ck_ui, (void*)0);

	/////////////////////////////////////////////////////////////
	//modified on 2014.02.27
	mcbx = label_new(ncwin, SYS_CBOX, NULL, 0, &rl);
	ipent1 = label_new(ncwin, NULL, g_masterip, RGB_BLACK, &ri);
	update_master_label(is_master());

	/////////////////////////////////////////////////////////////

	button_new(ncwin, BN_RET1501, &r, go_home, (void*)1);
	build_savecancel_buttons((void *)ncwin, ncfg_save, ncfg_cancel);
}

/* DONE */
void
show_sysnet_win(void)
{
	if (utk_window_top() != ncwin) {
		utk_window_show(ncwin);
	}
}

/* called in sys_gen.c when devid changed. */
void
update_master_label(int m)
{
	utk_widget_update_state(mcbx, m);
	if (m) {
		memset(g_masterip, 0, 16);
	}
}

/* called when master is discovered. */
void
update_masterip(char *ip)
{
	memset(g_masterip, 0, 16);
	if (!is_master()&&ip != NULL) {
		strncpy(g_masterip, ip, 16);
	}
	if (ipent1)
		utk_label_update(ipent1);
}


