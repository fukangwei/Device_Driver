#include "led.h"
#include "delay.h"
#include "sys.h"
#include "usart.h"
#include "24l01.h"

#define SEND_MESSAGE 0

#if SEND_MESSAGE

int main ( void ) {
    u8 Tx_Buf[32];
    SystemInit();
    delay_init ( 72 );
    NVIC_Configuration();
    uart_init ( 9600 );
    LED_Init();
    NRF24L01_Init();

    while ( NRF24L01_Check() ) { /* 检测不到24L01 */
        LED0 = !LED0;
        delay_ms ( 500 );
    }

    TX_Mode();
    Tx_Buf[1] = 1;

    while ( 1 ) {
        NRF24L01_TxPacket ( Tx_Buf ); /* 发送数据 */
        LED0 = ~LED0;
        delay_ms ( 1000 );
    }
}

#else

int main ( void ) {
    u8 Rx_Buf[32];
    SystemInit();
    delay_init ( 72 );
    NVIC_Configuration();
    uart_init ( 9600 );
    LED_Init();
    NRF24L01_Init();

    while ( NRF24L01_Check() ) {
        LED1 = !LED1;
        delay_ms ( 500 );
    }

    RX_Mode();

    while ( 1 ) {
        NRF24L01_RxPacket ( Rx_Buf );

        if ( Rx_Buf[1] == 1 ) {
            LED0 = !LED0;
            delay_ms ( 300 );
        }

        Rx_Buf[1] = 0;
        delay_ms ( 500 );
    }
}

#endif