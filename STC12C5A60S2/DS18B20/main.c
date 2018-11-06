#include "STC12C5A.H"
#include "DS18B20.h"
#include "type.h"

uchar Buf[7];

void delay ( uint count ) {
    uint i;

    while ( count ) {
        i = 200;

        while ( i > 0 ) {
            i--;
        }

        count--;
    }
}

void UART_init ( void ) {
    SCON = 0x50;
    TMOD = 0x20;
    TH1 = 0xFD;
    TL1 = 0xFD;
    TR1 = 1;
}

void UART_send_byte ( uchar dat ) {
    SBUF = dat;

    while ( TI == 0 );

    TI = 0;
}

void UART_send_string ( uchar *buf ) {
    while ( *buf != '\0' ) {
        UART_send_byte ( *buf++ );
    }
}

void main ( void ) {
    uint wendu = 0;
    UART_init();

    while ( 1 ) {
        tmpchange();
        wendu = tmp();
        Buf[0] = wendu / 100 + '0';
        Buf[1] = ( wendu / 10 ) % 10 + '0';
        Buf[2] = '.';
        Buf[3] = wendu % 10 + '0';
        Buf[4] = '\r';
        Buf[5] = '\n';
        Buf[6] = '\0';
        UART_send_string ( Buf );
        delay ( 1000 );
    }
}