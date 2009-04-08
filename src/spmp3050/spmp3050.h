#include <stdint.h>

#define	IO_BASE			((void*)0x10000000)

#define GPIO_BASE		(IO_BASE + 0x1100)

#define UART_BASE		(IO_BASE + 0x1800)
#define	UART(n)			(UART_BASE + (n << 5))
#define UART_FIFO(n)	(*(uint8_t*)(UART(n) + 0x02))
#define UART_STATUS(n)	(*(uint8_t*)(UART(n) + 0x0A))
#define UART_ENABLE(n)	(*(uint8_t*)(UART_BASE + 0x80)) |= (1 << n)
#define UART_DISABLE(n)	(*(uint8_t*)(UART_BASE + 0x80)) &= ~(1 << n)

#define UART_TX_BUSY	0x02
#define UART_RX_VALID	0x04

#define PWR_BASE		(IO_BASE + 0xB000)
#define PWR_REG			(PWR_BASE + 0x64)