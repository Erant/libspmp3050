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

#include <boot.h>
#include <platform.h>
#include "uart.h"

#define UART_NR 1

/*
 * Setup boot information.
 */
static void
bootinfo_setup(void)
{

	bootinfo->video.text_x = 80;
	bootinfo->video.text_y = 25;

	/*
	 * On-board SSRAM - 4M
	 */
	bootinfo->ram[0].base = 0x24;
	bootinfo->ram[0].size = 0x800000 - 0x24;
	bootinfo->ram[0].type = MT_USABLE;

	bootinfo->nr_rams = 1;
}

#ifdef DEBUG
#ifdef CONFIG_DIAG_SERIAL
/*
 * Put chracter to serial port
 */
static void
serial_putc(int c)
{
	UART_SendByte(UART_NR, (unsigned char)c);
}

/*
 * Setup serial port
 */
static void
serial_setup(void)
{
	UART_Init(UART_NR);
}
#endif /* !CONFIG_DIAG_SERIAL */

/*
 * Print one chracter
 */
void
machine_putc(int c)
{

#ifdef CONFIG_DIAG_SERIAL
	if (c == '\n')
		serial_putc('\r');
	serial_putc(c);
#endif /* !CONFIG_DIAG_SERIAL */
}
#endif /* !DEBUG */

/*
 * Panic handler
 */
void
machine_panic(void)
{

#if defined(DEBUG) && defined(CONFIG_DIAG_SERIAL)
	/* kernel panic (can I use printf here?) */
	printf("KERNEL PANIC OMG NOES!!!!!\n");
#endif

	/* spin to infinity */
	while(1);
}

/*
 * Setup machine state
 */
void
machine_setup(void)
{

#if defined(DEBUG) && defined(CONFIG_DIAG_SERIAL)
	serial_setup();
#endif
	bootinfo_setup();
}
