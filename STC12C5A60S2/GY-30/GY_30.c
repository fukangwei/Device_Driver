#include "STC12C5A.H"
#include "intrins.h"
#include "GY_30.h"

BYTE BUF[8]; /* 接收数据缓存区 */

static void delay1ms ( void ) { /* 误差为“-0.018084490741us” */
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

static void Delay5us ( void ) { /* 误差为“-0.026765046296us” */
    unsigned char a;

    for ( a = 12; a > 0; a-- );

    _nop_(); /* if Keil,require use intrins.h */
}

void BH1750_Start ( void ) { /* 光照传感器起始信号 */
    SDA = 1; /* 拉高数据线 */
    SCL = 1; /* 拉高时钟线 */
    Delay5us();
    SDA = 0; /* 产生下降沿 */
    Delay5us();
    SCL = 0; /* 拉低时钟线 */
}

void BH1750_Stop ( void ) { /* 光照传感器停止信号 */
    SDA = 0; /* 拉低数据线 */
    SCL = 1; /* 拉高时钟线 */
    Delay5us();
    SDA = 1; /* 产生上升沿 */
    Delay5us();
}

void BH1750_SendACK ( bit ack ) { /* 发送应答信号。参数ack为0是ACK，为1是NAK */
    SDA = ack; /* 写应答信号 */
    SCL = 1; /* 拉高时钟线 */
    Delay5us();
    SCL = 0; /* 拉低时钟线 */
    Delay5us();
}

bit BH1750_RecvACK ( void ) { /* 接收应答信号 */
    SCL = 1; /* 拉高时钟线 */
    Delay5us();
    CY = SDA; /* 读应答信号 */
    SCL = 0; /* 拉低时钟线 */
    Delay5us();
    return CY;
}

void BH1750_SendByte ( BYTE dat ) { /* 向IIC总线发送一个字节数据 */
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

BYTE BH1750_RecvByte ( void ) { /* 从IIC总线接收一个字节数据 */
    BYTE i;
    BYTE dat = 0;
    SDA = 1; /* 使能内部上拉，准备读取数据 */

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
    BH1750_Start(); /* 起始信号 */
    BH1750_SendByte ( SlaveAddress ); /* 发送设备地址 + 写信号 */
    BH1750_SendByte ( REG_Address ); /* 内部寄存器地址 */
    BH1750_Stop(); /* 发送停止信号 */
}

void Init_BH1750 ( void ) { /* BH1750初始化函数 */
    Single_Write_BH1750 ( 0x01 );
}

void Multiple_read_BH1750 ( void ) { /* 连续读出BH1750内部数据 */
    uchar i;
    BH1750_Start(); /* 起始信号 */
    BH1750_SendByte ( SlaveAddress + 1 ); /* 发送设备地址 + 读信号 */

    for ( i = 0; i < 3; i++ ) { /* 连续读取2个地址数据，存储在BUF中 */
        BUF[i] = BH1750_RecvByte(); /* BUF[0]存储0x32地址中的数据 */

        if ( i == 3 ) {
            BH1750_SendACK ( 1 ); /* 最后一个数据需要回NOACK */
        } else {
            BH1750_SendACK ( 0 ); /* 回应ACK */
        }
    }

    BH1750_Stop(); /* 停止信号 */
    delay_nms ( 5 );
}