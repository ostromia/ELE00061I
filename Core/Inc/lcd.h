#ifndef __LCD_H
#define __LCD_H

#include "gpio.h"

void lcd_init(GPIO RS, GPIO E, GPIO D4, GPIO D5, GPIO D6, GPIO D7);
void lcd_set_cursor(GPIO RS, GPIO E, GPIO D4, GPIO D5, GPIO D6, GPIO D7, int x, int y);
void lcd_clear(GPIO RS, GPIO E, GPIO D4, GPIO D5, GPIO D6, GPIO D7);
void lcd_write_string(GPIO RS, GPIO E, GPIO D4, GPIO D5, GPIO D6, GPIO D7, char *string);

#endif
