#include <stdint.h>
#include "../spmp3050/spmp3050.h"

extern inline void GPIO_Init(){
	*((uint32_t*)GPIO_DISABLE) = 0x00;
}

extern inline void GPIO_UnitOn(){
	*((uint8_t*)GPIO_A_DIR) |= 0x02;
	(*(uint8_t*)GPIO_A_OUT) |= 0x02;
	//(*(uint8_t*)(IO_BASE + 0x1F0)) &= ~0x02;
}

extern inline void GPIO_UnitOff(){
	(*(uint8_t*)GPIO_A_OUT) &= ~0x02;
}