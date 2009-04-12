@echo off
arm-eabi-gcc -Wall -c -mcpu=arm926ej-s -O2 -std=c99 crt0.c gpio.c cbl.c uart.c xmodem.c
arm-eabi-ld -T ldscript cbl.o crt0.o gpio.o uart.o xmodem.o -o cbl.elf
arm-eabi-objcopy -O binary cbl.elf cbl.bin
rm *.elf
rm *.o
