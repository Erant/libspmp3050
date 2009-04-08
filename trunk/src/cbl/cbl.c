#include <stdint.h>
#include "../spmp3050/spmp3050.h"
#include "uart.h"
#include "pwr.h"

void printString(char* str);
void printMemory(uint32_t* ptr);
char* itoa(uint32_t val, char* buf);

char itoa_buf[9];


void _start(){
	uint8_t recv;
	UART_Init(1);
	
	while(1){
		printString("Welcome to Crappy BootLoader (CBL) v0.01 (build 15)\r\n\n");
		printString("1. Poke memory\r\n");
		printString("2. Peek memory\r\n");
		printString("3. Upload binary\r\n");
		printString("4. Continue boot\r\n");
		printString("5. Turn unit off\r\n\n");
		do{
			while(UART_ReceiveBufferEmpty(1));
			recv = UART_ReceiveByte(1);
		}while(recv < 0x31 || recv > 0x35);
		recv -= 0x30;
		switch(recv){
			case 1:	// POKE
				*((uint8_t*)0x1000B320) = 0x00;
				*((uint8_t*)0x1000B321) = 0x00;
				*((uint8_t*)0x1000B322) = 0x00;
				*((uint8_t*)0x1000B323) = 0x00;
				PWR_UnitOn();
				printString("Enter an 8 digit hexadecimal address to poke:\r\n");
				// TODO
				printString("Enter a hexadecimal number to poke:\r\n");
				// TODO
				break;
			case 2: // PEEK
				printMemory((uint32_t*)0x100001EC);
				printMemory((uint32_t*)0x100001F0);
				printMemory((uint32_t*)0x10001100);
				printMemory((uint32_t*)0x10001108);
				printMemory((uint32_t*)0x1000B064);
				printMemory((uint32_t*)0x1000B068);
				printMemory((uint32_t*)0x1000B320);
				break;
			case 3: // Upload binary
				((void (*)(void))0x240CB734)();
				break;
			case 4: // Continue boot.
				*((uint8_t*)GPIO_BASE) = 0x00;
				*((uint8_t*)(GPIO_BASE + 0x08)) = 0x4E;
				*((uint8_t*)(IO_BASE + 0x1EC)) &= ~0x4E;
				break;
			case 5: // Continue boot.
				PWR_UnitOff();
				break;
		}
	}
}

void printString(char* str){
	for(; *str != '\0'; str++)
		UART_SendByte(1, *str);
}

char lut[] = "0123456789ABCDEF";
char* itoa(uint32_t val, char* buf){
	int i = 7;
	for(; i >= 0; i--){
		buf[i] = lut[val & 0xF];
		val = val >> 4;
	}
	buf[8] = '\0';
	return buf;
}

void printMemory(uint32_t* ptr){
	printString("0x");
	printString(itoa((uint32_t)ptr, itoa_buf));
	printString(": 0x");
	printString(itoa(*ptr, itoa_buf));
	printString("\r\n");
}