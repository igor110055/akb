#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "cgic.h"
#include "cntl.h"

static char *modname[] = {"休闲模式", "就餐模式", "工作模式", "电视模式", "浪漫模式", "派对模式", "会客模式", "睡眠模式"};
static char *fn0[] = {"开", "关"};
static char *fn1[] = {"开", "关", "弱风", "中风", "强风", "自动"};
static char *fn2[] = {"开", "关", "弱风", "中风", "强风", "自动", "制冷", "制热", "抽湿", "换气"};
static char *devName[] = {"背景音乐", "窗帘", "地暖", "新风", "空调"};

static int  fnCnt[5] = {2, 2, 2, 6, 10};
static char **pFn[5] = {fn0, fn0, fn0, fn1, fn2};

//////////////////////////////////////////////////////
//                 linkage page                     //
//////////////////////////////////////////////////////
static char  devId[LOC_MAX][8][5];
static char  lnkFn[LOC_MAX][8][5];

static char  g_devId[LOC_MAX][8][5];
static char  g_lnkFn[LOC_MAX][8][5];

/* DONE */
static void
init_lnk_struct()
{
	int i, j;
	for (i=0; i<LOC_MAX; ++i)
	for (j=0; j<8; ++j)
	{
		memset(&devId[i][j], 0, 5);
		memset(&lnkFn[i][j], 0, 5);
		memset(&g_devId[i][j], 0, 5);
		memset(&g_lnkFn[i][j], 0, 5);
	}
}

/* DONE */
static void
store_lnk_data()
{
	int  i,j;
	FILE *fp;

	fp = fopen(LKCNF, "w");
	if (!fp) return;

	for (i=0; i<LOC_MAX; ++i)
	{
		for (j=0; j<8; ++j)
		{
			fwrite(&devId[i][j][0], 5, 1, fp);
			fwrite(&lnkFn[i][j][0], 5, 1, fp);
		}
	}
	fflush(fp);
	fclose(fp);
}

/* DONE */
static void
load_lnk_data()
{
	int   fd;
	int   i, j;
	struct stat  st;

	if (stat(LKCNF, &st) < 0) return;
	if (st.st_size != ((LOC_MAX<<3)*10)) return;
	if ((fd = open(LKCNF, O_RDONLY)) < 0) return;

	for (i=0; i<LOC_MAX; ++i)
	for (j=0; j<8; ++j)
	{
		read(fd, &g_devId[i][j][0], 5);
		read(fd, &g_lnkFn[i][j][0], 5);
	}
	close(fd);
}

