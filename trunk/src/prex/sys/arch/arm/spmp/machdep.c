/*-
 * Copyright (c) 2008, Kohsuke Ohtani
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

/*
 * machdep.c - machine-dependent routines for the SPMP305x
 */

#include <kernel.h>
#include <cpu.h>
#include <page.h>
#include <syspage.h>
#include <locore.h>
#include <cpufunc.h>
#include <platform.h>

/*
 * Reset system.
 */
void
machine_reset(void)
{
}

/*
 * Idle
 */
void
machine_idle(void)
{
}

/*
 * Set system power
 */
void
machine_setpower(int state)
{
}

/*
 * Machine-dependent startup code
 */
void
machine_init(void)
{
	printf("machine init\n");
	
	cpu_init();
	cache_init();

	page_reserve(SYSPAGE_BASE, SYSPAGE_SIZE);
	vector_copy(SYSPAGE_BASE);
	
	#define REG(x) (*(volatile uint8_t*)(IO_BASE + x))
	
	/* REG(8) = 0x79; */
	/* REG(0x111) = 0xB2; */
	/* Unit turn on */
	GPIO_DISABLE = 0x00;
	GPIO_A_DIR |= 0x2;
	GPIO_A_OUT |= 0x2;

	*((uint32_t*)0x10000008) = 0xFFFFFFFF;
	*((uint32_t*)0x10000110) = 0xFFFFFFFF;

/*
 * This is majorly broken! GPIO madness.
	{
		uint8_t val;
		
		val = REG(0xB1) & 1;
		
		switch ((REG(0xB2) & 6) >> 1) {
			case 0:
				REG(0x136) = 0xB;
				if (val == 1) REG(0x132) = 0;
				break;
		
			case 1:
				REG(0x136) = 0xD;
				if (val == 1) {
					REG(0x123) = 6;
					REG(0x132) = 0;
				}
				break;
		
			case 2:
				REG(0x136) = 0xF;
				if (val == 1) {
					REG(0x123) = 7;
					REG(0x132) = 0;
				}
				break;
		}
	}
	SYS_REG = 1;
	*/
}
