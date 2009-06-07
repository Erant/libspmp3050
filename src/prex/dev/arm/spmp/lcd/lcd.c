#include <driver.h>

#include <sys/ioctl.h>
#include <sys/tty.h>
#include <prex/ioctl.h>

#include "lcd_util.h"

/* Forward functions */
static int lcd_init(void);
static int lcd_open(device_t dev, int mode);
static int lcd_close(device_t dev);
static int lcd_read(device_t, char *, size_t *, int);
static int lcd_write(device_t, char *, size_t *, int);
static int lcd_ioctl(device_t, u_long, void *);

/*
 * Driver structure
 */
struct driver lcd_drv = {
	/* name */	"LCD controller",
	/* order */	4,
	/* init */	lcd_init,
};

/*
 * Device I/O table
 */
static struct devio lcd_io = {
	/* open */	NULL,
	/* close */	NULL,
	/* read */	NULL,
	/* write */	NULL,
	/* ioctl */	lcd_ioctl,
	/* event */	NULL,
};

static device_t lcd_dev;	/* device object */

/* wrappers for lcd_util */
static int lcd_init(void)
{
  int deviceType = 3;

  /* Create LCD device as an alias of the registered device. */
  lcd_dev = device_create(&lcd_io, "lcd", DF_CHR);
  if (lcd_dev == DEVICE_NULL)
    return -1;

  LCD_Init( deviceType );

  return 0;
}

static int lcd_ioctl(device_t dev, u_long cmd, void *arg)
{
	switch(cmd){
		case LCDIOC_SET_FB:
			/* printf("Setting framebuffer to %08X.\n", *((void**)arg)); */
			LCD_SetFramebuffer(arg);
			return 0;
		case LCDIOC_SET_BACKLIGHT:
			/* printf("Setting backlight.\n"); */
			LCD_SetBacklight(((int)arg));
			return 0;
		case LCDIOC_DRAW:
			/* printf("Drawing framebuffer.\n"); */
			LCD_Draw();
			return 0;
	}
	printf("Whoops, wrong ioctl received!\n");
	return -1;
}

