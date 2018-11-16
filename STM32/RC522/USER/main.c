#include "delay.h"
#include "sys.h"
#include "usart.h"
#include "string.h"
#include "rc522.h"

unsigned char RevBuffer[30] = {0}; /* Ω” ’ª∫≥Â«¯ */
unsigned char DefaultKey[6] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06}; /* √‹¬Î */
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
        if ( PcdRequest ( PICC_REQALL, &RevBuffer[2] ) == MI_OK ) { /* —∞ø® */
            if ( PcdAnticoll ( MLastSelectedSnr ) == MI_OK ) { /* ∑¿≥ÂÕª */
                if ( PcdSelect ( MLastSelectedSnr ) == MI_OK ) { /* —°‘Òø®∫≈ */
                    printf ( "I get a card! " );

                    if ( PcdAuthState ( PICC_AUTHENT1A, 63, DefaultKey, MLastSelectedSnr ) == MI_OK ) { /* –£—Èø®√‹¬Î */
                        printf ( "ok\r\n" );
                        memset ( RevBuffer, 0, 16 );
                    }
                }
            }
        }

        delay_ms ( 50 );
    }
}