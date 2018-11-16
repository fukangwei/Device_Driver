#include "STC12C5A.H"
#include "intrins.h"
#include "string.h"
#include "stdio.h"
#include "GY_30.h"

extern BYTE BUF[];

void delay500ms ( void ) { 
    unsigned char a, b, c;

    for ( c = 93; c > 0; c-- )
        for ( b = 178; b > 0; b-- )
            for ( a = 82; a > 0; a-- );
}

static void delay1ms ( void ) {
    unsigned char a, b;

    for ( b = 21; b > 0; b-- )
        for ( a = 130; a > 0; a-- );

    _nop_();
}

static void delay_nms ( unsigned int n ) {
    while ( n-- ) {
        delay1ms();
    }
}

void UART_init ( void ) {
    PCON &= 0x7F;
    SCON = 0x50;
    AUXR &= 0xFB;
    BRT = 0xFD;
    AUXR |= 0x01;
    AUXR |= 0x10;
    ES = 1;
}

void UART_send_byte ( uchar dat ) {
    ES = 0;
    TI = 0;
    SBUF = dat;

    while ( TI == 0 );

    TI = 0;
    ES = 1;
}

void UART_send_string ( uchar *buf ) {
    while ( *buf != '\0' ) {
        UART_send_byte ( *buf++ );
    }
}

int main ( void ) {
    float temp = 0.0f; /* Ҫ��ʾ����ֵ */
    unsigned int dis_data = 0; /* �Ӵ������ж�ȡ������ */
    unsigned char show_buf[30] = {0}; /* ���ݷ��ͻ����� */
    UART_init(); /* ���ڳ�ʼ�� */
    Init_BH1750(); /* BH1750��ʼ�� */

    while ( 1 ) {
        Single_Write_BH1750 ( 0x01 ); /* power on */
        Single_Write_BH1750 ( 0x10 ); /* H-resolution mode */
        delay_nms ( 180 ); /* ��ʱ180ms */
        Multiple_read_BH1750(); /* �����������ݣ��洢��BUF�� */
        dis_data = BUF[0];
        dis_data = ( dis_data << 8 ) + BUF[1]; /* �ϳ����ݣ����������� */
        temp = ( float ) dis_data / 1.2;
        sprintf ( show_buf, "Light is %d LX\r\n", ( int ) temp );
        UART_send_string ( show_buf );
        memset ( show_buf, 0, sizeof ( show_buf ) );
        delay500ms();
    }
}