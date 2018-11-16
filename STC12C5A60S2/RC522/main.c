#include <STC12C5A.H>
#include "string.h"
#include "uart.h"
#include "mfrc522.h"

unsigned char idata RevBuffer[30] = {0}; /* ���ջ����� */
unsigned char code DefaultKey[6] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06}; /* ���� */
unsigned char idata MLastSelectedSnr[4] = {0};
unsigned char data_in[16] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0xFF, 0x07, 0x80, 0x69, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06};

void main() {
    UART_Init();
    PcdReset();
    PcdAntennaOff();
    PcdAntennaOn();
    M500PcdConfigISOType ( 'A' );

    while ( 1 ) {
        if ( PcdRequest ( PICC_REQALL, &RevBuffer[2] ) == MI_OK ) { /* Ѱ�� */
            if ( PcdAnticoll ( MLastSelectedSnr ) == MI_OK ) { /* ����ͻ */
                if ( PcdSelect ( MLastSelectedSnr ) == MI_OK ) { /* ѡ�񿨺� */
                    send_str ( "I get a card! " );

                    if ( PcdAuthState ( PICC_AUTHENT1A, 63, DefaultKey, MLastSelectedSnr ) == MI_OK ) { /* У�鿨���� */
                        send_str ( "ok\r\n" );
                        memset ( RevBuffer, 0, 16 );
                    }
                }
            }
        }

        delay_10ms ( 50 );
    }
}