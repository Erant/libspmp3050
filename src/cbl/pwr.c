#include <stdint.h>
#include "../spmp3050/spmp3050.h"

void PWR_UnitOn(){
	*((uint32_t*)(PWR_BASE + 0x320)) = 0x00;
	*((uint8_t*)PWR_REG) |= 0x02;
	(*(uint8_t*)(PWR_REG + 4)) |= 0x02;
	(*(uint8_t*)(IO_BASE + 0x1F0)) &= ~0x02;
	
}

void PWR_UnitOff(){
	(*(uint8_t*)PWR_REG) = 0x00;
}