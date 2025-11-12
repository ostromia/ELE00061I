#ifndef __LCD_H
#define __LCD_H

#include "gpio.h"

void lcd_init(GPIO RS, GPIO E, GPIO D4, GPIO D5, GPIO D6, GPIO D7);
void lcd_set_cursor(GPIO E, GPIO D4, GPIO D5, GPIO D6, GPIO D7, int x, int y);
void lcd_clear(GPIO E, GPIO D4, GPIO D5, GPIO D6, GPIO D7);

extern void LCR_MicroDelay (uint32_t delay);

void LCD_Set_Data(uint8_t data);
void LCD_Set_RS(uint8_t data);
void LCD_Set_RW(uint8_t data);
void LCD_Set_E(uint8_t data);
void LCD_Update(void);

extern void LCR_LCD_Init(void);
extern void LCR_LCD_Clear (void);
extern void LCR_LCD_GoToXY (int x, int y);
extern void LCR_LCD_WriteChar (char ch);
extern void LCR_LCD_WriteString (char *ch);
extern void LCR_LCD_DefineChar (int ch, char *data);

#endif
