#include <stdint.h>
#include "uart.h"
#include "../spmp3050/spmp3050.h"

void printString(char* str);
char* itoa(uint32_t val, char* buf);

char itoa_buf[9];


void _start(){
	uint8_t recv;
	UART_Init(1);
	
	*((uint8_t*)GPIO_DIR) &= ~0x10;
	*((uint8_t*)GPIO_PULL) &= ~0x10;
	*((uint8_t*)GPIO_OUT) &= ~0x10;
	
	while(1){
		printString("Welcome to Crappy BootLoader (CBL) v0.01 (build 6)\r\n\n");
		printString("1. Poke memory\r\n");
		printString("2. Peek memory\r\n");
		printString("3. Upload binary\r\n");
		printString("4. Continue boot\r\n\n");
		do{
			while(UART_ReceiveBufferEmpty(1));
			recv = UART_ReceiveByte(1);
		}while(recv < 0x31 || recv > 0x34);
		
		recv -= 0x30;
		switch(recv){
			case 1:	// POKE
				printString("Enter an 8 digit hexadecimal address to poke:\r\n");
				// TODO
				printString("Enter a hexadecimal number to poke:\r\n");
				// TODO
				break;
			case 2: // PEEK
				printString("Testing: ");
				printString(itoa(0x12345678, itoa_buf));
				printString("\r\n");
				break;
			case 3: // Upload binary
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
	int i = 0;
	for(; i < 8; i++){
		buf[i] = val & 0xF;
		val = val >> 4;
	}
	buf[8] = '\0';
	return buf;
}