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

int
main(int argc, char *argv[])
{
  int x,y;
  int cpu_mul = 1, mem_mul = 3;
  struct info_timer info;
 	u_long start, end;

    int fd = open( "/dev/lcd", O_RDWR );
    if (fd!=-1)
      printf("got lcd device in fd %d\n", fd );
    else
      printf("NO LCD device driver in /dev/lcd, omgwtf\n");

 again:

  switch(argc){
	case 1:
		break;
	case 3:
		sscanf(argv[2], "%d", &mem_mul);
	case 2:
		sscanf(argv[1], "%d", &cpu_mul);
  }
  if(cpu_mul < 1)
	cpu_mul = 1;

  if(mem_mul < 3)
	mem_mul = 3;


  /* these should set the cpu and memory speed, but they screw up my serial, so i've removed them */

/*  *((uint8_t*)0x10000122) = cpu_mul; 
  *((uint8_t*)0x10000123) = mem_mul;
  *((uint8_t*)0x10000136) = 0xF;  */

  sys_info(INFO_TIMER, &info);
  printf("Hello World!\n");
  printf("we're running at %d hz\n", info.hz );
  for(x=0;x<10;x++)
    {
      sleep(1);
      printf("i=%d\n", x );
      if (fd!=-1)
        {
          unsigned char ch = x&1;
          write( fd,  &ch, 1  );
        }
    }

  sys_time(&start);


  float zoom = 3.0;
  for(y=0;y<40;y++)
    {
    for(x=0;x<80;x++)
      {
        const int MaxIterations = 64;
        const float Limit = 4;
        int numIterations = 0;
        float a1 = (x-40.0)*0.1/zoom;
        float ax = a1;
        float b1 = (y-20.0)*0.2/zoom;
        float ay = b1;
        
        do 
          {
            ++numIterations;
            float a2 = (a1 * a1) - (b1 * b1) + ax;
            float b2 = (2 * a1 * b1) + ay;
            if ((a2 * a2) + (b2 * b2) > Limit)
              break;
            
            ++numIterations;
            a1 = (a2 * a2) - (b2 * b2) + ax;
            b1 = (2 * a2 * b2) + ay;
            if ((a1 * a1) + (b1 * b1) > Limit)
              break;
          } while (numIterations < MaxIterations);
        
        {
          unsigned char c = " .-=xX%@"[numIterations&7];
          putchar( c );
        }

      }
      printf("\n");
}

  sys_time(&end); 
  *((uint32_t*)0x10000136) = 0xB;
  printf("\n\n\nmandelbrot took %d ticks\n", end-start );
  {
    int time = 1000*(end-start) / info.hz;
    printf("which should roughly be %d milliseconds\n", time );
  }

  execve("/boot/cmdbox", 0, 0);
  printf("exec failed, but we can still live on.. restarting..\n");
  goto again;

  return 0;
}
