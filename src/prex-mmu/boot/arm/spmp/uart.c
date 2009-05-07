#define	IO_BASE				0x10000000

#define UART_BASE			(IO_BASE + 0x1800)
#define	UART(n)				(UART_BASE + (n << 5))
#define UART_FIFO(n)			( *(volatile unsigned char*) (UART(n) + 0x02) )
#define UART_STATUS(n)			( *(volatile unsigned char*) (UART(n) + 0x0A) )
#define UART_ENABLE(n)			( *(volatile unsigned char*) ((UART_BASE + 0x80)) |= (1 << n))
#define UART_DISABLE(n)			( *(volatile unsigned char*) ((UART_BASE + 0x80)) &= ~(1 << n))

#define UART_TX_BUSY			0x02
#define UART_RX_VALID			0x04

/* Init the specified UART. Currently uses 'magic' values to init the registers. */
void UART_Init(int uart){
	volatile unsigned char * uart_base = UART(uart);
	uart_base[0x0] = 0x68;
	uart_base[0x1] = 0x00;
	uart_base[0x4] = 0xD0;
	uart_base[0x5] = 0x11;
	uart_base[0xF] = 0x88;
	
	UART_ENABLE(uart);
}

/* Send a byte over the specified UART. Precondition is that the UART has been inited. */
void UART_SendByte(int uart, unsigned char byte){
	while(UART_STATUS(uart) & UART_TX_BUSY);
	UART_FIFO(uart) = byte;
}

/* Receive a byte from the specified UART. Precondition is that the UART has been inited. */
unsigned char UART_ReceiveByte(int uart){
	return UART_FIFO(uart);
}

int	UART_ReceiveBufferEmpty(int uart){
	if(!(UART_STATUS(uart) & UART_RX_VALID))
		return 0;
	return 1;
}
