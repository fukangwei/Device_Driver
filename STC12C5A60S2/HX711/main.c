#include <reg52.h>
#include "HX711.h"
#include "stdio.h"

unsigned long Weight_Maopi = 0;
long Weight_Shiwu = 0;

/* 因为不同的传感器特性曲线不是很一致，因此每一个传感器需要矫正这里这个参数才能使测量值很准确。
   当发现测试出来的重量偏大时，增加该数值。如果测试出来的重量偏小时，减小改数值。该值可以为小数 */
#define GapValue 430

void Delay_ms ( unsigned int n );
void Get_Maopi();
void Get_Weight();

void uart_init ( void ) {
    TMOD = 0x20; /* 即0010_0000、定时器/计数器1、工作方式2 */
    TH1 = 0xfd; /* 设置波特率为9600 */
    TL1 = 0xfd;
    TR1 = 1; /* 启动定时器/计数器1 */
    SCON = 0x50; /* 0101_0000、串口工作方式1、允许串行控制 */
    PCON = 0x00; /* 设置“SMOD = 0” */
    IE = 0x90; /* CPU允许中断，串行允许中断 */
    TI = 1; /* 直接使用printf必须加入此句才能实现发送 */
}

void main ( void ) {
    uart_init();
    Delay_ms ( 3000 ); /* 延时，等待传感器稳定 */
    Get_Maopi(); /* 称毛重 */

    while ( 1 ) {
        Get_Weight(); /* 称重 */
        printf ( "%ldg\r\n", Weight_Shiwu );
        Delay_ms ( 1000 );
    }
}

void Get_Weight() { /* 称重函数 */
    Weight_Shiwu = HX711_Read();
    Weight_Shiwu = Weight_Shiwu - Weight_Maopi; /* 获取净重 */

    if ( Weight_Shiwu >= 0 ) {
        Weight_Shiwu = ( unsigned int ) ( ( float ) Weight_Shiwu / GapValue ); /* 计算实物的实际重量 */
    } else {
        Weight_Shiwu = 0;
    }
}

void Get_Maopi() { /* 获取毛重 */
    Weight_Maopi = HX711_Read();
}

void Delay_ms ( unsigned int n ) {
    unsigned int  i, j;

    for ( i = 0; i < n; i++ )
        for ( j = 0; j < 123; j++ );
}