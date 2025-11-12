#include "tinkertech.h"

void setup() {
	GPIO RS = { LCD_RS_GPIO_Port, LCD_RS_Pin };
	GPIO E  = { LCD_E_GPIO_Port,  LCD_E_Pin  };
	GPIO D4 = { LCD_D4_GPIO_Port, LCD_D4_Pin };
	GPIO D5 = { LCD_D5_GPIO_Port, LCD_D5_Pin };
	GPIO D6 = { LCD_D6_GPIO_Port, LCD_D6_Pin };
	GPIO D7 = { LCD_D7_GPIO_Port, LCD_D7_Pin };

	RCC->AHBENR |= RCC_AHBENR_GPIOAEN | RCC_AHBENR_GPIOBEN | RCC_AHBENR_GPIOFEN;

	lcd_init(RS, E, D4, D5, D6, D7);
	lcd_set_cursor(RS, E, D4, D5, D6, D7, 0, 0);
	lcd_write_string(RS, E, D4, D5, D6, D7, "Hello World!");
}

void loop() {

}
