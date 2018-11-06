#include "STC12C5A.H"
#include"24L01.h"

sbit LED1 = P1 ^ 4;

void delay_ms ( uint z ) {
    uint x, y;

    for ( x = z; x > 0; x-- )
        for ( y = 110; y > 0; y-- );
}

#define SEND_MESSAGE 1

#if SEND_MESSAGE

int main ( void ) {
    uchar Tx_Buf1[32]; /* ���ݷ����ݴ��� */
    init_NRF24L01();
    TX_Mode(); /* ����ģʽֻ������һ�� */
    Tx_Buf1[1] = 1; /* װ�����ݣ�ע�����ݷ�����֮�󲻻ᱻ��� */

    while ( 1 ) {
        nRF24L01_TxPacket ( Tx_Buf1 ); /* ������������ */
        LED1 = !LED1;
        delay_ms ( 2000 );
    }
}

#else

int main ( void ) {
    uchar Rx_Buf[32]; /* ���ݽ����ݴ��� */
    init_NRF24L01();
    RX_Mode(); /* ����ģʽ�����ö�� */

    while ( 1 ) {
        RX_Mode();
        nRF24L01_RxPacket ( Rx_Buf );

        if ( Rx_Buf[1] == 1 ) {
            LED1 = ~LED1;
        }

        Rx_Buf[1] = 0x00;
        delay_ms ( 1000 );
    }
}

#endif