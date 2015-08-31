#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <ctype.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <linux/reboot.h>
#include "cgic.h"
#include "basic.h"
#include "cntl.h"

static char  user[36];
static char  pass[36];
static char *usr = "21232f297a57a5a743894a0e4a801fc3";
static char *pwd = "d437df002f7a5c8555c107af8a643977";

extern int reboot(int cmd);

/* DONE */
static int
auth_check()
{
	memset(user, '\0', 36);
	memset(pass, '\0', 36);
	cgiFormString("user", user, 36);
	cgiFormString("passwd", pass, 36);
	if (strcmp(user, usr) == 0&&strcmp(pass, pwd) == 0)
		return 0;
	else
		return -1;
}

static int
check_from()
{
	char str[4];
	memset(str, 0, 4);
	if (cgiFormString("cstage", str, 4) == cgiFormSuccess)
		return atoi(str);
	else
		return -1;
}

static int
what_action()
{
	char str[16];
	memset(str, 0, 16);
	if (cgiFormString("next", str, 16) == cgiFormSuccess)
		return 1;
	else if (cgiFormString("finish", str, 16) == cgiFormSuccess)
		return 0;
	else
		return 2;
}

int
cgiMain()
{
	int act;
	int ret;

	act = check_from();
	switch (act)
	{
	case -1:
		do_error("用户名或密码错误");
		break;
	case 0:
		if (auth_check() < 0)
		{
			do_error("用户名或密码错误");
			return 0;
		}
		show_basic_page();
		break;
	case 1:
		handle_basic();
		break;
	case 2:
		handle_security();
		break;
	case 3:
		ret = what_action();
		if (ret == 0)
			reboot(LINUX_REBOOT_CMD_RESTART);
		else if (ret == 1)
			show_cdev_page();
		break;
	case 4:
		handle_cdev();
		break;
	case 5:
		handle_breq();
		break;
	case 6:
		handle_scene();
		break;
	case 7:
		handle_slnk();
		break;
	case 128:
		reboot(LINUX_REBOOT_CMD_RESTART);
		break;
	default:
		break;
	}

	return 0;
}


