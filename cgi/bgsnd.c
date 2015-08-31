#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "cgic.h"
#include "cntl.h"

static int  g_bsmod = 0;
int  g_bsCnt = 0;
unsigned char g_bsAddr[16] = {0xff};
unsigned char g_bsLoc[16] = {0xff};

static int  bsmod = 0;
static int  bsCnt = 0;
static unsigned char bsAddr[16] = {0xff};

void
load_bsinfo()
{
	FILE  *fp;
	char  *ptr;
	char  *p;
	char   buf[64];
	int    first;
	int    addr;
	int    loc;

	g_bsCnt = 0;
	g_bsmod = 0;
	memset(g_bsAddr, 0xff, 16);
	if ((fp = fopen(BCONF, "r")) == NULL) return;

	first = 1;
	while (!feof(fp))
	{
		memset(buf, '\0', 64);
		fgets(buf, 64, fp);
		p = buf;
		while (*p!='\0'&&isspace(*p))
			++p;
		if (!isdigit(*p)) continue;
		if (first == 1)
		{
			g_bsmod = atoi(p);
			first = 0;
			if (g_bsmod <= 0) break;
		}
		else
		{
			ptr = strchr(p, ':');
			if (ptr == NULL||ptr == p) continue;
			sscanf(p,"%d:%d", &loc, &addr);
			if (loc < 16&&addr < 0xff)
			{
				g_bsLoc[g_bsCnt] = loc;
				g_bsAddr[loc] = (unsigned char)addr;
				g_bsCnt++;
			}
		}
	}
	fclose(fp);

	if (g_bsmod <= 0||g_bsCnt == 0)
		unlink(BCONF);
}

static void
store_bsinfo()
{
	FILE   *fp = NULL;
	int     i;

	if (bsmod <= 0||bsCnt == 0)
	{
		unlink(BCONF);
		return;
	}
	if ((fp = fopen(BCONF, "w")) == NULL) return;
	fprintf(fp, "%d\n", bsmod);
	for (i = 0; i < 16; ++i)
	{
		if (bsAddr[i] < 0xff)
			fprintf(fp, "%d:%d\n", i, bsAddr[i]);
	}
	fflush(fp);
	fclose(fp);
}

////////////////////////////////////////////////
/*               CGI OUT FUNCTIONS            */
////////////////////////////////////////////////
static void
show_bs_head(void)
{
	fprintf(cgiOut, "<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Transitional//EN\" \"http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd\">\n");
	fprintf(cgiOut, "<html xmlns=\"http://www.w3.org/1999/xhtml\">\n");
	fprintf(cgiOut, "<head><meta http-equiv=\"Content-Type\" content=\"text/html; charset=gb2312\" />\n");
	fprintf(cgiOut, "<title>背景音乐配置</title>\n");
	fprintf(cgiOut, "<script type=\"text/JavaScript\">\n");
	fprintf(cgiOut, "<!--\nvar gCnt=%d;\n", g_locCnt);
	
	fprintf(cgiOut, "function disableEntries(v){\nvar o,id,j;\n");
	fprintf(cgiOut, "for(j=0; j<gCnt; ++j){\nid='bsAddr'+String(j);\n");
	fprintf(cgiOut, "o=document.getElementById(id);\no.disabled=v;\n}\n}\n\n");

	fprintf(cgiOut, "function btClicked(i){\ni = (i > 0)?0:1;\ndisableEntries(i);\n}\n\n");

	fprintf(cgiOut, "function checkBsSet(){\nvar id,o,i,n;\n");
	fprintf(cgiOut, "o=document.getElementsByName('oBt');\n");
	fprintf(cgiOut, "if (o[0].checked)\nreturn 1;\n");
	fprintf(cgiOut, "else if (o[1].checked||o[2].checked)\n{\nn = 0;\n");
	fprintf(cgiOut, "for(i=0; i<gCnt; ++i){\nid='bsAddr'+String(i);\n");
	fprintf(cgiOut, "o=document.getElementById(id);\nif (o.value != '')\n++n;\n}\nreturn n;\n}\nreturn 0;\n}\n\n");
	
	fprintf(cgiOut, "function fnSkip(){\nvar o = document.getElementById('oCnt');\no.value = 0;\noForm.submit();\n}\n\n");	
	fprintf(cgiOut, "function fnSubmit(){\nvar cnt, o;\n");
	fprintf(cgiOut, "if (checkBsSet() == 0)\n{\nalert('请先输入配置信息然后保存');\nreturn;\n}\nelse\n{\n");
	fprintf(cgiOut, "o = document.getElementById('oCnt');\no.value = gCnt;\noForm.submit();\n}\n}\n\n");

	fprintf(cgiOut, "--></script></head>\n");
}

