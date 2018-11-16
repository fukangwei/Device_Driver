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

void send ( unsigned char d ) { /* ����1����һ���ֽڵ����ݣ��β�d��Ϊ���������� */
    SBUF = d; /* ������д�뵽���ڻ��� */

    while ( !TI ); /* �ȴ�������� */
}

void send_str ( unsigned char *dat ) { /* ����1�����ַ��� */
    while ( *dat ) {
        send ( *dat );
        dat++;
    }
}

void send_str_len ( unsigned char *dat, unsigned int len ) { /* ����1����ָ�����ȵ��ַ��� */
    unsigned char i = 0;

    for ( ; i < len; i++ ) {
        send ( dat[i] );
    }
}

void uart() interrupt 4 { /* ����1��������λͨѶ */
    if ( RI ) { /* �յ����� */
        RI = 0; /* ���ж����� */
        IE2 |= 0x01;
    } else if ( TI ) { /* ������һ�ֽ����� */
        TI = 0;
    }

    ES = 1; /* �������ж� */
}

void timer0() interrupt 1 {
    if ( count < 20000 ) {
        count++;

        if ( count >= 20000 ) { /* ÿ100�λ���1�� */
            count = 0;
        }
    }
}