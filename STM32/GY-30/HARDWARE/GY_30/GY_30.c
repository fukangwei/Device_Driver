#include "sys.h"
#include "GY_30.h"
#include "delay.h"
#include "myiic.h"

BYTE BUF[8]; /* 接收数据缓存区 */

void BH1750_Start ( void ) { /* 光照传感器起始信号 */
    IIC_Start();
}

void BH1750_Stop ( void ) { /* 光照传感器停止信号 */
    IIC_Stop();
}

void BH1750_SendACK ( BYTE ack ) { /* 发送应答信号。参数ack为0是ACK，为1是NAK */
    if ( ack == 0 ) {
        IIC_Ack();
    } else {
        IIC_NAck();
    }
}

BYTE BH1750_RecvACK ( void ) { /* 接收应答信号 */
    return IIC_Wait_Ack();
}

void BH1750_SendByte ( BYTE dat ) { /* 向IIC总线发送一个字节数据 */
    IIC_Send_Byte ( dat );
    BH1750_RecvACK();
}

BYTE BH1750_RecvByte ( void ) { /* 从IIC总线接收一个字节数据 */
    return IIC_Read_Byte ( 0 );
}

void Single_Write_BH1750 ( uchar REG_Address ) {
    BH1750_Start(); /* 起始信号 */
    BH1750_SendByte ( SlaveAddress ); /* 发送设备地址 + 写信号 */
    BH1750_SendByte ( REG_Address ); /* 内部寄存器地址 */
    BH1750_Stop(); /* 发送停止信号 */
}

void Init_BH1750 ( void ) { /* BH1750初始化函数 */
    IIC_Init();
    delay_ms ( 50 );
    Single_Write_BH1750 ( 0x01 );
}

void Multiple_read_BH1750 ( void ) { /* 连续读出BH1750内部数据 */
    uchar i;
    BH1750_Start(); /* 起始信号 */
    BH1750_SendByte ( SlaveAddress + 1 ); /* 发送设备地址 + 读信号 */

    for ( i = 0; i < 3; i++ ) { /* 连续读取2个地址数据，存储在BUF中 */
        BUF[i] = BH1750_RecvByte(); /* BUF[0]存储0x32地址中的数据 */

        if ( i == 3 ) {
            BH1750_SendACK ( 1 ); /* 最后一个数据需要回应NOACK */
        } else {
            BH1750_SendACK ( 0 ); /* 回应ACK */
        }
    }

    BH1750_Stop(); /* 停止信号 */
    delay_ms ( 5 );
}