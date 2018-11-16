#include <STC12C5A.H>
#include "uart.h"

unsigned int count = 0, temp = 0;

void UART_Init() {
    EA = 0;
    SCON = 0x50;
    TMOD = 0x20;
    TL1 = 0xfd;
    TH1 = 0xfd;
    TR1 = 1;
    ES = 1;
    AUXR &= 0x7F;
    TMOD &= 0xF0;
    TMOD |= 0x02;
    TL0 = 0xA4;
    TH0 = 0xA4;
    TF0 = 0;
    TR0 = 1;
    ET0 = 1;
    EA = 1;
}

void send ( unsigned char d ) { /* 串口1发送一个字节的数据，形参d即为待发送数据 */
    SBUF = d; /* 将数据写入到串口缓冲 */

    while ( !TI ); /* 等待发送完毕 */
}

void send_str ( unsigned char *dat ) { /* 串口1发送字符串 */
    while ( *dat ) {
        send ( *dat );
        dat++;
    }
}

void send_str_len ( unsigned char *dat, unsigned int len ) { /* 串口1发送指定长度的字符串 */
    unsigned char i = 0;

    for ( ; i < len; i++ ) {
        send ( dat[i] );
    }
}

void uart() interrupt 4 { /* 串口1用来与上位通讯 */
    if ( RI ) { /* 收到数据 */
        RI = 0; /* 清中断请求 */
        IE2 |= 0x01;
    } else if ( TI ) { /* 发送完一字节数据 */
        TI = 0;
    }

    ES = 1; /* 允许串口中断 */
}

void timer0() interrupt 1 {
    if ( count < 20000 ) {
        count++;

        if ( count >= 20000 ) { /* 每100次花费1秒 */
            count = 0;
        }
    }
}