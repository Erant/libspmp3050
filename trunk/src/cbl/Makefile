OBJECTS=crt0.o gpio.o cbl.o uart.o xmodem.o
GCCPREFIX=~/arm/usr/bin/arm-crosstool-linux-gnueabi

cbl.bin:	cbl.elf
	$(GCCPREFIX)-objcopy -O binary cbl.elf cbl.bin

cbl.elf:	$(OBJECTS)
	$(GCCPREFIX)-ld -T ldscript $(OBJECTS) -o cbl.elf

%.o:	%.c
	$(GCCPREFIX)-gcc -Wall -c -mcpu=arm926ej-s -O2 -std=c99 $<