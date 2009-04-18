#include <stdint.h>

// Initializes the timer, takes the timer number, the period, the divider and the flags as argument.

void TMR_Init(int timer, int period, int div, uint8_t flags){
	TIMER_PERIOD(timer) = period - 1;
	TIMER_COUNTER(timer) = (initial * 100) - 1;
	TIMER_FLAGS(timer) = flags;
	TIMER_ENABLE |= 1 << timer;
	IRQ_ENABLE_LO |= 1 << (timer + 4);
}