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

static void write_lcd_byte (GPIO RS, GPIO E, GPIO D4, GPIO D5, GPIO D6, GPIO D7, enum eLCD_OP op, uint8_t data) {
	if (op == WRITE_DATA) set_gpio(RS, 1);
	else if (op == WRITE_INSTRUCTION) set_gpio(RS, 0);
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

void lcd_write_string(GPIO RS, GPIO E, GPIO D4, GPIO D5, GPIO D6, GPIO D7, char *string) {
    while (*string) {
        while (isbusy());
        write_lcd_byte(RS, E, D4, D5, D6, D7, WRITE_DATA, *string++);
    }
}

void lcd_clear(GPIO RS, GPIO E, GPIO D4, GPIO D5, GPIO D6, GPIO D7) {
	while(isbusy());
	write_lcd_byte(RS, E, D4, D5, D6, D7, WRITE_INSTRUCTION, 0x01);
}

void lcd_set_cursor(GPIO RS, GPIO E, GPIO D4, GPIO D5, GPIO D6, GPIO D7, int x, int y) {
    while(isbusy());
    uint8_t data = (y == 0) ? (0x80 | (x & 0x3F)) : (0xC0 | (x & 0x3F));
    write_lcd_byte(RS, E, D4, D5, D6, D7, WRITE_INSTRUCTION, data);
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
	write_lcd_byte(RS, E, D4, D5, D6, D7, WRITE_INSTRUCTION, 0x28);

	// Display ON/OFF Control: ON, no cursor
	while(isbusy());
	write_lcd_byte(RS, E, D4, D5, D6, D7, WRITE_INSTRUCTION, 0x0c);

	// Clear the display
	while(isbusy());
	write_lcd_byte(RS, E, D4, D5, D6, D7, WRITE_INSTRUCTION, 0x01);

	// Entry Mode Set: increment address (move right)
	while(isbusy());
	write_lcd_byte(RS, E, D4, D5, D6, D7, WRITE_INSTRUCTION, 0x06);
}
