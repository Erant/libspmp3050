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


#ifdef CONFIG_MMU

/*
 * Virtual and physical address mapping
 *
 *      { virtual, physical, size, type }
 */
struct mmumap mmumap_table[] =
{
	/*
	 * Internal SRAM (8M)
	 */
	{ 0xA4000000, 0x24000000, 0x800000, VMT_RAM },

	/*
	 * IO memory range (512M)
	 */
	{ 0x90000000, 0x10000000, 0x10000000, VMT_IO },

	{ 0,0,0,0 }
};
#endif

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

#ifdef CONFIG_CACHE
	cache_init();
#endif

	page_reserve(virt_to_phys(SYSPAGE_BASE), SYSPAGE_SIZE);
	vector_copy(ARM_VECTORS);

#ifdef CONFIG_MMU
	mmu_init(mmumap_table);
#endif
}
