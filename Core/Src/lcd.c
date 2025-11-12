#include <main.h>
#include <lcd.h>
#include <stdbool.h>

// LCR_MicroDelay attempts to delay by the requested number of microseconds
// The factors were determined experimentally for the Nucleo-G071RB board
// running at 64 MHz with no compiler optimisations.

enum eLCD_OP { READ_INSTRUCTION, WRITE_INSTRUCTION, READ_DATA, WRITE_DATA };

uint8_t LCD_shift = 0;

void LCR_MicroDelay (uint32_t delayInMicroSeconds) {
  float compensation = (float)SystemCoreClock / (float)16e6;
  volatile unsigned long x = (unsigned long)(compensation * (36 * delayInMicroSeconds >> 4));
  while (x-- > 0);
}

///////////////////////////////////////////////////////////////////////////////
// LCD driver functions start here:
// I'll slow all the accesses down a bit, since I don't think the LCD
// controller will be able to keep up with the ARM-Cortex.  Experimentally,
// a delay of around XX us seems to be enough, and it makes writing to the
// LCD so fast that you can't see the individual characters appear.  It's
// also less likely to be interrupted by a reset in the middle of an operation,
// and this can upset it (I'm still not entirely clear why this happens, or
// how to kick it out of whatever random state it gets into at these times).
#define LCD_DELAY_CONST 500

