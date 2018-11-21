#include "led.h"
#include "delay.h"
#include "sys.h"
#include "usart.h"
#include "lcd.h"

#define DISABLE_JTAG do \
    { \
        RCC->APB2ENR |= 0x00000001; /* ����afioʱ�� */ \
        AFIO->MAPR = ( 0x00FFFFFF & AFIO->MAPR ) | 0x04000000; /* �ر�JTAG */ \
    }while(0);

int main ( void ) {
    DISABLE_JTAG; /* �ر�JTAG���� */
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