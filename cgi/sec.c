#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <ctype.h>
#include <arpa/inet.h>
#include "cgic.h"
#include "basic.h"
#include "cntl.h"

#define  ZCONF   "/opt/conf/zone.conf"

/* security info from client */
static unsigned char new_za[16] = {0};
static unsigned char new_zt[16] = {0};
static unsigned char new_zl[16] = {0};
static unsigned char new_sl[16] = {0};
static unsigned char new_ll[16] = {0};
static char new_zn[16][16];
static int  new_zcnt = 0;
static int  new_ebns = 0;
static int  i_mod;

/* security info read from config file */
static unsigned char sAddr[16] = {0};
static unsigned char dType[16] = {0};
static unsigned char lnkage[16] = {0};
static unsigned char slnk[16] = {0};
static unsigned char lloc[16] = {0};
static char zname[16][16];
static int  zcnt = 0;
static int  ebns = 0;
static int  g_mod;

static int  unknown = 0;
static char *rstr[2] = {"您没有修改任何配置,点击'继续'将进入室内控制配置界面,点击'完成'将关闭浏览器.",
	"设置成功,点击'继续'将进入室内控制配置界面,点击'完成'系统将自动重启."};

static void
read_security()
{
	FILE  *fp;
	char  *ptr;
	char   buf[256];
	char   temp[96];
	int    addr;
	int    type;
	int    linkage;
	int    loc;
	int    mod;
	int    k = 0;
	int    ret;

	memset(sAddr, 0, 16);
	memset(dType, 0, 16);
	memset(lnkage, 0, 16);
	memset(slnk, 0, 16);
	memset(lloc, 0, 16);
	memset(zname, 0, 16*16);

	if ((fp = fopen(ZCONF, "r")) == NULL) return;

	fgets(buf, 256, fp);
	ptr = buf;
	while (*ptr != '\0'&&isspace(*ptr))ptr++;
	if (isdigit(*ptr))
		g_mod = atoi(ptr);

	while(!feof(fp)&&k < 16)
	{
		memset(buf, '\0', 256);
		memset(temp, '\0', 96);
	
		fgets(buf, 256, fp);
		ptr = buf;

		while (*ptr != '\0'&&isspace(*ptr)) ptr++;
		if (!isdigit(*ptr)) continue;
		if (strchr(ptr, ':') == NULL) continue;
		ret = sscanf(ptr, "%d:%d:%d:%d:%d:%s", &addr, &type, &linkage, &mod, &loc, temp);
		if (ret == 6)
		{
			sAddr[k] = (unsigned char)addr;
			dType[k] = (unsigned char)type;
			lnkage[k] = (unsigned char)linkage;
			slnk[k] = (unsigned char)mod;
			lloc[k] = (unsigned char)loc;
			ptr = trim(temp);
			if (ptr != NULL)
				strncpy(zname[k], ptr, 15);
			++k;
		}
		else if (ret == 2)
		{
			g_mod = addr;
			ebns = type;
		}
		else if (ret == 1)
		{
			g_mod = addr;
			ebns = 0;
		}
	}
	zcnt = k;
	fclose(fp);
}

static void
save_security()
{
	FILE *fp = NULL;
	int   i;

	if (new_zcnt == 0) return;
	if ((fp = fopen(ZCONF, "w")) == NULL) return;
	fprintf(fp, "%d:%d\n", i_mod, new_ebns);
	for (i = 0; i < new_zcnt; ++i)
	{
		fprintf(fp, "%d:%d:%d:%d:%d:%s\n", new_za[i], new_zt[i], new_zl[i], new_sl[i], new_ll[i], new_zn[i]);
	}
	fflush(fp);
	fclose(fp);
}

