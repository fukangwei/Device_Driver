#ifndef __MYIIC_H
#define __MYIIC_H
#include "sys.h"

#define SDA_IN()  {GPIOC->CRL &= 0XFFF0FFFF; GPIOC->CRL |= 8 << (4*4);}
#define SDA_OUT() {GPIOC->CRL &= 0XFFF0FFFF; GPIOC->CRL |= 3 << (4*4);}

#define IIC_SCL  PCout(3)
#define IIC_SDA  PCout(4)
#define READ_SDA PCin(4)

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