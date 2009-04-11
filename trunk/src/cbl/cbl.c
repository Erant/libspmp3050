#include <stdint.h>
#include <stddef.h>
#include "../spmp3050/spmp3050.h"
#include "uart.h"
#include "gpio.h"
#include "xmodem.h"

void printString(char* str);
char* receiveString(char* buf);
void printMemory(uint32_t* ptr);
void printhex(uint8_t *buf, size_t n, uint32_t addr, int linew);
uint32_t atoi(char* buf);
char* itoa(uint32_t val, char* buf);
char* ctoa(uint8_t val, char* buf);

char itoa_buf[9];
char rx_buf[256];

// -- convenience wrappers to help with the xmodem implementation.

// returns a single byte 0..255, can return -1 if 'timeout'
int serialGetByte()  
{
  unsigned char recv;

  // you could turn this into a for loop that also checks the current time, to introduce a timeout.
  while(UART_ReceiveBufferEmpty(1)) ;
  recv = UART_ReceiveByte(1);
  return recv;
}

void serialPutByte( unsigned char c )
{
  UART_SendByte( 1, c );
}

uint8_t* xmodemWritePointer;

int xmodemWriterHelper( unsigned char * data, int size )
{
  // copy received data into the xmodemWritePointer location, and advance it.
  while(size--)
    {
      *xmodemWritePointer = *data;
      data++;
      xmodemWritePointer++;
    }
  return 0;
}

void flushDCache(){
	__asm__("1:			MRC p15, 0, r15, c7, c10, 3\n\t"
			"			BNE 1b");  // test and clean
}

void invalidateICache(){
	__asm__("MCR p15, 0, r15, c7, c5, 0");
}

int main(){
	uint8_t recv;
	uint32_t prev_poke = 0, prev_peek = 0, offset = 0x24003000;
	uint32_t prev_gpio = 0, gpio;
	UART_Init(1);
	printString("\r\n");
	
	while(1){
		printString("Welcome to Crappy BootLoader (CBL) v0.01 (build 16)\r\n\n");
		printString("1. Poke memory\r\n");
		printString("2. Peek memory\r\n");
		printString("3. Upload binary\r\n");
		printString("4. Continue boot\r\n");
		printString("5. Turn unit off\r\n");
		printString("6. Show GPIO status\r\n\n");
		do{
			while(UART_ReceiveBufferEmpty(1));
			recv = UART_ReceiveByte(1);
		}while(recv < '1' || recv > '6');
		recv -= 0x30;
		switch(recv){
			case 1:	// POKE
				printString("Enter an 8 digit hexadecimal address to poke (0x");
				printString(itoa(prev_poke, itoa_buf));
				printString(") :\r\n0x");				
				if(receiveString(rx_buf) == NULL)
					break;
				if(*rx_buf)
					prev_poke = atoi(rx_buf);
				
				printString("\r\nEnter a 2 digit hexadecimal number to poke:\r\n0x");
				*((uint8_t*)prev_poke) = atoi(receiveString(rx_buf));
				printString("\r\n");
				break;
			case 2: // PEEK
				printString("Enter an 8 digit hexadecimal address to peek (0x");
				printString(itoa(prev_peek, itoa_buf));
				printString(") :\r\n0x");
				if(receiveString(rx_buf) == NULL)
					break;
				if(*rx_buf)
					prev_peek = atoi(rx_buf);
				
				printString("\r\nEnter a hexadecimal length to read:\r\n0x");
				int read_length = atoi(receiveString(rx_buf));
				printString("\r\n");
				printhex((uint8_t*)prev_peek, read_length, prev_peek, 16);
				printString("Press any button to continue.\r\n");
				while(UART_ReceiveBufferEmpty(1));
				UART_ReceiveByte(1);
				break;
			case 3: // Upload binary
				printString("Enter an 8 digit hexadecimal address to load your binary to (0x");
				printString(itoa(offset, itoa_buf));
				printString(") :\r\n0x");
				if(receiveString(rx_buf) == NULL)
					break;
				if(*rx_buf)
					offset = atoi(rx_buf);
				
				for(int i = 0; i < 0xFFFFFF; i++);
				// xmodem transfer.
				xmodemInit( serialPutByte, serialGetByte );
				xmodemWritePointer = (uint8_t*)offset;
				xmodemReceive( xmodemWriterHelper );
				
				break;
			case 4: // Continue boot.
				printString("Enter an 8 digit hexadecimal address to jump to (0x");
				printString(itoa(offset, itoa_buf));
				printString(") :\r\n0x");
				if(receiveString(rx_buf) == NULL)
					break;
				if(*rx_buf)
					offset = atoi(rx_buf);
				
				flushDCache();
				invalidateICache();
				printString("\r\nJumping to 0x");
				printString(itoa(offset, itoa_buf));
				printString(", bye bye!\r\n");
				((void (*)(void))offset)();
				break;
			case 5: // Abort boot.
				GPIO_UnitOff();
				while(1);
				break;
			case 6:
				while(UART_ReceiveBufferEmpty(1)){
					gpio = *((uint32_t*)0x1000B06C);
					if(gpio != prev_gpio){
						printString(itoa(~gpio, itoa_buf));
						printString("\r\n");
					}
					prev_gpio = gpio;
				}
				UART_ReceiveByte(1);
		}
	}
}

