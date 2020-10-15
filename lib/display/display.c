#include "display.h"

void lcd_init(int fd){
  // Initialise display
  lcd_byte(fd, 0x33, LCD_CMD); // Initialise
  lcd_byte(fd, 0x32, LCD_CMD); // Initialise
  lcd_byte(fd, 0x06, LCD_CMD); // Cursor move direction
  lcd_byte(fd, 0x0C, LCD_CMD); // 0x0F On, Blink Off
  lcd_byte(fd, 0x28, LCD_CMD); // Data length, number of lines, font size
  lcd_byte(fd, 0x01, LCD_CMD); // Clear display
  delayMicroseconds(500);
}

void lcd_byte(int fd, int bits, int mode){
  //Send byte to data pins
  // bits = the data
  // mode = 1 for data, 0 for command
  int bits_high;
  int bits_low;
  // uses the two half byte writes to LCD
  bits_high = mode | (bits & 0xF0) | LCD_BACKLIGHT ;
  bits_low = mode | ((bits << 4) & 0xF0) | LCD_BACKLIGHT ;

  // High bits
  wiringPiI2CReadReg8(fd, bits_high);
  lcd_toggle_enable(fd, bits_high);

  // Low bits
  wiringPiI2CReadReg8(fd, bits_low);
  lcd_toggle_enable(fd, bits_low);
}

void lcd_toggle_enable(int fd, int bits){
  // Toggle enable pin on LCD display
  delayMicroseconds(500);
  wiringPiI2CReadReg8(fd, (bits | ENABLE));
  delayMicroseconds(500);
  wiringPiI2CReadReg8(fd, (bits & ~ENABLE));
  delayMicroseconds(500);
}

// go to location on LCD
void lcdLoc(int fd, int line){
  lcd_byte(fd, line, LCD_CMD);
}

// clr lcd go home loc 0x80
void clrLcd(int fd){
  lcd_byte(fd, 0x01, LCD_CMD);
  lcd_byte(fd, 0x02, LCD_CMD);
}

// this allows use of any size string
void typeln(int fd, const char *s){

  while ( *s ) lcd_byte(fd, *(s++), LCD_CHR);

}
