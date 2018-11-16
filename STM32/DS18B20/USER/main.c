#include "delay.h"
#include "sys.h"
#include "usart.h"
#include "led.h"
#include "lcd.h"
#include "ds18b20.h"

int main ( void ) {
    short temp;
    SystemInit();
    delay_init ( 72 );
    NVIC_Configuration();
    uart_init ( 9600 );
    LED_Init();
    LCD_Init();
    POINT_COLOR = RED;
    LCD_ShowString ( 60, 50, "Mini STM32" );
    LCD_ShowString ( 60, 70, "DS18B20 TEST" );
    LCD_ShowString ( 60, 90, "ATOM@ALIENTEK" );
    LCD_ShowString ( 60, 110, "2011/1/1" );

    while ( DS18B20_Init() ) {
        LCD_ShowString ( 60, 130, "DS18B20 Check Failed!" );
        delay_ms ( 500 );
        LCD_ShowString ( 60, 130, "Please Check!        " );
        delay_ms ( 500 );
        LED0 = !LED0;
    }

    LCD_ShowString ( 60, 130, "DS18B20 Ready!     " );
    POINT_COLOR = BLUE;
    LCD_ShowString ( 60, 150, "Temperate:   . C" );

    while ( 1 ) {
        temp = DS18B20_Get_Temp();

        if ( temp < 0 ) {
            temp = -temp;
            LCD_ShowChar ( 140, 150, '-', 16, 0 );
        }

        LCD_ShowNum ( 148, 150, temp / 10, 2, 16 );
        LCD_ShowNum ( 172, 150, temp % 10, 1, 16 );
        delay_ms ( 200 );
        LED0 = !LED0;
    }
}