#ifndef _LCD_UTIL_H
#define _LCD_UTIL_H


#define MODE_HW_DRAW	0
#define	MODE_SW_DRAW	1

void delay_ms(int);
void delay_us(int);

void LCD_SetBacklight(int val);
void LCD_SetMode(int mode);
void LCD_Draw();
void LCD_Init(int lcd_type);
void LCD_SetFramebuffer(void* fb);

#endif /* !_LCD_UTIL_H */
