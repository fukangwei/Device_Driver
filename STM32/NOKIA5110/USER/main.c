#include "delay.h"
#include "sys.h"
#include "nokia_5110.h"

int main ( void ) {
    SystemInit();
    delay_init ( 72 );
    NVIC_Configuration();
    LCD_init();

    while ( 1 ) {
        LCD_write_english_string ( 0, 0, " Welcome!" );
        LCD_write_english_string ( 0, 3, " Nokia5110 LCD" );
        LCD_write_chinese_string ( 12, 4, 12, 4, 0, 5 );
    }
}