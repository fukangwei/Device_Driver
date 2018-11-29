#ifndef __NOKIA5110_H_
#define __NOKIA5110_H_
#include "sys.h"

#define SCLK    PAout(4)
#define SDIN    PAout(3)
#define LCD_DC  PAout(2)
#define LCD_CE  PAout(1)
#define LCD_RST PAout(0)

void LCD_init ( void );
void LCD_write_english_string ( unsigned char X, unsigned char Y, char *s );
void LCD_write_chinese_string ( unsigned char X, unsigned char Y, unsigned char ch_with,
                                unsigned char num, unsigned char line, unsigned char row );
void LCD_draw_bmp_pixel ( unsigned char X, unsigned char Y, unsigned char *map, unsigned char Pix_x, unsigned char Pix_y );
#endif