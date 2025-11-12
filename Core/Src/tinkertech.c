#include "tinkertech.h"

void setup() {
	LCR_LCD_Init();
	LCR_LCD_GoToXY(0, 0);
	LCR_LCD_WriteString("Hello, World!", 14);
}

void loop() {

}
