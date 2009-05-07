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
 * serial.c - Serial console driver for SPMP
 */

#include <driver.h>
#include <sys/ioctl.h>
#include <sys/tty.h>
#include "../../../sys/arch/arm/spmp/platform.h"

#define TERM_COLS	80
#define TERM_ROWS	25

#define	UART_N		1	/* the only useful one? */

#define SERIAL_IRQ	3


/* Forward functions */
static int serial_init(void);
static int serial_read(device_t, char *, size_t *, int);
static int serial_write(device_t, char *, size_t *, int);
static int serial_ioctl(device_t, u_long, void *);
static void serial_putc(char c);

/*
 * Driver structure
 */
struct driver serial_drv = {
	/* name */	"Serial Console",
	/* order */	4,
	/* init */	serial_init,
};

/*
 * Device I/O table
 */
static struct devio serial_io = {
	/* open */	NULL,
	/* close */	NULL,
	/* read */	serial_read,
	/* write */	serial_write,
	/* ioctl */	serial_ioctl,
	/* event */	NULL,
};

static device_t serial_dev;	/* device object */
static struct tty serial_tty;	/* tty structure */
static irq_t serial_irq;	/* handle for irq */

static int serial_read(device_t dev, char *buf, size_t *nbyte, int blkno)
{
	return tty_read(&serial_tty, buf, nbyte);
}

static int serial_write(device_t dev, char *buf, size_t *nbyte, int blkno)
{
	return tty_write(&serial_tty, buf, nbyte);
}

static int serial_ioctl(device_t dev, u_long cmd, void *arg)
{

	return tty_ioctl(&serial_tty, cmd, arg);
}

static void serial_putc(char c)
{
	while (UART_STATUS(UART_N) & UART_TX_BUSY);
	UART_FIFO(UART_N) = c;
}

/*
 * Start output operation.
 */
static void serial_start(struct tty *tp)
{
	int c;

	sched_lock();
	while ((c = ttyq_getc(&tp->t_outq)) >= 0) {
		if (c == '\n')
			serial_putc('\r');
		serial_putc(c);
	}
	sched_unlock();
}

static int serial_buffer_empty(int uart){
	if(!(UART_STATUS(uart) & UART_RX_VALID))
		return 0;
	return 1;
}

/*
 * Interrupt service routine
 */
static int serial_isr(int irq)
{
	int c;
	while(!serial_buffer_empty(UART_N))
		tty_input(UART_FIFO(UART_N), &serial_tty);
	IRQ_FLAG_LO |= 1 << SERIAL_IRQ;	/* Clear the interrupt. */

	return 0;
}


static int serial_puts(char *str)
{
	char *pc = str;
	while (*pc != 0) serial_putc(*pc++);

	/* This shouldn't really go here, but what the hell... */
	tty_done(&serial_tty);
	return 0;
}

/*
 * Initialize
 */

static int serial_init(void)
{

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
	
	serial_puts("initializing serial tty\r\n");

	/* Create device object */
	serial_dev = device_create(&serial_io, "console", DF_CHR);
	ASSERT(serial_dev);

	tty_attach(&serial_io, &serial_tty);

	serial_puts("initializing serial isr\r\n");
	
	serial_irq = irq_attach(SERIAL_IRQ, IPL_COMM, 0, &serial_isr, NULL);
	ASSERT(serial_irq != NULL);
	
	IRQ_ENABLE_LO |= 1 << SERIAL_IRQ;
	IRQ_MASK_LO |= 1 << SERIAL_IRQ;

	serial_tty.t_oproc = serial_start;
	serial_tty.t_winsize.ws_row = (u_short)TERM_ROWS;
	serial_tty.t_winsize.ws_col = (u_short)TERM_COLS;
	return 0;
}
