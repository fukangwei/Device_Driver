#include "spi.h"

void delay_10us ( char n ) {
    int i;

    while ( n-- )
        for ( i = 0; i < 5; i++ );
}

void delay_ms ( int num ) {
    int x, y;

    for ( y = 0; y < num; y++ ) {
        for ( x = 0; x < 500; x ) {
            x++;
        }
    }
}

void SPI_Init ( void ) {
    SPCTL = 0XD0;
    SPSTAT = 0XC0;
    AUXR1 = 0x00;
}

unsigned char  SPI_Write ( unsigned char txdata ) { /* SPI单字节写入函数 */
    unsigned char rxdata;
    rxdata = 0x00;
    SPDAT = txdata;

    while ( ! ( SPSTAT & 0x80 ) );

    rxdata = SPDAT;
    SPSTAT = 0XC0;
    delay_10us ( 2 );
    return rxdata;
}