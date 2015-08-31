#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

int
main(int argc, char *argv[])
{
	open_relay();
	while (1) {
		relay_on(1);
		sleep(1);
		relay_on(0);
		sleep(1);
	}
	exit(0);
}

