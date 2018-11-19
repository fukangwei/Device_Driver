#include "led.h"
#include "delay.h"
#include "sys.h"
#include "usart.h"
#include "lcd.h"

int main ( void ) {
    u8 x = 0;
    SystemInit();
    delay_init ( 72 );
    NVIC_Configuration();
    uart_init ( 9600 );
    LED_Init();
    LCD_Init();
    POINT_COLOR = RED;

    while ( 1 ) {
        switch ( x ) {
            case 0:
                LCD_Clear ( WHITE );
                break;

            case 1:
                LCD_Clear ( BLACK );
                break;

            case 2:
                LCD_Clear ( BLUE );
                break;

            case 3:
                LCD_Clear ( RED );
                break;

            case 4:
                LCD_Clear ( MAGENTA );
                break;

            case 5:
                LCD_Clear ( GREEN );
                break;

            case 6:
                LCD_Clear ( CYAN );
                break;

            case 7:
                LCD_Clear ( YELLOW );
                break;

            case 8:
                LCD_Clear ( BRRED );
                break;

            case 9:
                LCD_Clear ( GRAY );
                break;

            case 10:
                LCD_Clear ( LGRAY );
                break;

            case 11:
                LCD_Clear ( BROWN );
                break;
        }

        POINT_COLOR = RED;
        LCD_ShowString ( 30, 50, "Mini STM32 ^_^" );
        x++;

        if ( x == 12 ) {
            x = 0;
        }

        LED0 = !LED0;
        delay_ms ( 1000 );
    }
}