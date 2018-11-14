#include "led.h"
#include "delay.h"
#include "sys.h"
#include "usart.h"
#include "MPU6050.h"

int main ( void ) {
    SystemInit();
    delay_init ( 72 );
    NVIC_Configuration();
    uart_init ( 9600 );
    LED_Init();
    InitMPU6050();

    while ( 1 ) {
        printf ( "%d   %d  %d\r\n", GetData ( ACCEL_XOUT_H ), GetData ( ACCEL_YOUT_H ), GetData ( ACCEL_ZOUT_H ) );
        delay_ms ( 500 );
        LED1 = !LED1;
    }
}