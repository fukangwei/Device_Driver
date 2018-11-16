#include "GY273.h"
#include "delay.h"
#include "sys.h"
#include "myiic.h"
#include "stdio.h"

uint BUF[8]; /* 数据接收缓存区 */
uint ge, shi, bai, qian, wan;

void conversion ( uint temp_data ) {
    wan = temp_data / 10000;
    temp_data = temp_data % 10000; /* 取余运算 */
    qian = temp_data / 1000;
    temp_data = temp_data % 1000; /* 取余运算 */
    bai = temp_data / 100;
    temp_data = temp_data % 100; /* 取余运算 */
    shi = temp_data / 10;
    temp_data = temp_data % 10; /* 取余运算 */
    ge = temp_data;
}

void HMC5883_Start() { /* 起始信号 */
    IIC_Start();
}

void HMC5883_Stop() { /* 停止信号 */
    IIC_Stop();
}

void HMC5883_SendACK ( uchar ack ) { /* 发送应答信号，参数ack为0是ACK，为1是NAK */
    if ( ack == 0 ) {
        IIC_Ack();
    } else {
        IIC_NAck();
    }
}

uchar HMC5883_RecvACK() { /* 接收应答信号 */
    return IIC_Wait_Ack();
}

void HMC5883_SendByte ( BYTE dat ) { /* 向IIC总线发送一个字节数据 */
    IIC_Send_Byte ( dat );
}

BYTE HMC5883_RecvByte() { /* 从IIC总线接收一个字节数据 */
    return IIC_Read_Byte ( 0 );
}

void Single_Write_HMC5883 ( uchar REG_Address, uchar REG_data ) {
    HMC5883_Start(); /* 起始信号 */
    HMC5883_SendByte ( SlaveAddress ); /* 发送设备地址 + 写信号 */
    HMC5883_SendByte ( REG_Address ); /* 内部寄存器地址 */
    HMC5883_SendByte ( REG_data ); /* 内部寄存器数据 */
    HMC5883_Stop(); /* 发送停止信号 */
    delay_ms ( 10 );
}

void Multi_read_HMC5883 ( void ) { /* 连续读出HMC5883内部角度数据，地址范围0x3至0x8 */
    uchar i;
    HMC5883_Start(); /* 起始信号 */
    HMC5883_SendByte ( SlaveAddress ); /* 发送设备地址 + 写信号 */
    HMC5883_SendByte ( 0x03 ); /* 发送存储单元地址，从0x3开始 */
    HMC5883_Start(); /* 起始信号 */
    HMC5883_SendByte ( SlaveAddress + 1 ); /* 发送设备地址 + 读信号 */

    for ( i = 0; i < 6; i++ ) { /* 连续读取6个地址数据，存储于BUF中 */
        BUF[i] = HMC5883_RecvByte(); /* BUF[0]存储0x32地址中的数据 */

        if ( i == 5 ) {
            HMC5883_SendACK ( 1 ); /* 最后一个数据需要回NOACK */
        } else {
            HMC5883_SendACK ( 0 ); /* 回应ACK */
        }
    }

    HMC5883_Stop(); /* 停止信号 */
    delay_ms ( 80 );
}

void Init_HMC5883 ( void ) { /* 初始化HMC5883 */
    IIC_Init();
    Single_Write_HMC5883 ( 0x02, 0x00 );
    delay_ms ( 200 );
}