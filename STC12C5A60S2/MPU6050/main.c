#include "STC12C5A.H"
#include <math.h>
#include <stdio.h>
#include <INTRINS.H>

typedef unsigned char  uchar;
typedef unsigned short ushort;
typedef unsigned int   uint;

sbit SCL = P2 ^ 0; /* IIC时钟引脚定义 */
sbit SDA = P2 ^ 1; /* IIC数据引脚定义 */

/* 定义MPU6050内部地址 */
#define SMPLRT_DIV   0x19 /* 陀螺仪采样率，典型值0x07(125Hz) */
#define CONFIG       0x1A /* 低通滤波频率，典型值0x06(5Hz) */
#define GYRO_CONFIG  0x1B /* 陀螺仪自检及测量范围，典型值0x18(不自检，2000deg/s) */
#define ACCEL_CONFIG 0x1C /* 加速计自检、测量范围及高通滤波频率，典型值0x01(不自检，2G，5Hz) */
#define ACCEL_XOUT_H 0x3B
#define ACCEL_XOUT_L 0x3C
#define ACCEL_YOUT_H 0x3D
#define ACCEL_YOUT_L 0x3E
#define ACCEL_ZOUT_H 0x3F
#define ACCEL_ZOUT_L 0x40
#define TEMP_OUT_H   0x41
#define TEMP_OUT_L   0x42
#define GYRO_XOUT_H  0x43
#define GYRO_XOUT_L  0x44
#define GYRO_YOUT_H  0x45
#define GYRO_YOUT_L  0x46
#define GYRO_ZOUT_H  0x47
#define GYRO_ZOUT_L  0x48
#define PWR_MGMT_1   0x6B /* 电源管理，典型值0x00(正常启用) */
#define WHO_AM_I     0x75 /* IIC地址寄存器(默认数值0x68，只读) */
#define SlaveAddress 0xD0 /* IIC写入时的地址字节数据 */

void delay ( unsigned int k );
void InitMPU6050();
void Delay5us();
void I2C_Start();
void I2C_Stop();
void I2C_SendACK ( bit ack );
bit I2C_RecvACK();
void I2C_SendByte ( uchar dat );
uchar I2C_RecvByte();
void I2C_ReadPage();
void I2C_WritePage();
void display_ACCEL_x();
void display_ACCEL_y();
void display_ACCEL_z();
uchar Single_ReadI2C ( uchar REG_Address ); /* 读取I2C数据 */
void Single_WriteI2C ( uchar REG_Address, uchar REG_data ); /* 向I2C写入数据 */

void delay ( unsigned int k ) {
    unsigned int i, j;

    for ( i = 0; i < k; i++ ) {
        for ( j = 0; j < 121; j++ );
    }
}

void Delay5us() {
    _nop_();
    _nop_();
    _nop_();
    _nop_();
    _nop_();
    _nop_();
    _nop_();
    _nop_();
    _nop_();
    _nop_();
    _nop_();
    _nop_();
    _nop_();
    _nop_();
    _nop_();
    _nop_();
    _nop_();
    _nop_();
    _nop_();
    _nop_();
    _nop_();
    _nop_();
    _nop_();
    _nop_();
}

void I2C_Start() { /* I2C起始信号 */
    SDA = 1; /* 拉高数据线 */
    SCL = 1; /* 拉高时钟线 */
    Delay5us();
    SDA = 0; /* 产生下降沿 */
    Delay5us();
    SCL = 0; /* 拉低时钟线 */
}

void I2C_Stop() { /* I2C停止信号 */
    SDA = 0; /* 拉低数据线 */
    SCL = 1; /* 拉高时钟线 */
    Delay5us();
    SDA = 1; /* 产生上升沿 */
    Delay5us();
}

void I2C_SendACK ( bit ack ) { /* I2C发送应答信号，参数ack为0是ACK，为1是NAK */
    SDA = ack; /* 写应答信号 */
    SCL = 1; /* 拉高时钟线 */
    Delay5us();
    SCL = 0; /* 拉低时钟线 */
    Delay5us();
}

bit I2C_RecvACK() { /* I2C接收应答信号 */
    SCL = 1; /* 拉高时钟线 */
    Delay5us();
    CY = SDA; /* 读应答信号 */
    SCL = 0; /* 拉低时钟线 */
    Delay5us();
    return CY;
}

