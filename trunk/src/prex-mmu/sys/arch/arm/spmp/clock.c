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
#include <platform.h>

/* Interrupt vector for timer (TMR1) */
#define CLOCK_IRQ	5
#define PERIOD		(1000 / CONFIG_HZ)

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
static int clock_isr(int irq)
{

	irq_lock();
	/* printf("tick\n"); */
	timer_tick();
	clear_interrupt(CLOCK_IRQ);	/* Clear timer interrupt */
	irq_unlock();

	return INT_DONE;
}

static void debug_interrupts_tightloop()
{
  /* monitor IRQ_FLAG for 10000000 iterations and print any changes to serial (should take about 5-10 seconds i guess) */
  static unsigned int lastflags_lo=0, lastflags_hi=0;
  unsigned int a;
      
  DPRINTF(("sitting in a pretty tight loop to see if things break : ["));
      
  for(a=0;a<1000000;a++)
    {
      unsigned int lo = IRQ_FLAG_LO;
      unsigned int hi = IRQ_FLAG_HI;
      if (lo != lastflags_lo)
        {
          DPRINTF(("a=%d, and irq_flag_lo went to %08x\n", a, lo ));
          lastflags_lo = lo;
        }
      if (hi != lastflags_hi)
        {
          DPRINTF(("a=%d, and irq_flag_hi went to %08x\n", a, hi));
          lastflags_hi = hi;
        }

      if ( (a&0xffff)==0)
        DPRINTF(("*"));
    }
  DPRINTF(("]\n"));
  DPRINTF(("looks like things are still ok.\n"));
}


/*
 * Initialize clock H/W chip.
 * Setup clock tick rate and install clock ISR.
 */
void clock_init(void)
{
	irq_t clock_irq;
	
	/* setup timer values */
	int timer = CLOCK_IRQ - 4;
	int period = 12 * PERIOD;
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
	IRQ_MASK_LO |= 1 << (timer + 4);

	DPRINTF(("Clock rate: %d ticks/sec\n", CONFIG_HZ));
}
