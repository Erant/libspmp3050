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

void printhex(uint8_t *buf, size_t n, uint32_t addr, int linew){
	size_t pos,i, l, j;
	for (pos=0;pos<n;pos+=linew,addr+=linew){
		l = pos+linew>n?n:pos+linew;
		printf("%08X   ");
		for (i=pos,j=0;i<pos+linew;i++,j++){
			if (i<n){
				printf("%02X ", buf[i]);
			}
			else
			{
				printf("   ");
			}
			if(!((j+1)%4))
				printf(" ");
		}
		if (linew%4)
			printf(" ");
		for (i=pos;i<l;i++){
			if(buf[i]>=0x20&&buf[i]<=0x7F)
				printf("%c", buf[i]);
			else
				printf("%c", '.');
		}
		printf("\n");
	}
}


int main(int argc, char *argv[])
{
	char buf[512];
	int i = 0;
    int fd = open( "/dev/nand", O_RDONLY );
    if (fd < 0){
		printf("Failed to open NAND device\n");
		return 0;
	}
	for(; i < 10; i++){
		read(fd, buf, sizeof(buf));
		printhex(buf, sizeof(buf), i * sizeof(buf), 16);
	}
}

