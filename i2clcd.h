/**
 * @file
 * @code #include "i2clcd.h"
 * @endcode
 * 
 * @brief A Library for Interfacing AVR with LCDs that have I2C bus.
 * 
 * Basic functions for use with HD44780 based LCDs that use PCF8574 I/O expander.
 *  
 * Requires: I2C Master library by Peter Fleury pfleury@gmx.ch
 * 
 * @author Sandeep Kumar http://www.github.com/w0qs1
 * @copyright (C) 2021 Sandeep Kumar, GNU General Public License Version 3
 * 
 * @par Usage Example:
 * 
 * @code
 * #include <avr/io.h>
 * #include <util/delay.h>
 * #include "i2cmaster.h"
 * #include "i2clcd.h"
 * 
 * #define LCD_ADDRESS 0x4E // Address of the PCF8574
 * #define BACKLIGHT 1 // Backlight ON
 * 
 * int main(void) {
 *     lcd_init(LCD_ADDRESS, BACKLIGHT); // Initialize the LCD with BACKLIGHT ON
 *     lcd_clear(); // Clear the Display
 * 
 *     lcd_setcursor(0, 0); // Set the cursor to first line, first column
 *     lcd_print("Hello, World!"); // Print "Hello, World!" on the LCD
 * }
 * @endcode
 */

// The connection of the PCF8574 to the LCD:
/*
P0 <-> RS
P1 <-> RW
P2 <-> EN
P3 <-> BL

P4 <-> D4
P5 <-> D5
P6 <-> D6
P7 <-> D7
*/

// The following defines set the bit positions for the control signals
#define RS 0
#define RW 1
#define EN 2
#define BL 3

// some global variables
uint8_t lcd_address;
uint8_t backlight = 0;
uint8_t cursor = 0;
uint8_t blink = 0;

/**
 * @brief  To check the status of the LCD Busy Flag (BF)
 * @note   Returns a non zero uint8_t if the LCD is busy. Else returns 0
 * @return a uint8_t
 */
uint8_t lcd_is_busy()
{
	uint8_t data;
	i2c_start(lcd_address + I2C_WRITE);

	i2c_write((0 << RS) | (1 << RW) | (0 << EN) | (backlight << BL));
	i2c_write((0 << RS) | (1 << RW) | (1 << EN) | (backlight << BL));

	i2c_start(lcd_address + I2C_READ);
	data = i2c_readNak();

	i2c_start(lcd_address + I2C_WRITE);

	i2c_write((0 << RS) | (0 << RW) | (0 << EN) | (backlight << BL));

	return (data & (1 << 7));
}

/**
 * @brief  To write a command to the LCD
 * @param  data: Command to be sent
 * @return None
 */
void lcd_write_cmnd(uint8_t data)
{
	i2c_start(lcd_address + I2C_WRITE);

	i2c_write((data & 0xF0) | (0 << RS) | (0 << RW) | (0 << EN) | (backlight << BL)); 		// Higher nibble first
	i2c_write((data & 0xF0) | (0 << RS) | (0 << RW) | (1 << EN) | (backlight << BL)); 		// Enable (1)
	_delay_us(1);																	  		// Pulse width for enable
	i2c_write((data & 0xF0) | (0 << RS) | (0 << RW) | (0 << EN) | (backlight << BL)); 		// Enable (0)
	i2c_stop();
	_delay_ms(2);

	i2c_start(lcd_address + I2C_WRITE);

	i2c_write(((data & 0x0F) << 4) | (0 << RS) | (0 << RW) | (0 << EN) | (backlight << BL)); // Lower nibble
	i2c_write(((data & 0x0F) << 4) | (0 << RS) | (0 << RW) | (1 << EN) | (backlight << BL)); // Enable (1)
	_delay_us(1);																			 // Pulse width for enable
	i2c_write(((data & 0x0F) << 4) | (0 << RS) | (0 << RW) | (0 << EN) | (backlight << BL)); // Enable (0)
	i2c_stop();
	_delay_ms(2);
}

/**
 * @brief  To write a byte of data to the LCD
 * @param  data: Data to be written to the LCD
 * @return None
 */
void lcd_write_data(uint8_t data)
{
	i2c_start(lcd_address + I2C_WRITE);

	i2c_write((data & 0xF0) | (1 << RS) | (0 << RW) | (0 << EN) | (backlight << BL)); 		// Higher nibble first
	i2c_write((data & 0xF0) | (1 << RS) | (0 << RW) | (1 << EN) | (backlight << BL)); 		// Enable (1)
	_delay_us(1);																	  		// Pulse width for enable
	i2c_write((data & 0xF0) | (1 << RS) | (0 << RW) | (0 << EN) | (backlight << BL));	 	// Enable (0)
	i2c_stop();
	_delay_ms(2);

	i2c_start(lcd_address + I2C_WRITE);

	i2c_write(((data & 0x0F) << 4) | (1 << RS) | (0 << RW) | (0 << EN) | (backlight << BL)); // Lower nibble
	i2c_write(((data & 0x0F) << 4) | (1 << RS) | (0 << RW) | (1 << EN) | (backlight << BL)); // Enable (1)
	_delay_us(1);																			 // Pulse width for enable
	i2c_write(((data & 0x0F) << 4) | (1 << RS) | (0 << RW) | (0 << EN) | (backlight << BL)); // Enable (0)
	i2c_stop();
	_delay_ms(2);
}

