#include <stdint.h>
#include "uart.h"
#include "../spmp3050/spmp3050.h"

void printString(char* str);
char* itoa(uint32_t val, char* buf);

char itoa_buf[9];


void _start(){
	uint8_t recv;
	UART_Init(1);

	while(1){
		printString("Welcome to Crappy BootLoader (CBL) v0.01 (build 12)\r\n\n");
		printString("1. Poke memory\r\n");
		printString("2. Peek memory\r\n");
		printString("3. Upload binary\r\n");
		printString("4. Continue boot\r\n\n");
		do{
			while(UART_ReceiveBufferEmpty(1));
			recv = UART_ReceiveByte(1);
		}while(recv < 0x31 || recv > 0x36);
		
		recv -= 0x30;
		switch(recv){
			case 1:	// POKE
				printString("Enter an 8 digit hexadecimal address to poke:\r\n");
				// TODO
				printString("Enter a hexadecimal number to poke:\r\n");
				// TODO
				break;
			case 2: // PEEK
				printString("0x1000B064: 0x");
				uint32_t val = *((uint8_t*)0x1000B064) & 0xFF;
				printString(itoa(val, itoa_buf));
				printString("\r\n");
				break;
			case 3: // Upload binary
				((void (*)(void))0x240CB734)();
				break;
			case 4: // Continue boot.
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