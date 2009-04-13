#include "../spmp3050/spmp3050.h"

void LCD_SetBacklight(int val){
	DEV_ENABLE |= 0x8;
	if(val)
		DEV_ENABLE_OUT |= 0x8;
	else
		DEV_ENABLE_OUT &= ~0x8;
}