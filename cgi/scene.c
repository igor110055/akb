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

/* scene data of the mode */
static char  g_mbid[LOC_MAX][8];
/* scene number: < 0xff */
static char  g_msce[LOC_MAX][8];

/* scene data of the mode */
static char  mbid[LOC_MAX][8];
/* scene number: < 0xff */
static char  msce[LOC_MAX][8];

static char *modname[] = {"休闲模式", "就餐模式", "工作模式", "电视模式", "浪漫模式", "派对模式", "会客模式", "睡眠模式"};

static void
init_scene_struct()
{
	int i;

	for (i=0; i<LOC_MAX; ++i)
	{
		memset(g_mbid[i], 0xff, 8);
		memset(g_msce[i], 0xff, 8);
		memset(mbid[i], 0xff, 8);
		memset(msce[i], 0xff, 8);
	}
}

/* DONE */
static unsigned short
load_sce_data()
{
	FILE  *fp;
	int   i;
	unsigned short ret = 0;

	fp = fopen(LCONF, "r");
	if (fp != NULL)
	{
		for (i=0; i<LOC_MAX; ++i)
		{
			fread((void*)g_mbid[i], 8, 1, fp);
			fread((void*)g_msce[i], 8, 1, fp);
		}
		ret = 1;
		fclose(fp);
	}
	
	return ret;
}

static void
store_sce_data()
{
	FILE  *fp;
	int   i;

	fp = fopen(LCONF, "w");
	if (fp != NULL)
	{
		for (i=0; i<LOC_MAX; ++i)
		{
			fwrite((void*)mbid[i], 8, 1, fp);
			fwrite((void*)msce[i], 8, 1, fp);
		}
		sync();
		fclose(fp);
	}
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
	fprintf(cgiOut, "<title>场景调用信息配置</title>\n");
	fprintf(cgiOut, "<script type=\"text/JavaScript\">\n");
	fprintf(cgiOut, "<!--\n");

	fprintf(cgiOut, "function fnSkip(){\n\tvar o = document.getElementById('oCnt');\n\to.value = 0;\n\toForm.submit();\n}\n\n");
	
	fprintf(cgiOut, "function fnClearCfg(){\n");
	fprintf(cgiOut, "\tvar o = document.getElementById('oCnt');\n");
	fprintf(cgiOut, "\to.value = -1;\n");
	fprintf(cgiOut, "\toForm.submit();\n}\n\n");

	fprintf(cgiOut, "function fnSubmit(){\n\toForm.submit();\n}\n\n");

	fprintf(cgiOut, "--></script>\n</head>\n");
}

/* DONE */
static void
show_title()
{
	int i;
	fprintf(cgiOut, "<tr style='COLOR: #0076C8; BACKGROUND-COLOR: #F4FAFF;'>\n");
  fprintf(cgiOut, "<td>场所</td>\n");
  for (i=0; i<sizeof(modname)/sizeof(char*); ++i)
  {
  	fprintf(cgiOut, "<td align='center'>%s</td>\n", modname[i]);
	}
  fprintf(cgiOut, "</tr>\n");
}

/* DONE */
static void
show_scene_row(int i)
{
	int j;
	fprintf(cgiOut, "<tr style='COLOR: #0076C8; BACKGROUND-COLOR: #F4FAFF;'>\n");
  fprintf(cgiOut, "<td>%s</td>\n", g_loc[i]);
  for (j=0; j<sizeof(modname)/sizeof(char*); ++j)
  {
  	fprintf(cgiOut, "<td align='center'>\n");
  	if (g_mbid[i][j] < 0xff)
  		fprintf(cgiOut, "<input type='text' name='mbid%d%d' size='2' maxlength='2' value='%d' />\n", i, j, g_mbid[i][j]);
  	else
  		fprintf(cgiOut, "<input type='text' name='mbid%d%d' size='2' maxlength='2' />\n", i, j);
  	if (g_msce[i][j] < 0xff)
  		fprintf(cgiOut, "<input type='text' name='msce%d%d' size='4' maxlength='3' value='%d' />\n", i, j, g_msce[i][j]);
  	else
  		fprintf(cgiOut, "<input type='text' name='msce%d%d' size='4' maxlength='3' />\n", i, j);
  	fprintf(cgiOut, "</td>\n");
  }
  fprintf(cgiOut, "</tr>\n");
}

