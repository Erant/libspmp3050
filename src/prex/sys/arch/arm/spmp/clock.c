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
 * clock.c - clock driver
 */

#include <kernel.h>
#include <timer.h>
#include <irq.h>

/* Interrupt vector for timer (TMR1) */
#define CLOCK_IRQ	5

/* The clock rate per second - 1Mhz */
#define CLOCK_RATE	1000000L

/* The initial counter value */
#define TIMER_COUNT	(CLOCK_RATE / HZ)

/* Timer 1 registers */
#define	IO_BASE				0x10000000
#define IRQ_FLAG_LO			(*(volatile uint32_t*)(IO_BASE + 0x10C0))
#define IRQ_FLAG_HI			(*(volatile uint32_t*)(IO_BASE + 0x10C4))
#define IRQ_ENABLE_LO			(*(volatile uint32_t*)(IO_BASE + 0x10D0))
#define TIMER_PERIOD(n)			(*(volatile uint16_t*)(IO_BASE + 0x1318 + (n << 1)))
#define TIMER_ENABLE			(*(volatile uint8_t*)(IO_BASE + 0x1044))
#define TIMER_COUNTER(n)		(*(volatile uint32_t*)(IO_BASE + 0x1030 + (n << 2)))
#define TIMER_FLAGS(n)			(*(volatile uint8_t*)(IO_BASE + 0x1040 + n))
#define TIMER_REPEAT			0x10

void clear_interrupt(int nr) {
	if(nr > 31){
		IRQ_FLAG_HI |= 1 << (nr - 32);
		return;
	}
	IRQ_FLAG_LO |= 1 << nr;
}

/*
 * Clock interrupt service routine.
 * No H/W reprogram is required.
 */
static int
clock_isr(int irq)
{

	irq_lock();
	printf("tick\n");
	timer_tick();
	clear_interrupt(4);	/* Clear timer interrupt */
	irq_unlock();

	return INT_DONE;
}

/*
 * Initialize clock H/W chip.
 * Setup clock tick rate and install clock ISR.
 */
void
clock_init(void)
{
	irq_t clock_irq;
	
	/* setup timer values */
	int timer = 0;
	int period = 12000;
	int div = 10;
	uint8_t flags = TIMER_REPEAT;
	TIMER_PERIOD(timer) = period - 1;
	TIMER_COUNTER(timer) = (div * 100) - 1;
	TIMER_FLAGS(timer) = flags;

	/* Install ISR */
	clock_irq = irq_attach(CLOCK_IRQ, IPL_CLOCK, 0, &clock_isr, NULL);
	ASSERT(clock_irq != NULL);

	/* enable timer */
	TIMER_ENABLE |= 1 << timer;
	
	/* enable timer interrupt */
	IRQ_ENABLE_LO |= 1 << (timer + 4);

	DPRINTF(("Clock rate: %d ticks/sec\n", CONFIG_HZ));
}
