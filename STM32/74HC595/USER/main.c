#include "led.h"
#include "delay.h"
#include "sys.h"

unsigned char segout[8] = {0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80};
unsigned char tab[] = {0x00, 0x6C, 0x92, 0x82, 0x44, 0x28, 0x10, 0x00};

int main ( void ) {
    unsigned char i;
    delay_init();
    LED_Init();

    while ( 1 ) {
        for ( i = 0; i < 8; i++ ) {
            SendSeg ( segout[i] );
            Send2Byte ( ~tab[i], 0xff );
            Out595();
            delay_ms ( 1 );
            Send2Byte ( 0xff, 0xff );
            Out595();
        }
    }
}