/////////////////////////////////////////////
///               cgi out                 ///
/////////////////////////////////////////////
#if 0
{
#endif

static void
show_lnk_head()
{
	fprintf(cgiOut, "<style type='text/css'>\n<!--\n");
	fprintf(cgiOut, "#Layer0 {\nposition:absolute;\nz-index:1;\nvisibility: show;\n}\n");
	fprintf(cgiOut, "#Layer1 {\nposition:absolute;\nz-index:2;\nvisibility: hidden;\n}\n");
	fprintf(cgiOut, "#Layer2 {\nposition:absolute;\nz-index:3;\nvisibility: hidden;\n}\n");
	fprintf(cgiOut, "#Layer3 {\nposition:absolute;\nz-index:4;\nvisibility: hidden;\n}\n");
	fprintf(cgiOut, "#Layer4 {\nposition:absolute;\nz-index:5;\nvisibility: hidden;\n}\n");
	fprintf(cgiOut, "#Layer5 {\nposition:absolute;\nz-index:6;\nvisibility: hidden;\n}\n");
	fprintf(cgiOut, "#Layer6 {\nposition:absolute;\nz-index:7;\nvisibility: hidden;\n}\n");
	fprintf(cgiOut, "#Layer7 {\nposition:absolute;\nz-index:8;\nvisibility: hidden;\n}\n");
	fprintf(cgiOut, "-->\n</style>\n\n");

	fprintf(cgiOut, "<script type=\"text/JavaScript\">\n<!--\n");
	
	fprintf(cgiOut, "function MM_showLayers(x) {\nvar i,id,obj;\n");
	fprintf(cgiOut, "for (i=0; i<8; ++i){\nid='Layer'+String(i);\n");
	fprintf(cgiOut, "obj=document.getElementById(id);\n");
	fprintf(cgiOut, "if (obj!=null){\nif (obj.style)\nobj=obj.style;\n");
	fprintf(cgiOut, "if (x == i)\nobj.visibility='visible';\n");
	fprintf(cgiOut, "else\nobj.visibility='hidden';\n}\n}\n}\n\n");

	fprintf(cgiOut, "function swap_bgcolor(i) {\nvar j, obj;\n");
	fprintf(cgiOut, "obj = document.getElementById('thead');\n");
	fprintf(cgiOut, "obj.cells(i).style.backgroundColor='#99FFFF';\n");
	fprintf(cgiOut, "for(j=0;j<obj.cells.length;j++){\n");
	fprintf(cgiOut, "if (j!=i)\nobj.cells(j).style.backgroundColor='#b9d8f3';\n}\n}\n\n");

	fprintf(cgiOut, "function fnSubmit() {\n");
	fprintf(cgiOut, "oForm.submit();\n}\n\n");

	fprintf(cgiOut, "function fnSkip() {\n");
	fprintf(cgiOut, "var o = document.getElementById('oCnt');\n");
	fprintf(cgiOut, "o.value = 0;\noForm.submit();\n}\n\n");

	fprintf(cgiOut, "function fnClearCfg(){\n");
	fprintf(cgiOut, "var o = document.getElementById('oCnt');\n");
	fprintf(cgiOut, "o.value = -1;\noForm.submit();\n}\n\n");
	fprintf(cgiOut, "-->\n</script>\n</head>\n");
}

/* DONE */
static void
show_lnk_body_head()
{
	int i;
	int w[8] = {94, 94, 93, 93, 94, 94, 93, 93};
	fprintf(cgiOut, "<body>\n<table width='800' align='center' bgcolor='#b9d8f3'>\n");
	fprintf(cgiOut, "<tr style='color:#ffffff;text-align:center;'>\n");
	fprintf(cgiOut, "<td colspan='8'><h3><strong>场景联动配置</strong></h3></td>\n</tr>\n");
	fprintf(cgiOut, "<tr id='thead' style='color:#ffffff;text-align:center;'>\n");
	for (i=0; i<8; ++i)
	{
		fprintf(cgiOut, "<td width='%d' style='cursor:pointer;' onfocus='MM_showLayers(%d);swap_bgcolor(%d);' ", w[i], i, i);
		if (i == 0)
			fprintf(cgiOut, "bgcolor='#99FFFF' ");
		fprintf(cgiOut, "><strong>%s</strong></td>\n", modname[i]);
	}
	fprintf(cgiOut, "</tr>\n");
}

/* DONE */
static void
show_lnk_begin()
{
	fprintf(cgiOut, "<tr style='COLOR: #0076C8; BACKGROUND-COLOR: #F4FAFF;'>\n");
	fprintf(cgiOut, "<td align='center' colspan='2'>&nbsp;</td>\n");
	fprintf(cgiOut, "<td align='center' colspan='2'><input type='button' value='清空配置' onclick='fnClearCfg()' /></td>\n");
	fprintf(cgiOut, "<td align='center' colspan='2'><input type='button' value='跳过此步' onclick='fnSkip()' /></td>\n");
	fprintf(cgiOut, "<td align='center' colspan='2'><input type='button' value='保存配置' onclick='fnSubmit()' /></td>\n</tr>\n");
	fprintf(cgiOut, "<form name='oForm' method='post' action='/cgi-bin/set.cgi'>\n");
	fprintf(cgiOut, "<tr>\n<td colspan='8'>\n");
}

/* DONE */
static void
show_lnk_end()
{
	fprintf(cgiOut, "</td>\n</tr>\n");
	fprintf(cgiOut, "<tr style='COLOR: #0076C8; BACKGROUND-COLOR: #F4FAFF;'>\n");
	fprintf(cgiOut, "<td colspan='8'>\n");
	fprintf(cgiOut, "<input type='hidden' name='cstage' value='7' />\n");
	fprintf(cgiOut, "<input type='hidden' name='oCnt' id='oCnt' value='%d' />\n", g_locCnt);
	fprintf(cgiOut, "</td>\n</tr>\n</form>\n");
	fprintf(cgiOut, "</table>\n</body>\n</html>\n\n");
}

/* DONE */
static void
show_lnk_cell(int layers, int rows, int type)
{
	int  i;

	fprintf(cgiOut, "<td align='center'>\n");
	fprintf(cgiOut, "<select name='oId%d%d%d' id='oId%d%d%d'>\n", rows, layers, type, rows, layers, type);
	if (g_devId[rows][layers][type] == 0) 
		fprintf(cgiOut, "<option value='0' selected>无联动</option>\n");
	else
		fprintf(cgiOut, "<option value='0'>无联动</option>\n");
	for (i=1; i<=16; ++i)
	{
		if (g_devId[rows][layers][type] == i)
			fprintf(cgiOut, "<option value='%d' selected>%d号%s</option>\n", i, i, devName[type]);
		else
			fprintf(cgiOut, "<option value='%d'>%d号%s</option>\n", i, i, devName[type]);
	}
	if (g_devId[rows][layers][type] > 16)
		fprintf(cgiOut, "<option value='17' selected>全部%s</option>\n", devName[type]);
	else
		fprintf(cgiOut, "<option value='17'>全部%s</option>\n", devName[type]);
	fprintf(cgiOut, "</select>\n");

	fprintf(cgiOut, "<select name='oFn%d%d%d' id='oFn%d%d%d'>\n", rows, layers, type, rows, layers, type);
	for (i=0; i<fnCnt[type]; ++i)
	{
		if (g_lnkFn[rows][layers][type] == i)
			fprintf(cgiOut, "<option value='%d' selected>%s</option>\n", i, pFn[type][i]);
		else
			fprintf(cgiOut, "<option value='%d'>%s</option>\n", i, pFn[type][i]);
	}
/*
	if (type < 3)
	{
		for (i=0; i<sizeof(fn0)/sizeof(char*); ++i)
		{
			if (g_lnkFn[rows][layers][type] == i)
				fprintf(cgiOut, "<option value='%d' selected>%s</option>\n", i, fn0[i]);
			else
				fprintf(cgiOut, "<option value='%d'>%s</option>\n", i, fn0[i]);
		}
	}
	else
	{
	}
*/
	fprintf(cgiOut, "</select></td>\n");
}

/* DONE */
static void
show_layers(int i)
{
	int j;
	char *lnkStr[] = {"联动音乐", "联动窗帘", "联动地暖", "联动新风", "联动空调"};

	fprintf(cgiOut, "<div id='Layer%d'>\n<table width='800' border='0' bgcolor='#b9d8f3'>\n", i);
	fprintf(cgiOut, "<tr style='COLOR: #0076C8; BACKGROUND-COLOR: #F4FAFF;'>\n");
	fprintf(cgiOut, "<td align='center'>场所</td>");
	for (j=0; j<sizeof(lnkStr)/sizeof(char*); ++j)
	{
		fprintf(cgiOut, "<td align='center'>%s</td>\n", lnkStr[j]);
	}
	fprintf(cgiOut, "</tr>\n");

	for (j=0; j<g_locCnt; ++j)
	{
		fprintf(cgiOut, "<tr style='COLOR: #0076C8; BACKGROUND-COLOR: #F4FAFF;'>\n");
		fprintf(cgiOut, "<td align='center'>%s</td>", g_loc[j]);
		show_lnk_cell(i, j, 0);
		show_lnk_cell(i, j, 1);
		show_lnk_cell(i, j, 2);
		show_lnk_cell(i, j, 3);
		show_lnk_cell(i, j, 4);
		fprintf(cgiOut, "</tr>\n");
	}
	fprintf(cgiOut, "</table>\n</div>\n");
}

/* DONE */
static void
show_lnk_body()
{
	int i;
	show_lnk_body_head();
	show_lnk_begin();
	for (i=0; i<sizeof(modname)/sizeof(char*); ++i)
	{
		show_layers(i);
	}
	show_lnk_end();
}

/* DONE */
void
show_lnk_page()
{
	read_site(1);
	load_lnk_data();
	cgiHeaderContentType("text/html");
	fprintf(cgiOut, "<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Transitional//EN\" \"http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd\">\n");
	fprintf(cgiOut, "<html xmlns=\"http://www.w3.org/1999/xhtml\">\n");
	fprintf(cgiOut, "<head>\n<meta http-equiv=\"Content-Type\" content=\"text/html; charset=gb2312\" />\n");
	fprintf(cgiOut, "<title>场景联动配置</title>\n");
	show_lnk_head();
	show_lnk_body();
}

#if 0
}
#endif

