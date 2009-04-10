@echo off
arm-eabi-gcc -Wall -c crt0.c pwr.c cbl.c uart.c xmodem.c
arm-eabi-ld -T ldscript cbl.o crt0.o pwr.o uart.o xmodem.o -o cbl.elf
arm-eabi-objcopy -O binary cbl.elf cbl.bin
rm *.elf
rm *.o
