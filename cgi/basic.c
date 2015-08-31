#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <ctype.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "cgic.h"
#include "basic.h"
#include "cntl.h"

#define  GCONF    "/opt/conf/ef.conf"
#define  HCONF    "/opt/conf/host.conf"
#define  VCONF    "/opt/conf/vod.conf"
#define  SCONF    "/opt/conf/site.conf"
//#define  NCONF    "/opt/conf/net.conf"

#define  VOD_MAX   7
#define  LOC_MAX   16

#define  make_nr(b, u, r)  ((r) + (u)*10000 + (b)*100000)
#define  BLD_NR(nr)  ((nr)/100000)
#define  UNI_NR(nr)  (((nr)/10000)%10)
#define  RM_NR(nr)   ((nr)%10000)

/* general info from client */
static int  scr_svr = 0;
static int  elv_s = 0;
static int  elv_t = 0;
static int  cam = 0;

static int  mtype = 0;
static unsigned int _nr = 0;
static int  hostid = 0;

static int  _bnr;
static int  _unr;
static int  _rnr;

/* general info read from config file */
static int   g_scrsvr = 0;
static int   g_elvs = 0;
static int   g_elvt = 0;
static int   g_cam = 0;

static int   g_mtype = 0;
static unsigned int  g_nr = 0;
static int   b_nr = 0;
static int   u_nr = 0;
static int   r_nr = 0;
static int   g_hostid = 0;
static int   host = 0;

/* location info from clent */
static char  Loc[LOC_MAX][16];
static int   LocCnt = 0;

/* location info read from config file */
static char  loc[LOC_MAX][16];
static int   locCnt = 0;

/* vod info from client */
static char  new_vn[VOD_MAX][16];
static char  new_vi[VOD_MAX][16];
static int   new_vt[VOD_MAX] = {0};
static int   new_vcnt = 0;

/* vod info read from config file */
static char  vname[VOD_MAX][16];
static char  vip[VOD_MAX][16];
static int   vt[VOD_MAX] = {0};
static int   vcnt = 0;

/*
static char   g_prip[16] = {0};
static char   g_sgip[16] = {0};
static char   g_servip[16] = {0};
static int    g_aud = 1;
static int    g_auitvl = 0;
static char   g_gw[16] = {0};

static char   prip[16] = {0};
static char   sgip[16] = {0};
static char   servip[16] = {0};
static int    aud = 1;
static int    auitvl = 0;
static char   gw[16] = {0};
*/

static char *fname[] = {GCONF, HCONF, VCONF, SCONF};

