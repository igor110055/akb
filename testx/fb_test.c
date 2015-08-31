#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "mxc_sdk.h"

int
main(int argc, char *argv[])
{
	RECT  bg = {.left=0, .top=0, .w=1024, .h=600};

	RECT  fg = {.left=0, .top=0, .w=800, .h=480};
	RECT  disp = {.left=80, .top=0, .w=640, .h=480};
	unsigned char  r = 0xff;
	unsigned char  g = 0xff;
	unsigned char  b = 0xff;
	int i, j, k;
	unsigned short  clr[4] = {RGB_RED, RGB_GREEN, RGB_BLUE, RGB_WHITE};

	if (system_init() < 0) {
		printf("--- system init error ---\n");
		exit(0);
	}

	while (1) {
	mxc_fb_set_color(&fg, RGB_BLUE);
	sleep(2);
	mxc_fb_set_color(&bg, RGB_WHITE);
	sleep(2);
	for (i = 1; i <= 2; ++i) {
		r = 0xff - 0x7f*i;
		for (j = 1; j <= 2; ++j) {
			g = 0xff - 0x7f*j;
			for (k = 1; k <= 2; ++k) {
				b = 0xff - 0x7f*k;
				mxc_fb_set_color(&bg, RGB565(r, g, b));
				sleep(1);
			}
		}
	}

	mxc_fb_set_color(&bg, RGB_RED);
	sleep(2);

	mxc_lcd_on(0);
	sleep(3);
	mxc_lcd_on(1);
/*
	mxc_fb_set_color(&bg, RGB_WHITE);
	for (i=0; i<9; ++i) {
		mxc_fb_set_brightness((unsigned char)i);
		sleep(1);
	}
*/
	mxc_fb_set_color(&bg, RGB_RED);
	sleep(3);
	
	mxc_fb_set_color(&bg, RGB_GREEN);
	sleep(3);
	
	mxc_fb_set_color(&bg, RGB_BLUE);
	sleep(3);
	}

/*
	while (1) {
		dav_fb_fill_rect_bf(0, 0, 1024, 600, clr[i]);
		dav_fb_show();
		sleep(2);
		i = (i+1)&0x03;
	}
*/
		
	system_uninit();

	exit(0);
}


