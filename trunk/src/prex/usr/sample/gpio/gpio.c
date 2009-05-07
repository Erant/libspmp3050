/*
 * Copyright (c) 2005-2006, Tristan Schaap
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

int main(int argc, char *argv[]){
    uint32_t buttons[2];
    uint32_t prev_buttons[] = {0, 0};
    int fd = open( "/dev/gpio", O_RDWR );
    if (fd >= 0){
	printf("Got GPIO device in fd %d\n", fd);
    }
    else
    {
	printf("Opening GPIO device failed, exiting.\n");
	return -1;
    }
    printf("Reading buttons:\n");
    while(1){
        read(fd, &buttons, sizeof(buttons));
	if(prev_buttons[0] != buttons[0] || prev_buttons[1] != buttons[1])
	    printf("GPIO_A: 0x%08X\nGPIO_B: 0x%08X\n", buttons[0], buttons[1]);

	prev_buttons[0] = buttons[0];
	prev_buttons[1] = buttons[1];
    }

    /* We never get here :P */
    close(fd);
}
