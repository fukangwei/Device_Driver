#include "STC12C5A.H"
#include "SI4432_spi.h"
#include "string.h"

sbit LED1 = P1 ^ 4;

static void delay_ms ( uint z ) {
    uint x, y;

    for ( x = z; x > 0; x-- )
        for ( y = 110; y > 0; y-- );
}

#define SEND_MESSAGE 0

#if SEND_MESSAGE

int main ( void ) {
    RF4432_Init();
    RF4432_SetRxMode();
    RF4432_TxBuf[0] = 1;

    while ( 1 ) {
        RF4432_TxPacket();
        LED1 = !LED1;
        delay_ms ( 500 );
    }
}

#else

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

void main ( void ) {
    RF4432_Init();
    RF4432_SetRxMode();

    while ( 1 ) {
        if ( RF4432_RxPacket() ) {
            RF4432_ReadBurestReg ( FIFO_ACCESS, RF4432_RxBuf, RF4432_RxBuf_Len );

            if ( RF4432_RxBuf[0] == 1 ) {
                LED1 = ~LED1;
            }
        }

        RF4432_SetRxMode();
        memset ( RF4432_RxBuf, 0, sizeof ( RF4432_RxBuf ) );
        Delay500ms();
    }
}

#endif