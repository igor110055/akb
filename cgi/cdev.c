#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <ctype.h>
#include "cgic.h"
#include "cntl.h"

typedef struct _cdev {
	unsigned char bid;
	unsigned char addr;
	unsigned char addr1;
	unsigned char reserved;
} cdev_t;

static cdev_t  g_cdev[4][LOC_MAX];
static unsigned char g_cmod[4];

static cdev_t  r_cdev[4][LOC_MAX];
static unsigned char r_cmod[4];

char  g_loc[LOC_MAX][16];
int   g_locCnt = 0;

static char *defName[] = {"客厅", "餐厅", "主卧", "次卧", "厨房", "卫生间"};

char*
trim(char *str)
{
	int   n;
	char *ptr;

	if (str == NULL) return NULL;
	ptr = str;
	while(*ptr != '\0'&&isspace(*ptr))
		ptr++;
	n = strlen(ptr);
	while (n > 0)
	{
		if (isspace(ptr[n-1]))
		{
			ptr[n-1] = '\0';
			n--;
		}
		else
			break;
	}

	if (n == 0)
		return NULL;
	else
		return ptr;
}

//////////////////////////////////////
/* DONE */
static void
init_arr()
{
	int i;
	for (i=0; i<4; ++i)
		memset(&g_cdev[i][0], 0xff, LOC_MAX*sizeof(cdev_t));
	for (i=0; i<LOC_MAX; ++i)
		memset(g_loc[i], 0, 16);
}

/* DONE */
static void
def_site()
{
	int i;
	g_locCnt = sizeof(defName)/sizeof(char*);
	for (i = 0; i < g_locCnt; ++i)
		strcpy(g_loc[i], defName[i]);
}

/* DONE */
void
read_site(int flag)
{
	FILE  *fp;
	char   buf[64];
	char  *p;
	int    i;

	if ((fp = fopen(SCONF, "r")) == NULL)
	{
		if (flag == 1)
			def_site();
		return;
	}

	i = 0;
	while(!feof(fp)&&i < 16)
	{
		memset(buf, '\0', 64);
		fgets(buf, 64, fp);
		p = buf;
		while (*p != '\0'&&isspace(*p))
			++p;
		if (*p == '\0') continue;
		if((p[0]=='#')||((p[0]=='/')&&(p[1]=='/'))) continue;

		strncpy(g_loc[i], p, 15);
		++i;
	}

	g_locCnt = i;
	fclose(fp);
	if (flag == 1&&g_locCnt == 0)
		def_site();
}

/* DONE */
static void
load_cdev_data()
{
	int   fd;
	int   i;
	struct stat  st;

	for (i=0; i<4; i++)
	{
		memset(&g_cdev[i][0], 0, sizeof(cdev_t)*LOC_MAX);
	}

	if (stat(DEVCNF, &st) < 0) return;
	if (st.st_size != (LOC_MAX*4*sizeof(cdev_t)+4)) return;
	if ((fd = open(DEVCNF, O_RDONLY)) < 0) return;
	for (i=0; i<4; i++)
	{
		read(fd, &g_cmod[i], 1);
		read(fd, &g_cdev[i][0], sizeof(cdev_t)*LOC_MAX);
	}
}

/* DONE */
static void
store_cdev_data()
{
	int fd;
	int i;
	
	fd = open(DEVCNF, O_WRONLY|O_CREAT);
	if (fd < 0) return;
	for (i=0; i<4; ++i)
	{
		write(fd, &r_cmod[i], 1);
		write(fd, &r_cdev[i][0], sizeof(cdev_t)*LOC_MAX);
	}
	fdatasync(fd);
	close(fd);
}

