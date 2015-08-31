#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <unistd.h>

#define KB_IOC_MAGIC    0xfd
#define KB_DEJITTER     _IOW(KB_IOC_MAGIC, 1, int)
#define KB_EXTIN        _IOW(KB_IOC_MAGIC, 2, int)
#define KB_START        _IO(KB_IOC_MAGIC, 3)

static char  cmd[64];

int
main(int argc, char *argv[])
{
	fd_set  rset;
	int  fd = -1;
	int  r;
	int  i;

	fd = open("/dev/kbinp", O_RDONLY);
	if (fd < 0) {
		printf("--- open /dev/kbinp failed ---\n");
		exit(0);
	}
	
	if (argc == 3) {
		int arg = atoi(argv[1]);
		ioctl(fd, KB_DEJITTER, arg);
		arg = atoi(argv[2]);
		ioctl(fd, KB_EXTIN, arg);
	}
	
	ioctl(fd, KB_START, 0);

	while (1) {
		FD_ZERO(&rset);
		FD_SET(fd, &rset);
		r = select(fd+1, &rset, NULL, NULL, NULL);
		if (r > 0&&FD_ISSET(fd, &rset)) {
			r = read(fd, cmd, sizeof(cmd));
			if (r > 0) {
				for (i = 0; i < r; ++i) {
					printf("------ key-%d: %s -------\n", (cmd[i]>>1), (cmd[i]&1)?"rising":"falling");
				}
			}
		}
	}
	close(fd);
	printf("--- exit ---");

	exit(0);
}


