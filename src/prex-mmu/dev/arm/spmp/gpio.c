/*-
 * Copyright (c) 2009, Tristan Schaap
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
static int gpio_read(device_t dev, char *buf, size_t *nbyte, int blkno);
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
    /* write */	NULL,
    /* ioctl */	gpio_ioctl,
    /* event */	NULL,
};

static device_t gpio_dev;	/* device object */


static int gpio_read(device_t dev, char *buf, size_t *nbyte, int blkno)
{
	if(*nbyte < sizeof(uint32_t))
		return -1;

	*nbyte = sizeof(uint32_t);

	switch(blkno){
		case GPIO_IOC_BANK_A:
			(*(uint32_t*)buf) = GPIO_A_IN;
			return 0;
		case GPIO_IOC_BANK_B:
			(*(uint32_t*)buf) = GPIO_B_IN;
			return 0;
	}
	return -1;
}

/*
static int gpio_write(device_t dev, char *buf, size_t *nbyte, int blkno)
{
	return 0;
}

*/

uint32_t* gpio_bank[2][GPIO_IOC_NR_BANKS] = {{&GPIO_A_OUT, &GPIO_B_OUT}, {&GPIO_A_DIR, &GPIO_B_DIR}};

static int gpio_ioctl(device_t dev, u_long cmd, void *arg)
{
	uint32_t val;
	gpio_ioc_struct* buf;
	switch(cmd){
	case GPIO_IOC_GET:
		((uint32_t*)arg)[0] = GPIO_A_IN;
		((uint32_t*)arg)[1] = GPIO_B_IN;
		return 0;
	case GPIO_IOC_SET:
		buf = ((gpio_ioc_struct*)arg);
		val = *(gpio_bank[buf->gpio_reg_type][buf->gpio_bank]);
		val &= ~(buf->mask);
		val |= buf->val;
		*(gpio_bank[buf->gpio_reg_type][buf->gpio_bank]) = val;
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
