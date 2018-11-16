#include <STC12C5A.H>
#include "spi.h"
#include "rf4432.h"

#define uchar unsigned char
#define uint  unsigned int

sbit LED = P1 ^ 4;

char RF4432_RxBuf[RF4432_RxBuf_Len] = {0};
char RF4432_TxBuf[RF4432_TxBuf_Len] = {1};

void Delay500ms() {
    unsigned char i, j, k;
    i = 22;
    j = 3;
    k = 227;

    do {
        do {
            while ( --k );
        } while ( --j );
    } while ( --i );
}

#define SEND_MESSAGE 1

#if SEND_MESSAGE

void main ( void ) {
    SPI_Init();
    RF4432_Init();
    RF4432_SetRxMode();
    RF4432_TxBuf[0] = 1;

    while ( 1 ) {
        RF4432_TxPacket();
        LED = !LED;
        Delay500ms();
    }
}

#else

void main ( void ) {
    SPI_Init();
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
        Delay500ms();
    }
}

#endif