/* DONE */
static void
show_bs_body_head(void)
{
	fprintf(cgiOut, "<body>\n<table width='800' border='0' align='center' bgcolor='#b9d8f3'>\n");
	fprintf(cgiOut, "<form name='oForm' id='oForm' method='post' action='/cgi-bin/set.cgi'>\n");
	fprintf(cgiOut, "<tr style='color:#ffffff;text-align:center;'>\n");
	fprintf(cgiOut, "<td colspan='8'><h3><strong>背景音乐配置</strong></h3></td></tr>\n");
	fprintf(cgiOut, "<tr style='COLOR: #0076C8; BACKGROUND-COLOR: #F4FAFF;'>\n");
  fprintf(cgiOut, "<td colspan='2'>背景音乐控制器类型</td>\n");
  if (g_bsmod == 0)
  	fprintf(cgiOut, "<td colspan='2'><label><input type='radio' name='oBt' value='0' onclick='btClicked(0)' checked />无</label></td>\n");
  else
  	fprintf(cgiOut, "<td colspan='2'><label><input type='radio' name='oBt' value='0' onclick='btClicked(0)' />无</label></td>\n");
  
  if (g_bsmod == 1)
  	fprintf(cgiOut, "<td colspan='2'><label><input type='radio' name='oBt' value='1' onclick='btClicked(1)' checked />ROCK</label></td>\n");
  else
  	fprintf(cgiOut, "<td colspan='2'><label><input type='radio' name='oBt' value='1' onclick='btClicked(1)' />ROCK</label></td>\n");
  
  if (g_bsmod == 2)
  	fprintf(cgiOut, "<td colspan='2'><label><input type='radio' name='oBt' value='2'  onclick='btClicked(2)' checked/>BYRON</label></td>\n</tr>\n");
  else
  	fprintf(cgiOut, "<td colspan='2'><label><input type='radio' name='oBt' value='2' onclick='btClicked(2)' />BYRON</label></td>\n</tr>\n");

	fprintf(cgiOut, "<tr style='COLOR: #0076C8; BACKGROUND-COLOR: #F4FAFF;'><td colspan='8'>&nbsp;</td></tr>\n");

	fprintf(cgiOut, "<tr style='COLOR: #0076C8; BACKGROUND-COLOR: #F4FAFF;'>\n");
	fprintf(cgiOut, "<td width='94'>位置</td>\n");
	fprintf(cgiOut, "<td width='93'>地址/通道</td>\n");
	fprintf(cgiOut, "<td width='94'>位置</td>\n");
	fprintf(cgiOut, "<td width='93'>地址/通道</td>\n");
	fprintf(cgiOut, "<td width='94'>位置</td>\n");
	fprintf(cgiOut, "<td width='93'>地址/通道</td>\n");
	fprintf(cgiOut, "<td width='94'>位置</td>\n");
	fprintf(cgiOut, "<td width='93'>地址/通道</td></tr>\n");
}

