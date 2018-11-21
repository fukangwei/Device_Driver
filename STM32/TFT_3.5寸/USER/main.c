#include "led.h"
#include "delay.h"
#include "sys.h"
#include "usart.h"
#include "lcd.h"

#define DISABLE_JTAG do \
    { \
        RCC->APB2ENR |= 0x00000001; /* 开启afio时钟 */ \
        AFIO->MAPR = ( 0x00FFFFFF & AFIO->MAPR ) | 0x04000000; /* 关闭JTAG */ \
    }while(0);

int main ( void ) {
    DISABLE_JTAG; /* 关闭JTAG功能 */
    SystemInit();
    delay_init ( 72 );
    NVIC_Configuration();
    uart_init ( 9600 );
    LED_Init();
    LCD_Init();
    LCD_Clear ( WHITE );
    POINT_COLOR = BLUE;
    LCD_ShowString ( 60, 50, 200, 16, 16, "Mini STM32" );

    while ( 1 ) {
    }
}