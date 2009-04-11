#include <stdint.h>

#define	IO_BASE			((volatile void*)0x10000000)

#define UART_BASE		(IO_BASE + 0x1800)
#define	UART(n)			(UART_BASE + (n << 5))
#define UART_FIFO(n)	(*(volatile uint8_t*)(UART(n) + 0x02))
#define UART_STATUS(n)	(*(volatile uint8_t*)(UART(n) + 0x0A))
#define UART_ENABLE(n)	(*(volatile uint8_t*)(UART_BASE + 0x80)) |= (1 << n)
#define UART_DISABLE(n)	(*(volatile uint8_t*)(UART_BASE + 0x80)) &= ~(1 << n)

#define UART_TX_BUSY	0x02
#define UART_RX_VALID	0x04

#define GPIO_BASE		(IO_BASE + 0xB000)
#define GPIO_A			(GPIO_BASE + 0x60)
#define GPIO_A_DIR		(*(volatile uint32_t*)(GPIO_A + 0x4))
#define GPIO_A_OUT		(*(volatile uint32_t*)(GPIO_A + 0x8))
#define GPIO_A_IN		(*(volatile uint32_t*)(GPIO_A + 0xC))

#define GPIO_B			(GPIO_BASE + 0xE0)
#define GPIO_B_DIR		(*(volatile uint32_t*)(GPIO_B + 0x4))
#define GPIO_B_OUT		(*(volatile uint32_t*)(GPIO_B + 0x8))
#define GPIO_B_IN		(*(volatile uint32_t*)(GPIO_B + 0xC))

#define GPIO_DISABLE	(*(volatile uint32_t*)(GPIO_BASE + 0x320))