/////////////////////////////////////////////////
//             cgi out handler                 //
/////////////////////////////////////////////////
#if 0
{
#endif

/* DONE */
static void
show_head(void)
{
	fprintf(cgiOut, "<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Transitional//EN\" \"http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd\">\n");
	fprintf(cgiOut, "<html xmlns=\"http://www.w3.org/1999/xhtml\">\n");
	fprintf(cgiOut, "<head>\n<meta http-equiv=\"Content-Type\" content=\"text/html; charset=gb2312\" />\n");
	fprintf(cgiOut, "<title>设备控制信息配置</title>\n");
	fprintf(cgiOut, "<script type=\"text/JavaScript\">\n");
	fprintf(cgiOut, "<!--\n");

	fprintf(cgiOut, "function fnSkip(){\n\tvar o = document.getElementById('oCnt');\n\to.value = 0;\n\toForm.submit();\n}\n\n");
	
	fprintf(cgiOut, "function fnClearCfg(){\n");
	fprintf(cgiOut, "\tvar o = document.getElementById('oCnt');\n");
	fprintf(cgiOut, "\to.value = -1;\n");
	fprintf(cgiOut, "\toForm.submit();\n}\n\n");
	
	fprintf(cgiOut, "function disableBid(i,v) {\n");
	fprintf(cgiOut, "var o,name,j;\nfor(j=0;j<%d;j++){\n", g_locCnt);
	fprintf(cgiOut, "name = 'oBid'+String(j)+String(i);\n");
	fprintf(cgiOut, "o=document.getElementsByName(name);\n");
	fprintf(cgiOut, "o[0].disabled = v;\n}\n}\n\n");
	
	fprintf(cgiOut, "function fnChangeMode(o,k){\n");
	fprintf(cgiOut, "if (o.selectedIndex == 0) {\n");
	fprintf(cgiOut, "disableBid(k,true);\n} else {\n");
	fprintf(cgiOut, "disableBid(k,false);\n}\n}\n\n");

	fprintf(cgiOut, "function fnSubmit(){\n\toForm.submit();\n}\n\n");

	fprintf(cgiOut, "--></script>\n</head>\n");
}

/* DONE */
static void
show_title()
{
	fprintf(cgiOut, "<tr style='COLOR: #0076C8; BACKGROUND-COLOR: #F4FAFF;'>\n");
  fprintf(cgiOut, "<td></td>\n");
  fprintf(cgiOut, "<td colspan='2' align='center'>窗帘</td>\n");
  fprintf(cgiOut, "<td colspan='2' align='center'>地暖</td>\n");
  fprintf(cgiOut, "<td colspan='2' align='center'>新风</td>\n");
  fprintf(cgiOut, "<td colspan='2' align='center'>空调</td>\n</tr>\n");
}

/* DONE */
static void
show_boardid_row()
{
	int j;
	fprintf(cgiOut, "<tr style='COLOR: #0076C8; BACKGROUND-COLOR: #F4FAFF;'>\n");
  fprintf(cgiOut, "<td>控制模式</td>\n");
  for (j=0; j<4; ++j)
  {
  	fprintf(cgiOut, "<td colspan='2' align='center'>\n");
  	fprintf(cgiOut, "<select name='oCmod%d' onchange='fnChangeMode(this,%d)'>\n", j, j);
  	if (g_cmod[j] == 0)
  		fprintf(cgiOut, "<option value='0' selected>本地模式</option>\n");
  	else
  		fprintf(cgiOut, "<option value='0'>本地模式</option>\n");
  	
  	if (g_cmod[j] == 1)
  		fprintf(cgiOut, "<option value='1' selected>网络模式</option>\n");
  	else
  		fprintf(cgiOut, "<option value='1'>网络模式</option>\n");
  	fprintf(cgiOut, "</select></td>\n");
  }
  fprintf(cgiOut, "</tr>\n");
  fprintf(cgiOut, "<tr align='center' style='COLOR: #0076C8; BACKGROUND-COLOR: #F4FAFF;'>\n");
  fprintf(cgiOut, "<td>&nbsp;</td>\n");
  for (j=0; j<4; ++j)
  {
  	fprintf(cgiOut, "<td>控制板id</td>\n");
  	fprintf(cgiOut, "<td>地址</td>\n");
  }
	fprintf(cgiOut, "</tr>\n");
}

/* DONE */
static void
show_cdev_row(int i)
{
	int j;
	fprintf(cgiOut, "<tr style='COLOR: #0076C8; BACKGROUND-COLOR: #F4FAFF;'>\n");
  fprintf(cgiOut, "<td>%s</td>\n", g_loc[i]);
  for (j=0; j<4; ++j)
  {
  	if (g_cmod[j] == 0)
  		fprintf(cgiOut, "<td align='center'><input type='text' name='oBid%d%d' size='4' maxlength='1' disabled /></td>\n", i, j);
  	else
  	{
  		if (g_cdev[j][i].bid == 0xff)
  			fprintf(cgiOut, "<td align='center'><input type='text' name='oBid%d%d' size='4' maxlength='1' /></td>\n", i, j);
  		else
  			fprintf(cgiOut, "<td align='center'><input type='text' name='oBid%d%d' size='4' maxlength='1' value='%d' /></td>\n", i, j, g_cdev[j][i].bid);
  	}
		if (j == 1)
		{
			if (g_cdev[j][i].addr == 0xff&&g_cdev[j][i].addr1 == 0xff)
  			fprintf(cgiOut, "<td align='center'><input type='text' name='oAddr%d%d' size='8' maxlength='5' /></td>\n", i, j);
  		else
  			fprintf(cgiOut, "<td align='center'><input type='text' name='oAddr%d%d' size='8' maxlength='5' value='%d' /></td>\n",
  			i, j, g_cdev[j][i].addr|(g_cdev[j][i].addr1<<8));
  	}
  	else
  	{
  		if (g_cdev[j][i].addr == 0xff)
  			fprintf(cgiOut, "<td align='center'><input type='text' name='oAddr%d%d' size='8' maxlength='5' /></td>\n", i, j);
  		else
  			fprintf(cgiOut, "<td align='center'><input type='text' name='oAddr%d%d' size='8' maxlength='5' value='%d' /></td>\n",
  			i, j, g_cdev[j][i].addr);
  	}
  }
  fprintf(cgiOut, "</tr>\n");
}

/* DONE */
static void
show_body(void)
{
	int  i;
//	char *desc = "请填写各场所对应设备的id号(从1开始)";
	fprintf(cgiOut, "<body>\n");
	fprintf(cgiOut, "<table width='800' border='0' align='center' bgcolor='#b9d8f3'>\n");
	fprintf(cgiOut, "<tr style='color:#ffffff;text-align:center;'>\n");
	fprintf(cgiOut, "<td colspan='9'><h3><strong>设备控制信息配置</strong></h3></td>\n</tr>\n");
	show_title();
	fprintf(cgiOut, "<form name='oForm' method='post' action='/cgi-bin/set.cgi'>\n");
	show_boardid_row();
	for (i=0; i<g_locCnt; ++i)
		show_cdev_row(i);
	fprintf(cgiOut, "<tr style='COLOR: #0076C8; BACKGROUND-COLOR: #F4FAFF;'>\n");
	fprintf(cgiOut, "<td colspan='9'><input type='hidden' name='cstage' value='4' />");
	fprintf(cgiOut, "<input type='hidden' name='oCnt' id='oCnt' value='%d' /></td></tr>\n", g_locCnt);
	fprintf(cgiOut, "</form>\n");
	fprintf(cgiOut, "<tr style='COLOR: #0076C8; BACKGROUND-COLOR: #F4FAFF;'>\n");
	fprintf(cgiOut, "<td colspan='3' align='center'></td>\n");
	fprintf(cgiOut, "<td align='center'><input type='button' value='清空配置' onclick='fnClearCfg()' /></td>\n");
	fprintf(cgiOut, "<td align='center'><input type='button' value='跳过此步' onclick='fnSkip()' /></td>\n");
	fprintf(cgiOut, "<td align='center'><input type='button' value='保存配置' onclick='fnSubmit()' /></td>\n");
	fprintf(cgiOut, "<td colspan='3' align='center'></td>\n");
	fprintf(cgiOut, "</tr>\n</table>\n</body>\n");
}

/* DONE */
void
show_cdev_page(void)
{
	init_arr();
	read_site(1);
	load_cdev_data();
	cgiHeaderContentType("text/html");
	show_head();
	show_body();
}

#if 0
}
#endif

