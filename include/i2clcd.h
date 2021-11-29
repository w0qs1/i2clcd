#ifndef I2CLCD_H
#define I2CLCD_H

#include <avr/io.h>
#include <util/delay.h>
#include "i2cmaster.h"

/**
 * @file
 * @code #include "i2clcd.h"
 * @endcode
 * 
 * @brief A Library for Interfacing AVR with LCDs that have I2C bus.
 * 
 * Basic functions for use with HD44780 based LCDs that use PCF8574 I/O expander.
 * As of version 2.0, multiple LCDs can be used at once
 *  
 * Requires: I2C Master library by Peter Fleury pfleury@gmx.ch
 * 
 * @version 2.0
 * @author Sandeep Kumar http://www.github.com/w0qs1
 * @copyright (C) 2021 Sandeep Kumar, GNU General Public License Version 3
 * 
 * @par Usage Example:
 * 
 * @code
 * #include <avr/io.h>
 * #include <util/delay.h>
 * #include <i2cmaster.h>
 * #include <i2clcd.h>
 *  
 * #define LCD_ADDRESS1 0x4E
 * #define LCD_ADDRESS2 0x4C (A0 of PCF8574 is pulled low)
 *  
 * uint8_t msg1[] = "Display One";
 * uint8_t msg2[] = "Display Two";
 * 
 * int main(void) {
 *  	i2clcd lcd1;
 * 	i2clcd lcd2;
 * 
 * 	lcd_init(&lcd1, LCD_ADDRESS1, 1);	// Initialize LCD and clear
 * 	lcd_clear(&lcd1);
 * 
 * 	lcd_init(&lcd2, LCD_ADDRESS2, 1);
 * 	lcd_clear(&lcd2);
 * 
 * 	lcd_setcursor(&lcd1, 0, 0);			// Set cursor to home position and print
 * 	lcd_print(&lcd1, msg1);
 * 
 * 	lcd_setcursor(&lcd2, 0, 0);
 * 	lcd_print(&lcd2, msg2);
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

/**
 * @brief  This is creates a new LCD instance
 * @note   The address of the instance must be passed in a function call
 */
typedef struct i2clcd{
	uint8_t lcd_address;
	uint8_t backlight;
	uint8_t cursor;
	uint8_t blink;
}i2clcd;

/**
 * @brief  To check the status of the LCD Busy Flag (BF)
 * @note   Returns a non zero uint8_t if the LCD is busy. Else returns 0
 * @param  lcd: Address of the LCD instance
 * @return a uint8_t
 */
extern uint8_t lcd_is_busy(i2clcd *);

/**
 * @brief  To write a command to the LCD
 * @param  lcd: Address of the LCD instance
 * @param  data: Command to be sent
 * @return None
 */
extern void lcd_write_cmnd(i2clcd *, uint8_t);

/**
 * @brief  To write a byte of data to the LCD
 * @param  lcd: Address of the LCD instance
 * @param  data: Data to be written to the LCD
 * @return None
 */
extern void lcd_write_data(i2clcd *, uint8_t);

/**
 * @brief  To read byte(s) of the data from the LCD
 * @param  lcd: Address of the LCD instance
 * @param  data[]: Buffer to hold the read data
 * @param  address: Address of the DDRAM from which the data to be read
 * @param  len: Length of the data (in bytes)
 * @param  cursordir: Direction of read. Left if 0 else Right
 * @return None
 */
extern void lcd_read_ddram(i2clcd *, uint8_t *, uint8_t, uint8_t, uint8_t);

/**
 * @brief  To store a symbol in the CGRAM of the LCD(Character Generator RAM)
 * @param  lcd: Address of the LCD instance
 * @param  data[]: Symbol data
 * @param  address: Location to be stored at
 * @return None
 */
extern void lcd_store_char(i2clcd *, uint8_t *, uint8_t);

/**
 * @brief  To print a symbol on the LCD
 * @param  lcd: Address of the LCD instance
 * @param  address: Location of the symbol in the CGRAM
 * @return None
 */
extern void lcd_print_char(i2clcd *, uint8_t);

/**
 * @brief  To initialize the LCD
 * @note   This sets to LCD in 4-bit mode and sets the cursor properties from lcd_cursor()
 * @param  lcd: Address of the LCD instance
 * @return None
 */
extern void lcd_init(i2clcd *, uint8_t, uint8_t);

/**
 * @brief  To set the cursor visibility and blinking
 * @note   To be used before lcd_init()
 * @param  lcd: Address of the LCD instance
 * @param  cr: Visibility of the cursor (1 -> CURSOR ON, 0 -> CURSOR OFF)
 * @param  bn: Blinking of the cursor (1 -> BLINKING ON, 0 -> BLINKING OFF)
 * @return None
 */
extern void lcd_cursor(i2clcd *, uint8_t, uint8_t);

/**
 * @brief  To turn off the display
 * @param  lcd: Address of the LCD instance
 * @param  bl: Backlight (ON if 1 else OFF)
 * @return None
 */
extern void lcd_off(i2clcd *, uint8_t);

/**
 * @brief  To clear the LCD
 * @param  lcd: Address of the LCD instance
 * @return None
 */
extern void lcd_clear(i2clcd *);

/**
 * @brief  To set the location of the cursor of the display
 * @param  lcd: Address of the LCD instance
 * @param  row: Line number (starts from 0)
 * @param  col: Column number (starts from 0)
 * @return None
 */
extern void lcd_setcursor(i2clcd *, uint8_t, uint8_t);

/**
 * @brief  To print a NULL terminated string
 * @param  lcd: Address of the LCD instance
 * @param  data[]: NULL terminated string
 * @return None
 */
extern void lcd_print(i2clcd *, uint8_t *);

#endif
