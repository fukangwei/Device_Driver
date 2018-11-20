#include "delay.h"
#include "sys.h"
#include "usart.h"
#include "GY_30.h"
#include "string.h"
#include "stdio.h"

extern BYTE BUF[];

int main ( void ) {
    float temp = 0.0f; /* Ҫ��ʾ����ֵ */
    unsigned int dis_data = 0; /* �Ӵ������ж�ȡ������ */
    char show_buf[30] = {0}; /* ���ݷ��ͻ����� */
    SystemInit();
    delay_init ( 72 );
    NVIC_Configuration();
    uart_init ( 9600 );
    Init_BH1750();

    while ( 1 ) {
        Single_Write_BH1750 ( 0x01 ); /* power on */
        Single_Write_BH1750 ( 0x10 ); /* H-resolution mode */
        delay_ms ( 180 ); /* ��ʱ180ms */
        Multiple_read_BH1750(); /* �����������ݣ��洢��BUF�� */
        dis_data = BUF[0];
        dis_data = ( dis_data << 8 ) + BUF[1]; /* �ϳ����ݣ����������� */
        temp = ( float ) dis_data / 1.2;
        sprintf ( show_buf, "Light is %d LX\r\n", ( int ) temp );
        printf ( "%s", show_buf );
        memset ( show_buf, 0, sizeof ( show_buf ) );
        delay_ms ( 500 );
    }
}