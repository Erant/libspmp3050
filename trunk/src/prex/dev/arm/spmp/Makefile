include $(SRCDIR)/mk/own.mk

TARGET=		dev.o
TYPE=		OBJECT
OBJS-y= 	../arm/start.o

OBJS-$(CONFIG_SERIAL)+=	        serial.o
OBJS-$(CONFIG_LCD) += lcd/lcd.o lcd/lcd_util.o lcd/lcd_init.o
OBJS-$(CONFIG_BUTTON) += buttons.o
OBJS-$(CONFIG_GPIO) += gpio.o
OBJS-$(CONFIG_NAND) += nand.o

include $(SRCDIR)/mk/dev.mk
