#include "led.h"
#include "delay.h"
#include "sys.h"
#include "usart.h"
#include "WS2811.h"

extern unsigned long WsDat[];

int main ( void ) {
    SystemInit();
    delay_init ( 72 );
    NVIC_Configuration();
    uart_init ( 9600 );
    WS_Init();

    while ( 1 ) {
        WsDat[0] = Blue;
        WS_SetAll();
        delay_ms ( 100 );
        WsDat[0] = Red;
        WS_SetAll();
        delay_ms ( 100 );
        WsDat[0] = Green;
        WS_SetAll();
        delay_ms ( 100 );
    }
}