/////////////// load config /////////////////////
#if 0
{
#endif

/* DONE */
static void
read_general(void)
{
	FILE *fp;
	char  buf[256];
	char  key[32];
	char  value[128];
	char *addr = NULL;
	int   offset;
	char *ptr;

	if ((fp = fopen(GCONF, "r")) == NULL) return;
	while(!feof(fp))
	{
		memset(buf, '\0', 256);
		memset(key, '\0', 32);
		memset(value, '\0', 128);
		
		fgets(buf, 256, fp);
		ptr = buf;
		while (*ptr!='\0'&&isspace(*ptr))
			++ptr;
		if (!isalpha(*ptr)) continue;
		
		addr = strchr(ptr, '=');
		if (addr == NULL) continue;
		offset = addr - ptr;
		if (offset == 0) continue;
		ptr[offset] = ' ';
		sscanf(ptr, "%s %s", key, value);

		if(strncasecmp("ScrSvr", key, 6) == 0)
		{
			if (strlen(value) > 0)
				g_scrsvr = atoi(value);
		}
		else if(strncasecmp("ElvSt", key, 5) == 0)
		{
			if (strlen(value) > 0)
				g_elvs = atoi(value);
		}
		else if(strncasecmp("PanNr", key, 5) == 0)
		{
			if (strlen(value) > 0)
				g_elvt = atoi(value);
		}
		else if(strncasecmp("Cam", key, 3) == 0)
		{
			if (strlen(value) > 0)
				g_cam = atoi(value);
			if (g_cam > 1)
				g_cam = 0;
		}
	}
	fclose(fp);
}

/* DONE */
static void
read_host_conf(void)
{
	FILE *fp;
	char  buf[128];
	int   t;
	unsigned int nr;
	int   id;
	char *p;
	char *ptr;

	if ((fp = fopen(HCONF, "r")) == NULL) return;
	while(!feof(fp))
	{
		memset(buf, '\0', 128);	
		fgets(buf, 128, fp);
		p = buf;
		while (*p != '\0'&&isspace(*p))
			++p;
		if (!isdigit(*p)) continue;
		if ((ptr = strchr(p, ':')) == NULL) continue;
		ptr++;
		if (strchr(ptr, ':') == NULL) continue;
		sscanf(p,"%d:%u:%d", &t, &nr, &id);
		if (t == 0||t == 1)
			g_mtype = t;
		else
			g_mtype = 0;
		g_nr = nr;
		g_hostid = id;
		if (g_mtype == 1)
		{
			b_nr = BLD_NR(g_nr);
			u_nr = UNI_NR(g_nr);
			r_nr = RM_NR(g_nr);
		}
		break;
	}

	fclose(fp);
}

/* DONE */
static void
read_loc(void)
{
	FILE  *fp;
	char   buf[64];
	char  *p;
	int    i;

	for (i = 0; i < LOC_MAX; ++i)
		memset(loc[i], 0, LOC_MAX);

	if ((fp = fopen(SCONF, "r")) == NULL)
	{
		return;
	}

	i = 0;
	while(!feof(fp)&&i < LOC_MAX)
	{
		memset(buf, '\0', 64);
		fgets(buf, 64, fp);
		p = buf;
		while (*p != '\0'&&isspace(*p))
			++p;
		if (*p == '\0') continue;
		if((p[0]=='#')||((p[0]=='/')&&(p[1]=='/'))) continue;

		strncpy(loc[i], p, 15);
		++i;
	}

	locCnt = i;
	fclose(fp);
}

/* DONE */
static void
read_vod(void)
{
	FILE *fp;
	char  buf[128];
	char  key[32];
	char  value[64];
	char *addr = NULL;
	char *ptr;
	int   len;
	int   i;
	int   t;
	struct in_addr ia;
	
	for (i = 0; i < VOD_MAX; ++i)
	{
		memset(vname[i], '\0', 16);
		memset(vip[i], '\0', 16);
	}

	if ((fp = fopen(VCONF, "r")) == NULL) return;
	i = 0;
	while(!feof(fp)&&i < VOD_MAX)
	{
		t = 0;
		memset(buf, '\0', 128);
		memset(key, '\0', 32);
		memset(value, '\0', 64);

		fgets(buf, 128, fp);
		ptr = buf;

		while (*ptr != '\0'&&isspace(*ptr)) ptr++;
		if (!isdigit(*ptr)) continue;

		t = strtol(ptr, (char**)&ptr, 10);
		while (*ptr != '\0'&&*ptr != ':') ptr++;
		if (*ptr != ':') continue;
		ptr++;

		while (*ptr != '\0'&&isspace(*ptr)) ptr++;
		if (*ptr == '\0') continue;

		addr = strchr(ptr, '=');
		if (addr == NULL||addr == ptr) continue;
		*addr = ' ';
		sscanf(ptr,"%s %s", key, value);
		len = strlen(key);
		if (len == 0) continue;

		ptr = &key[len-1];
		while (ptr != key&&isspace(*ptr)) 
		{
			*ptr = '\0';
			ptr--;
		}

		if (strlen(value) == 0) continue;
		ptr = value;
		while (*ptr != '\0'&&!isdigit(*ptr)) ptr++;
		if (!isdigit(*ptr)) continue;

		if (inet_aton(ptr, &ia) > 0)
		{
			strncpy(vname[i], key, 15);
			inet_ntop(AF_INET, &ia, vip[i], 16);
			vt[i] = t;
			++i;
		}
	}
	vcnt = i;
	fclose(fp);
}

/*
static int
u_bad_addr(struct in_addr addr)
{
	unsigned long hAddr = ntohl(addr.s_addr);
	return (hAddr == 0x7F000001
	  || hAddr == 0
	  || hAddr == 0xffffffff);
}

static int
u_mcast_addr(struct in_addr addr)
{
	unsigned long hAddr = ntohl(addr.s_addr);
	return (hAddr > 0xE0000000 && hAddr <= 0xEFFFFFFF);
}

static int
is_valid_unip(char *val)
{
	struct in_addr ia;
	return (inet_aton(val, &ia)&&!u_bad_addr(ia)&&!u_mcast_addr(ia));
}

static void
read_nconf(void)
{
	FILE *fp;
	char  buf[256];
	char  key[32];
	char  value[128];
	char *addr = NULL;
	char *ptr;

	if ((fp = fopen(NCONF, "r")) == NULL) return;
					
	while(!feof(fp))
	{
		memset(buf, '\0', 256);
		memset(key, '\0', 32);
		memset(value, '\0', 128);	
		fgets(buf, 256, fp);
		
		ptr = buf;
		while (*ptr!='\0'&&isspace(*ptr))
			++ptr;
		if (!isalpha(*ptr))continue;
		
		addr = strchr(ptr, '=');
		if (addr == NULL) continue;
		*addr = ' ';
		if (sscanf(ptr, "%s %s", key, value) != 2) continue;

		if (strncasecmp("PrIP", key, 4) == 0)
		{
			if (is_valid_unip(value))
				strcpy(g_prip, value);
		}
		else if (strncasecmp("SgIP", key, 4) == 0)
		{
			if (is_valid_unip(value))
				strcpy(g_sgip, value);
		}
		else if(strncasecmp("ServerIP", key, 8) == 0)
		{
			if (is_valid_unip(value))
				strcpy(g_servip, value);
		}
		else if (strncasecmp("GateWay", key, 7) == 0)
		{
			if (is_valid_unip(value))
				strcpy(g_gw, value);
		}
		else if(strncasecmp("AutoUd", key, 6) == 0)
		{
			if (strlen(value) > 0)
				g_aud = atoi(value);
		}
		else if(strncasecmp("AUdItvl", key, 7) == 0)
		{
			if (strlen(value) > 0)
				g_auitvl = atoi(value);
		}
	}

	fclose(fp);
	return;
}
*/

#if 0
}
#endif

