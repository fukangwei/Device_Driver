#include "delay.h"
#include "sys.h"
#include "usart.h"
#include "string.h"
#include "rc522.h"

unsigned char RevBuffer[30] = {0}; /* ���ջ����� */
unsigned char DefaultKey[6] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06}; /* ���� */
unsigned char MLastSelectedSnr[4] = {0};

int main ( void ) {
    SystemInit();
    delay_init ( 72 );
    NVIC_Configuration();
    uart_init ( 9600 );
    rc522_init();
    PcdReset();
    PcdAntennaOff();
    PcdAntennaOn();
    M500PcdConfigISOType ( 'A' );

    while ( 1 ) {
        if ( PcdRequest ( PICC_REQALL, &RevBuffer[2] ) == MI_OK ) { /* Ѱ�� */
            if ( PcdAnticoll ( MLastSelectedSnr ) == MI_OK ) { /* ����ͻ */
                if ( PcdSelect ( MLastSelectedSnr ) == MI_OK ) { /* ѡ�񿨺� */
                    printf ( "I get a card! " );

                    if ( PcdAuthState ( PICC_AUTHENT1A, 63, DefaultKey, MLastSelectedSnr ) == MI_OK ) { /* У�鿨���� */
                        printf ( "ok\r\n" );
                        memset ( RevBuffer, 0, 16 );
                    }
                }
            }
        }

        delay_ms ( 50 );
    }
}