void LCD_Set_Data(uint8_t data) {
	// This takes the lowest four bits in data and puts them on the
	// relevant pins of the LCD (LCD_D4 to LCD_D7) in four-bit mode.
	GPIOA->BSRR = (data & 0x1) ? 0x1 << 12 : 0x1 << 28;
	GPIOB->BSRR = (data & 0x2) ? 0x1 << 0 : 0x1 << 16;
	GPIOB->BSRR = (data & 0x4) ? 0x1 << 7 : 0x1 << 23;
	GPIOB->BSRR = (data & 0x8) ? 0x1 << 6 : 0x1 << 22;
}
void LCD_Set_RS(uint8_t data) {
	// Sets the RS control line to either high or low:
	GPIOA->BSRR = data ? 0x1 << 9 : 0x1 << 25;
}
void LCD_Set_RW(uint8_t data) {
	// Sets the RW control line to either high or low:
	GPIOF->BSRR = data ? 0x1 << 1 : 0x1 << 17;
}
void LCD_Set_E(uint8_t data) {
	// Sets the RW control line to either high or low:
	GPIOA->BSRR = data ? 0x1 << 10 : 0x1 << 26;
}
uint8_t LCR_LCD_IsBusy () {
  // For now, I'll just use the delay version of this.  Wait for a ms:
  LCR_MicroDelay(1000);
  return 0;
}
void LCR_LCD_Write (enum eLCD_OP op, uint8_t data) {
  // Writes a byte to the LCD.  This assumes four-bit mode.
  if (op == WRITE_DATA) LCD_Set_RS(1);
  else if (op == WRITE_INSTRUCTION) LCD_Set_RS(0);
  else return;

  unsigned int toWrite_High = (data >> 4) & 0x0f;
  unsigned int toWrite_Low = data & 0x0f;
  LCD_Set_Data(toWrite_High);
  LCR_MicroDelay(LCD_DELAY_CONST); LCD_Set_E(1);
  LCR_MicroDelay(LCD_DELAY_CONST); LCD_Set_E(0);
  LCD_Set_Data(toWrite_Low);
  LCR_MicroDelay(LCD_DELAY_CONST); LCD_Set_E(1);
  LCR_MicroDelay(LCD_DELAY_CONST); LCD_Set_E(0);
}
void LCR_LCD_Init (void) {
  // The LCD uses GPIOs A, B and F, so these clocks are required:
  RCC->AHBENR |= RCC_AHBENR_GPIOAEN | RCC_AHBENR_GPIOBEN | RCC_AHBENR_GPIOFEN;

  // Relevant control GPIO pins are PA8 (RS), PF1 (RW), PF0 (E)
  /*LCR_Set_As_Output(8, GPIOA);
  LCR_Set_As_Output(1, GPIOF);
  LCR_Set_As_Output(0, GPIOF);

  // And for data, GPIOB 1, 6, 7 and 0
  LCR_Set_As_Output(0, GPIOB);
  LCR_Set_As_Output(1, GPIOB);
  LCR_Set_As_Output(6, GPIOB);
  LCR_Set_As_Output(7, GPIOB);
  */

  // The three writes in the following block are a workaround for
  // an interesting problem.  If this is a power on, then the LCD
  // will have powered up in the default eight-bit mode.  In this
  // case, all three writes will be treated as eight-bit writes,
  // confirming the eight-bit mode.  However if this was a reset
  // of the processor, then the LCD won't have been reset, and
  // will already be in four-bit mode.  So you can't just start
  // writing assuming that the LCD was in eight-bit mode.
  //
  // If it was in four-bit mode, then the first two of these
  // writes will be treated as one combination eight-bit write,
  // setting the LCD back into eight-bit mode.  The third write
  // is then an eight-bit write confirming this.  Once the LCD is
  // in the known eight-bit mode, it can be put into four-bit
  // mode unambiguously.
  //
  // You can't wait for the LCD to say it's ready yet, as the
  // instruction set has to be chosen first (see datasheet).
  // So these operations have to go slowly:

  LCR_MicroDelay(15000);
  LCD_Set_RS(0); // Set LCD_RS low
  LCD_Set_RW(0); // Set LCD_RW low
  LCD_Set_Data(3);  // Set the LCD_D4-D7 to 0b0011
  LCR_MicroDelay(5000); LCD_Set_E(1); // Set LCD_E high
  LCR_MicroDelay(5000); LCD_Set_E(0); // Set LCD_E low
  LCR_MicroDelay(5000); LCD_Set_E(1); // Set LCD_E high
  LCR_MicroDelay(5000); LCD_Set_E(0); // Set LCD_E low
  LCR_MicroDelay(5000); LCD_Set_E(1); // Set LCD_E high
  LCR_MicroDelay(5000); LCD_Set_E(0); // Set LCD_E low

  // Now LCD should be in eight-bit mode no matter where it started
  // from, so we can set the LCD to four-bit mode unambiguously with
  // one write cycle:
  LCD_Set_Data(2);  // Set the LCD_D4-D7 to 0b0010
  LCR_MicroDelay(5000); LCD_Set_E(1); // Set LCD_E high
  LCR_MicroDelay(5000); LCD_Set_E(0); // Set LCD_E low

  // Now can set the other control bits: two-line and 5*8 pixels
  while (LCR_LCD_IsBusy());
  LCR_LCD_Write(WRITE_INSTRUCTION, 0x28);

  // Display ON/OFF Control: ON, no cursor
  while (LCR_LCD_IsBusy());
  LCR_LCD_Write(WRITE_INSTRUCTION, 0x0c);

  // Clear the display
  while (LCR_LCD_IsBusy());
  LCR_LCD_Write(WRITE_INSTRUCTION, 0x01);

  // Entry Mode Set: increment address (move right)
  while (LCR_LCD_IsBusy());
  LCR_LCD_Write(WRITE_INSTRUCTION, 0x06);
}
void LCR_LCD_Clear (void) {
  while (LCR_LCD_IsBusy());
  LCR_LCD_Write(WRITE_INSTRUCTION, 0x01);
}
void LCR_LCD_GoToXY (int x, int y) {
  while (LCR_LCD_IsBusy());
  if( y == 0 ) {
    LCR_LCD_Write(WRITE_INSTRUCTION, 0x80 | (x & 0x3F));
  }
  else if( y == 1 ) {
    LCR_LCD_Write(WRITE_INSTRUCTION, 0xC0 | (x & 0x3F));
  }
}
void LCR_LCD_WriteChar (char ch) {
  // Write a character to the data register on the LCD:
  while (LCR_LCD_IsBusy());
  LCR_LCD_Write(WRITE_DATA, ch);
}
void LCR_LCD_WriteString (char *s, int maxLength) {
  while(*s && maxLength-- > 0) {
	// LCR_MicroDelay(10000); // This works, but
    while (LCR_LCD_IsBusy()){} // This does not
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
