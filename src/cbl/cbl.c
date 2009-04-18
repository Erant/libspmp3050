#include <stdint.h>
#include <stddef.h>
#include "../spmp3050/spmp3050.h"
#include "uart.h"
#include "lcd.h"
#include "gpio.h"
#include "xmodem.h"
#include "util.h"
#include "timer.h"

void printString(char* str);
char* receiveString(char* buf);
void printMemory(uint32_t* ptr);
void printhex(uint8_t *buf, size_t n, uint32_t addr, int linew);
uint32_t atoi(char* buf);
char* itoa(uint32_t val, char* buf);
char* ctoa(uint8_t val, char* buf);
void IRQ_Handler_4(void);
void IRQ_Handler_5(void);

void (*set_irq_handler)(int, int) = (void (*)(int,int))0x24028910;

char itoa_buf[9];
char rx_buf[256];

uint16_t* fb = (uint16_t*)0x24164700;

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

extern void stub;
void dontcall() __attribute__((noreturn));

void dontcall(){
	__asm__("stub:	LDR r0, tag\n"
			"		BX r0\n"
			"tag:	.word _start\n"
			);
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
	// No clobber list, because r15 (PC) is not actually modified.
	__asm__ __volatile__ (
		"1:			MRC p15, 0, r15, c7, c10, 3\n"
		"			BNE 1b\n");  // test and clean
}

void invalidateICache(){
	// No clobber list, because r15 (PC) is not actually modified.
	__asm__ __volatile__("MCR p15, 0, r15, c7, c5, 0\n");
}

int strlen(char* str){
	char* start = str;
	while(*str != '\0')
		str++;
	return str - start - 1;
}

int main(){
	uint8_t recv;
	uint32_t prev_poke = 0, prev_peek = 0, offset = 0x24026EB4;
	uint32_t r0, r1, r2, r3;
	UART_Init(1);
	printString("\r\n");
	
	while(1){
		printString("Welcome to Crappy BootLoader (CBL) v0.01 (build 16)\r\n\n");
		printString("1. Poke memory\r\n");
		printString("2. Peek memory\r\n");
		printString("3. Upload binary\r\n");
		printString("4. Call function\r\n");
		printString("5. Jump to offset\r\n");
		printString("6. Inject return stub\r\n");
		printString("7. Initialize timers\r\n");
		printString("8. Draw logo to screen\r\n\r\n");
		
		// Wait for a selection
		do{
			while(UART_ReceiveBufferEmpty(1));
			recv = UART_ReceiveByte(1);
		}while(recv < '1' || recv > '8');
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
				
				printString("\r\nEnter a hexadecimal number to poke:\r\n0x");
				
				uint32_t poke = atoi(receiveString(rx_buf));
				printString("\r\n");
				
				// Check what kind of input we got. 2, 4 or 8 bytes.
				int len = strlen(rx_buf);
				if(len <= 2){	
					*((uint8_t*)prev_poke) = (uint8_t)poke;
					break;
				}
				if(len <= 4){
					*((uint16_t*)prev_poke) = (uint16_t)poke;
					break;
				}
				*((uint32_t*)prev_poke) = poke;
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
			case 4: // Call function
				printString("R0: 0x");
				if(receiveString(rx_buf) == NULL)
					break;
				r0 = atoi(rx_buf);
				printString("\r\n");
				
				printString("R1: 0x");
				if(receiveString(rx_buf) == NULL)
					break;
				r1 = atoi(rx_buf);
				printString("\r\n");
				
				printString("R2: 0x");
				if(receiveString(rx_buf) == NULL)
					break;
				r2 = atoi(rx_buf);
				printString("\r\n");
				
				printString("R3: 0x");
				if(receiveString(rx_buf) == NULL)
					break;
				r3 = atoi(rx_buf);
				printString("\r\n");
				// No break, use the next case.
				
			case 5: // Continue boot.
				printString("Enter an 8 digit hexadecimal address to jump to (0x");
				printString(itoa(offset, itoa_buf));
				printString(") :\r\n0x");
				if(receiveString(rx_buf) == NULL)
					break;
				if(*rx_buf)
					offset = atoi(rx_buf);
				
				
				printString("\r\nJumping to 0x");
				printString(itoa(offset, itoa_buf));
				printString(", bye bye!\r\n");
				
				flushDCache();
				invalidateICache();
				__asm__ __volatile__(
						"	LDR r0, %0\n"
						"	LDR r1, %1\n"
						"	LDR r2, %2\n"
						"	LDR r3, %3\n"
						"	BLX %4\n"
						: // No output registers
						: "m" (r0), "m" (r1), "m" (r2), "m" (r3), "r" (offset)
						: "r0", "r1", "r2", "r3"
						);
				break;
			case 6:	// Inject the return to CBL stub.
				printString("Enter an 8 digit hexadecimal address to inject the stub to:\r\n0x");
				
				if(receiveString(rx_buf) == NULL)
					break;
				
				printString("\r\n");
				
				uint32_t* stub_ptr = (uint32_t*)&stub;
				uint32_t* dest_ptr = (uint32_t*)atoi(rx_buf);
				
				dest_ptr[0] = stub_ptr[0];
				dest_ptr[1] = stub_ptr[1];
				dest_ptr[2] = stub_ptr[2];
				
				break;	
			case 7: // Do the timer demo.
				set_irq_handler(4, (int)IRQ_Handler_4);
				set_irq_handler(5, (int)IRQ_Handler_5);
				TMR_Init(0, 12000, 10, TIMER_REPEAT);
				enable_interrupts();
				break;
			case 8: // LCD crap testing
				LCD_Init(3);
				LCD_WriteFramebuffer(fb);
				LCD_SetBacklight(1);
				break;
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
			buf--;
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
	char tmp;
	while((*buf >= '0' && *buf <= '9') || (*buf >= 'a' && *buf <= 'f') || (*buf >= 'A' && *buf <= 'F')){
		tmp = *buf;
		if(tmp >= 'a' && tmp <= 'f')
			tmp -= 'a' - 'A';
		if(tmp >= 'A' && tmp <= 'F')
			tmp -= 0x7;
		tmp -= 0x30;
		
		ret = ret << 4;
		ret |= tmp;
		
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

void clear_interrupt(int nr){
	if(nr > 31){
		IRQ_FLAG_HI |= 1 << (nr - 32);
		return;
	}
	IRQ_FLAG_LO |= 1 << nr;
}


char* spinny = "\\-/|";

void IRQ_Handler_4(void){
	static int index = 0;
	clear_interrupt(4);
	
	if(!spinny[index])
		index = 0;
	
	UART_FIFO(1) = '\b';
	UART_FIFO(1) = spinny[index];
	index++;
}

void IRQ_Handler_5(void){
	clear_interrupt(5);
	UART_FIFO(1) = '5';
}
