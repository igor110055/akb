#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

static unsigned char  cmd[64];

int
main(int argc, char *argv[])
{
	int  i = 0;
	int  v = 0;
	char c;
	int  gfd = -1;

	gfd = open("/dev/gpio", O_WRONLY);
	if (gfd < 0) {
		printf("--- open gpio failed ---\n");
		exit(0);
	}

	if (argc == 1) {
		for (i = 0; i < 64; ++i) {
			cmd[i] = (i << 1)|1;
		}

		while (1) {
			sleep(2);
			write(gfd, cmd, 64);
			v ^= 1;
			if (v == 0) {
				for (i = 0; i < 64; ++i) {
					cmd[i] &= ~1;
				}
			} else {
				for (i = 0; i < 64; ++i) {
					cmd[i] |= 1;
				}
			}
		}
	} else if (argc == 2) {
		i = atoi(argv[1]);
		c = (i << 1)|1;
		write(gfd, &c, 1);
		sleep(3);
		c = (i << 1);
		write(gfd, &c, 1);
	} else if (argc == 3) {
		
		i = atoi(argv[1]);
		v = atoi(argv[2]);
		c = i << 1;
		if (v) {
			c |= 1;
		}
		write(gfd, &c, 1);
	}
	close(gfd);
	exit(0);
}