void I2C_SendByte ( uchar dat ) { /* 向I2C总线发送一个字节数据 */
    uchar i;

    for ( i = 0; i < 8; i++ ) { /* 8位计数器 */
        dat <<= 1; /* 移出数据的最高位 */
        SDA = CY; /* 送数据口 */
        SCL = 1; /* 拉高时钟线 */
        Delay5us();
        SCL = 0; /* 拉低时钟线 */
        Delay5us();
    }

    I2C_RecvACK();
}

uchar I2C_RecvByte() { /* 从I2C总线接收一个字节数据 */
    uchar i;
    uchar dat = 0;
    SDA = 1; /* 使能内部上拉，准备读取数据 */

    for ( i = 0; i < 8; i++ ) { /* 8位计数器 */
        dat <<= 1;
        SCL = 1; /* 拉高时钟线 */
        Delay5us();
        dat |= SDA; /* 读数据 */
        SCL = 0; /* 拉低时钟线 */
        Delay5us();
    }

    return dat;
}

void Single_WriteI2C ( uchar REG_Address, uchar REG_data ) { /* 向I2C设备写入一个字节数据 */
    I2C_Start(); /* 起始信号 */
    I2C_SendByte ( SlaveAddress ); /* 发送设备地址 + 写信号 */
    I2C_SendByte ( REG_Address ); /* 内部寄存器地址 */
    I2C_SendByte ( REG_data ); /* 内部寄存器数据 */
    I2C_Stop(); /* 发送停止信号 */
}

uchar Single_ReadI2C ( uchar REG_Address ) { /* 从I2C设备读取一个字节数据 */
    uchar REG_data;
    I2C_Start(); /* 起始信号 */
    I2C_SendByte ( SlaveAddress ); /* 发送设备地址 + 写信号 */
    I2C_SendByte ( REG_Address ); /* 发送存储单元地址，从0开始 */
    I2C_Start(); /* 起始信号 */
    I2C_SendByte ( SlaveAddress + 1 ); /* 发送设备地址 + 读信号 */
    REG_data = I2C_RecvByte(); /* 读出寄存器数据 */
    I2C_SendACK ( 1 ); /* 接收应答信号 */
    I2C_Stop(); /* 停止信号 */
    return REG_data;
}

void InitMPU6050() { /* 初始化MPU6050 */
    Single_WriteI2C ( PWR_MGMT_1, 0x00 ); /* 解除休眠状态 */
    Single_WriteI2C ( SMPLRT_DIV, 0x07 );
    Single_WriteI2C ( CONFIG, 0x06 );
    Single_WriteI2C ( GYRO_CONFIG, 0x18 );
    Single_WriteI2C ( ACCEL_CONFIG, 0x01 );
}

int GetData ( uchar REG_Address ) { /* 获取数据 */
    uchar H, L;
    H = Single_ReadI2C ( REG_Address );
    L = Single_ReadI2C ( REG_Address + 1 );
    return ( H << 8 ) + L; /* 合成数据 */
}

void init_uart() {
    TMOD = 0x21;
    TH1 = 0xfd;
    TL1 = 0xfd;
    SCON = 0x50;
    PS = 1;
    TR0 = 1;
    TR1 = 1;
    ET0 = 1;
    ES = 1;
    EA = 1;
    TI = 1;
}

void main() {
    float x, y, z;
    float tempx, tempy, tempz;
    delay ( 500 ); /* 上电时需要延时 */
    init_uart();
    InitMPU6050();
    delay ( 150 );

    while ( 1 ) {
        x = ( ( float ) GetData ( ACCEL_XOUT_H ) ) / 1000.0;
        y = ( ( float ) GetData ( ACCEL_YOUT_H ) ) / 1000.0;
        z = ( ( float ) GetData ( ACCEL_ZOUT_H ) ) / 1000.0;
        tempx = ( atan ( x / ( sqrt ( z * z + y * y ) ) ) ) * 180 / 3.14;
        tempy = ( atan ( y / ( sqrt ( z * z + x * x ) ) ) ) * 180 / 3.14;
        tempz = ( atan ( ( sqrt ( x * x + y * y ) ) / z ) ) * 180 / 3.14;
        printf ( "x: %.2f, y: %.2f, z: %.2f\n", tempx, tempy, tempz );
        delay ( 1000 );
    }
}