#include "../spmp3050/spmp3050.h"

void LCD_SetBacklight(int val){
	DEV_ENABLE |= 0x8;
	if(val)
		DEV_ENABLE_OUT |= 0x8;
	else
		DEV_ENABLE_OUT &= ~0x8;
}

void LCD_CtrlWrite(int reg, int val){
	LCD_AddrWrite(reg);
	LCD_CmdWrite(val);
}



void LCD_AddrWrite(uint16_t val){
	volatile uint8_t* lcd_base = LCD_BASE;
	
	LCD_DATA = val;
	
	lcd_base[0xE4] = ((val & 0x30000) >> 14) | (lcd_base[0xE4] & 0x3);
	
	LCD_DATA_DIR |= LCD_DATA_OUT;
	
	LCD_CTRL = (LCD_CTRL & ~0x3C) | LCD_CS;
	
	LCD_CTRL |= LCD_WR;
	delay(3);
	LCD_CTRL &= ~LCD_WR;
	
	LCD_DATA_DIR |= LCD_DATA_OUT;
	
	LCD_CTRL |= LCD_nRS;
}

void LCD_CmdWrite(uint16_t val){
	volatile uint8_t* lcd_base = LCD_BASE;
	
	LCD_DATA = val;
	
	lcd_base[0xE4] = ((val & 0x30000) >> 14) | (lcd_base[0xE4] & 0x3);
	
	LCD_DATA_DIR |= LCD_DATA_OUT;
	
	LCD_CTRL = (LCD_CTRL & ~0x3C) | LCD_CS | LCD_nRS;
	
	LCD_CTRL |= LCD_WR;
	delay(3);
	LCD_CTRL &= ~LCD_WR;
	
	lLCD_DATA_DIR |= LCD_DATA_OUT;
	
	LCD_CTRL &= ~LCD_nRS;
}