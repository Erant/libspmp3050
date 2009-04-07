#ifndef _UART_H
#define _UART_H

#include <stdint.h>

void UART_Init(int uart);
void UART_SendByte(int uart, uint8_t byte);
uint8_t UART_ReceiveByte(int uart);
int	UART_ReceiveBufferEmpty(int uart);

#endif