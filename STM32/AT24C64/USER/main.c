#include "delay.h"
#include "sys.h"
#include "usart.h"
#include "led.h"
#include "string.h"
#include "24cxx.h"

const u8 TEXT_Buffer[] = {"MiniSTM32 IIC TEST"};
#define SIZE sizeof(TEXT_Buffer)

u8 datatemp[SIZE] = {0};

int main ( void ) {
    SystemInit();
    delay_init ( 72 );
    NVIC_Configuration();
    uart_init ( 9600 );
    LED_Init();
    AT24CXX_Init();

    while ( AT24CXX_Check() ) { /* 检测不到24c02 */
        printf ( "I have some problems!\r\n" );
        delay_ms ( 500 );
    }

    AT24CXX_Write ( 0, ( u8 * ) TEXT_Buffer, SIZE ); /* 写数据 */
    delay_ms ( 500 );
    AT24CXX_Read ( 0, datatemp, SIZE ); /* 读数据 */
    printf ( "The date is %s\r\n", datatemp );

    while ( 1 ) {
        LED = !LED;
        delay_ms ( 500 );
    }
}