/* DONE */
static void
show_body(void)
{
	int  i;
	char *desc = "每一项对应两个输入框,左边输入板子id,右边输入场景号.";
	fprintf(cgiOut, "<body>\n");
	fprintf(cgiOut, "<table width='1024' border='0' align='center' bgcolor='#b9d8f3'>\n");
	fprintf(cgiOut, "<tr style='color:#ffffff;text-align:center;'>\n");
	fprintf(cgiOut, "<td colspan='9'><h3><strong>场景调用信息配置</strong></h3></td>\n</tr>\n");
	show_title();
	fprintf(cgiOut, "<form name='oForm' method='post' action='/cgi-bin/set.cgi'>\n");
	for (i=0; i<g_locCnt; ++i)
		show_scene_row(i);
	fprintf(cgiOut, "<tr style='COLOR: #0076C8; BACKGROUND-COLOR: #F4FAFF;'>\n");
	fprintf(cgiOut, "<td colspan='9'><input type='hidden' name='cstage' value='6' />");
	fprintf(cgiOut, "<input type='hidden' name='oCnt' id='oCnt' value='%d' />%s</td></tr>\n", g_locCnt, desc);
	fprintf(cgiOut, "</form>\n");
	fprintf(cgiOut, "<tr style='COLOR: #0076C8; BACKGROUND-COLOR: #F4FAFF;'>\n");
	fprintf(cgiOut, "<td align='center' colspan='3'></td>\n");
	fprintf(cgiOut, "<td align='center'><input type='button' value='清空配置' onclick='fnClearCfg()' /></td>\n");
	fprintf(cgiOut, "<td align='center'><input type='button' value='跳过此步' onclick='fnSkip()' /></td>\n");
	fprintf(cgiOut, "<td align='center'><input type='button' value='保存配置' onclick='fnSubmit()' /></td>\n");
	fprintf(cgiOut, "<td align='center' colspan='3'></td>\n");
	fprintf(cgiOut, "</tr>\n</table>\n</body>\n");
}

/* DONE */
void
show_scene_page(void)
{
	init_scene_struct();
	read_site(1);
	load_sce_data();
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
static int
get_scene(int n)
{
	int i, j;
	char nstr[12];
	char str[8];

	for (i=0; i<n; ++i)
	{
		for (j=0; j<sizeof(modname)/sizeof(char*); ++j)
		{
			memset(nstr, 0, 12);
			sprintf(nstr, "mbid%d%d", i, j);
			if (cgiFormString(nstr, str, 8) == cgiFormSuccess)
				mbid[i][j] = atoi(str);
			
			memset(nstr, 0, 12);
			sprintf(nstr, "msce%d%d", i, j);
			if (cgiFormString(nstr, str, 8) == cgiFormSuccess)
				msce[i][j] = atoi(str);
		}
	}

	for (i=0; i<n; ++i)
	{
		if (memcmp(g_mbid[i], mbid[i], 8) != 0)
			return 1;
	}

	for (i=0; i<n; ++i)
	{
		if (memcmp(g_msce[i], msce[i], 8) != 0)
			return 1;
	}
	return 0;
}

/* DONE */
void
handle_scene(void)
{
	int cnt;
	char str[8] = {0};

	if (cgiFormString("oCnt", str, 8) == cgiFormSuccess)
		cnt = atoi(str);
	else
		return;

	if (cnt == -1)
	{/* clear config */
		if (access(LCONF, F_OK) == 0)
		{
			unlink(LCONF);
			set_reboot_flag();
		}
		show_scene_page();
		return;
	}
	else if (cnt == 0)
	{/* skip it */
		if ((access(LCONF, F_OK) == 0)
		&&((access(DEVCNF, F_OK) == 0)||(access(BCONF, F_OK) == 0)))
			show_lnk_page();
		else
			do_finish();
		return;
	}

	init_scene_struct();
	load_sce_data();
	if (get_scene(cnt) > 0)
	{
		store_sce_data();
		set_reboot_flag();
	}

	if ((access(LCONF, F_OK) == 0)
	&&((access(DEVCNF, F_OK) == 0)||(access(BCONF, F_OK) == 0)))
		show_lnk_page();
	else
		do_finish();
}

#if 0
}
#endif


