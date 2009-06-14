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
 * buttons.c - Button device driver for the SPMP
 */

#include <driver.h>
#include <sys/ioctl.h>
#include <prex/prex.h>
#include "../../../sys/arch/arm/spmp/platform.h"

#define BUT_ROWS	3
#define	BUT_COLS	4

/* Forward functions */
static int but_init(void);
static int but_read(device_t dev, char *buf, size_t *nbyte, int blkno);
static int but_ioctl(device_t, u_long, void *);

/*
 * Driver structure
 */
struct driver but_drv = {
    /* name */	"Button Driver",
    /* order */	5,
    /* init */	but_init,
};

/*
 * Device I/O table
 */
static struct devio but_io = {
    /* open */	NULL,
    /* close */	NULL,
    /* read */	but_read,
    /* write */	NULL,
    /* ioctl */	but_ioctl,
    /* event */	NULL,
};

/*
 * Globals
 */

static device_t but_dev;	/* device object */
static device_t gpio_dev;	/* Device to read from */
static button but_map[32];
static int but_map_size = 0;

static uint32_t read_button_array(){
	int i = 0;
	int size = sizeof(uint32_t);
	uint32_t ret = 0;
	uint32_t read;
/*
	gpio_ioc_struct buf;

	buf.gpio_bank = GPIO_IOC_BANK_A;
	buf.gpio_reg_type = GPIO_IOC_OUT;
	buf.mask = 0x70;
	buf.val = 0x10;

	device_ioctl(gpio_dev, GPIO_IOC_SET, buf);
*/
	for(; i < BUT_ROWS; i++){
		GPIO_A_OUT &= ~0x70;
		GPIO_A_OUT |= 0x10 << i;

		/* device_read(gpio_dev, &read, &size, GPIO_IOC_BANK_A); */
		
		ret <<= BUT_COLS;
		read = GPIO_A_IN;
		ret |= (read >> 8) & 0xF;

		/* buf.val <<= 1;
		device_ioctl(gpio_dev, GPIO_IOC_SET, buf);
		*/
	}
	/*
	*((uint32_t*)GPIO_A) &= ~0xF00;
	GPIO_A_OUT = 0x0000001F;
	GPIO_A_DIR = 0x0000007F;
	ret = GPIO_A_IN;
	*/
	return ret;
}

static uint32_t translate_buttons(uint32_t val){
	int i = 0;
	uint32_t ret = 0;
	for(; i < but_map_size; i++){
		if(but_map[i].src_mask & val)
			ret |= but_map[i].dst_mask;
	}
	return ret;
}

static int but_read(device_t dev, char *buf, size_t *nbyte, int blkno)
{
	uint32_t buttons = read_button_array();
	*((uint32_t*)buf) = translate_buttons(buttons);
	return 0;
}

static int but_ioctl(device_t dev, u_long cmd, void *arg)
{
	int i = 0;
	switch(cmd){
		case BUT_IOC_ADD_MAP:
			if(but_map_size >= 32)
				return -1;

			but_map[but_map_size] = *((button*)arg);
			but_map_size++;
			return 0;
		case BUT_IOC_DEL_MAP:
			for(; i < but_map_size; i++){
				if(but_map[i].src_mask == ((button*)arg)->src_mask){
					but_map_size--;
					/* memmove(&but_map[i], &but_map[i+1], but_map_size - i); */
					return 0;
				}
			}

		case BUT_IOC_CLEAR_MAP:
			but_map_size = 0;
			return 0;
		case BUT_IOC_GET_RAW:
			*((uint32_t*)arg) = read_button_array();
			return 0;
	}

	return -1;
}

/*
 * Initialize
 */

static int but_init(void)
{
	gpio_ioc_struct buf;
	/* Register the device */
	but_dev = device_create(&but_io, "buttons", DF_CHR);
	ASSERT(but_dev);
/*
	if(device_open("gpio", DO_RDWR, &gpio_dev)){
		printf("buttons.c: Couldn't open GPIO device.\n");
		return -1;
	}
*/
	GPIO_A_PULL_ENABLE |= 0xF00;
	GPIO_A_PULL_SELECT &= ~0xF00;
	GPIO_A_DIR &= ~0xF00;
	GPIO_A_DIR |= 0x070;
	GPIO_A_OUT &= ~0x70;
	GPIO_A_OUT |= 0x10;

/*
	buf.gpio_bank = GPIO_IOC_BANK_A;
	buf.gpio_reg_type = GPIO_IOC_DIR;
	buf.mask = 0xF70;
	buf.val = 0x070;

	device_ioctl(gpio_dev, GPIO_IOC_SET, buf);
	
	buf.gpio_reg_type = GPIO_IOC_OUT;
	buf.mask = 0x70;
	buf.val = 0;

	device_ioctl(gpio_dev, GPIO_IOC_SET, buf);
*/
	but_map_size = 0;
	return 0;
}
