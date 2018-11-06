#include <reg52.h>
#include "ds1302.h"

extern uint8 set_ds1302_time[7];
extern uint8 read_ds1302_time[7];

static void delay ( uint16 n ) {
    while ( n-- );
}

void uart_init ( void ) {
    TMOD = 0x21;
    SCON = 0x50;
    TH1  = 0xFD;
    TL1  = 0xFD;
    TR1  = 1;
}

void UART_Send_Byte ( uint8 dat ) {
    SBUF = dat;

    while ( TI == 0 );

    TI = 0;
}

/* 将数据转换成ASCII码，并通过UART发送出去 */
void UART_Send_Dat ( uint8 dat ) {
    UART_Send_Byte ( dat / 16 + '0' );
    UART_Send_Byte ( dat % 16 + '0' );
}

int main ( void ) {
    uart_init();
    set_time ( &set_ds1302_time ); /* 设定时间值 */

    while ( 1 ) {
        read_time ( &read_ds1302_time );
        UART_Send_Dat ( read_ds1302_time[6] ); /* 年 */
        UART_Send_Byte ( '-' );
        UART_Send_Dat ( read_ds1302_time[4] ); /* 月 */
        UART_Send_Byte ( '-' );
        UART_Send_Dat ( read_ds1302_time[3] ); /* 日 */
        UART_Send_Byte ( ' ' );
        UART_Send_Dat ( read_ds1302_time[2] ); /* 时 */
        UART_Send_Byte ( ':' );
        UART_Send_Dat ( read_ds1302_time[1] ); /* 分 */
        UART_Send_Byte ( ':' );
        UART_Send_Dat ( read_ds1302_time[0] ); /* 秒 */
        UART_Send_Byte ( '\r' );
        UART_Send_Byte ( '\n' );
        delay ( 20000 );
    }
}