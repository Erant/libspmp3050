#ifndef _LCD_UTIL_H
#define _LCD_UTIL_H

void delay_ms(int);
void delay_us(int);

void LCD_SetBacklight(int val);
void LCD_Draw();
void LCD_Init(int lcd_type);
void LCD_SetFramebuffer(void* fb);

#endif /* !_LCD_UTIL_H */
