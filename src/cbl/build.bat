@echo off
arm-eabi-gcc -Wall -c cbl.c uart.c
arm-eabi-ld -T ldscript cbl.o uart.o -o cbl.elf
arm-eabi-objcopy -O binary cbl.elf cbl.bin
rm *.elf
rm *.o