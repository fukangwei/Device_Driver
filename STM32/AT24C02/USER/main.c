#include "led.h"
#include "delay.h"
#include "sys.h"
#include "usart.h"
#include "myiic.h"
#include "24cxx.h"
#include "string.h"

const u8 TEXT_Buffer[] = {"MiniSTM32 IIC TEST"};

u8 SHOW_Buffer[50] = {0};

#define SIZE sizeof(TEXT_Buffer)

int main ( void ) {
    SystemInit();
    delay_init ( 72 );
    NVIC_Configuration();
    uart_init ( 9600 );
    LED_Init();
    AT24CXX_Init();

    while ( AT24CXX_Check() ) { /* ¼ì²â²»µ½24c02 */
        delay_ms ( 500 );
        LED0 = !LED0;
    }

    while ( 1 ) {
        memset ( SHOW_Buffer, 0, sizeof ( SHOW_Buffer ) );
        AT24CXX_Write ( 0, ( u8 * ) TEXT_Buffer, SIZE );
        printf ( "24C02 Write Finished!\r\n" );
        delay_ms ( 500 );
        AT24CXX_Read ( 0, SHOW_Buffer, SIZE );
        printf ( "24C02 Read %s\r\n", SHOW_Buffer );
        delay_ms ( 500 );
    }
}