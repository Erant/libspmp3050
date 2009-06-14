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
#include <math.h>

#define MAX_BUTTONS 12
#define BUT_LEFT		0x1
#define BUT_RIGHT		0x2
#define BUT_UP			0x4
#define BUT_DOWN		0x8
#define	BUT_A			0x10
#define BUT_B			0x20
#define	BUT_X			0x40
#define	BUT_OK			0x80
#define	BUT_VOL_UP		0x100
#define	BUT_VOL_DOWN	0x200
#define	BUT_ESC			0x400
#define BUT_PICTURE		0x800



char buttons[MAX_BUTTONS][32] = {"left","right","up", "down", "A", "B", "X", "OK", "volume up", "volume down", "ESC", "picture"};
uint32_t button_map[] = {BUT_LEFT, BUT_RIGHT, BUT_UP, BUT_DOWN, BUT_A, BUT_B, BUT_X, BUT_OK, BUT_VOL_UP, BUT_VOL_DOWN, BUT_ESC, BUT_PICTURE};

int main(int argc, char *argv[])
{
	device_t but_dev;
	button but_struct;
	uint32_t raw, prev_raw = 0;
	int raw_size = sizeof(raw);
	int i = 0;
	
	if(device_open("buttons", 0, &but_dev)){
		printf("Couldn't open button device, exiting...\n");
		return -1;
	}

	for(; i < MAX_BUTTONS; i++){
		printf("Please press the %s button.\n", buttons[i]);
		prev_raw = 0;
		do{
			device_ioctl(but_dev, BUT_IOC_GET_RAW, &raw);
		}while(raw == prev_raw);

		but_struct.dst_mask = button_map[i];
		but_struct.src_mask = raw;
		device_ioctl(but_dev, BUT_IOC_ADD_MAP, &but_struct);

		do{
			device_ioctl(but_dev, BUT_IOC_GET_RAW, &raw);
		}while(raw);
	}

	while(1){
		device_read(but_dev, &raw, &raw_size, 0);
		if(raw != prev_raw){
			printf("Buttons pressed: %s", raw ? "" : "none.");
			for(i = 0; i < MAX_BUTTONS; i++){
				if(raw & button_map[i])
					printf("%s ", buttons[i]);
			}
			printf("\n");
		}
		prev_raw = raw;
	}

	device_close(but_dev);
}
