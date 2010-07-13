/*-
 * Copyright (c) 2008-2009, Kohsuke Ohtani
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
 * spmpuart.c - spmp305x UART driver
 */

#include <driver.h>
#include <tty.h>
#include <serial.h>

#include "platform.h"

#define DEBUG_SPMPUART

#ifdef DEBUG_SPMPUART
#define DPRINTF(a) printf (a)
#else
#define DPRINTF(a)
#endif

#define EXT_CLK		12000000
#define	UART_SPEED	115200
#define	UART_CLKDIV	EXT_CLK / UART_SPEED

#define UART_IRQCLR		(*(volatile uint8_t*)(0x100010c0))

#define	UART_N	1
#define SERIAL_IRQ	3

/* Forward functions */
static int	spmpuart_init(struct driver *);
static void	spmpuart_xmt_char(struct serial_port *, char);
static char	spmpuart_rcv_char(struct serial_port *);
static void	spmpuart_set_poll(struct serial_port *, int);
static void	spmpuart_start(struct serial_port *);
static void	spmpuart_stop(struct serial_port *);


struct driver spmpuart_driver = {
	/* name */	"spmp305x UART driver",
	/* devops */	NULL,
	/* devsz */	0,
	/* flags */	0,
	/* probe */	NULL,
	/* init */	spmpuart_init,
	/* detach */	NULL,
};

static struct serial_ops spmpuart_ops = {
	/* xmt_char */	spmpuart_xmt_char,
	/* rcv_char */	spmpuart_rcv_char,
	/* set_poll */	spmpuart_set_poll,
	/* start */	spmpuart_start,
	/* stop */	spmpuart_stop,
};

static struct serial_port spmpuart_port;


static void
spmpuart_xmt_char(struct serial_port *sp, char c)
{
	while(UART_STATUS(UART_N) & UART_TX_BUSY);
	UART_FIFO(UART_N) = c;
}

static char
spmpuart_rcv_char(struct serial_port *sp)
{
	while (!(UART_STATUS(UART_N) & UART_RX_VALID));
	return UART_FIFO(UART_N);
}

static void
spmpuart_set_poll(struct serial_port *sp, int on)
{
	volatile unsigned char * uart_base = (volatile unsigned char*)UART(UART_N);
	
	if (on) uart_base[0x6] = 0;			// disable irq
	else uart_base[0x6] = UART_IRQ_RX;	// enable irq
}

static int
spmpuart_isr(void *arg)
{
	struct serial_port *sp = arg;
	uint32_t irq_lo;
	char c;
	
//	UART_IRQCLR = 8;

	while(1) {
		if ((UART_STATUS(UART_N) & 4) == 4) break;
		c = UART_FIFO(UART_N);
		if (c != 0) serial_rcv_char(sp, c);
		else break;
	}

	return 0;
}

static void
spmpuart_start(struct serial_port *sp)
{
	uint32_t flag;
	
	/* Initialize port */
	volatile unsigned char  * uart_base   = (volatile unsigned char*)UART(UART_N);
	volatile unsigned short * uart_clkdiv = *((volatile unsigned short*)UART(UART_N));
	
	/* Magic values gleaned from the disasm */
	uart_clkdiv    = UART_CLKDIV;
	uart_base[0x4] = 0xD0;
	uart_base[0x5] = 0x11;
	uart_base[0xF] = 0x88;
//	uart_base[0x9] = 4;
	uart_base[0x6] = UART_IRQ_RX;
	
	sp->irq = irq_attach(SERIAL_IRQ, IPL_COMM, 0, &spmpuart_isr, IST_NONE, sp);

	IRQ_ENABLE_LO |= 1 << SERIAL_IRQ;
	IRQ_MASK_LO |= 1 << SERIAL_IRQ;

	return 0;
}

static void
spmpuart_stop(struct serial_port *sp)
{
	DPRINTF("[spmpuart] : STOP\n");
}

static int
spmpuart_init(struct driver *self)
{
	serial_attach(&spmpuart_ops, &spmpuart_port);
	return 0;
}
