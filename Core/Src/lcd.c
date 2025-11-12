#include "tinkertech.h"

#define LCD_DELAY_CONST 500

static void delay(uint32_t delayInMicroSeconds) {
	float compensation = (float)SystemCoreClock / (float)16e6;
	volatile unsigned long x = (unsigned long)(compensation * (36 * delayInMicroSeconds >> 4));
	while (x-- > 0);
}

static int isbusy() {
	delay(1000);
	return 0;
}

static void set_gpio(GPIO gpio, uint8_t data) {
	gpio.port->BSRR = data ? gpio.pin : gpio.pin << 16;
}

static void set_lcd_data(GPIO D4, GPIO D5, GPIO D6, GPIO D7, uint8_t data) {
    D4.port->BSRR = (data & 0x1) ? D4.pin : D4.pin << 16;
    D5.port->BSRR = (data & 0x2) ? D5.pin : D5.pin << 16;
    D6.port->BSRR = (data & 0x4) ? D6.pin : D6.pin << 16;
    D7.port->BSRR = (data & 0x8) ? D7.pin : D7.pin << 16;
}


enum eLCD_OP { READ_INSTRUCTION, WRITE_INSTRUCTION, READ_DATA, WRITE_DATA };

static void write_lcd_byte (GPIO E, GPIO D4, GPIO D5, GPIO D6, GPIO D7, enum eLCD_OP op, uint8_t data) {
	if (op == WRITE_DATA) LCD_Set_RS(1);
	else if (op == WRITE_INSTRUCTION) LCD_Set_RS(0);
	else return;

	unsigned int toWrite_High = (data >> 4) & 0x0f;
	unsigned int toWrite_Low = data & 0x0f;

	set_lcd_data(D4, D5, D6, D7, toWrite_High);
	delay(500);
	set_gpio(E, 1);
	delay(500);
	set_gpio(E, 0);
	set_lcd_data(D4, D5, D6, D7, toWrite_Low);
	delay(500);
	set_gpio(E, 1);
	delay(500);
	set_gpio(E, 0);
}






GPIO RS = { LCD_RS_GPIO_Port, LCD_RS_Pin };
GPIO E  = { LCD_E_GPIO_Port,  LCD_E_Pin  };
GPIO D4 = { LCD_D4_GPIO_Port, LCD_D4_Pin };
GPIO D5 = { LCD_D5_GPIO_Port, LCD_D5_Pin };
GPIO D6 = { LCD_D6_GPIO_Port, LCD_D6_Pin };
GPIO D7 = { LCD_D7_GPIO_Port, LCD_D7_Pin };

void LCD_Set_RS(uint8_t data) {
	RS.port->BSRR = data ? RS.pin : RS.pin << 16;
}

void LCD_Set_E(uint8_t data) {
	E.port->BSRR = data ? E.pin : E.pin << 16;
}
//
void LCD_Set_Data(uint8_t data) {
    D4.port->BSRR = (data & 0x1) ? D4.pin : D4.pin << 16;
    D5.port->BSRR = (data & 0x2) ? D5.pin : D5.pin << 16;
    D6.port->BSRR = (data & 0x4) ? D6.pin : D6.pin << 16;
    D7.port->BSRR = (data & 0x8) ? D7.pin : D7.pin << 16;
}
//
//
//

//
void LCR_LCD_Write (enum eLCD_OP op, uint8_t data) {
  // Writes a byte to the LCD.  This assumes four-bit mode.
  if (op == WRITE_DATA) LCD_Set_RS(1);
  else if (op == WRITE_INSTRUCTION) LCD_Set_RS(0);
  else return;

  unsigned int toWrite_High = (data >> 4) & 0x0f;
  unsigned int toWrite_Low = data & 0x0f;
  LCD_Set_Data(toWrite_High);
  delay(LCD_DELAY_CONST);
  LCD_Set_E(1);
  delay(LCD_DELAY_CONST);
  LCD_Set_E(0);
  LCD_Set_Data(toWrite_Low);
  delay(LCD_DELAY_CONST);
  LCD_Set_E(1);
  delay(LCD_DELAY_CONST);
  LCD_Set_E(0);
}

void LCR_LCD_Clear (void) {
	  while(isbusy());
  LCR_LCD_Write(WRITE_INSTRUCTION, 0x01);
}

void LCR_LCD_GoToXY (int x, int y) {
	  while(isbusy());
  if( y == 0 ) {
    LCR_LCD_Write(WRITE_INSTRUCTION, 0x80 | (x & 0x3F));
  }
  else if( y == 1 ) {
    LCR_LCD_Write(WRITE_INSTRUCTION, 0xC0 | (x & 0x3F));
  }
}

void LCR_LCD_WriteChar (char ch) {
  // Write a character to the data register on the LCD:
	  while(isbusy());
  LCR_LCD_Write(WRITE_DATA, ch);
}

void LCR_LCD_WriteString(char *s) {
    while (*s) {
        while (isbusy());
        LCR_LCD_Write(WRITE_DATA, *s++);
    }
}


void LCR_LCD_DefineChar (int ch, char *data) {
	// Defines the character (ch can be 0 - 7 inclusive) to have the
	// pattern defined in the first eight entries in the data array.
	if (ch < 0 || ch > 7) return;
	LCR_LCD_Write(WRITE_INSTRUCTION, 0x40 + ch * 8);
	for (int u = 0; u < 8; u++) {
		LCR_LCD_Write(WRITE_DATA, data[u]);
	}
	LCR_LCD_Write(WRITE_INSTRUCTION, 0x80);
}



void lcd_init(GPIO RS, GPIO E, GPIO D4, GPIO D5, GPIO D6, GPIO D7) {
	delay(15000);
	set_gpio(RS, 0);

	set_lcd_data(D4, D5, D6, D7, 3);
	delay(5000);

	set_gpio(E, 1);
	delay(5000);
	set_gpio(E, 0);
	delay(5000);
	set_gpio(E, 1);
	delay(5000);
	set_gpio(E, 0);
	delay(5000);
	set_gpio(E, 1);
	delay(5000);
	set_gpio(E, 0);

	set_lcd_data(D4, D5, D6, D7, 2);
	delay(5000);
	set_gpio(E, 1);
	delay(5000);
	set_gpio(E, 0);

	while(isbusy());
	write_lcd_byte(E, D4, D5, D6, D7, WRITE_INSTRUCTION, 0x28);

	// Display ON/OFF Control: ON, no cursor
	while(isbusy());
	write_lcd_byte(E, D4, D5, D6, D7, WRITE_INSTRUCTION, 0x0c);

	// Clear the display
	while(isbusy());
	write_lcd_byte(E, D4, D5, D6, D7, WRITE_INSTRUCTION, 0x01);

	// Entry Mode Set: increment address (move right)
	while(isbusy());
	write_lcd_byte(E, D4, D5, D6, D7, WRITE_INSTRUCTION, 0x06);
}
