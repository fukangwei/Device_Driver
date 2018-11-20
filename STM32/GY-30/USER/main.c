#include "delay.h"
#include "sys.h"
#include "usart.h"
#include "GY_30.h"
#include "string.h"
#include "stdio.h"

extern BYTE BUF[];

int main ( void ) {
    float temp = 0.0f; /* 要显示的数值 */
    unsigned int dis_data = 0; /* 从传感器中读取的数据 */
    char show_buf[30] = {0}; /* 数据发送缓冲区 */
    SystemInit();
    delay_init ( 72 );
    NVIC_Configuration();
    uart_init ( 9600 );
    Init_BH1750();

    while ( 1 ) {
        Single_Write_BH1750 ( 0x01 ); /* power on */
        Single_Write_BH1750 ( 0x10 ); /* H-resolution mode */
        delay_ms ( 180 ); /* 延时180ms */
        Multiple_read_BH1750(); /* 连续读出数据，存储在BUF中 */
        dis_data = BUF[0];
        dis_data = ( dis_data << 8 ) + BUF[1]; /* 合成数据，即光照数据 */
        temp = ( float ) dis_data / 1.2;
        sprintf ( show_buf, "Light is %d LX\r\n", ( int ) temp );
        printf ( "%s", show_buf );
        memset ( show_buf, 0, sizeof ( show_buf ) );
        delay_ms ( 500 );
    }
}