/////////////////////////////////////////////////
//             cgi in handler                  //
/////////////////////////////////////////////////

#if 0
{
#endif

/* DONE */
static void
get_cmod()
{
	char name[8];
	int  i;
	int  x;
	char *mstr[2] = {"0", "1"};

	for (i=0; i<4; ++i)
	{
		memset(name, 0, 8);
		sprintf(name, "oCmod%d", i);
		if (cgiFormSelectSingle(name, mstr, 2, &x, 0) == cgiFormSuccess)
			r_cmod[i] = x;
	}
}

/* DONE */
static void
get_devid(int n)
{
	int i, j;
	char nstr[12];
	char str[8];

	for (j=0; j<4; ++j)
	{
		for (i=0; i<n; ++i)
		{
			memset(nstr, 0, 12);
			sprintf(nstr, "oBid%d%d", i, j);
			if (cgiFormString(nstr, str, 8) == cgiFormSuccess)
				r_cdev[j][i].bid = atoi(str);
		}
	}
}

/* DONE */
static void
get_devAddr(int n)
{
	int i, j;
	char nstr[12];
	char str[8];

	for (j=0; j<4; ++j)
	{
		for (i=0; i<n; ++i)
		{
			memset(nstr, 0, 12);
			sprintf(nstr, "oAddr%d%d", i, j);
			if (cgiFormString(nstr, str, 8) == cgiFormSuccess)
			{
				unsigned short a;
				a = atoi(str);
				r_cdev[j][i].addr = (unsigned char)(a&0xff);
				if (j == 1)
					r_cdev[j][i].addr1 = (unsigned char)(a>>8);
			}
		}
	}
}

/* DONE */
static void
save_devid()
{
	int i;

	if (memcmp(r_cmod, g_cmod, 4))
	{
		set_reboot_flag();
		store_cdev_data();
		return;
	}

	for (i=0; i<4; ++i)
	{
		if (memcmp(g_cdev[i], r_cdev[i], LOC_MAX*sizeof(cdev_t)))
		{
			set_reboot_flag();
			store_cdev_data();
			return;
		}
	}
}

/* DONE */
void
handle_cdev(void)
{
	int cnt;
	int i;
	char str[8] = {0};

	if (cgiFormString("oCnt", str, 8) == cgiFormSuccess)	
		cnt = atoi(str);
	else
		return;

	if (cnt == -1)
	{/* clear config */
		if (access(DEVCNF, F_OK) == 0)
		{
			unlink(DEVCNF);
			set_reboot_flag();
		}
		show_cdev_page();
		return;
	}
	else if (cnt == 0)
	{/* skip it */
		show_bgsnd_page();
		return;
	}

	for (i=0; i<4; ++i)
		memset(&r_cdev[i][0], 0xff, LOC_MAX*sizeof(cdev_t));
	load_cdev_data();
	get_cmod();
	get_devid(cnt);
	get_devAddr(cnt);
	save_devid();
	show_bgsnd_page();
}

#if 0
}
#endif

