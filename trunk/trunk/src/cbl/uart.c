#include "../spmp3050/spmp3050.h"

// Init the specified UART. Currently uses 'magic' values to init the registers.
void UART_Init(int uart){
	uint8_t* uart_base = UART(uart);
	uart_base[0x0] = 0x68;
	uart_base[0x1] = 0x00;
	uart_base[0x4] = 0xD0;
	uart_base[0x5] = 0x11;
	uart_base[0xF] = 0x88;
	
	UART_ENABLE(uart);
}

// Send a byte over the specified UART. Precondition is that the UART has been inited.
void UART_SendByte(int uart, uint8_t byte){
	while(UART_STATUS(uart) & UART_TX_BUSY);
	UART_FIFO(uart) = byte;
}

// Receive a byte from the specified UART. Precondition is that the UART has been inited.
uint8_t UART_ReceiveByte(int uart){
	return UART_FIFO(uart);
}

int	UART_ReceiveBufferEmpty(int uart){
	if(!(UART_STATUS(uart) & UART_RX_VALID))
		return 0;
	return 1;
}