#include <reg52.h>
#include "HX711.h"
#include "stdio.h"

unsigned long Weight_Maopi = 0;
long Weight_Shiwu = 0;

/* ��Ϊ��ͬ�Ĵ������������߲��Ǻ�һ�£����ÿһ����������Ҫ�������������������ʹ����ֵ��׼ȷ��
   �����ֲ��Գ���������ƫ��ʱ�����Ӹ���ֵ��������Գ���������ƫСʱ����С����ֵ����ֵ����ΪС�� */
#define GapValue 430

void Delay_ms ( unsigned int n );
void Get_Maopi();
void Get_Weight();

void uart_init ( void ) {
    TMOD = 0x20; /* ��0010_0000����ʱ��/������1��������ʽ2 */
    TH1 = 0xfd; /* ���ò�����Ϊ9600 */
    TL1 = 0xfd;
    TR1 = 1; /* ������ʱ��/������1 */
    SCON = 0x50; /* 0101_0000�����ڹ�����ʽ1�������п��� */
    PCON = 0x00; /* ���á�SMOD = 0�� */
    IE = 0x90; /* CPU�����жϣ����������ж� */
    TI = 1; /* ֱ��ʹ��printf�������˾����ʵ�ַ��� */
}

void main ( void ) {
    uart_init();
    Delay_ms ( 3000 ); /* ��ʱ���ȴ��������ȶ� */
    Get_Maopi(); /* ��ë�� */

    while ( 1 ) {
        Get_Weight(); /* ���� */
        printf ( "%ldg\r\n", Weight_Shiwu );
        Delay_ms ( 1000 );
    }
}

void Get_Weight() { /* ���غ��� */
    Weight_Shiwu = HX711_Read();
    Weight_Shiwu = Weight_Shiwu - Weight_Maopi; /* ��ȡ���� */

    if ( Weight_Shiwu >= 0 ) {
        Weight_Shiwu = ( unsigned int ) ( ( float ) Weight_Shiwu / GapValue ); /* ����ʵ���ʵ������ */
    } else {
        Weight_Shiwu = 0;
    }
}

void Get_Maopi() { /* ��ȡë�� */
    Weight_Maopi = HX711_Read();
}

void Delay_ms ( unsigned int n ) {
    unsigned int  i, j;

    for ( i = 0; i < n; i++ )
        for ( j = 0; j < 123; j++ );
}