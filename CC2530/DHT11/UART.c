#include <ioCC2530.h>

#define uchar unsigned char
#define uint unsigned int

void InitUart(); /* ��ʼ������ */
void Uart_Send_String ( unsigned char *Data, int len );

void InitUart() {
    CLKCONCMD &= ~0x40; /* ����ϵͳʱ��ԴΪ32MHZ���� */

    while ( CLKCONSTA & 0x40 ); /* �ȴ������ȶ� */

    CLKCONCMD &= ~0x47; /* ����ϵͳ��ʱ��Ƶ��Ϊ32MHZ */
    PERCFG = 0x00; /* λ��1 P0�� */
    P0SEL = 0x3c; /* P0_2��P0_3��P0_4��P0_5�������ڣ��ڶ����� */
    P2DIR &= ~0XC0; /* P0������ΪUART0�����ȼ� */
    U0CSR |= 0x80;  /* UART��ʽ */
    U0GCR |= 11; /* U0GCR��U0BAUD��� */
    U0BAUD |= 216; /* ��������Ϊ115200 */
    UTX0IF = 0; /* UART0��TX�жϱ�־��ʼ��λ0 */
}

void Uart_Send_String ( uchar *Data, int len ) {
    int j;

    for ( j = 0; j < len; j++ ) {
        U0DBUF = *Data++;

        while ( UTX0IF == 0 );

        UTX0IF = 0;
    }
}