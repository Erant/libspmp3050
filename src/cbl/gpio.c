#include <stdint.h>
#include "../spmp3050/spmp3050.h"

inline void GPIO_Init(){
	*((uint32_t*)GPIO_DISABLE) = 0x00;
}

inline void GPIO_UnitOn(){
	*((uint8_t*)GPIO_DIR) |= 0x02;
	(*(uint8_t*)GPIO_OUT) |= 0x02;
	//(*(uint8_t*)(IO_BASE + 0x1F0)) &= ~0x02;
}

inline void GPIO_UnitOff(){
	(*(uint8_t*)GPIO_OUT) &= ~0x02;
}