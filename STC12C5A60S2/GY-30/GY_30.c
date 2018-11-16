#include "STC12C5A.H"
#include "intrins.h"
#include "GY_30.h"

BYTE BUF[8]; /* �������ݻ����� */

static void delay1ms ( void ) { /* ���Ϊ��-0.018084490741us�� */
    unsigned char a, b;

    for ( b = 21; b > 0; b-- )
        for ( a = 130; a > 0; a-- );

    _nop_(); /* if Keil, require use intrins.h */
}

static void delay_nms ( unsigned int n ) {
    while ( n-- ) {
        delay1ms();
    }
}

static void Delay5us ( void ) { /* ���Ϊ��-0.026765046296us�� */
    unsigned char a;

    for ( a = 12; a > 0; a-- );

    _nop_(); /* if Keil,require use intrins.h */
}

void BH1750_Start ( void ) { /* ���մ�������ʼ�ź� */
    SDA = 1; /* ���������� */
    SCL = 1; /* ����ʱ���� */
    Delay5us();
    SDA = 0; /* �����½��� */
    Delay5us();
    SCL = 0; /* ����ʱ���� */
}

void BH1750_Stop ( void ) { /* ���մ�����ֹͣ�ź� */
    SDA = 0; /* ���������� */
    SCL = 1; /* ����ʱ���� */
    Delay5us();
    SDA = 1; /* ���������� */
    Delay5us();
}

void BH1750_SendACK ( bit ack ) { /* ����Ӧ���źš�����ackΪ0��ACK��Ϊ1��NAK */
    SDA = ack; /* дӦ���ź� */
    SCL = 1; /* ����ʱ���� */
    Delay5us();
    SCL = 0; /* ����ʱ���� */
    Delay5us();
}

bit BH1750_RecvACK ( void ) { /* ����Ӧ���ź� */
    SCL = 1; /* ����ʱ���� */
    Delay5us();
    CY = SDA; /* ��Ӧ���ź� */
    SCL = 0; /* ����ʱ���� */
    Delay5us();
    return CY;
}

void BH1750_SendByte ( BYTE dat ) { /* ��IIC���߷���һ���ֽ����� */
    BYTE i;

    for ( i = 0; i < 8; i++ ) {
        dat <<= 1;
        SDA = CY;
        SCL = 1;
        Delay5us();
        SCL = 0;
        Delay5us();
    }

    BH1750_RecvACK();
}

BYTE BH1750_RecvByte ( void ) { /* ��IIC���߽���һ���ֽ����� */
    BYTE i;
    BYTE dat = 0;
    SDA = 1; /* ʹ���ڲ�������׼����ȡ���� */

    for ( i = 0; i < 8; i++ ) {
        dat <<= 1;
        SCL = 1;
        Delay5us();
        dat |= SDA;
        SCL = 0;
        Delay5us();
    }

    return dat;
}

void Single_Write_BH1750 ( uchar REG_Address ) {
    BH1750_Start(); /* ��ʼ�ź� */
    BH1750_SendByte ( SlaveAddress ); /* �����豸��ַ + д�ź� */
    BH1750_SendByte ( REG_Address ); /* �ڲ��Ĵ�����ַ */
    BH1750_Stop(); /* ����ֹͣ�ź� */
}

void Init_BH1750 ( void ) { /* BH1750��ʼ������ */
    Single_Write_BH1750 ( 0x01 );
}

void Multiple_read_BH1750 ( void ) { /* ��������BH1750�ڲ����� */
    uchar i;
    BH1750_Start(); /* ��ʼ�ź� */
    BH1750_SendByte ( SlaveAddress + 1 ); /* �����豸��ַ + ���ź� */

    for ( i = 0; i < 3; i++ ) { /* ������ȡ2����ַ���ݣ��洢��BUF�� */
        BUF[i] = BH1750_RecvByte(); /* BUF[0]�洢0x32��ַ�е����� */

        if ( i == 3 ) {
            BH1750_SendACK ( 1 ); /* ���һ��������Ҫ��NOACK */
        } else {
            BH1750_SendACK ( 0 ); /* ��ӦACK */
        }
    }

    BH1750_Stop(); /* ֹͣ�ź� */
    delay_nms ( 5 );
}