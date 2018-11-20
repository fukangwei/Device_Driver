#include "sys.h"
#include "GY_30.h"
#include "delay.h"
#include "myiic.h"

BYTE BUF[8]; /* �������ݻ����� */

void BH1750_Start ( void ) { /* ���մ�������ʼ�ź� */
    IIC_Start();
}

void BH1750_Stop ( void ) { /* ���մ�����ֹͣ�ź� */
    IIC_Stop();
}

void BH1750_SendACK ( BYTE ack ) { /* ����Ӧ���źš�����ackΪ0��ACK��Ϊ1��NAK */
    if ( ack == 0 ) {
        IIC_Ack();
    } else {
        IIC_NAck();
    }
}

BYTE BH1750_RecvACK ( void ) { /* ����Ӧ���ź� */
    return IIC_Wait_Ack();
}

void BH1750_SendByte ( BYTE dat ) { /* ��IIC���߷���һ���ֽ����� */
    IIC_Send_Byte ( dat );
    BH1750_RecvACK();
}

BYTE BH1750_RecvByte ( void ) { /* ��IIC���߽���һ���ֽ����� */
    return IIC_Read_Byte ( 0 );
}

void Single_Write_BH1750 ( uchar REG_Address ) {
    BH1750_Start(); /* ��ʼ�ź� */
    BH1750_SendByte ( SlaveAddress ); /* �����豸��ַ + д�ź� */
    BH1750_SendByte ( REG_Address ); /* �ڲ��Ĵ�����ַ */
    BH1750_Stop(); /* ����ֹͣ�ź� */
}

void Init_BH1750 ( void ) { /* BH1750��ʼ������ */
    IIC_Init();
    delay_ms ( 50 );
    Single_Write_BH1750 ( 0x01 );
}

void Multiple_read_BH1750 ( void ) { /* ��������BH1750�ڲ����� */
    uchar i;
    BH1750_Start(); /* ��ʼ�ź� */
    BH1750_SendByte ( SlaveAddress + 1 ); /* �����豸��ַ + ���ź� */

    for ( i = 0; i < 3; i++ ) { /* ������ȡ2����ַ���ݣ��洢��BUF�� */
        BUF[i] = BH1750_RecvByte(); /* BUF[0]�洢0x32��ַ�е����� */

        if ( i == 3 ) {
            BH1750_SendACK ( 1 ); /* ���һ��������Ҫ��ӦNOACK */
        } else {
            BH1750_SendACK ( 0 ); /* ��ӦACK */
        }
    }

    BH1750_Stop(); /* ֹͣ�ź� */
    delay_ms ( 5 );
}