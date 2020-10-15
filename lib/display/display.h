/*
*
* by Lewis Loflin www.bristolwatch.com lewis@bvu.net
* http://www.bristolwatch.com/rpi/i2clcd.htm
* Using wiringPi by Gordon Henderson
*
*
* Port over lcd_i2c.py to C and added improvements.
* Supports 16x2 and 20x4 screens.
* This was to learn now the I2C lcd displays operate.
* There is no warrenty of any kind use at your own risk.
*
* Some adjustments were made at https://github.com/icaropires version
*/

#ifndef DISPLAY_H_
#define DISPLAY_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>
#include <stdio.h>

#include <wiringPiI2C.h>
#include <wiringPi.h>

// Define some device constants
#define LCD_CHR  1 // Mode - Sending data
#define LCD_CMD  0 // Mode - Sending command

#define LINE1  0x80 // 1st line
#define LINE2  0xC0 // 2nd line

#define LCD_BACKLIGHT   0x08  // On

// #define ENABLE  0b00000100 // Enable bit
#define ENABLE (1 << 2) // Enable bit

void lcd_init(int fd);
void lcd_byte(int fd, int bits, int mode);
void lcd_toggle_enable(int fd, int bits);

// added by Lewis
void lcdLoc(int fd, int line); //move cursor
void clrLcd(int fd); // clr LCD return home
void typeln(int fd, const char *s);

#ifdef __cplusplus
}
#endif

#endif /* DISPLAY_H_ */
