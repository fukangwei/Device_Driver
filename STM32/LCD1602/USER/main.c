#include "led.h"
#include "lcd1602.h"
#include "delay.h"
#include "sys.h"
#include "usart.h"

int main ( void ) {
    SystemInit();
    delay_init ( 72 );
    NVIC_Configuration();
    uart_init ( 9600 );
    LED_Init();
    LCD1602_Init();
    LCD1602_Show_Str ( 2, 1, "I love STM32" );

    while ( 1 ) {
        LED0 = !LED0;
        delay_ms ( 200 );
    }
}