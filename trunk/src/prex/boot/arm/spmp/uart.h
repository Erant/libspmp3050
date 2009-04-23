#ifndef _UART_H
#define _UART_H

void UART_Init(int uart);
void UART_SendByte(int uart, unsigned char byte);
uint8_t UART_ReceiveByte(int uart);
int	UART_ReceiveBufferEmpty(int uart);

#endif
