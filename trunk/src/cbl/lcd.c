#include "../spmp3050/spmp3050.h"
#include "util.h"

int lcd_mode = 0;

void LCD_SetBacklight(int val){
	DEV_ENABLE |= 0x8;
	if(val)
		DEV_ENABLE_OUT |= 0x8;
	else
		DEV_ENABLE_OUT &= ~0x8;
}

void LCD_AddrWrite(uint16_t val){
	LCD_DATA = val;
	
	LCD_CTRL = LCD_CS;
	LCD_CTRL = LCD_CS | LCD_WR;
	LCD_CTRL = LCD_CS;
}

void LCD_CmdWrite(uint16_t val){
	LCD_DATA = val;
	
	LCD_CTRL = LCD_CS | LCD_nRS;
	LCD_CTRL = LCD_CS | LCD_nRS | LCD_WR;
	LCD_CTRL = LCD_CS | LCD_nRS;
}

void LCD_CtrlWrite(int reg, int val){
	LCD_DATA_DIR |= LCD_OUT;
	LCD_AddrWrite(reg);
	LCD_CmdWrite(val);
}

void LCD_WriteFramebuffer(void* buf){
	uint16_t* fb = (uint16_t*) buf;
	LCD_DATA_DIR |= LCD_OUT;
	
	LCD_AddrWrite(0x22);
	
	LCD_CTRL = LCD_CS | LCD_nRS;
	for(int i = 0; i < 320 * 240; i++){
		LCD_DATA = fb[i];
		LCD_CTRL = LCD_WR | LCD_CS | LCD_nRS;
		LCD_CTRL = LCD_CS | LCD_nRS;
	}
}

void LCD_Init_3(){
	//--- Init sequence ---//
	LCD_CtrlWrite(0xE5, 0x8000);
	
	LCD_CtrlWrite(0x00, 0x0001);
	LCD_CtrlWrite(0x01, 0x0100);
	LCD_CtrlWrite(0x02, 0x0700);
	LCD_CtrlWrite(0x03, 0x1030);
	LCD_CtrlWrite(0x04, 0x0000);
	LCD_CtrlWrite(0x08, 0x0202);
	LCD_CtrlWrite(0x09, 0x0000);
	LCD_CtrlWrite(0x0A, 0x0000);
	LCD_CtrlWrite(0x0C, 0x0000);
	LCD_CtrlWrite(0x0D, 0x0000);
	LCD_CtrlWrite(0x0F, 0x0000);
	
	//--- Power on sequence ---//
	LCD_CtrlWrite(0x10, 0x0000);
	LCD_CtrlWrite(0x11, 0x0007);
	LCD_CtrlWrite(0x12, 0x0000);
	LCD_CtrlWrite(0x13, 0x0000);
	delay_ms(200);
	LCD_CtrlWrite(0x10, 0x17B0);
	LCD_CtrlWrite(0x11, 0x0137);
	delay_ms(50);
	LCD_CtrlWrite(0x12, 0x013C);
	delay_ms(50);
	LCD_CtrlWrite(0x13, 0x1800);
	LCD_CtrlWrite(0x29, 0x0016);
	delay_ms(50);
	LCD_CtrlWrite(0x20, 0x0000);
	LCD_CtrlWrite(0x21, 0x0000);
	
	//--- Adjust the Gamma Curve ---//
	LCD_CtrlWrite(0x30, 0x0006);
	LCD_CtrlWrite(0x31, 0x0407);
	LCD_CtrlWrite(0x32, 0x0200);
	LCD_CtrlWrite(0x35, 0x0007);
	LCD_CtrlWrite(0x36, 0x0F07);
	LCD_CtrlWrite(0x37, 0x0506);
	LCD_CtrlWrite(0x38, 0x0203);
	LCD_CtrlWrite(0x39, 0x0607);
	LCD_CtrlWrite(0x3C, 0x0601);
	LCD_CtrlWrite(0x3D, 0x1F00);
	
	//--- Set GRAM Area ---//
	LCD_CtrlWrite(0x50, 0x0000);
	LCD_CtrlWrite(0x51, 0x00EF);
	LCD_CtrlWrite(0x52, 0x0000);
	LCD_CtrlWrite(0x53, 0x013F);
	LCD_CtrlWrite(0x60, 0x2700);
	LCD_CtrlWrite(0x61, 0x0001);
	LCD_CtrlWrite(0x6A, 0x0000);
	
	//--- Partial display control ---//
	LCD_CtrlWrite(0x80, 0x0000);
	LCD_CtrlWrite(0x81, 0x0000);
	LCD_CtrlWrite(0x82, 0x0000);
	LCD_CtrlWrite(0x83, 0x0000);
	LCD_CtrlWrite(0x84, 0x0000);
	LCD_CtrlWrite(0x85, 0x0000);
	
	 //--- Panel Control ---//
	LCD_CtrlWrite(0x90, 0x0010);
	LCD_CtrlWrite(0x92, 0x0000);
	LCD_CtrlWrite(0x93, 0x0003);
	LCD_CtrlWrite(0x95, 0x0110);
	LCD_CtrlWrite(0x97, 0x0000);
	LCD_CtrlWrite(0x98, 0x0000);
	LCD_CtrlWrite(0x07, 0x0173);
	
	LCD_AddrWrite(0x22);
}

void LCD_Reset(){
	LCD_RESET_REG &= ~LCD_RESET;
	delay_us(10);
	LCD_RESET_REG |= LCD_RESET;
	delay_us(50);
}

void LCD_Init(int lcd_type){
	volatile uint8_t* lcd_base = LCD_BASE;
	LCD_DATA_EXT &= 0x3;
	
	// Magic register pokes. Peripheral turn-on?
	*((volatile uint32_t*)0x10000008) |= 0x100;
	*((volatile uint32_t*)0x10000110) |= 0x02004000;

	lcd_base[0] |= 0x1;
	lcd_base[0xF] |= 0x1;
	lcd_base[0x1B9] |= 0x80;
	
	LCD_Reset();
	
	switch(lcd_type){
		case 0:
			break;
		case 1:
			break;
		case 2:
			break;
		case 3:
			LCD_Init_3();
			break;
		case 4:
			break;
		case 5:
			break;
		case 6:
			break;
		case 7:
			break;
		default:
			return;
	}
	
	lcd_base[0x1B2] &= ~0x1;
	lcd_base[0x1BA] |= 0x1;
	lcd_base[0x1B2] |= 0x1;
}