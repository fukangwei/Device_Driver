#include "led.h"
#include "delay.h"
#include "sys.h"
#include "usart.h"
#include "string.h"
#include "rf4432.h"

unsigned char RF4432_RxBuf[RF4432_RxBuf_Len] = {0};
char RF4432_TxBuf[RF4432_TxBuf_Len] = {1};

#define SEND_MESSAGE 0

#if SEND_MESSAGE

int main ( void ) {
    delay_init();
    NVIC_Configuration();
    uart_init ( 9600 );
    LED_Init();
    RF4432_Init();
    RF4432_SetRxMode();
    RF4432_TxBuf[0] = 1;

    while ( 1 ) {
        RF4432_TxPacket();
        LED = !LED;
        delay_ms ( 500 );
    }
}

#else

int main ( void ) {
    delay_init();
    NVIC_Configuration();
    uart_init ( 9600 );
    LED_Init();
    RF4432_Init();
    RF4432_SetRxMode();

    while ( 1 ) {
        if ( RF4432_RxPacket() ) {
            RF4432_ReadBurestReg ( FIFO_ACCESS, RF4432_RxBuf, RF4432_RxBuf_Len );

            if ( RF4432_RxBuf[0] == 1 ) {
                LED = ~LED;
            }
        }

        RF4432_SetRxMode();
        memset ( RF4432_RxBuf, 0, sizeof ( RF4432_RxBuf ) );
        delay_ms ( 500 );
    }
}

#endif