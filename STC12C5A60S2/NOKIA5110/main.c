#include <reg51.h>
#include "nokia_5110.h"
#include "bmp_pixel.h"

void main ( void ) {
    LCD_init();

    while ( 1 ) {
        LCD_write_english_string ( 0, 0, " Welcome!" );
        LCD_write_english_string ( 0, 3, " Nokia5110 LCD" );
        LCD_write_chinese_string ( 12, 4, 12, 4, 0, 5 );
    }
}