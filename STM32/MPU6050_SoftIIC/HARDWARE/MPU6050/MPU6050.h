#ifndef MPU6050
#define MPU6050

typedef unsigned char  uchar;
typedef unsigned short ushort;
typedef unsigned int   uint;

#define SMPLRT_DIV   0x19 /* �����ǲ����� */
#define CONFIG       0x1A /* ��ͨ�˲�Ƶ�� */
#define GYRO_CONFIG  0x1B /* �������Լ켰������Χ */
#define ACCEL_CONFIG 0x1C /* ���ټ��Լ졢������Χ����ͨ�˲�Ƶ�� */
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
#define PWR_MGMT_1   0x6B /* ��Դ���� */
#define WHO_AM_I     0x75 /* IIC��ַ�Ĵ��� */

#define SlaveAddress 0xD0

void InitMPU6050 ( void );
int GetData ( uchar REG_Address );

#endif