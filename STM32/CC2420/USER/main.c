#include "led.h"
#include "delay.h"
#include "sys.h"
#include "cc2420.h"

extern uint8  CC2420_PSDU[];

#define SEND_MESSAGE 0

int main ( void ) {
    delay_init();
    LED_Init();
    CC2420_IOinit();
    VREG_EN = 1;
    delay_ms ( 1000 );
    CC2420_Init();
    CC2420_SetRxMode();
    CC2420_PSDU[0] = 23 + 48 + 2;

    while ( 1 ) {
#if SEND_MESSAGE
        CC2420_PSDU[25] = 100;
        CC2420_WriteTXFIFO();
        CC2420_TxPacket();
        CC2420_SetRxMode();
        LED = !LED;
        delay_ms ( 500 );
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