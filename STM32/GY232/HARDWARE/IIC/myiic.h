#ifndef __MYIIC_H
#define __MYIIC_H
#include "sys.h"

#define SDA_IN()  {GPIOC->CRH &= 0XFFFF0FFF; GPIOC->CRH |= 8 << 12;}
#define SDA_OUT() {GPIOC->CRH &= 0XFFFF0FFF; GPIOC->CRH |= 3 << 12;}

#define IIC_SCL  PCout(12)
#define IIC_SDA  PCout(11)
#define READ_SDA PCin(11)

void IIC_Init ( void );
void IIC_Start ( void );
void IIC_Stop ( void );
void IIC_Send_Byte ( u8 txd );
u8 IIC_Read_Byte ( unsigned char ack );
u8 IIC_Wait_Ack ( void );
void IIC_Ack ( void );
void IIC_NAck ( void );

void IIC_Write_One_Byte ( u8 daddr, u8 addr, u8 data );
u8 IIC_Read_One_Byte ( u8 daddr, u8 addr );
#endif