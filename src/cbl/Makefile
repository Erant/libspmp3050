include Makefile.config
OBJECTS=gpio.o cbl.o uart.o xmodem.o lcd.o util.o crt0.o

cbl.bin:	cbl.elf
	$(GCCPREFIX)-objcopy -O binary cbl.elf cbl.bin

cbl.elf:	$(OBJECTS)
	$(GCCPREFIX)-ld -T ldscript $(OBJECTS) -o cbl.elf

%.o:	%.c
	$(GCCPREFIX)-gcc -Wall -c -mcpu=arm926ej-s -O2 -std=c99 $<

clean:	
	rm -f $(OBJECTS) cbl.bin cbl.elf