////////////////////////////////////////
///////          cgi out       /////////
////////////////////////////////////////
#if 0
{
#endif
/* DONE */
static void
show_dtype_option(int idx)
{
	int  i;
	char *v[8] = {
		"紧急按钮",
		"瓦斯探测器",
		"烟感",
		"红外",
		"门磁",
		"窗磁",
		"振动探测器",
		"其它"
		};

	if (g_mod == 0)
		fprintf(cgiOut, "<td align=\"center\"><select name=\"dtorT%d\">\n", idx);
	else
		fprintf(cgiOut, "<td align=\"center\"><select name=\"dtorT%d\" disabled>\n", idx);
	for (i = 0; i < 8; ++i)
	{
		if (idx < zcnt&&i == dType[idx])
			fprintf(cgiOut, "<option value=\"%d\" selected=\"selected\" >%s</option>\n", i, v[i]);
		else 
			fprintf(cgiOut, "<option value=\"%d\">%s</option>\n", i, v[i]);
	}
  fprintf(cgiOut, "</select></td>\n");
}

/* DONE */
static void
show_linkage_option(int idx)
{
	int  i;
	char *v[2] = {"无联动", "灯光联动"};

	if (g_mod == 0)
		fprintf(cgiOut, "<td align='center'><select name='linkage%d'>\n", idx);
	else
		fprintf(cgiOut, "<td align='center'><select name='linkage%d' disabled>\n", idx);
	for (i = 0; i < 2; ++i)
	{
		if (idx < zcnt&&i == lnkage[idx])
			fprintf(cgiOut, "<option value='%d' selected='selected' >%s</option>\n", i, v[i]);
		else
			fprintf(cgiOut, "<option value='%d'>%s</option>\n", i, v[i]);
	}
  fprintf(cgiOut, "</select></td>\n");
}

/* DONE */
static void
show_scelnk_option(int idx)
{
	int  i;
	char *v[] = {"无联动", "休闲模式", "就餐模式", "工作模式", "电视模式", "浪漫模式", "派对模式", "会客模式"};

	if (g_mod == 0)
		fprintf(cgiOut, "<td align='center'><select name='slnk%d'>\n", idx);
	else
		fprintf(cgiOut, "<td align='center'><select name='slnk%d' disabled>\n", idx);
	for (i = 0; i < sizeof(v)/sizeof(char*); ++i)
	{
		if (idx < zcnt&&i == slnk[idx])
			fprintf(cgiOut, "<option value='%d' selected='selected' >%s</option>\n", i, v[i]);
		else
			fprintf(cgiOut, "<option value='%d'>%s</option>\n", i, v[i]);
	}
  fprintf(cgiOut, "</select></td>\n");
}

/* DONE */
static void
show_lloc_option(int idx)
{
	int  i;
	
	if (g_mod == 0)
		fprintf(cgiOut, "<td align='center'><select name='lloc%d'>\n", idx);
	else
		fprintf(cgiOut, "<td align='center'><select name='lloc%d' disabled>\n", idx);
	for (i = 0; i < g_locCnt; ++i)
	{
		if (idx < zcnt&&i == lloc[idx])
			fprintf(cgiOut, "<option value='%d' selected='selected' >%s</option>\n", i, g_loc[i]);
		else
			fprintf(cgiOut, "<option value='%d'>%s</option>\n", i, g_loc[i]);
	}
  fprintf(cgiOut, "</select></td>\n");
}

/* DONE */
static void
show_sec_cfg_row(int idx)
{
	fprintf(cgiOut, "<tr style=\"COLOR: #0076C8; BACKGROUND-COLOR: #F4FAFF;\"><td align=\"center\">\n");
	if (idx < zcnt)
		fprintf(cgiOut, "<input name=\"addr%d\" type=\"text\" size=\"4\" maxlength=\"4\" style=\"color:#000000;background-color:#ffffff;\" value=\"%d\" /></td>\n", idx, sAddr[idx]);
	else
		fprintf(cgiOut, "<input name=\"addr%d\" type=\"text\" size=\"4\" maxlength=\"4\" style=\"color:#000000;background-color:#ffffff;\" /></td>\n", idx);

	show_dtype_option(idx);
	show_linkage_option(idx);
	show_scelnk_option(idx);
	show_lloc_option(idx);
	fprintf(cgiOut, "<td align=\"center\">\n");
	if ((idx < zcnt)&&(strlen(zname[idx]) > 0))
		fprintf(cgiOut, "<input name=\"zname%d\" type=\"text\" size=\"12\" maxlength=\"16\" style=\"color:#000000;background-color:#ffffff;\" value=\"%s\" /></td></tr>\n", idx, zname[idx]);
	else
		fprintf(cgiOut, "<input name=\"zname%d\" type=\"text\" size=\"12\" maxlength=\"16\" style=\"color:#000000;background-color:#ffffff;\" /></td></tr>\n", idx);
}

/* DONE */
static void
show_security()
{
	int  i;

  fprintf(cgiOut, "<table width='800' border='0' align='center' bgcolor='#b9d8f3'>\n");
  fprintf(cgiOut, "<tr style='COLOR: #ffffff; text-align:center;'>\n");
  fprintf(cgiOut, "<td colspan=\"6\"><h3><strong>安防报警配置</strong></h3></td></tr>\n");
  fprintf(cgiOut, "<tr style='COLOR: #0076C8; BACKGROUND-COLOR: #F4FAFF;'>\n");
	fprintf(cgiOut, "<td align='center'>\n");
	fprintf(cgiOut, "<select name='oMod' onchange='fnModSelect(this)'>\n");
	if (g_mod == 0)
	{
		fprintf(cgiOut, "<option value='0' selected>本地模式</option>\n");
		fprintf(cgiOut, "<option value='1'>网络模式</option>\n");
	}
	else
	{
		fprintf(cgiOut, "<option value='0'>本地模式</option>\n");
		fprintf(cgiOut, "<option value='1' selected>网络模式</option>\n");
	}
	fprintf(cgiOut, "</select>\n</td>\n");
	
	if (ebns == 0)  
  	fprintf(cgiOut, "<td><label><input type='checkbox' name='oEbnSiren' />紧急按钮触发警报声音</label></td>\n");
  else
  	fprintf(cgiOut, "<td><label><input type='checkbox' name='oEbnSiren' checked />紧急按钮触发警报声音</label></td>\n");
	
	fprintf(cgiOut, "<td colspan='4'>\n</td>\n</tr>\n");
  fprintf(cgiOut, "<tr style='COLOR: #0076C8; BACKGROUND-COLOR: #F4FAFF;'>\n");
  fprintf(cgiOut, "<td align=\"center\">报警模块地址</td>\n");
	fprintf(cgiOut, "<td align=\"center\">探测器类型</td>\n");
	fprintf(cgiOut, "<td align=\"center\">灯光联动</td>\n");
	fprintf(cgiOut, "<td align=\"center\">场景联动</td>\n");
	fprintf(cgiOut, "<td align=\"center\">联动场所</td>\n");
	fprintf(cgiOut, "<td align=\"center\">防区名称</td></tr>\n");

	for (i = 0; i < 16; ++i)
	{
		show_sec_cfg_row(i);
	}

	fprintf(cgiOut, "<tr align=\"center\" style=\"COLOR: #0076C8; BACKGROUND-COLOR: #F4FAFF;\">\n");
  fprintf(cgiOut, "<td colspan=\"6\" align=\"left\" style=\"font-weight:bold\">安防报警配置说明： <br/>\n");
  fprintf(cgiOut, "上面表格每一行代表一个防区，第一行代表1号防区，依次递增。 </td>\n");
	fprintf(cgiOut, "</tr></table>\n");
}

/* DONE */
static void
show_head()
{
	fprintf(cgiOut, "<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Transitional//EN\" \"http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd\">\n");
	fprintf(cgiOut, "<html xmlns=\"http://www.w3.org/1999/xhtml\">\n");
	fprintf(cgiOut, "<head><meta http-equiv=\"Content-Type\" content=\"text/html; charset=gb2312\" />\n");
	fprintf(cgiOut, "<title>WS-H960智能控制终端安防配置界面</title>\n");
	fprintf(cgiOut, "<script type=\"text/JavaScript\">\n");
	fprintf(cgiOut, "<!--\n");
	fprintf(cgiOut, "function fnSkip(){\n\tvar o = document.getElementById('oCnt');\n\to.value = 0;\n\toForm.submit();\n}\n\n");
	fprintf(cgiOut, "function fnClearCfg(){\n");
	fprintf(cgiOut, "\tvar o = document.getElementById('oCnt');\n");
	fprintf(cgiOut, "\to.value = -1;\n");
	fprintf(cgiOut, "\toForm.submit();\n}\n\n");
	fprintf(cgiOut, "function fnSubmit(){\n\toForm.submit();\n}\n\n");
	
	fprintf(cgiOut, "function disable_info(v){\nvar o, id, i;\n");
	fprintf(cgiOut, "for (i=0; i<16; ++i){\n");
	fprintf(cgiOut, "id = 'dtorT'+String(i);\n");
	fprintf(cgiOut, "o = document.getElementsByName(id);\n");
	fprintf(cgiOut, "o[0].disabled = v;\n");
	fprintf(cgiOut, "id = 'linkage'+String(i);\n");
	fprintf(cgiOut, "o = document.getElementsByName(id);\n");
	fprintf(cgiOut, "o[0].disabled = v;\n");
	fprintf(cgiOut, "id = 'slnk'+String(i);\n");
	fprintf(cgiOut, "o = document.getElementsByName(id);\n");
	fprintf(cgiOut, "o[0].disabled = v;\n");
	fprintf(cgiOut, "id = 'lloc'+String(i);\n");
	fprintf(cgiOut, "o = document.getElementsByName(id);\n");
	fprintf(cgiOut, "o[0].disabled = v;\n}\n}\n\n");

	fprintf(cgiOut, "function fnModSelect(o){\n");
	fprintf(cgiOut, "if (o.selectedIndex == 0)\ndisable_info(0);\n");
	fprintf(cgiOut, "else\ndisable_info(1);\n}\n\n");
	fprintf(cgiOut, "--></script>\n</head>\n");
}

/* DONE */
void
show_sec_page()
{
	read_site(1);
	read_security();
	cgiHeaderContentType("text/html");
	show_head();
	fprintf(cgiOut, "<body>\n<form name='oForm' id='oForm' method='post' action='/cgi-bin/set.cgi'>\n");
	show_security();
	show_tail(2, g_locCnt+1);
	fprintf(cgiOut, "</form>\n</body>\n</html>\n");
}
#if 0
}
#endif

