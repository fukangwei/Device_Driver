#include "STC12C5A.H"
#include "type.h"

sbit DS = P0 ^ 2; /* ����DS18B20�����߽ӿ� */

static uint temp; /* �洢�¶�ֵ */

static void delay1us ( void ) { /* ��ʱ1΢��(�Ͼ�ȷ��Լ1.2΢��) */
}

static void dsreset ( void ) { /* DS18B20��λ(��ʼ��)���� */
    uint i;
    DS = 0;
    i = 161;

    while ( i-- > 0 ) {
        delay1us(); /* 480΢��ĸ�λ���� */
    }

    DS = 1;
    i = 140;

    while ( i-- > 0 ) {
        delay1us(); /* �ȴ�DS18B20�Ĵ�������(480΢��) */
    }
}

static bit tmpreadbit ( void ) { /* ��DS18B20һλ���� */
    uint i;
    bit dat;
    DS = 0; /* �½��ص��¶�ʱ��ʼ */
    delay1us();
    DS = 1; /* �����߱������ٱ���1΢�룬Ȼ�����߱��ͷ� */
    i = 10;

    while ( i-- > 0 ) {
        delay1us();
    }

    dat = DS; /* �ѿ���������ʱ�����15΢�����ڵ���� */
    i = 40;

    while ( i-- > 0 ) {
        delay1us(); /* ��ʱ���������Ϊ60΢�� */
    }

    return ( dat );
}

static uchar tmpread ( void ) { /* ��DS18B20��һ���ֽ� */
    uchar i, j, dat;
    dat = 0;

    for ( i = 1; i <= 8; i++ ) {
        j = tmpreadbit();
        dat = ( j << 7 ) | ( dat >> 1 );
    }

    return ( dat );
}

static void tmpwritebyte ( uchar dat ) { /* ��DS18B20дһ���ֽ� */
    uint i;
    uchar j;
    bit testb;

    for ( j = 1; j <= 8; j++ ) {
        testb = dat & 0x01;
        dat = dat >> 1;

        if ( testb ) { /* ���д���λΪ1 */
            DS = 0;
            i = 1; /* ����1΢��Ļָ�ʱ�� */

            while ( i-- > 0 ) {
                delay1us();
            }

            DS = 1;
            i = 50;

            while ( i-- > 0 ) {
                delay1us(); /* дʱ��������ٳ���60΢�� */
            }
        } else { /* ���д���λΪ0 */
            DS = 0;
            i = 50;

            while ( i-- > 0 ) {
                delay1us();
            }

            DS = 1;
            i = 1;

            while ( i-- > 0 ) {
                delay1us();
            }
        }
    }
}

void tmpchange ( void ) { /* DS18B20�¶ȸı� */
    dsreset();
    tmpwritebyte ( 0xcc ); /* ����ROMָ�� */
    tmpwritebyte ( 0x44 ); /* �¶�ת��ָ�� */
}

uint tmp ( void ) { /* �õ�DS18B20�¶�ֵ */
    float tt;
    uchar a, b;
    dsreset();
    tmpwritebyte ( 0xcc );
    tmpwritebyte ( 0xbe );
    a = tmpread();
    b = tmpread();
    temp = b;
    temp <<= 8; /* �����ֽ������������ֵ */
    temp = temp | a;
    tt = temp * 0.0625;
    temp = tt * 10 + 0.5; /* ��0.5����һ���¶� */
    return temp;
}