/////////////// cgi out /////////////////////
#if 0
{
#endif

/* DONE */
static void
show_hostinf()
{
	/******************  row begin *****************/
	fprintf(cgiOut, "<tr>\n<td style='color:#ffffff;'><strong>机型和门牌号设置</strong></td>\n");
	fprintf(cgiOut, "<td colspan='3' style='COLOR: #0076C8; BACKGROUND-COLOR: #F4FAFF;'>&nbsp;</td>\n</tr>\n");
	/******************  row end *****************/

	fprintf(cgiOut, "<tr style='COLOR:#0076C8; BACKGROUND-COLOR:#F4FAFF;'>\n<td>\n");
	fprintf(cgiOut, "<label><input type='radio' name='mtype' value='1' onclick='fnHandleMtype(1)' ");
	if (g_mtype == 1)
		fprintf(cgiOut, "checked='checked' ");
	fprintf(cgiOut, "/>普通型</label></td>\n");

	fprintf(cgiOut, "<td><input name='bNr' type='text' size='4' maxlength='2' style='color:#000000;background-color:#ffffff;' ");
	if (g_mtype == 1)
		fprintf(cgiOut, "value='%d' ", b_nr);
	else
		fprintf(cgiOut, "disabled ");
	fprintf(cgiOut, "/>幢</td>\n");
  
	fprintf(cgiOut, "<td><input name='uNr' type='text' size='4' maxlength='1' style='color:#000000;background-color:#ffffff;' ");
	if (g_mtype == 1)
		fprintf(cgiOut, "value='%d' ", u_nr);
	else
		fprintf(cgiOut, "disabled ");
	fprintf(cgiOut, "/>单元</td>\n");

	fprintf(cgiOut, "<td><input name='rNr' type='text' size='6' maxlength='4' style='color:#000000;background-color:#ffffff;' ");
	if (g_mtype == 1)
		fprintf(cgiOut, "value='%d' ", r_nr);
	else
		fprintf(cgiOut, "disabled ");
	fprintf(cgiOut, "/>室</td></tr>\n");

	fprintf(cgiOut, "<tr style='COLOR: #0076C8; BACKGROUND-COLOR: #F4FAFF;'>\n");
	fprintf(cgiOut, "<td><label><input type='radio' name='mtype' value='0' onclick='fnHandleMtype(0)' ");
	if (g_mtype == 0)
		fprintf(cgiOut, "checked='checked' ");
	fprintf(cgiOut, "/>别墅型</label></td>\n");

	fprintf(cgiOut, "<td><input name='sNr' type='text' size='8' maxlength='6' style='color:#000000;background-color:#ffffff;' ");
	if (g_mtype == 0)
		fprintf(cgiOut, "value='%u' ", g_nr);
	else
		fprintf(cgiOut, "disabled ");
	fprintf(cgiOut, "/>幢</td>\n<td>分机号</td>\n");
	fprintf(cgiOut, "<td><input name='hostid' type='text' size='4' maxlength='1' style='color:#000000;background-color:#ffffff;' value='%d' /></td>\n</tr>\n", g_hostid);
	fprintf(cgiOut, "<tr style='COLOR: #0076C8; BACKGROUND-COLOR: #F4FAFF;'>\n");
	fprintf(cgiOut, "<td colspan='4'>注：分机号为0表示主机,对于别墅型最大分机号是4,对于普通型最大是2</td>\n</tr>\n");
}

/* DONE */
static void
show_elv_inf()
{
	fprintf(cgiOut, "<tr><td style='color:#ffffff;'><strong>电梯信息配置</strong></td>\n");
	fprintf(cgiOut, "<td colspan='3' style='COLOR: #0076C8; BACKGROUND-COLOR: #F4FAFF;'>&nbsp;</td></tr>\n");
	fprintf(cgiOut, "<tr style='COLOR: #0076C8; BACKGROUND-COLOR: #F4FAFF;'>\n");
  
	fprintf(cgiOut, "<td>电梯类型</td>\n");
	fprintf(cgiOut, "<td><label><input type='radio' name='elvT' value='0' ");
	if (g_mtype == 0)
		fprintf(cgiOut, "disabled ");
	else if (g_elvt == 0)
		fprintf(cgiOut, "checked='checked' ");
	fprintf(cgiOut, "/>单开门</label></td>\n");
	fprintf(cgiOut, "<td>电梯起始楼层</td>\n");
	if (g_mtype == 0)
		fprintf(cgiOut, "<td><input type='text' name='elvS' size='4' maxlength='3' style='color:#000000;background-color:#ffffff;' disabled /></td>\n</tr>");
	else
		fprintf(cgiOut, "<td><input type='text' name='elvS' size='4' maxlength='3' style='color:#000000;background-color:#ffffff;' value='%d' /></td>\n</tr>", g_elvs);

	fprintf(cgiOut, "<tr style='COLOR: #0076C8; BACKGROUND-COLOR: #F4FAFF;'>\n");
	fprintf(cgiOut, "<td></td>\n");
	fprintf(cgiOut, "<td><label><input type='radio' name='elvT' value='1' ");
	if (g_mtype == 0)
		fprintf(cgiOut, "disabled ");
	else if (g_elvt == 1)
		fprintf(cgiOut, "checked='checked' ");
	fprintf(cgiOut, "/>双开门</label></td>\n");
	fprintf(cgiOut, "<td colspan='2'></td>\n</tr>");
}

/* DONE */
static void
show_general()
{
	fprintf(cgiOut, "<table width='800' border='0' align='center' bgcolor='#b9d8f3'>\n");
	fprintf(cgiOut, "<tr style='color:#ffffff;text-align:center;'>\n");
	fprintf(cgiOut, "<td colspan='4'><h3><strong>基本配置</strong></h3></td></tr>\n");

	/******************  row begin ***************/
	fprintf(cgiOut, "<tr style='COLOR: #0076C8; BACKGROUND-COLOR: #F4FAFF;'>\n");
	fprintf(cgiOut, "<td>屏保超时</td>\n<td>\n");
	fprintf(cgiOut, "<input name='scrsvr' type='text' style='color:#000000;background-color:#ffffff;' size='6' maxlength='3' value='%d' />", g_scrsvr);
	fprintf(cgiOut, "秒(&gt;10)</td>\n<td colspan='2'></td>\n</tr>");
	/******************  row end *****************/

	show_hostinf();
	show_elv_inf();
	
	fprintf(cgiOut, "<tr style='COLOR: #0076C8; BACKGROUND-COLOR: #F4FAFF;'>\n");
	fprintf(cgiOut, "<td>默认摄像头类型</td>\n");
	fprintf(cgiOut, "<td><label><input type='radio' name='camT' value='0' ");
	if (g_cam == 0)
	{
		fprintf(cgiOut, "checked='checked' ");
	}
	fprintf(cgiOut, "/>CMOS</label></td>\n");

	fprintf(cgiOut, "<td><label><input type='radio' name='camT' value='1' ");
	if (g_cam == 1)
	{
		fprintf(cgiOut, "checked='checked' ");
	}
	fprintf(cgiOut, "/>CCD</label></td>\n");
	fprintf(cgiOut, "<td></td>\n</tr>");

	fprintf(cgiOut, "<tr style='COLOR: #0076C8; BACKGROUND-COLOR: #F4FAFF;'>\n");
	fprintf(cgiOut, "<td colspan='4'>&nbsp;</td>\n</tr>\n</table>\n");
}

/* DONE */
static void
show_vod()
{
	int  i, j;

	fprintf(cgiOut, "<table width='800' border='0' align='center' bgcolor='#b9d8f3'>\n");
	fprintf(cgiOut, "<tr style='COLOR: #ffffff; text-align:center;'>\n");
	fprintf(cgiOut, "<td colspan='4'><strong>远程监控点设置</strong></td></tr>\n");
	fprintf(cgiOut, "<tr style='COLOR: #0076C8; BACKGROUND-COLOR: #F4FAFF;'>\n");
	fprintf(cgiOut, "<td width='93'>&nbsp;</td>\n");
	fprintf(cgiOut, "<td width='242' align='center'>名称</td>\n");
	fprintf(cgiOut, "<td width='239' align='center'>IP地址</td>\n");
	fprintf(cgiOut, "<td width='198' align='center'>类型/通道</td></tr>\n");

	for (i = 1; i <= VOD_MAX; ++i)
	{
		fprintf(cgiOut, "<tr style='COLOR: #0076C8; BACKGROUND-COLOR: #F4FAFF;'><td>监控点%d</td>\n", i);
		if (i <= vcnt)
		{
			fprintf(cgiOut, "<td><input name='vname%d' type='text' size='18' maxlength='16' style='color:#000000;background-color:#ffffff;' value='%s' /></td>\n", i, vname[i-1]);
  		fprintf(cgiOut, "<td><input name='vip%d' type='text' size='16' maxlength='16' style='color:#000000;background-color:#ffffff;' value='%s' /></td>\n", i, vip[i-1]);
		}
		else
		{
			fprintf(cgiOut, "<td><input name='vname%d' type='text' size='18' maxlength='16' style='color:#000000;background-color:#ffffff;' /></td>\n", i);
  		fprintf(cgiOut, "<td><input name='vip%d' type='text' size='16' maxlength='16' style='color:#000000;background-color:#ffffff;' /></td>\n", i);
		}
		
		fprintf(cgiOut, "<td><select name='DvrType%d'>\n", i);
		for (j = 0; j < 17; ++j)
		{
			if (vt[i-1] == j)
				fprintf(cgiOut, "<option value='%d' selected='selected'>%d</option>\n", j, j);
			else
				fprintf(cgiOut, "<option value='%d'>%d</option>\n", j, j);
		}
		fprintf(cgiOut, "</select>\n");
		fprintf(cgiOut, "</td>\n</tr>\n");
	}
	fprintf(cgiOut, "<tr style='COLOR: #0076C8; BACKGROUND-COLOR: #F4FAFF;'><td colspan='4'>&nbsp;</td>\n</tr>\n");
	fprintf(cgiOut, "</table>\n");
}

/* DONE */
static void
show_site()
{
	int  i, j, idx;
	char *desc = "如果没有配置,默认是:0:客厅,1:餐厅,2:主卧,3:次卧,4:厨房,5:卫生间";

	fprintf(cgiOut, "<table width='800' border='0' align='center' bgcolor='#b9d8f3'>\n");
	fprintf(cgiOut, "<tr style='COLOR: #ffffff; text-align:center;'>\n");
	fprintf(cgiOut, "<td colspan=\"4\"><strong>房间信息配置(如：客厅、餐厅等)</strong></strong></td></tr>\n");
 
	for (j = 0; j < 4; ++j)
	{
		fprintf(cgiOut, "<tr style=\"COLOR: #0076C8; BACKGROUND-COLOR: #F4FAFF;\">\n");
  	for (i = 0; i < 4; ++i)
  	{
  		idx = j*4+i;
  		if (idx < locCnt)
  		{
  			fprintf(cgiOut, "<td>%02d：<input name=\"loc%d\" type=\"text\" size=\"10\" maxlength=\"16\" style=\"color:#000000;background-color:#ffffff;\" value=\"%s\" /></td>\n", idx+1, idx+1, loc[idx]);
  		}
  		else
  		{
  			fprintf(cgiOut, "<td>%02d：<input name=\"loc%d\" type=\"text\" size=\"10\" maxlength=\"16\" style=\"color:#000000;background-color:#ffffff;\" /></td>\n", idx+1, idx+1);
  		}
		}
		fprintf(cgiOut, "</tr>\n");
	}
	fprintf(cgiOut, "<tr align='center' style='COLOR: #0076C8; BACKGROUND-COLOR: #F4FAFF;'>\n");
	fprintf(cgiOut, "<td colspan='4' align='left' style='font-weight:bold'>%s</td>\n</tr>\n</table>\n", desc);
}

static void
show_netinfo()
{
}

/* DONE */
static void
show_head()
{
	fprintf(cgiOut, "<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Transitional//EN\" \"http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd\">\n");
	fprintf(cgiOut, "<html xmlns=\"http://www.w3.org/1999/xhtml\">\n");
	fprintf(cgiOut, "<head><meta http-equiv=\"Content-Type\" content=\"text/html; charset=gb2312\" />\n");
	fprintf(cgiOut, "<title>WS-H960智能控制终端基本配置界面</title>\n");
	fprintf(cgiOut, "<script type=\"text/JavaScript\">\n<!--\n");

	fprintf(cgiOut, "function disableNormal(i) {\nvar obj;\n");
	fprintf(cgiOut, "obj = document.getElementsByName('bNr');\n");
	fprintf(cgiOut, "obj(0).disabled = i;\n");
	fprintf(cgiOut, "obj = document.getElementsByName('uNr');\n");
	fprintf(cgiOut, "obj(0).disabled = i;\n");
	fprintf(cgiOut, "obj = document.getElementsByName('rNr');\n");
	fprintf(cgiOut, "obj(0).disabled = i;\n}\n\n");

	fprintf(cgiOut, "function disableVilla(i) {\nvar obj;\n");
	fprintf(cgiOut, "obj = document.getElementsByName('sNr');\n");
	fprintf(cgiOut, "obj(0).disabled = i;\n}\n\n");
	
	fprintf(cgiOut, "function disableElv(i) {\nvar obj;\n");
	fprintf(cgiOut, "obj = document.getElementsByName('elvS');\n");
	fprintf(cgiOut, "obj(0).disabled = i;\n");
	fprintf(cgiOut, "obj = document.getElementsByName('elvT');\n");
	fprintf(cgiOut, "obj(0).disabled = i;\n");
	fprintf(cgiOut, "obj(1).disabled = i;\n}\n\n");

	fprintf(cgiOut, "function fnHandleMtype(i) {\n");
	fprintf(cgiOut, "if (i == 0) {\n");
	fprintf(cgiOut, "disableElv(1);\n");
	fprintf(cgiOut, "disableNormal(1);\n");
	fprintf(cgiOut, "disableVilla(0);\n}\n");
	fprintf(cgiOut, "else if (i == 1) {\n");
	fprintf(cgiOut, "disableElv(0);\n");
	fprintf(cgiOut, "disableNormal(0);\n");
	fprintf(cgiOut, "disableVilla(1);\n}\n}\n\n");
	
	fprintf(cgiOut, "function IsNum(NUM){\nvar i,j,strTemp;\n");
	fprintf(cgiOut, "strTemp=\"0123456789\";\n");
	fprintf(cgiOut, "if ( NUM.length== 0) return false;\n");
	fprintf(cgiOut, "for (i=0;i<NUM.length;i++) {\n");
	fprintf(cgiOut, "j=strTemp.indexOf(NUM.charAt(i));\n");
	fprintf(cgiOut, "if (j==-1) return false;}\n");
	fprintf(cgiOut, "return true;\n}\n\n");

	fprintf(cgiOut, "function checkType(obj) {\n");
	fprintf(cgiOut, "if (!obj.mtype[0].checked&&!obj.mtype[1].checked){\n");
	fprintf(cgiOut, "alert(\"请选择机型！\");\n");
	fprintf(cgiOut, "return false;\n}\n");
	fprintf(cgiOut, "if ((obj.mtype[0].checked&&(!IsNum(obj.bNr.value)||!IsNum(obj.uNr.value)||!IsNum(obj.rNr.value)))||(obj.mtype[1].checked&&!IsNum(obj.sNr.value)))\n");
	fprintf(cgiOut, "{\nalert(\"请输入正确门牌号！\");\nreturn false;\n}\n");
	fprintf(cgiOut, "if (obj.hostid.value !=''&&!IsNum(obj.hostid.value)){\n");
	fprintf(cgiOut, "alert(\"分机号：请输入数字！\");\n");
	fprintf(cgiOut, "return false;\n}\nreturn true;\n}\n\n");
	
	fprintf(cgiOut, "function fnSkip(){\n\tvar o = document.getElementById('oCnt');\n");
	fprintf(cgiOut, "\tvar o1 = document.getElementById('oForm');\n");
	fprintf(cgiOut, "\to.value = 0;\n\to1.submit();\n}\n\n");
	
	fprintf(cgiOut, "function fnClearCfg(){\n");
	fprintf(cgiOut, "\tvar o = document.getElementById('oCnt');\n");
	fprintf(cgiOut, "\tvar o1 = document.getElementById('oForm');\n");
	fprintf(cgiOut, "\to.value = -1;\n");
	fprintf(cgiOut, "\to1.submit();\n}\n\n");

	fprintf(cgiOut, "function fnSubmit(){\n");
	fprintf(cgiOut, "var o = document.getElementById('oForm');\n");
	fprintf(cgiOut, "if (checkType(o) == true)\n");
	fprintf(cgiOut, "{\nvar o1 = document.getElementById('oCnt');\n");
	fprintf(cgiOut, "o1.value = 1;\n");
	fprintf(cgiOut, "o.submit();\n");
	fprintf(cgiOut, "}\n");
	fprintf(cgiOut, "}\n\n");

	fprintf(cgiOut, "-->\n</script>\n</head>\n");
}

/* DONE */
void
show_tail(int stage, int count)
{
	fprintf(cgiOut, "<table width='800' border='0' align='center'>\n");
	fprintf(cgiOut, "<tr style='COLOR: #0076C8; BACKGROUND-COLOR: #F4FAFF;'>\n");
	fprintf(cgiOut, "<td colspan='5'><input type='hidden' name='cstage' value='%d' />", stage);
	fprintf(cgiOut, "<input type='hidden' name='oCnt' id='oCnt' value='%d' /></td></tr>\n", count);
	fprintf(cgiOut, "<tr style='COLOR: #0076C8; BACKGROUND-COLOR: #F4FAFF;'>\n");
	fprintf(cgiOut, "<td align='center'></td>\n");
	fprintf(cgiOut, "<td align='center'><input type='button' value='清空配置' onclick='fnClearCfg()' /></td>\n");
	fprintf(cgiOut, "<td align='center'><input type='button' value='跳过此步' onclick='fnSkip()' /></td>\n");
	fprintf(cgiOut, "<td align='center'><input type='button' value='保存配置' onclick='fnSubmit()' /></td>\n");
	fprintf(cgiOut, "<td align='center'></td>\n");
	fprintf(cgiOut, "</tr>\n</table>\n");
}

/* DONE */
void
show_basic_page()
{
	read_general();
	read_host_conf();
	read_loc();
	read_vod();
//	read_nconf();

	cgiHeaderContentType("text/html");
	show_head();
	fprintf(cgiOut, "<body>\n<form name='oForm' id='oForm' method='post' action='/cgi-bin/set.cgi'>\n");
	show_general();
	show_site();
	show_vod();
	show_tail(1, 0);
	fprintf(cgiOut, "</form>\n</body>\n</html>\n");
}

/* DONE */
void
do_error(char *s)
{
	cgiHeaderContentType("text/html");
	fprintf(cgiOut, "<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Transitional//EN\" \"http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd\">\n");
	fprintf(cgiOut, "<html xmlns=\"http://www.w3.org/1999/xhtml\">\n");
	fprintf(cgiOut, "<head><meta http-equiv=\"Content-Type\" content=\"text/html; charset=gb2312\" />\n");
	fprintf(cgiOut, "<TITLE>发生错误</TITLE></HEAD>\n");
	fprintf(cgiOut, "<BODY>\n");
	fprintf(cgiOut, "<br><br>\n");
	fprintf(cgiOut, "<p><CENTER><H1>%s</H1></CENTER></p>\n", s);
	fprintf(cgiOut, "</BODY></HTML>\n");
}

#if 0
}
#endif

