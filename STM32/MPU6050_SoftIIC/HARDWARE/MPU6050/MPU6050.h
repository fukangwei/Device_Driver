#ifndef MPU6050
#define MPU6050

typedef unsigned char  uchar;
typedef unsigned short ushort;
typedef unsigned int   uint;

#define SMPLRT_DIV   0x19 /* 陀螺仪采样率 */
#define CONFIG       0x1A /* 低通滤波频率 */
#define GYRO_CONFIG  0x1B /* 陀螺仪自检及测量范围 */
#define ACCEL_CONFIG 0x1C /* 加速计自检、测量范围及高通滤波频率 */
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
#define PWR_MGMT_1   0x6B /* 电源管理 */
#define WHO_AM_I     0x75 /* IIC地址寄存器 */

#define SlaveAddress 0xD0

void InitMPU6050 ( void );
int GetData ( uchar REG_Address );

#endif