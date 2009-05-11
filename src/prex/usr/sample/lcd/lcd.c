/*
 * Copyright (c) 2005-2006, Kohsuke Ohtani
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the author nor the names of any co-contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <prex/prex.h>
#include <prex/ioctl.h>
#include <sys/ioctl.h>
#include "logo.h"

#define RGB(r,g,b)	((((r >> 3) & 0x1F) << 11) | (((g >> 2) & 0x3F) << 5) | ((b >> 3) & 0x1F))
int main(int argc, char *argv[])
{
	device_t lcddev;
	uint16_t* fb;
	uint8_t pixel[3];
	uint8_t* image = header_data;
	int i, j;
	if(device_open("lcd", 0, &lcddev)){
		printf("Couldn't open LCD device, exiting...\n");
		return -1;
	}

	/* Can result in unaligned framebuffer! */
	fb = malloc(320 * 240 * 2);
	/* fb = memalign(0x400, 320 * 240 * 2); seems to be broken in Prex... */
	if(!fb){
		printf("Couldn't allocate framebuffer, exiting...\n");
		return -1;
	}

	device_ioctl(lcddev, LCDIOC_SET_FB, fb);
/*
	printf("Now drawing alternating patterns to the framebuffer for 60 seconds.\n");
	for(j = 0; j < 0xFFFF; j++){
		for(i = 0; i < 320 * 240; i++)
			fb[i] = j * i;

		device_ioctl(lcddev, LCDIOC_DRAW, NULL);
		timer_sleep(20, 0);
	}
*/
	for(j = 0; j < 320 * 240; j++){
		HEADER_PIXEL(image, pixel);
		fb[j] = RGB(pixel[0], pixel[1], pixel[2]);
	}
	device_ioctl(lcddev, LCDIOC_DRAW, NULL);
	device_ioctl(lcddev, LCDIOC_SET_BACKLIGHT, 1);
	device_close(lcddev);
}