//////////////////////////////////////
//        save config file          //
//////////////////////////////////////
#if 0
{
#endif

/* DONE */
static void
save_general()
{
	FILE *fp = NULL;
	if ((fp = fopen(GCONF, "w")) == NULL) return;
	fprintf(fp, "ScrSvr = %d\n", scr_svr);	
	fprintf(fp, "ElvSt = %d\n", elv_s);
	fprintf(fp, "PanNr = %d\n", elv_t);
	fprintf(fp, "Cam = %d\n", cam);
	fflush(fp);
	fclose(fp);
}

/* DONE */
static void
save_host_conf()
{
	FILE  *fp = NULL;

	if ((fp = fopen(HCONF, "w")) == NULL) return;

	fprintf(fp, "%d:%u:%d\n", mtype, _nr, hostid);

	fflush(fp);
	fclose(fp);
}

/* DONE */
static void
save_site()
{
	FILE   *fp = NULL;
	int     i;

	if (LocCnt == 0)
	{
		unlink(SCONF);
		return;
	}

	if ((fp = fopen(SCONF, "w")) == NULL) return;
	for (i = 0; i < LocCnt; ++i)
	{
		fprintf(fp, "%s\n", Loc[i]);
	}
	fflush(fp);
	fclose(fp);
}

/* DONE */
static void
save_vod()
{
	FILE   *fp = NULL;
	int     i;

	if (new_vcnt == 0)
	{
		unlink(VCONF);
		return;
	}

	if ((fp = fopen(VCONF, "w")) == NULL) return;
	for (i = 0; i < new_vcnt; ++i)
	{
		fprintf(fp, "%d:%s=%s\n", new_vt[i], new_vn[i], new_vi[i]);
	}
	fflush(fp);
	fclose(fp);
}

#if 0
}
#endif

