#include "iocc2530.h"

#define uint unsigned int
#define uchar unsigned char

#define Ds18b20Data P0_6 /* �¶ȴ��������� */

#define ON  0x01 /* ��ȡ�ɹ�����0x00��ʧ�ܷ���0x01 */
#define OFF 0x00

unsigned char temp; /* �����¶���Ϣ */

void Ds18b20Delay ( uint k ) { /* ʱ��Ƶ��Ϊ32M */
    uint i, j;

    for ( i = 0; i < k; i++ )
        for ( j = 0; j < 2; j++ );
}

void Ds18b20InputInitial ( void ) { /* ���ö˿�Ϊ���� */
    P0DIR &= 0xbf;
}

void Ds18b20OutputInitial ( void ) { /* ���ö˿�Ϊ��� */
    P0DIR |= 0x40;
}

uchar Ds18b20Initial ( void ) { /* ds18b20��ʼ������ʼ���ɹ�����0x00��ʧ�ܷ���0x01 */
    uchar Status = 0x00;
    uint CONT_1 = 0;
    uchar Flag_1 = ON;
    Ds18b20OutputInitial();
    Ds18b20Data = 1;
    Ds18b20Delay ( 260 );
    Ds18b20Data = 0;
    Ds18b20Delay ( 750 );
    Ds18b20Data = 1;
    Ds18b20InputInitial();

    while ( ( Ds18b20Data != 0 ) && ( Flag_1 == ON ) ) { /* �ȴ�ds18b20��Ӧ�����з�ֹ��ʱ���� */
        /* �ȴ�Լ60ms���� */
        CONT_1++;
        Ds18b20Delay ( 10 );

        if ( CONT_1 > 8000 ) {
            Flag_1 = OFF;
        }

        Status = Ds18b20Data;
    }

    Ds18b20OutputInitial();
    Ds18b20Data = 1;
    Ds18b20Delay ( 100 );
    return Status;
}

void Ds18b20Write ( uchar infor ) {
    uint i;
    Ds18b20OutputInitial();

    for ( i = 0; i < 8; i++ ) {
        if ( ( infor & 0x01 ) ) {
            Ds18b20Data = 0;
            Ds18b20Delay ( 6 );
            Ds18b20Data = 1;
            Ds18b20Delay ( 50 );
        } else {
            Ds18b20Data = 0;
            Ds18b20Delay ( 50 );
            Ds18b20Data = 1;
            Ds18b20Delay ( 6 );
        }

        infor >>= 1;
    }
}

uchar Ds18b20Read ( void ) {
    uchar Value = 0x00;
    uint i;
    Ds18b20OutputInitial();
    Ds18b20Data = 1;
    Ds18b20Delay ( 10 );

    for ( i = 0; i < 8; i++ ) {
        Value >>= 1;
        Ds18b20OutputInitial();
        Ds18b20Data = 0;
        Ds18b20Delay ( 3 );
        Ds18b20Data = 1;
        Ds18b20Delay ( 3 );
        Ds18b20InputInitial();

        if ( Ds18b20Data == 1 ) {
            Value |= 0x80;
        }

        Ds18b20Delay ( 15 );
    }

    return Value;
}

void Temp_test ( void ) { /* �¶ȶ�ȡ���� */
    uchar V1, V2;
    Ds18b20Initial();
    Ds18b20Write ( 0xcc );
    Ds18b20Write ( 0x44 );
    Ds18b20Initial();
    Ds18b20Write ( 0xcc );
    Ds18b20Write ( 0xbe );
    V1 = Ds18b20Read();
    V2 = Ds18b20Read();
    temp = ( ( V1 >> 4 ) + ( ( V2 & 0x07 ) * 16 ) );
}