#include <ioCC2530.h>

#define uchar unsigned char
#define uint unsigned int

void InitUart(); /* 初始化串口 */
void Uart_Send_String ( unsigned char *Data, int len );

void InitUart() {
    CLKCONCMD &= ~0x40; /* 设置系统时钟源为32MHZ晶振 */

    while ( CLKCONSTA & 0x40 ); /* 等待晶振稳定 */

    CLKCONCMD &= ~0x47; /* 设置系统主时钟频率为32MHZ */
    PERCFG = 0x00; /* 位置1 P0口 */
    P0SEL = 0x3c; /* P0_2、P0_3、P0_4、P0_5用作串口，第二功能 */
    P2DIR &= ~0XC0; /* P0优先作为UART0，优先级 */
    U0CSR |= 0x80;  /* UART方式 */
    U0GCR |= 11; /* U0GCR与U0BAUD配合 */
    U0BAUD |= 216; /* 波特率设为115200 */
    UTX0IF = 0; /* UART0的TX中断标志初始置位0 */
}

void Uart_Send_String ( uchar *Data, int len ) {
    int j;

    for ( j = 0; j < len; j++ ) {
        U0DBUF = *Data++;

        while ( UTX0IF == 0 );

        UTX0IF = 0;
    }
}