#include <stdint.h>
#include "../spmp3050/spmp3050.h"

extern inline void GPIO_Init(){
	GPIO_DISABLE = 0x00;
}

extern inline void GPIO_UnitOn(){
	GPIO_A_DIR |= 0x02;
	GPIO_A_OUT |= 0x02;
}

extern inline void GPIO_UnitOff(){
	GPIO_A_OUT &= ~0x02;
}