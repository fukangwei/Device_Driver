#include "myiic.h"
#include "MPU6050.h"
#include "delay.h"

static void Single_WriteI2C ( uchar REG_Address, uchar REG_data ) {
    IIC_Start();
    IIC_Send_Byte ( SlaveAddress );
    IIC_Send_Byte ( REG_Address );
    IIC_Send_Byte ( REG_data );
    IIC_Stop();
}

static uchar Single_ReadI2C ( uchar REG_Address ) {
    uchar REG_data;
    IIC_Start();
    IIC_Send_Byte ( SlaveAddress );
    IIC_Send_Byte ( REG_Address );
    IIC_Start();
    IIC_Send_Byte ( SlaveAddress + 1 );
    REG_data = IIC_Read_Byte ( 0 );
    IIC_Stop();
    return REG_data;
}

void InitMPU6050 ( void ) {
    IIC_Init();
    delay_ms ( 200 );
    Single_WriteI2C ( PWR_MGMT_1,   0x00 );
    Single_WriteI2C ( SMPLRT_DIV,   0x07 );
    Single_WriteI2C ( CONFIG,       0x06 );
    Single_WriteI2C ( GYRO_CONFIG,  0x18 );
    Single_WriteI2C ( ACCEL_CONFIG, 0x01 );
    delay_ms ( 200 );
}

int GetData ( uchar REG_Address ) {
    uchar H, L;
    H = Single_ReadI2C ( REG_Address );
    L = Single_ReadI2C ( REG_Address + 1 );
    return ( H << 8 ) + L;
}