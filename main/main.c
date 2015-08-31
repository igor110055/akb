#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include "ulib.h"
#include "mxc_sdk.h"
#include "utkwindow.h"
#include "utkdesktop.h"
#include "utkinit.h"
#include "utkmain.h"
#include "gui.h"
#include "stb.h"
#include "config.h"
#include "pzone.h"
#include "recver.h"
#include "query.h"
#include "zopt.h"
#include "rs485.h"
#include "pinyin.h"
#include "hashtbl.h"
#include "qtimer.h"
#include "znode.h"

#define  KEY_TONE     "/media/ui/tick0.wav"
#define  ALARM_PATH   "/media/alarms"

int
init_pipe(int pfd[2])
{
	if (socketpair(AF_UNIX, SOCK_STREAM, 0, pfd) < 0) {
		perror("socketpair()");
		return -1;
	}
	u_set_nonblock(pfd[0]);
	u_set_nonblock(pfd[1]);

	return 0;
}

static void
on_quit(void)
{
	utk_uninit();
	printf("--------- on exit ---------\n");
}

static void
sigexit(int dummy)
{
	printf("--- RECEIVED SIGNAL: %d ---\n", dummy);
//	on_quit();
	exit(0);
//	reboot(LINUX_REBOOT_CMD_RESTART);
}

static void
init_signals(void)
{
	struct sigaction sa;
	sa.sa_flags = 0;
	sigemptyset(&sa.sa_mask);
	sigaddset(&sa.sa_mask, SIGSEGV);
	sigaddset(&sa.sa_mask, SIGBUS);
	sigaddset(&sa.sa_mask, SIGABRT);
	sigaddset(&sa.sa_mask, SIGTERM);
	sigaddset(&sa.sa_mask, SIGHUP);
	sigaddset(&sa.sa_mask, SIGINT);
	sigaddset(&sa.sa_mask, SIGPIPE);
	sigaddset(&sa.sa_mask, SIGCHLD);
	sigaddset(&sa.sa_mask, SIGALRM);
	sigaddset(&sa.sa_mask, SIGUSR1);
	sigaddset(&sa.sa_mask, SIGUSR2);

#if 0
	sa.sa_handler = sigexit;
	sigaction(SIGSEGV, &sa, NULL);

	sa.sa_handler = sigexit;
	sigaction(SIGBUS, &sa, NULL);
	
	sa.sa_handler = sigexit;
	sigaction(SIGABRT, &sa, NULL);

	sa.sa_handler = sigexit;
	sigaction(SIGTERM, &sa, NULL);
#endif

	sa.sa_handler = SIG_IGN;
	sigaction(SIGHUP, &sa, NULL);

	sa.sa_handler = SIG_IGN;
	sigaction(SIGINT, &sa, NULL);

	sa.sa_handler = SIG_IGN;
	sigaction(SIGPIPE, &sa, NULL);

	sa.sa_handler = SIG_IGN;
	sigaction(SIGCHLD, &sa, NULL);

	sa.sa_handler = SIG_IGN;
	sigaction(SIGALRM, &sa, NULL);

	sa.sa_handler = SIG_IGN;
	sigaction(SIGUSR1, &sa, NULL);

	sa.sa_handler = SIG_IGN;
	sigaction(SIGUSR2, &sa, NULL);
}

static void
system_conf(void)
{
	zhead_init();
	hashtable_init();
	py_init();
	pzinfo_init();
	devid_conf();
	netconf_load();
	u_route_add(NULL, NULL, "224.0.0.0", "240.0.0.0", 1);

	general_conf();
	uart_conf();
	load_iftype();

	load_genchr();
	load_usr();

	load_zone();
	load_pzone();
	init_pzinfo();

	load_pztmr(0);
	syst_load();

	load_einfo();
	load_ainfo();
	load_alarm();

	fi_save();

	rs485_init();
	relay_init();
}

int
main(int argc, char *argv[])
{
	int  cal = 0;

	cal = utk_init(0);
	if (cal < 0) exit(0);
	af_init(KEY_TONE, ALARM_PATH);
	atexit(on_quit);
	init_signals();

	system_conf();
	start_ncfg_thread();
	start_query_thread();
	build_login_gui();
	build_tscal_ui();
	build_main_gui();
	build_map_gui();
	build_einfo_gui();
	build_pzone_gui();
	build_zone_gui();
	build_pzck_gui();
	build_tmr_gui();
	build_tmrck_gui();
	build_ainfo_gui();
	build_zinfo_gui();
	build_sysp_gui();
	build_usrm_gui();
	build_ucfg_gui();
	build_gencfg_gui();
	build_zmgr_gui();
	build_zcfg_gui();
	build_zalrm_gui();
	build_arment_gui();
	build_bpent_gui();
	build_stent_gui();
	build_syst_gui();
	build_host_gui();
	build_slave_gui();
	build_sysnet_gui();
	build_sysuart_gui();
	build_sysgen_gui();
	build_systmr_gui();
	build_sysalr_gui();

	rs485_start();
	start_service();
	start_netif();
	init_qtimer();

	if (cal == 1) {
		show_tscal();
	} else {
		utk_desktop_show();
	}

	utk_main();
	utk_uninit();

	exit(0);
}