int
get_item_value(char *name)
{
	char str[8];
	memset(str, 0, 8);
	if (cgiFormString(name, str, 8) == cgiFormSuccess)
		return atoi(str);
	else
		return 255;
}

////////////////////////////////////////////////
void
set_reboot_flag()
{
  int fd;
  if (access("/tmp/ifreboot", F_OK) == -1)
  {
    fd = creat("/tmp/ifreboot", 0777);
    if (fd >= 0)
    {
      write(fd, "reboot", 6);
      fdatasync(fd);
      close(fd);
    }
  }
}

int
get_reboot_flag()
{
  if (access("/tmp/ifreboot", F_OK) == -1)
    return 0;
  else
    return 1;
}

void
clear_reboot_flag()
{
  unlink("/tmp/ifreboot");
}
//////////////////////////////////////////////

static char *fstr[] = {"您没有修改任何配置", "您已经修改了配置,为了使配置生效,请按'完成'重启设备"};

void
do_finish()
{
	int i;
	i = get_reboot_flag();
	if (i == 1)
	  clear_reboot_flag();
	cgiHeaderContentType("text/html");
	fprintf(cgiOut, "<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Transitional//EN\" \"http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd\">\n");
	fprintf(cgiOut, "<html xmlns=\"http://www.w3.org/1999/xhtml\">\n");
	fprintf(cgiOut, "<head><meta http-equiv=\"Content-Type\" content=\"text/html; charset=gb2312\" />\n");
	fprintf(cgiOut, "<TITLE>配置完成</TITLE></HEAD>\n");
	fprintf(cgiOut, "<BODY>\n");
	fprintf(cgiOut, "<form name='oForm' method='post' action='/cgi-bin/set.cgi' >\n");
	fprintf(cgiOut, "<table width='800' border='0' align='center' cellpadding='0' cellspacing='0' bgcolor='#b9d8f3'>\n");
	fprintf(cgiOut, "<tr style='color:#ffffff;text-align:center;'>\n");
	fprintf(cgiOut, "<td colspan='4'><h1><strong>配置完成</strong></h1></td></tr>\n"); 
	fprintf(cgiOut, "<tr style='COLOR: #0076C8; BACKGROUND-COLOR: #F4FAFF;'>\n");
	fprintf(cgiOut, "<td width='186'>&nbsp;</td>\n");
	fprintf(cgiOut, "<td colspan='2'><h2>%s</h2></td>\n", fstr[i]);
	fprintf(cgiOut, "<td width='203'>&nbsp;</td>\n</tr>\n");
	fprintf(cgiOut, "<tr style='COLOR: #0076C8; BACKGROUND-COLOR: #F4FAFF;'>\n");
	fprintf(cgiOut, "<td colspan='4' align='center'><input type='hidden' name='cstage' value='128' /></td></tr>\n");
	fprintf(cgiOut, "<tr style='COLOR: #0076C8; BACKGROUND-COLOR: #F4FAFF;'>\n");
	fprintf(cgiOut, "<td align='center'>&nbsp;</td>\n");
	fprintf(cgiOut, "<td align='center'></td>\n");
	if (i == 0)
  		fprintf(cgiOut, "<td align='center'><input type='button' value='完成' onclick='window.close()' /></td>\n");
	else
		fprintf(cgiOut, "<td align='center'><input type='submit' value='完成' /></td>\n");
	fprintf(cgiOut, "<td align='center'>&nbsp;</td>\n");
	fprintf(cgiOut, "</tr>\n</table>\n</form>\n");
	fprintf(cgiOut, "</BODY></HTML>\n");
}


