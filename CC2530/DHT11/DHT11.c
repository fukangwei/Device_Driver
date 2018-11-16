#include <ioCC2530.h>

#define uint unsigned int
#define uchar unsigned char

#define wenshi P0_6

uchar ucharFLAG, uchartemp;
uchar shidu_shi, shidu_ge, wendu_shi, wendu_ge = 4;
uchar ucharT_data_H, ucharT_data_L, ucharRH_data_H, ucharRH_data_L, ucharcheckdata;
uchar ucharT_data_H_temp, ucharT_data_L_temp, ucharRH_data_H_temp, ucharRH_data_L_temp, ucharcheckdata_temp;
uchar ucharcomdata;

uchar temp[2] = {0, 0};
uchar temp1[5] = "temp=";
uchar humidity[2] = {0, 0};
uchar humidity1[9] = "humidity=";

void Delay_us() { /* 1us��ʱ */
    asm ( "nop" );
    asm ( "nop" );
    asm ( "nop" );
    asm ( "nop" );
    asm ( "nop" );
    asm ( "nop" );
    asm ( "nop" );
    asm ( "nop" );
    asm ( "nop" );
}

void Delay_10us() { /* 10us��ʱ */
    Delay_us();
    Delay_us();
    Delay_us();
    Delay_us();
    Delay_us();
    Delay_us();
    Delay_us();
    Delay_us();
    Delay_us();
    Delay_us();
}

void Delay_ms ( uint Time ) { /* ��ʱn���� */
    unsigned char i;

    while ( Time-- ) {
        for ( i = 0; i < 100; i++ ) {
            Delay_10us();
        }
    }
}

void COM ( void ) {
    uchar i;

    for ( i = 0; i < 8; i++ ) {
        ucharFLAG = 2;

        while ( ( !wenshi ) && ucharFLAG++ );

        Delay_10us();
        Delay_10us();
        Delay_10us();
        uchartemp = 0;

        if ( wenshi ) {
            uchartemp = 1;
        }

        ucharFLAG = 2;

        while ( ( wenshi ) && ucharFLAG++ );

        if ( ucharFLAG == 1 ) {
            break;
        }

        ucharcomdata <<= 1;
        ucharcomdata |= uchartemp;
    }
}

void DHT11 ( void ) { /* ��ʪ���������� */
    wenshi = 0;
    Delay_ms ( 19 ); /* > 18ms */
    wenshi = 1;
    P0DIR &= ~0x40; /* ��������IO�ڷ��� */
    Delay_10us();
    Delay_10us();
    Delay_10us();
    Delay_10us();

    if ( !wenshi ) {
        ucharFLAG = 2;

        while ( ( !wenshi ) && ucharFLAG++ );

        ucharFLAG = 2;

        while ( ( wenshi ) && ucharFLAG++ );

        COM();
        ucharRH_data_H_temp = ucharcomdata;
        COM();
        ucharRH_data_L_temp = ucharcomdata;
        COM();
        ucharT_data_H_temp = ucharcomdata;
        COM();
        ucharT_data_L_temp = ucharcomdata;
        COM();
        ucharcheckdata_temp = ucharcomdata;
        wenshi = 1;
        uchartemp = ( ucharT_data_H_temp + ucharT_data_L_temp + ucharRH_data_H_temp + ucharRH_data_L_temp );

        if ( uchartemp == ucharcheckdata_temp ) {
            ucharRH_data_H = ucharRH_data_H_temp;
            ucharRH_data_L = ucharRH_data_L_temp;
            ucharT_data_H = ucharT_data_H_temp;
            ucharT_data_L = ucharT_data_L_temp;
            ucharcheckdata = ucharcheckdata_temp;
        }

        wendu_shi = ucharT_data_H / 10;
        wendu_ge = ucharT_data_H % 10;
        shidu_shi = ucharRH_data_H / 10;
        shidu_ge = ucharRH_data_H % 10;
    } else { /* û�гɹ���ȡ������0 */
        wendu_shi = 0;
        wendu_ge = 0;
        shidu_shi = 0;
        shidu_ge = 0;
    }
}