//////////////////////////////////////////
//////////   get info from client  ///////
//////////////////////////////////////////
#if 0
{
#endif
/* DONE */
static int
get_zone_addr(int idx)
{
	char str[4];
	char name[8];

	memset(str, 0, 4);
	memset(name, 0, 8);
	sprintf(name, "addr%d", idx);
	if (cgiFormString(name, str, 4) == cgiFormSuccess)
	{
		new_za[idx] = (unsigned char)atoi(str);
		return 1;
	}
	else
		return 0;
}

/* DONE */
static int
get_detector_type(int idx)
{
	int   x;
	char  name[10];
	char *br[8] = {"0", "1", "2", "3", "4", "5", "6", "7"};

	memset(name, 0, 10);
	sprintf(name, "dtorT%d", idx);
	if (cgiFormSelectSingle(name, br, 8, &x, 0) == cgiFormSuccess)
	{
		new_zt[idx] = (unsigned char)x;
		return 1;
	}
	else
		return 0;
}

/* DONE */
static int
get_lnkage_type(int idx)
{
	int   x;
	char  name[12];
	char *br[2] = {"0", "1"};

	memset(name, 0, 12);
	sprintf(name, "linkage%d", idx);
	if (cgiFormSelectSingle(name, br, 2, &x, 0) == cgiFormSuccess)
	{
		new_zl[idx] = (unsigned char)x;
		return 1;
	}
	else
		return 0;
}

/* DONE */
static int
get_scene_lnkage(int idx)
{
	int   x;
	char  name[12];
	char *br[] = {"0", "1", "2", "3", "4", "5", "6", "7"};

	memset(name, 0, 12);
	sprintf(name, "slnk%d", idx);
	if (cgiFormSelectSingle(name, br, sizeof(br)/sizeof(char*), &x, 0) == cgiFormSuccess)
	{
		new_sl[idx] = (unsigned char)x;
		return 1;
	}
	else
		return 0;
}

/* DONE */
static int
get_scene_lloc(int idx)
{
	int   x;
	char  name[12];
	char *br[] = {"0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "10", "11", "12", "13", "14", "15"};

	memset(name, 0, 12);
	sprintf(name, "lloc%d", idx);
	if (cgiFormSelectSingle(name, br, sizeof(br)/sizeof(char*), &x, 0) == cgiFormSuccess)
	{
		new_ll[idx] = (unsigned char)x;
		return 1;
	}
	else
		return 0;
}

/* DONE */
static int
get_zone_name(int idx)
{
	char str[16];
	char name[12];

	memset(str, 0, 16);
	memset(name, 0, 12);
	sprintf(name, "zname%d", idx);
	if (cgiFormString(name, str, 16) == cgiFormSuccess)
	{
		memset(new_zn[idx], 0, 16);
		strncpy(new_zn[idx], str, 15);
		return 1;
	}
	else
		return 0;
}

/* DONE */
static int
get_zone_inf(int idx)
{
	if (get_zone_addr(idx) == 0) return 0;
	if (i_mod == 0)
	{
		if (get_detector_type(idx) == 0) return 0;
		if (get_lnkage_type(idx) == 0) return 0;
		if (get_scene_lnkage(idx) == 0) return 0;
		if (get_scene_lloc(idx) == 0) return 0;
	}
	if (get_zone_name(idx) == 0)
	{
		memset(new_zn[idx], 0, 16);
		snprintf(new_zn[idx], 15, "未命名%d", unknown++);
	}
	return 1;
}

/* DONE */
static int
if_sec_chged()
{
	int i;

	if (zcnt != new_zcnt||i_mod != g_mod) return 1;
	if (memcmp(new_za, sAddr, new_zcnt) != 0) return 1;
	if (memcmp(new_zt, dType, new_zcnt) != 0) return 1;
	if (memcmp(new_zl, lnkage, new_zcnt) != 0) return 1;
	if (memcmp(new_sl, slnk, new_zcnt) != 0) return 1;
	if (memcmp(new_ll, lloc, new_zcnt) != 0) return 1;
	for (i = 0; i < new_zcnt; ++i)
	{
		if (strcmp(new_zn[i], zname[i]) != 0) return 1;
	}
	return 0;
}

/* DONE */
static int
get_security()
{
	int  i;
	int  x = 0;
	char *m[] = {"0", "1"};

	cgiFormSelectSingle("oMod", m, 2, &x, 0);
	i_mod = x;
	if (cgiFormCheckboxSingle("oEbnSiren") == cgiFormSuccess)
		new_ebns = 1;
	else
		new_ebns = 0;

	for (i = 0; i < 16; ++i)
	{
		if (get_zone_inf(i) == 0) break;
	}
	new_zcnt = i;
	return if_sec_chged();
}
#if 0
}
#endif

/* DONE */
static void
do_anymore(int ret)
{
	int i;
	i = (ret == 0) ? 0 : 1;
	cgiHeaderContentType("text/html");
	fprintf(cgiOut, "<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Transitional//EN\" \"http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd\">\n");
	fprintf(cgiOut, "<html xmlns=\"http://www.w3.org/1999/xhtml\">\n");
	fprintf(cgiOut, "<head><meta http-equiv=\"Content-Type\" content=\"text/html; charset=gb2312\" />\n");
	fprintf(cgiOut, "<TITLE>是否继续</TITLE></HEAD>\n");
	fprintf(cgiOut, "<BODY>\n");
	fprintf(cgiOut, "<form name='oForm' method='post' action='/cgi-bin/set.cgi' >\n");
	fprintf(cgiOut, "<table width='800' border='0' align='center' cellpadding='0' cellspacing='0' bgcolor='#b9d8f3'>\n");
	fprintf(cgiOut, "<tr style='color:#ffffff;text-align:center;'>\n");
	fprintf(cgiOut, "<td colspan='4'><h1><strong>是否继续</strong></h1></td></tr>\n"); 
	fprintf(cgiOut, "<tr style='COLOR: #0076C8; BACKGROUND-COLOR: #F4FAFF;'>\n");
	fprintf(cgiOut, "<td width='186'>&nbsp;</td>\n");
	fprintf(cgiOut, "<td colspan='2'><h2>%s</h2></td>\n", rstr[i]);
	fprintf(cgiOut, "<td width='203'>&nbsp;</td>\n</tr>\n");
	fprintf(cgiOut, "<tr style='COLOR: #0076C8; BACKGROUND-COLOR: #F4FAFF;'>\n");
	fprintf(cgiOut, "<td colspan='4'><input type='hidden' name='cstage' value='3' /></td></tr>\n");
	fprintf(cgiOut, "<tr style='COLOR: #0076C8; BACKGROUND-COLOR: #F4FAFF;'>\n");
	fprintf(cgiOut, "<td>&nbsp;</td>\n");

	fprintf(cgiOut, "<td align='center'><input type='submit' name='next' value='继续' /></td>\n");

	if (i == 0)
  		fprintf(cgiOut, "<td align='center'><input type='button' value='完成' onclick='window.close()' /></td>\n");
	else
  		fprintf(cgiOut, "<td align='center'><input type='submit' name='finish' value='完成' /></td>\n");
	fprintf(cgiOut, "<td align='center'>&nbsp;</td>\n");
	fprintf(cgiOut, "</tr>\n</table>\n</form>\n");
	fprintf(cgiOut, "</BODY></HTML>\n");
}

/* DONE */
static void
clear_security()
{
	if (access(ZCONF, F_OK) == 0)
	{
		unlink(ZCONF);
		set_reboot_flag();
	}
}

/* DONE */
void
handle_security()
{
	int cnt;
	int ret = 0;
	cnt = get_count();
	if (cnt < -1)
		return;
	else if (cnt == -1)/* clear config */
	{
		clear_security();
		show_sec_page();
		return;
	}
	else if (cnt == 0)/* skip it */
	{
		ret = get_reboot_flag();
		do_anymore(ret);
		return;
	}

	unknown = 0;
	read_security();
	if (get_security() > 0)
	{
		save_security();
		set_reboot_flag();
	}
	ret = get_reboot_flag();
	do_anymore(ret);
}

