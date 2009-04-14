#ifndef _LCD_H
#define _LCD_H

void LCD_SetBacklight(int val);
void LCD_Init(int lcd_type);
void LCD_WriteFramebuffer(void* buf);

// TEMPORARY, TESTING PURPOSES ONLY.
void LCD_CtrlWrite(int reg, int val);
void LCD_CmdWrite(uint32_t val);
void LCD_AddrWrite(uint32_t val);

#endif