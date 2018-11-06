#include "iocc2530.h"
#include "stdio.h"
#include "ds18b20.h"

__near_func int putchar ( int c ) {
    UTX0IF = 0;
    U0DBUF = ( char ) c;

    while ( UTX0IF == 0 );

    return ( c );
}

void InitUART ( void ) {
    PERCFG = 0x00; /* (外设控制寄存器)USART_0的I/O位置为备用位置0 */
    P0SEL = 0x0c; /* P0_2和P0_3用作串口(外部设备功能) */
    P2DIR &= ~0XC0; /* (端口2方向和端口0外设优先级控制)P0优先作为UART0 */
    U0CSR |= 0x80; /* 设置为UART模式 */
    U0GCR |= 8; /* 波特率指数值，BAUD_E和BAUD_M决定了UART波特率 */
    U0BAUD |= 59; /* 波特率设为9600 */
    UTX0IF = 0; /* UART0的TX中断标志初始设置为0 */
}

/* 即延时xms毫秒 */
void Delayms ( int xms ) {
    int i, j;

    for ( i = xms; i > 0; i-- )
        for ( j = 587; j > 0; j-- );
}

void main() {
    CLKCONCMD &= ~0x40; /* 设置系统时钟源为32MHz晶振 */

    while ( CLKCONSTA & 0x40 ); /* 等待晶振稳定为32M */

    CLKCONCMD &= ~0x47; /* 设置系统主时钟频率为32MHz */
    P0SEL &= 0xbf; /* DS18B20的io口初始化 */
    InitUART();

    while ( 1 ) {
        Temp_test(); /* 温度检测 */
        printf ( "The temp is %d\r\n", temp );
        Delayms ( 500 );
    }
}