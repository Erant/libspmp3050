#include <arm/types.h>

#include "../../../sys/arch/arm/spmp/platform.h"
#include "lcd_util.h"
#include "lcd_init.h"

int lcd_mode = MODE_HW_DRAW;

void delay_ms( int ms )
{
  static int bananana;
  while(--ms)
    {
      int a;
      for (a=0;a<1000;a++)
        bananana = bananana* 11 ^ bananana + 2;
    }

  /*  timer_delay( ms ); */
}

void delay_us( int us )
{
  delay_ms( 1 + us / 1000 ); /* oh no you didn't (fixme) */
}

void LCD_SetMode(int mode){
	lcd_mode = mode;
}

/* 
 * Two flags are set here, some units have the backlight enable GPIO on 0x8,
 * some have it on 0x20.
 */

void LCD_SetBacklight(int val){
	DEV_ENABLE |= 0x28;
	if(val)
		DEV_ENABLE_OUT |= 0x28;
	else
		DEV_ENABLE_OUT &= ~0x28;
}

void LCD_AddrWrite(uint16_t val){
	LCD_DATA = val;
	
	LCD_DATA_DIR |= LCD_OUT;
	
	LCD_CTRL = LCD_CS;
	LCD_CTRL = LCD_CS | LCD_WR;
	LCD_CTRL = LCD_CS;
	LCD_CTRL = LCD_CS | LCD_nRS;
	
	LCD_DATA_DIR &= ~LCD_OUT;
}

void LCD_CmdWrite(uint16_t val){
	LCD_DATA = val;
	
	LCD_DATA_DIR |= LCD_OUT;
	
	LCD_CTRL = LCD_CS | LCD_nRS;
	LCD_CTRL = LCD_CS | LCD_nRS | LCD_WR;
	LCD_CTRL = LCD_CS | LCD_nRS;
	
	LCD_DATA_DIR &= ~LCD_OUT;
}

void LCD_CtrlWrite(int reg, int val){
	LCD_DATA_DIR |= LCD_OUT;
	LCD_AddrWrite(reg);
	LCD_CmdWrite(val);
}

void LCD_SetGamma(uint16_t *vals) {
	int addr;
	LCD_CtrlWrite(0x30, vals[0]);
	LCD_CtrlWrite(0x31, vals[1]);
	LCD_CtrlWrite(0x32, vals[2]);
	LCD_CtrlWrite(0x35, vals[3]);
	LCD_CtrlWrite(0x36, vals[4]);
	LCD_CtrlWrite(0x37, vals[5]);
	LCD_CtrlWrite(0x38, vals[6]);
	LCD_CtrlWrite(0x39, vals[7]);
	LCD_CtrlWrite(0x3C, vals[8]);
	LCD_CtrlWrite(0x3D, vals[9]);
}

void LCD_Draw(){
	int i = 0;
	uint16_t* fb = GFX_FB_START << 1;
	if(lcd_mode == MODE_HW_DRAW){
		GFX_DRAW = 1;
		return;
	}
	if(lcd_mode == MODE_SW_DRAW){
		LCD_AddrWrite(0x22);		/* Fixme, not all LCDs have addr 0x22 as GRAM */
		for(; i < 320 * 240; i++)	/* You didn't just hardcode the LCD size, didcha... */
			LCD_CmdWrite(fb[i]);
	}
}

void LCD_Reset(){
	LCD_RESET_REG &= ~LCD_RESET;
	delay_us(10);
	LCD_RESET_REG |= LCD_RESET;
	delay_us(50);
}

/*
 *	Inits the LCD based on the lcd_type. This function is based on a LOT of magic register pokes.
 *	Anything that uses the lcd_base pointer is a magic poke, and we don't know what it does.
 */

void LCD_DoMagic();

void LCD_Init(int lcd_type){
	volatile uint8_t* lcd_base = (volatile uint8_t*)LCD_BASE;
	LCD_DATA_EXT = 0;
	
	/* Magic register pokes. Peripheral turn-on? */
	*((volatile uint32_t*)0x10000008) = 0xFFFFFFFF;
	*((volatile uint32_t*)0x10000110) = 0xFFFFFFFF;

	LCD_ENABLE = 0x1; /* Enable LCD subsystem */
	LCD_GFX_UPDATE = 1;
	lcd_base[0x1B9] |= 0x80;
	
	LCD_Reset();

	master_lcd_init(lcd_type);
	
	lcd_base[0x1B2] &= ~0x1;
	lcd_base[0x1BA] |= 0x1;
	lcd_base[0x1B2] |= 0x1;
	
	/* Do the voodoo that makes hardware acceleration work. */
	LCD_DoMagic();
}

void LCD_SetFramebuffer(void* fb){
	GFX_FB_START = ((uint32_t)fb) >> 1;
	GFX_FB_END = (((uint32_t)fb) + LCD_WIDTH * LCD_HEIGHT * (LCD_BPP / 8)) >> 1;
	GFX_FB_HORIZ = LCD_WIDTH;
	GFX_FB_VERT = LCD_HEIGHT;
	LCD_GFX_UPDATE |= 2;	/* Possibly 'reinitialize framebuffer' */
}

void LCD_DoMagic(){
	uint16_t temp;
	volatile uint8_t* lcd_base = (volatile uint8_t*)LCD_BASE;
	/* LCD_init */
	lcd_base[0x242] = 0x5;
	lcd_base[0x203] &= ~0x1;
	lcd_base[0x204] = 0xD;
	lcd_base[0x205] = 1;
	lcd_base[0x194] |= 0x4;
	
	lcd_base[0x203] |= 0x1;
	
	LCD_SCREEN_HEIGHT = LCD_HEIGHT;
	LCD_SCREEN_WIDTH = LCD_WIDTH;
	LCD_SCREEN_T1 = 5;	
	LCD_SCREEN_T2 = 5;

	LCD_GFX_UPDATE = 1;
	
	/* init_more_gfx */
	
	lcd_base[0x100] = 0x4;
	
	lcd_base[0x1D1] = 0xA;
	lcd_base[0x226] &= ~0x1;
	
	lcd_base[0x1DB] = 0x00;
	lcd_base[0x1DC] = 0xFC;
	lcd_base[0x1DD] = 0x00;
	lcd_base[0x1DE] = 0xFF;
	lcd_base[0x1DF] = 0xFF;
	lcd_base[0x1E0] = 0xFF;
	
	/* Unaligned access for resize_screen */
	temp = LCD_WIDTH - 1;
	lcd_base[0x145] = temp & 0xFF;
	lcd_base[0x146] = (temp >> 8) & 0xFF;
	
	lcd_base[0x14D] = temp & 0xFF;
	lcd_base[0x14E] = (temp >> 8) & 0xFF;
	
	temp = LCD_HEIGHT - 1;
	lcd_base[0x147] = temp & 0xFF;
	lcd_base[0x148] = (temp >> 8) & 0xFF;

	lcd_base[0x14F] = temp & 0xFF;
	lcd_base[0x150] = (temp >> 8) & 0xFF;
}