// All helper functions, because we don't have a libc yet.
void printhex(uint8_t *buf, size_t n, uint32_t addr, int linew){
	size_t pos,i, l, j;
	for (pos=0;pos<n;pos+=linew,addr+=linew){
		l = pos+linew>n?n:pos+linew;
		printString(itoa(addr, itoa_buf));
		printString("   ");
		for (i=pos,j=0;i<pos+linew;i++,j++){
			if (i<n){
				printString(ctoa(buf[i],itoa_buf));
				printString(" ");
			}
			else
			{
				printString("   ");
			}
			if(!((j+1)%4))
				printString(" ");
		}
		if (linew%4)
			printString(" ");
		for (i=pos;i<l;i++){
			if(buf[i]>=0x20&&buf[i]<=0x7F)
				UART_SendByte(1, buf[i]);
			else
				UART_SendByte(1, '.');
		}
		printString("\r\n");
	}
}


void printString(char* str){
	for(; *str != '\0'; str++)
		UART_SendByte(1, *str);
}

char* receiveString(char* buf){
	char* ret = buf;
	do{
		while(UART_ReceiveBufferEmpty(1));
		*buf = UART_ReceiveByte(1);
		if(*buf>=0x20&&*buf<=0x7F){
			UART_SendByte(1, *buf);
		}
		
		if(*buf == 0x03){	// CTRL + C
			*buf = 0;
			return NULL;
		}
		
		if(*buf == '\b' && buf > ret){
			printString("\b \b");
		}
		else
		{
			buf++;
		}
	}while(*(buf - 1) != '\r');
	*(buf - 1) = '\0';
	return ret;
}

uint32_t atoi(char* buf){
	uint32_t ret = 0;
	while((*buf >= '0' && *buf <= '9') || (*buf >= 'a' && *buf <= 'f') || (*buf >= 'A' && *buf <= 'F')){
		if(*buf >= 'a' && *buf <= 'f')
			*buf -= 'a' - 'A';
		if(*buf >= 'A' && *buf <= 'F')
			*buf -= 0x7;
		*buf -= 0x30;
		
		ret = ret << 4;
		ret |= *buf;
		
		buf++;
	}
	return ret;
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

char* ctoa(uint8_t val, char* buf){
	buf[2] = '\0';
	buf[1] = lut[val & 0xF];
	buf[0] = lut[(val >> 4) & 0xF];
	return buf;
}

void printMemory(uint32_t* ptr){
	printString("0x");
	printString(itoa((uint32_t)ptr, itoa_buf));
	printString(": 0x");
	printString(itoa(*ptr, itoa_buf));
	printString("\r\n");
}