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
    PERCFG = 0x00; /* (������ƼĴ���)USART_0��I/Oλ��Ϊ����λ��0 */
    P0SEL = 0x0c; /* P0_2��P0_3��������(�ⲿ�豸����) */
    P2DIR &= ~0XC0; /* (�˿�2����Ͷ˿�0�������ȼ�����)P0������ΪUART0 */
    U0CSR |= 0x80; /* ����ΪUARTģʽ */
    U0GCR |= 8; /* ������ָ��ֵ��BAUD_E��BAUD_M������UART������ */
    U0BAUD |= 59; /* ��������Ϊ9600 */
    UTX0IF = 0; /* UART0��TX�жϱ�־��ʼ����Ϊ0 */
}

/* ����ʱxms���� */
void Delayms ( int xms ) {
    int i, j;

    for ( i = xms; i > 0; i-- )
        for ( j = 587; j > 0; j-- );
}

void main() {
    CLKCONCMD &= ~0x40; /* ����ϵͳʱ��ԴΪ32MHz���� */

    while ( CLKCONSTA & 0x40 ); /* �ȴ������ȶ�Ϊ32M */

    CLKCONCMD &= ~0x47; /* ����ϵͳ��ʱ��Ƶ��Ϊ32MHz */
    P0SEL &= 0xbf; /* DS18B20��io�ڳ�ʼ�� */
    InitUART();

    while ( 1 ) {
        Temp_test(); /* �¶ȼ�� */
        printf ( "The temp is %d\r\n", temp );
        Delayms ( 500 );
    }
}