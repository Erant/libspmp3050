/*-
 * Copyright (c) 2008, Tristan Schaap
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
 * gpio.c - GPIO device driver for the SPMP
 */

#include <driver.h>
#include <sys/ioctl.h>
#include "../../../sys/arch/arm/spmp/platform.h"

/* Forward functions */
static int gpio_init(void);
static int gpio_read(device_t, char *, size_t *, int);
static int gpio_write(device_t, char *, size_t *, int);
static int gpio_ioctl(device_t, u_long, void *);

/*
 * Driver structure
 */
struct driver gpio_drv = {
    /* name */	"GPIO Driver",
    /* order */	4,
    /* init */	gpio_init,
};

/*
 * Device I/O table
 */
static struct devio gpio_io = {
    /* open */	NULL,
    /* close */	NULL,
    /* read */	gpio_read,
    /* write */	gpio_write,
    /* ioctl */	gpio_ioctl,
    /* event */	NULL,
};

static device_t gpio_dev;	/* device object */

static int gpio_read(device_t dev, char *buf, size_t *nbyte, int blkno)
{
	if(*nbyte >= sizeof(uint64_t)){
		*((uint64_t*)buf) = GPIO_A_IN | ((uint64_t)GPIO_B_IN) << 32;
		return sizeof(uint32_t);
	}
	if(*nbyte >= sizeof(uint32_t)){
		*((uint32_t*)buf) = GPIO_A_IN;
		return sizeof(uint32_t);
	}
	if(*nbyte >= sizeof(uint16_t)){
		*((uint16_t*)buf) = (uint16_t)GPIO_A_IN;
		return sizeof(uint16_t);
	}
	if(*nbyte >= sizeof(uint8_t)){
		*((uint8_t*)buf) = (uint8_t)GPIO_A_IN;
		return sizeof(uint8_t);
	}
	return 0;
}

static int gpio_write(device_t dev, char *buf, size_t *nbyte, int blkno)
{
	return 0;
}

static int gpio_ioctl(device_t dev, u_long cmd, void *arg)
{
	switch(cmd){
	case GPIO_IOC_GET:
		((uint32_t*)arg)[0] = GPIO_A_IN;
		((uint32_t*)arg)[1] = GPIO_B_IN;
		return 0;
	case GPIO_IOC_SET:
		GPIO_A_OUT |= ((uint32_t*)arg)[0];
		GPIO_B_OUT |= ((uint32_t*)arg)[1];
		return 0;
	case GPIO_IOC_CLEAR:
		GPIO_A_OUT &= ~((uint32_t*)arg)[0];
		GPIO_A_OUT &= ~((uint32_t*)arg)[1];
		return 0;
	case GPIO_IOC_SET_DIR_OUT:
		GPIO_A_DIR |= ((uint32_t*)arg)[0];
		GPIO_B_DIR |= ((uint32_t*)arg)[1];
		return 0;
	case GPIO_IOC_SET_DIR_IN:
		GPIO_A_DIR &= ~((uint32_t*)arg)[0];
		GPIO_B_DIR &= ~((uint32_t*)arg)[1];
		return 0;
	}
	return -1;
}

/*
 * Initialize
 */

static int gpio_init(void)
{
	/* Initialize port */
	

	/* Register the device */
	gpio_dev = device_create(&gpio_io, "gpio", DF_CHR);
	ASSERT(gpio_dev);
	return 0;
}