/**
 * @brief  To read byte(s) of the data from the LCD
 * @param  data[]: Buffer to hold the read data
 * @param  address: Address of the DDRAM from which the data to be read
 * @param  len: Length of the data (in bytes)
 * @param  cursordir: Direction of read. Left if 0 else Right
 * @return None
 */
void lcd_read_ddram(uint8_t data[], uint8_t address, uint8_t len, uint8_t cursordir)
{
	cursordir &= 1;
	lcd_write_cmnd((1 << 4) | (cursordir << 2));	// Set the cursor shift
	lcd_write_cmnd(address);						// Set the DDRAM address to read from

	i2c_start(lcd_address + I2C_WRITE);
	i2c_write((1 << RS) | (1 << RW) | (0 << EN) | (backlight << BL));
	i2c_write((1 << RS) | (1 << RW) | (1 << EN) | (backlight << BL));

	uint8_t i;
	i2c_start(lcd_address + I2C_READ);
	for (i = 0; i < len - 1; i++)
	{
		data[i] = i2c_readAck();
	}
	data[i++] = i2c_readNak();

	i2c_start(lcd_address + I2C_WRITE);
	i2c_write((1 << RS) | (1 << RW) | (0 << EN) | (backlight << BL));
}

/**
 * @brief  To store a symbol in the CGRAM of the LCD(Character Generator RAM)
 * @param  data[]: Symbol data
 * @param  address: Location to be stored at
 * @return None
 */
void lcd_store_char(uint8_t data[], uint8_t address)
{
	address %= 8; 									// Overflow the address

	uint8_t char_addr = 0x40 + (address * 8);
	lcd_write_cmnd(char_addr); 						// Set the address

	for (uint8_t i = 0; i < 8; i++)
	{
		lcd_write_data(data[i]);					// Put the symbol data into the CGRAM
	}
}

/**
 * @brief  To print a symbol on the LCD
 * @param  address: Location of the symbol in the CGRAM
 * @return None
 */
void lcd_print_char(uint8_t address)
{
	address %= 8; 									// Overflow the address

	lcd_write_data(address);
}

/**
 * @brief  To initialize the LCD
 * @note   This sets to LCD in 4-bit mode and sets the cursor properties from lcd_cursor()
 * @param  address: I2C address of the PCF8574 (Usually it is 0x4E)
 * @param  bl: 1 -> BACKLIGHT ON, 0 -> BACKLIGHT OFF
 * @return None
 */
void lcd_init(uint8_t address, uint8_t bl)
{
	i2c_init();

	lcd_address = address;
	backlight = bl & 1;
	lcd_write_cmnd(0x02); 							// 4 bit mode
	lcd_write_cmnd(0x28); 							// 4 bit, 2 lines, 5x7 matrix
	lcd_write_cmnd(0x0C | ((cursor & 1) << 1) | ((blink & 1) << 0));
}

/**
 * @brief  To set the cursor visibility and blinking
 * @note   Must be used before lcd_init() to set the cursor properties
 * @param  cr: Visibility of the cursor (1 -> CURSOR ON, 0 -> CURSOR OFF)
 * @param  bn: Blinking of the cursor (1 -> BLINKING ON, 0 -> BLINKING OFF)
 * @return None
 */
void lcd_cursor(uint8_t cr, uint8_t bn)
{
	cursor = cr & 1;
	blink = bn & 1;
}

/**
 * @brief  To turn off the display
 * @param  bl: Backlight (ON if 1 else OFF)
 * @return None
 */
void lcd_off(uint8_t bl)
{
	backlight = bl & 1;
	lcd_write_cmnd(0x08);
}

/**
 * @brief  To clear the LCD
 * @return None
 */
void lcd_clear()
{
	lcd_write_cmnd(0x01);
}

/**
 * @brief  To set the location of the cursor of the display
 * @param  row: Line number (starts from 0)
 * @param  col: Column number (starts from 0)
 * @return None
 */
void lcd_setcursor(uint8_t row, uint8_t col)
{
	uint8_t lines[] = {0x80, 0xC0, 0x94, 0xD4};
	lcd_write_cmnd(lines[row] + col);
}

/**
 * @brief  To print a NULL terminated string
 * @param  data[]: NULL terminated string
 * @return None
 */
void lcd_print(uint8_t data[])
{
	uint8_t i = 0, len = 0;
	while (data[i++] != '\0')						// To get the length of the string
	{
		len++;
	}

	for (i = 0; i < len; i++)
	{
		lcd_write_data(data[i]);					// Write the data
	}
}
