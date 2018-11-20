#include "intrins.h"
#include "CC2420.h"
#include "public.h"

extern uint8 pdata CC2420_PSDU[];

sbit LED = P1 ^ 4;

#define SEND_MESSAGE 1

void IO_Init ( void ) {
    FIFO  = 1;
    MISO  = 1;
    CCA   = 1;
    SFD   = 1;
    FIFOP = 1;
}

void delay_ms ( uint16 n ) {
    while ( n-- ) {
        unsigned char a, b;

        for ( b = 21; b > 0; b-- )
            for ( a = 130; a > 0; a-- );

        _nop_();
    }
}

int main ( void ) {
    VREG_EN = 1;
    delay_ms ( 1000 );
    IO_Init();
    SPI_Init();
    CC2420_Init();
    CC2420_SetRxMode();
    CC2420_PSDU[0] = 23 + 48 + 2;

    while ( 1 ) {
#if SEND_MESSAGE
        delay_ms ( 500 );
        CC2420_PSDU[25] = 100;
        CC2420_WriteTXFIFO();
        CC2420_TxPacket();
        CC2420_SetRxMode();
        LED = !LED;
#else

        if ( CC2420_RxPacket() ) {
            CC2420_ReadRXFIFO();

            if ( CC2420_PSDU[25] == 100 ) {
                LED = !LED;
                CC2420_PSDU[25] = 0;
            }
        }

#endif
    }
}