#include <i2clcd.h>

uint8_t lcd_is_busy(i2clcd *lcd)
{
	uint8_t data;
	i2c_start_wait(lcd->lcd_address + I2C_WRITE);

	i2c_write((0 << RS) | (1 << RW) | (0 << EN) | (lcd->backlight << BL));
	i2c_write((0 << RS) | (1 << RW) | (1 << EN) | (lcd->backlight << BL));

	i2c_start_wait(lcd->lcd_address + I2C_READ);
	data = i2c_readNak();

	i2c_start_wait(lcd->lcd_address + I2C_WRITE);

	i2c_write((0 << RS) | (0 << RW) | (0 << EN) | (lcd->backlight << BL));

	return (data & (1 << 7));
}

void lcd_write_cmnd(i2clcd *lcd, uint8_t data)
{
	i2c_start_wait(lcd->lcd_address + I2C_WRITE);

	i2c_write((data & 0xF0) | (0 << RS) | (0 << RW) | (0 << EN) | (lcd->backlight << BL)); 			// Higher nibble first
	i2c_write((data & 0xF0) | (0 << RS) | (0 << RW) | (1 << EN) | (lcd->backlight << BL)); 			// Enable (1)
	_delay_us(1);																	  				// Pulse width for enable
	i2c_write((data & 0xF0) | (0 << RS) | (0 << RW) | (0 << EN) | (lcd->backlight << BL)); 			// Enable (0)
	i2c_stop();
	_delay_ms(2);

	i2c_start_wait(lcd->lcd_address + I2C_WRITE);

	i2c_write(((data & 0x0F) << 4) | (0 << RS) | (0 << RW) | (0 << EN) | (lcd->backlight << BL));	// Lower nibble
	i2c_write(((data & 0x0F) << 4) | (0 << RS) | (0 << RW) | (1 << EN) | (lcd->backlight << BL)); 	// Enable (1)
	_delay_us(1);																			 	 	// Pulse width for enable
	i2c_write(((data & 0x0F) << 4) | (0 << RS) | (0 << RW) | (0 << EN) | (lcd->backlight << BL)); 	// Enable (0)
	i2c_stop();
	_delay_ms(2);
}

void lcd_write_data(i2clcd *lcd, uint8_t data)
{
	i2c_start_wait(lcd->lcd_address + I2C_WRITE);

	i2c_write((data & 0xF0) | (1 << RS) | (0 << RW) | (0 << EN) | (lcd->backlight << BL)); 			// Higher nibble first
	i2c_write((data & 0xF0) | (1 << RS) | (0 << RW) | (1 << EN) | (lcd->backlight << BL)); 			// Enable (1)
	_delay_us(1);																	  				// Pulse width for enable
	i2c_write((data & 0xF0) | (1 << RS) | (0 << RW) | (0 << EN) | (lcd->backlight << BL));	 		// Enable (0)
	i2c_stop();
	_delay_ms(2);

	i2c_start_wait(lcd->lcd_address + I2C_WRITE);

	i2c_write(((data & 0x0F) << 4) | (1 << RS) | (0 << RW) | (0 << EN) | (lcd->backlight << BL)); 	// Lower nibble
	i2c_write(((data & 0x0F) << 4) | (1 << RS) | (0 << RW) | (1 << EN) | (lcd->backlight << BL)); 	// Enable (1)
	_delay_us(1);																			 		// Pulse width for enable
	i2c_write(((data & 0x0F) << 4) | (1 << RS) | (0 << RW) | (0 << EN) | (lcd->backlight << BL)); 	// Enable (0)
	i2c_stop();
	_delay_ms(2);
}

void lcd_read_ddram(i2clcd *lcd, uint8_t data[], uint8_t address, uint8_t len, uint8_t cursordir)
{
	cursordir &= 1;
	lcd_write_cmnd(lcd, (1 << 4) | (cursordir << 2));	// Set the cursor shift
	lcd_write_cmnd(lcd, address);						// Set the DDRAM address to read from

	i2c_start_wait(lcd->lcd_address + I2C_WRITE);
	i2c_write((1 << RS) | (1 << RW) | (0 << EN) | (lcd->backlight << BL));
	i2c_write((1 << RS) | (1 << RW) | (1 << EN) | (lcd->backlight << BL));

	uint8_t i;
	i2c_start_wait(lcd->lcd_address + I2C_READ);
	for (i = 0; i < len - 1; i++)
	{
		data[i] = i2c_readAck();
	}
	data[i++] = i2c_readNak();

	i2c_start_wait(lcd->lcd_address + I2C_WRITE);
	i2c_write((1 << RS) | (1 << RW) | (0 << EN) | (lcd->backlight << BL));
}

void lcd_store_char(i2clcd *lcd, uint8_t data[], uint8_t address)
{
	address %= 8; 										// Overflow the address

	uint8_t char_addr = 0x40 + (address * 8);
	lcd_write_cmnd(lcd, char_addr); 					// Set the address

	for (uint8_t i = 0; i < 8; i++)
	{
		lcd_write_data(lcd, data[i]);					// Put the symbol data into the CGRAM
	}
}

void lcd_print_char(i2clcd *lcd, uint8_t address)
{
	address %= 8; 									// Overflow the address

	lcd_write_data(lcd, address);
}

void lcd_init(i2clcd *lcd, uint8_t address, uint8_t bl)
{
	i2c_init();

	lcd->lcd_address = address;
	lcd->backlight = bl & 1;
	lcd_write_cmnd(lcd, 0x02); 							// 4 bit mode
	lcd_write_cmnd(lcd, 0x28); 							// 4 bit, 2 lines, 5x7 matrix
	lcd_write_cmnd(lcd, 0x0C);
	// Turn on the display with the cursor properties
}

void lcd_cursor(i2clcd *lcd, uint8_t cr, uint8_t bn)
{
	lcd->cursor = cr & 1;
	lcd->blink = bn & 1;
}

void lcd_off(i2clcd *lcd, uint8_t bl)
{
	lcd->backlight = bl & 1;
	lcd_write_cmnd(lcd, 0x08);
}

void lcd_clear(i2clcd *lcd)
{
	lcd_write_cmnd(lcd, 0x01);
}

void lcd_setcursor(i2clcd *lcd, uint8_t row, uint8_t col)
{
	uint8_t lines[] = {0x80, 0xC0, 0x94, 0xD4};
	lcd_write_cmnd(lcd, lines[row] + col);
}

void lcd_print(i2clcd *lcd, uint8_t data[])
{
	uint8_t i = 0, len = 0;
	while (data[i++] != '\0')						// To get the length of the string
	{
		len++;
	}

	for (i = 0; i < len; i++)
	{
		lcd_write_data(lcd, data[i]);			    // Write the data
	}
}