/* DONE */
static void
show_bs_body_end(void)
{
	fprintf(cgiOut, "<tr style='COLOR: #0076C8; BACKGROUND-COLOR: #F4FAFF;'>\n");
	fprintf(cgiOut, "<td colspan='8'>背景音乐设置说明： <br/>\n");
	fprintf(cgiOut, "1、如果类型选择'无'，则不必设置'地址/通道'项。 <br/>\n");
	fprintf(cgiOut, "2、对于ROCK背景音乐，输入框输入的是485地址，对于BYRON背景音乐，输入框输入的是通道号。 </td></tr>\n");
	fprintf(cgiOut, "<tr style='COLOR: #0076C8; BACKGROUND-COLOR: #F4FAFF;'>\n");
	fprintf(cgiOut, "<td colspan='2' align='center'><input type='hidden' name='cstage' value='5' /></td>\n");
	fprintf(cgiOut, "<td colspan='2' align='center'><input type='button' value='跳过此步' onclick='fnSkip();' /></td>\n");
	fprintf(cgiOut, "<td colspan='2' align='center'><input type='button' value='保存设置' onclick='fnSubmit();' /></td>\n");
	fprintf(cgiOut, "<td colspan='2' align='center'><input type='hidden' name='oCnt' id='oCnt' value='0' /></td></tr>\n");
	fprintf(cgiOut, "</form></table></body>\n");
}

/* DONE */
static void
show_bs_option(void)
{
	int rows;
	int r, c, i;
	if (g_locCnt == 0)return;
	rows = ((g_locCnt-1)>>2)+1;
	for (r=0; r<rows; ++r)
	{
		fprintf(cgiOut, "<tr style='COLOR: #0076C8; BACKGROUND-COLOR: #F4FAFF;'>\n");
		for (c=0; c<4; ++c)
		{
			i = r*4+c;
			if (i < g_locCnt)
			{
				fprintf(cgiOut, "<td>%s</td>\n", g_loc[i]);
				if ((g_bsCnt > 0)&&(g_bsAddr[i] < 0xff))
					fprintf(cgiOut, "<td><input name='bsAddr%d' id='bsAddr%d' type='text' value='%d' size='4' maxlength='3' style='color:#000000;background-color:#ffffff;' /></td>\n", i, i, g_bsAddr[i]);
				else
  					fprintf(cgiOut, "<td><input name='bsAddr%d' id='bsAddr%d' type='text' size='4' maxlength='3' style='color:#000000;background-color:#ffffff;' /></td>\n", i, i);
			}
			else
			{
				fprintf(cgiOut, "<td></td>\n");
  			fprintf(cgiOut, "<td></td>\n");
  		}
		}
		fprintf(cgiOut, "</tr>\n");
	}
}

/* DONE */
void
show_bgsnd_page(void)
{
	read_site(1);
	load_bsinfo();
	cgiHeaderContentType("text/html");
	show_bs_head();
	show_bs_body_head();
	show_bs_option();
	show_bs_body_end();
}

////////////////////////////////////////////////
/*              cgi in handler                */
////////////////////////////////////////////////
static void
get_bsinfo(int n)
{
	int   i;
	int   x;
	unsigned char v;
	char  name[12];
	char  str[8];
	char *val[3] = {"0", "1", "2"};
	
	if (cgiFormRadio("oBt", val, 3, &x, 0) == cgiFormSuccess)
		bsmod = x;
	if (bsmod <= 0) return;
	x = 0;
	for (i = 0; i < n; ++i)
	{
		memset(str, 0, 8);
		memset(name, 0, 12);
		sprintf(name, "bsAddr%d", i);
		if (cgiFormString(name, str, 8) == cgiFormSuccess)
		{
			if (strlen(str) > 0)
			{
				v = (unsigned char)atoi(str);
				if (v < 0xff)
				{
					bsAddr[i] = v;
					x++;
				}
			}
		}
	}
	bsCnt = x;
}

void
handle_breq()
{
	int cnt;
	char str[8] = {0};

	if (cgiFormString("oCnt", str, 8) == cgiFormSuccess)	
		cnt = atoi(str);
	else
		return;

	if (cnt == -1)
	{/* clear config */
		if (access(BCONF, F_OK) == 0)
		{
			unlink(BCONF);
			set_reboot_flag();
		}
		show_bgsnd_page();
		return;
	}
	else if (cnt == 0)
	{/* skip it */
		show_scene_page();
		return;
	}
	
	memset(bsAddr, 0xff, 16);
	bsCnt = 0;
	load_bsinfo();

	/* cnt:number of location items */
	get_bsinfo(cnt);
	if ((bsmod != g_bsmod)||(bsCnt != g_bsCnt)||memcmp(bsAddr, g_bsAddr, 16))
	{
		store_bsinfo();
		set_reboot_flag();
	}
	show_scene_page();
}

