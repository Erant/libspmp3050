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
//	while(UART_STATUS(UART_N) & UART_TX_BUSY);
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

	if (on) {
		/*
		 * Disable interrupt for polling mode.
		 */
		DPRINTF("[spmpuart] : enable polling mode\n");
	} else DPRINTF("[spmpuart] : disable polling mode\n");

}

static int
spmpuart_isr(void *arg)
{
	struct serial_port *sp = arg;
	uint32_t irq_lo;

DPRINTF("irq_ser\n");
	while(1) {
		if (!(UART_STATUS(UART_N) & UART_RX_VALID)) break;
		serial_rcv_char(sp, UART_FIFO(UART_N));
	}

	irq_lo = IRQ_FLAG_LO;
	IRQ_FLAG_LO = irq_lo | (1 << SERIAL_IRQ);

	return 0;
}

static void
spmpuart_start(struct serial_port *sp)
{
	uint32_t flag;
	
	DPRINTF("[spmpuart] : START\n");
	
	/* Initialize port */
	volatile unsigned char * uart_base = (volatile unsigned char*)UART(UART_N);
	
	/* Magic values gleaned from the disasm */
	uart_base[0x0] = 0x68;
	uart_base[0x1] = 0x00;
	uart_base[0x4] = 0xD0;
	uart_base[0x5] = 0x11;
	uart_base[0xF] = 0x88;
	
	UART_IRQ_REG(UART_N) |= UART_IRQ_RX;
	UART_ENABLE(UART_N);

	/* Create device object *
	serial_dev = device_create(&serial_io, "console", DF_CHR);
	ASSERT(serial_dev);

	tty_attach(&serial_io, &serial_tty);*/
	
	DPRINTF("[spmpuart] : START - attach irq\n");
	sp->irq = irq_attach(SERIAL_IRQ, IPL_COMM, 0, &spmpuart_isr, IST_NONE, sp);
	DPRINTF("[spmpuart] : START - attach irq done\n");
/*	ASSERT(serial_irq != NULL);
*/
	IRQ_ENABLE_LO |= 1 << SERIAL_IRQ;
	IRQ_MASK_LO |= 1 << SERIAL_IRQ;

	DPRINTF("[spmpuart] : initialized\n");
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
	DPRINTF("[spmpuart] : INIT\n");
//	serial_attach(&spmpuart_ops, &spmpuart_port);
	DPRINTF("[spmpuart] : INIT okay\n");
	return 0;
}
