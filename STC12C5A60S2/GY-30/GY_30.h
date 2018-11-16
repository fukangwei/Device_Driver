#ifndef _GY_30_H_
#define _GY_30_H_

#define uchar unsigned char
#define uint unsigned int

typedef unsigned char BYTE;
typedef unsigned short WORD;

/* ����������IIC�����еĴӵ�ַ�����ݡ�ALT ADDRESS�����Ž����޸ģ��ӵ�ʱΪ0x46���ӵ�ԴʱΪ0xB8 */
#define SlaveAddress 0x46

sbit SCL = P0 ^ 0; /* IICʱ�����Ŷ��� */
sbit SDA = P0 ^ 1; /* IIC�������Ŷ���  */

void BH1750_Start ( void );
void BH1750_Stop ( void );
void BH1750_SendACK ( bit ack );
bit BH1750_RecvACK ( void );
void BH1750_SendByte ( BYTE dat );
BYTE BH1750_RecvByte ( void );
void Single_Write_BH1750 ( uchar REG_Address );
void Init_BH1750 ( void );
void Multiple_read_BH1750 ( void );

#endif