//////////////////////////////////////
//       get info from client       //
//////////////////////////////////////
#if 0
{
#endif
/* DONE */
static int
get_general()
{
	char  str[8];
	int   x;
	char *vf[2] = {"0", "1"};
	int   ret = 0;

	memset(str, 0, 8);
	if (cgiFormString("scrsvr", str, 4) == cgiFormSuccess)
	{
		if (strlen(str))
			scr_svr = atoi(str);
	}
	if (scr_svr != g_scrsvr)
		ret++;

	if (cgiFormRadio("mtype", vf, 2, &x, 0) == cgiFormSuccess)
		mtype = x;
	if (mtype == 0)
	{
		memset(str, 0, 8);
		if (cgiFormString("sNr", str, 8) == cgiFormSuccess)
			_nr = atoi(str);
	}
	else if (mtype == 1)
	{
		memset(str, 0, 8);
		if (cgiFormString("bNr", str, 8) == cgiFormSuccess)
			_bnr = atoi(str);
		memset(str, 0, 8);
		if (cgiFormString("uNr", str, 8) == cgiFormSuccess)
			_unr = atoi(str);
		memset(str, 0, 8);
		if (cgiFormString("rNr", str, 8) == cgiFormSuccess)
			_rnr = atoi(str);
		_nr = make_nr(_bnr, _unr, _rnr);
	}
	memset(str, 0, 8);
	if (cgiFormString("hostid", str, 4) == cgiFormSuccess)
		hostid = atoi(str);
	
	if (mtype != g_mtype||hostid != g_hostid||_nr != g_nr)
	{
		ret++;
		host++;
	}

	if (mtype == 1)
	{
		memset(str, 0, 8);
		if (cgiFormString("elvS", str, 4) == cgiFormSuccess)
			elv_s = atoi(str);

		if (cgiFormRadio("elvT", vf, 2, &x, 0) == cgiFormSuccess)
			elv_t = x;
	
		if (elv_s != g_elvs||elv_t != g_elvt)
			ret++;
	}
	
	if (cgiFormRadio("camT", vf, 2, &x, 0) == cgiFormSuccess) {
		cam = x;
		if (cam != g_cam)
			ret++;
	}

	return ret;
}

/* DONE */
static int
if_loc_changed()
{
	int i;
	if (locCnt != LocCnt) return 1;

	for (i=0; i<LocCnt; i++)
	{
		if (strcmp(Loc[i], loc[i]) != 0)
			return 1;
	}
	return 0;
}

/* DONE */
static int
get_site()
{
	int  i;
	char *ptr;
	char str[16];
	char name[8];

	for (i = 0; i < 16; ++i)
	{
		memset(str, 0, 16);
		memset(name, 0, 8);
		sprintf(name, "loc%d", i+1);
		if (cgiFormString(name, str, 16) == cgiFormSuccess)
		{
			ptr = trim(str);
			if ((ptr!=NULL)&&(strlen(ptr) > 0))
			{
				memset(Loc[i], 0, 16);
				strncpy(Loc[i], ptr, 15);
			}
		}
		else
			break;
	}
	LocCnt = i;
	return if_loc_changed();
}

/* DONE */
static int
if_vod_changed()
{
	int i, j = 0;
	if (new_vcnt != vcnt) return 1;
	for (i = 0; i < new_vcnt; i++)
	{
		if (strcmp(new_vn[i], vname[i]) == 0)
		{
			if (strcmp(new_vi[i], vip[i]) == 0)
			{
				if (new_vt[i] == vt[i])
					j++;
			}
		}
	}
	if (j == new_vcnt)return 0;
	return 1;
}

/* DONE */
static int
get_vod()
{
	int  i;
	int  x;
	char *vf[17] = {"0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "10", "11", "12", "13", "14", "15", "16"};
	char str[16];
	char name[16];

	new_vcnt = 0;
	for (i = 0; i < VOD_MAX; ++i)
	{
		memset(str, 0, 16);
		memset(name, 0, 16);
		sprintf(name, "vname%d", i+1);
		if (cgiFormString(name, str, 16) == cgiFormSuccess)
		{
			memset(new_vn[i], 0, 16);
			strcpy(new_vn[i], str);

			memset(str, 0, 16);
			memset(name, 0, 16);
			sprintf(name, "vip%d", i+1);
			if (cgiFormString(name, str, 16) == cgiFormSuccess)
			{
				memset(new_vi[i], 0, 16);
				strcpy(new_vi[i], str);

				memset(name, 0, 16);
				sprintf(name, "DvrType%d", i+1);
				if (cgiFormSelectSingle(name, vf, 17, &x, 0) == cgiFormSuccess)
				{
					new_vt[i] = x;
					new_vcnt++;
				}
			}
		}
	}
	return if_vod_changed();
}

#if 0
}
#endif

//////////////// cgi in ////////////////////
#if 0
{
#endif

/* DONE */
int
get_count()
{
	char str[8] = {0};
	if (cgiFormString("oCnt", str, 8) == cgiFormSuccess)	
		return atoi(str);
	else
		return -2;
}

/* DONE */
static void
clear_general()
{
	int i;
	int ret = 0;

	for (i=0; i<sizeof(fname)/sizeof(char*); ++i)
	{
		if (access(fname[i], F_OK) == 0)
		{
			unlink(fname[i]);
			++ret;
		}
	}
	if (ret > 0)
		set_reboot_flag();
}

/* DONE */
static void
handle_general()
{
	int ret = 0;

	if (get_general() > 0)
	{
		save_general();
		if (host > 0)
			save_host_conf();
		++ret;
	}

	if (get_site() > 0)
	{
		save_site();
		++ret;
	}

	if (get_vod() > 0)
	{
		save_vod();
		++ret;
	}

	if (ret > 0)
		set_reboot_flag();
}

/* DONE */
void
handle_basic()
{
	int cnt;

	cnt = get_count();
	if (cnt < -1)
		return;
	else if (cnt == -1)/* clear config */
	{
		clear_general();
		show_basic_page();
		return;
	}
	else if (cnt == 0)/* skip it */
	{
		show_sec_page();
		return;
	}

	read_general();
	read_host_conf();
	read_loc();
	read_vod();
	handle_general();
	show_sec_page();
}

#if 0
}
#endif

