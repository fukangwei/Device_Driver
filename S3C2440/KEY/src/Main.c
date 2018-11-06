#define GLOBAL_CLK 1
#include <stdlib.h>
#include <string.h>
#include "def.h"
#include "option.h"
#include "2440addr.h"
#include "2440lib.h"
#include "2440slib.h"
#include "mmu.h"
#include "profile.h"
#include "memtest.h"

int  Wait_key ( void );
void key_init ( void );
void Led1_run ( void );
void Led1_init ( void );

void delay ( int times ) {
    int i, j;

    for ( i = 0; i < times; i++ )
        for ( j = 0; j < 400; j++ );
}

void Main ( void ) {
    Led1_init();
    key_init();

    while ( 1 ) {
        if ( Wait_key() ) {
            Led1_run();
        }
    }
}

void key_init ( void ) {
    rGPGCON &= ~ ( 3 << 0 );
}

void Led1_init ( void ) {
    rGPBCON &= ~ ( 3 << 10 );
    rGPBCON |= ( 1 << 10 );
}

void Led1_run ( void ) {
    rGPBDAT &= ~ ( 1 << 5 );
    delay ( 1000 );
    rGPBDAT |= ( 1 << 5 );
    delay ( 1000 );
}

int Wait_key ( void ) {
    if ( ! ( rGPGDAT & 0x1 ) ) {
        delay ( 200 );
    }

    if ( ! ( rGPGDAT & 0x1 ) ) {
        while ( ! ( rGPGDAT & 0x1 ) );

        return 1;
    } else {
        return 0;
    }
}