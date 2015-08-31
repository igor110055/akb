#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <errno.h>
#include <limits.h>

#ifdef OPEN_MAX
static int openmax = OPEN_MAX;
#else
static int openmax = 0;
#endif

#define OPEN_MAX_GUESS	1024
#define PATH   "/home/akbd"

static int
open_max(void)
{
	if(openmax==0){
		errno = 0;
		if ((openmax = sysconf(_SC_OPEN_MAX))<0){
			if(errno==0)
				openmax = OPEN_MAX_GUESS;	
		}
	}
	return openmax;
}

static int
daemon_init(void)
{
	struct sigaction   act;
	int    i, maxfd;

	if (fork() != 0) exit(0);
	if (setsid()<0) return -1;
	act.sa_handler = SIG_IGN;
	sigemptyset (&act.sa_mask);
	act.sa_flags = 0;
	sigaction(SIGHUP,&act,0);
	
	if (fork() != 0) exit(0);
	chdir("/home");
	umask(0);
	maxfd = open_max();
	for(i=0;i<maxfd;i++)
		close(i);

	open("/dev/null", O_RDWR);
	dup(0);
	dup(1);
	dup(2);
	return 0;
}

static pid_t
r_waitpid(pid_t pid, int *stat_loc, int options)
{
	int retval;

	while (((retval = waitpid(pid, stat_loc, options)) == -1) && (errno == EINTR));
	return retval;
}

int
main(int argc, char *argv[])
{
	pid_t ret;
	pid_t pid = -1;

	daemon_init();

	if ((pid = fork()) == 0) {
		printf("--- execute the child process ---\n");
		execl(PATH, PATH, (char *)0);
	} else if (pid > 0) {				
		for (;;) {
			ret = r_waitpid(pid, NULL, 0);
			printf("--- the child process exit ---\n");
			if (ret == pid||(ret == -1&&errno == ECHILD))
			{
				printf("--- fork the child process ---\n");
				if ((pid = fork()) == 0) {
					execl(PATH, PATH, (char *)0);
				}
			}
			sleep(5);
		}
	}
	return 0;
}

