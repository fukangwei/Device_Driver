#include "led.h"
#include "delay.h"
#include "sys.h"
#include "usart.h"
#include <math.h>
#include "SysTick.h"
#include "I2C_MPU6050.h"

extern uint32_t RunningTime;
long LastTime = 0, NowTime, TimeSpan; /* �����Խ��ٶȽ��л��ֵ� */

/* �����������ǵ�ƫ�� */
#define Gx_offset -1.196
#define Gy_offset -0.1157
#define Gz_offset 0.9125
#define pi 3.141592653

int16_t ax, ay, az;
int16_t gx, gy, gz; /* �洢ԭʼ���� */
float aax, aay, aaz, ggx, ggy, ggz; /* �洢����������� */
float Ax, Ay, Az; /* ��λg(9.8m/s^2) */
float Gx, Gy, Gz; /* ��λ����/s�� */

float Angle_accX, Angle_accY, Angle_accZ; /* �洢���ٶȼ�����ĽǶ� */

int main ( void ) {
    SystemInit();
    delay_init ( 72 );
    NVIC_Configuration();
    uart_init ( 9600 );
    LED_Init();
    I2C_MPU6050_Init();
    InitMPU6050();

    while ( 1 ) {
        ax = GetData ( ACCEL_XOUT_H );
        ay = GetData ( ACCEL_YOUT_H );
        az = GetData ( ACCEL_ZOUT_H );
        gx = GetData ( GYRO_XOUT_H );
        gy = GetData ( GYRO_YOUT_H );
        gz = GetData ( GYRO_ZOUT_H );
        /* ���������ǶԼ��ٶȽ������� */
        Ax = ax / 16384.00;
        Ay = ay / 16384.00;
        Az = az / 16384.00;
        /* �����������ü��ٶȼ����������ˮƽ������ϵ֮��ļн� */
        Angle_accX = atan ( Ax / Az ) * 180 / pi; /* ���ٶ��ǣ������л�û���ֵ������180��/pi */
        Angle_accY = atan ( Ay / Az ) * 180 / pi; /* ��ýǶ�ֵ�����ԡ�-1�������� */
        /* ���������ǶԽ��ٶ������� */
        ggx = gx / 16.38;
        ggy = gy / 16.38;
        ggz = gz / 16.38;
        /* �����ǶԽǶȽ��л��ִ��� */
        NowTime = RunningTime; /* ��ȡ��ǰ�������еĺ����� */
        TimeSpan = NowTime - LastTime; /* ����ʱ�������㲻�Ǻ��Ͻ� */
        /* �������о���ͨ���Խ��ٶȻ���ʵ�ָ�����ĽǶȲ�������Ȼ����������ʼ�Ƕȶ���0 */
        Gx = Gx + ( ggx - Gx_offset ) * TimeSpan / 1000;
        Gy = Gy + ( ggy - Gy_offset ) * TimeSpan / 1000;
        Gz = Gz + ( ggz - Gz_offset ) * TimeSpan / 1000;
        LastTime = NowTime;
        printf ( "Angle_accX = %f ��\r\n", Angle_accX );
        printf ( "Angle_accY = %f ��\r\n", Angle_accY );
        printf ( "GYRO_accX = %f ��\r\n", Gx );
        printf ( "GYRO_accY = %f ��\r\n", Gy );
        printf ( "GYRO_accZ = %f ��\r\n", Gz );
        LED1 = !LED1;
        delay_ms ( 500 );
    }
}