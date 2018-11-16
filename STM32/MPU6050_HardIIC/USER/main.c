#include "led.h"
#include "delay.h"
#include "sys.h"
#include "usart.h"
#include <math.h>
#include "SysTick.h"
#include "I2C_MPU6050.h"

extern uint32_t RunningTime;
long LastTime = 0, NowTime, TimeSpan; /* 用来对角速度进行积分的 */

/* 定义了陀螺仪的偏差 */
#define Gx_offset -1.196
#define Gy_offset -0.1157
#define Gz_offset 0.9125
#define pi 3.141592653

int16_t ax, ay, az;
int16_t gx, gy, gz; /* 存储原始数据 */
float aax, aay, aaz, ggx, ggy, ggz; /* 存储量化后的数据 */
float Ax, Ay, Az; /* 单位g(9.8m/s^2) */
float Gx, Gy, Gz; /* 单位“°/s” */

float Angle_accX, Angle_accY, Angle_accZ; /* 存储加速度计算出的角度 */

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
        /* 以下三行是对加速度进行量化 */
        Ax = ax / 16384.00;
        Ay = ay / 16384.00;
        Az = az / 16384.00;
        /* 以下三行是用加速度计算三个轴和水平面坐标系之间的夹角 */
        Angle_accX = atan ( Ax / Az ) * 180 / pi; /* 加速度仪，反正切获得弧度值，乘以180度/pi */
        Angle_accY = atan ( Ay / Az ) * 180 / pi; /* 获得角度值，乘以“-1”得正数 */
        /* 以下三行是对角速度做量化 */
        ggx = gx / 16.38;
        ggy = gy / 16.38;
        ggz = gz / 16.38;
        /* 以下是对角度进行积分处理 */
        NowTime = RunningTime; /* 获取当前程序运行的毫秒数 */
        TimeSpan = NowTime - LastTime; /* 积分时间这样算不是很严谨 */
        /* 下面三行就是通过对角速度积分实现各个轴的角度测量，当然假设各轴的起始角度都是0 */
        Gx = Gx + ( ggx - Gx_offset ) * TimeSpan / 1000;
        Gy = Gy + ( ggy - Gy_offset ) * TimeSpan / 1000;
        Gz = Gz + ( ggz - Gz_offset ) * TimeSpan / 1000;
        LastTime = NowTime;
        printf ( "Angle_accX = %f °\r\n", Angle_accX );
        printf ( "Angle_accY = %f °\r\n", Angle_accY );
        printf ( "GYRO_accX = %f °\r\n", Gx );
        printf ( "GYRO_accY = %f °\r\n", Gy );
        printf ( "GYRO_accZ = %f °\r\n", Gz );
        LED1 = !LED1;
        delay_ms ( 500 );
    }
}