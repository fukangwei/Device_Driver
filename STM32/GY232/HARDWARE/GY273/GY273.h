#ifndef GY273_H
#define GY273_H
#include "delay.h"
#include "sys.h"

#define uchar unsigned char
#define uint  unsigned int

#define SlaveAddress 0x3C /* 定义器件5883在IIC总线中的从地址 */
typedef unsigned char BYTE;
typedef unsigned short WORD;

void Init_HMC5883 ( void );
void conversion ( uint temp_data );
void  Single_Write_HMC5883 ( uchar REG_Address, uchar REG_data );
void  Multi_read_HMC5883 ( void );
void HMC5883_Start ( void );
void HMC5883_Stop ( void );
void HMC5883_SendACK ( uchar ack );
uchar  HMC5883_RecvACK ( void );
void HMC5883_SendByte ( BYTE dat );
BYTE HMC5883_RecvByte ( void );

#endif