/////////////////////////////////////////////
///                cgi in                 ///
/////////////////////////////////////////////
#if 0
{
#endif

/* DONE */
static void
get_lnk_info(int n)
{
	int i, j, k;
	int x;
	char name[16];
	char *v[] = {"0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "10",
		             "11", "12", "13", "14", "15", "16", "17"};

	for (k=0; k<n; ++k)
	for (i=0; i<8; ++i)
	for (j=0; j<5; j++)
	{
		memset(name, 0, 16);
		sprintf(name, "oId%d%d%d", k, i, j);
		if (cgiFormSelectSingle(name, v, sizeof(v)/sizeof(char*), &x, 0) == cgiFormSuccess)
		{
			devId[k][i][j] = x;
		}

		if (devId[k][i][j] > 0)
		{
			memset(name, 0, 16);
			sprintf(name, "oFn%d%d%d", k, i, j);
			if (cgiFormSelectSingle(name, v, fnCnt[j], &x, 0) == cgiFormSuccess)
			{
				lnkFn[k][i][j] = x;
			}
		}
	}
}

/* DONE */
static int
lnk_diff(int n)
{
	int i, j;
	for (i=0; i<n; ++i)
	{
		for (j=0; j<8; ++j)
		{
			if (memcmp(&devId[i][j][0], &g_devId[i][j][0], 5) != 0)
				return 1;
			if (memcmp(&lnkFn[i][j][0], &g_lnkFn[i][j][0], 5) != 0)
				return 1;
		}
	}
	return 0;
}

void
handle_slnk()
{
	int cnt;
	char str[8] = {0};
	
	if (cgiFormString("oCnt", str, 8) == cgiFormSuccess)
		cnt = atoi(str);
	else
	{
		do_finish();
		return;
	}
	if (cnt == -1)
	{
		if (access(LKCNF, F_OK) == 0)
		{
			unlink(LKCNF);
			set_reboot_flag();
		}
		show_lnk_page();
		return;
	}
	else if (cnt == 0)
	{
		do_finish();
		return;
	}

	init_lnk_struct();
	load_lnk_data();
	get_lnk_info(cnt);
	if (lnk_diff(cnt))
	{
		store_lnk_data();
		set_reboot_flag();
	}
	do_finish();
	return;
}

#if 0
}
#endif

