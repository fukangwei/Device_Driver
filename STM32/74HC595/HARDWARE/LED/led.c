#include "led.h"
#include "delay.h"

void LED_Init ( void ) {
    GPIO_InitTypeDef GPIO_InitStructure;
    RCC_APB2PeriphClockCmd ( RCC_APB2Periph_GPIOA, ENABLE );
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init ( GPIOA, &GPIO_InitStructure );
    GPIO_ResetBits ( GPIOA, GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5 );
}

void SendByte ( unsigned char dat ) {
    unsigned char i;

    for ( i = 0; i < 8; i++ ) {
        SRCLK = 0;

        if ( dat & 0x80 ) {
            SER = 1;
        } else {
            SER = 0;
        }

        dat <<= 1;
        SRCLK = 1;
    }
}

void Send2Byte ( unsigned char dat1, unsigned char dat2 ) {
    SendByte ( dat1 );
    SendByte ( dat2 );
}

void Out595 ( void ) {
    unsigned int i;
    LATCH = 0;

    for ( i = 0; i < 10; i++ );

    LATCH = 1;
    LATCH = 0;
}

void SendSeg ( unsigned char dat ) {
    unsigned char i;

    for ( i = 0; i < 8; i++ ) {
        SRCLK_B = 0;

        if ( dat & 0x80 ) {
            SER_B = 1;
        } else {
            SER_B = 0;
        }

        dat <<= 1;
        SRCLK_B = 1;
    }

    for ( i = 0; i < 10; i++ );

    LATCH_B = 1;
    LATCH_B = 0;
}