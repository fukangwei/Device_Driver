#include "LCD1602.h"

static void delay1ms ( uint8 i ) {
    while ( i-- ) {
        unsigned char a, b, c;

        for ( c = 1; c > 0; c-- )
            for ( b = 4; b > 0; b-- )
                for ( a = 109; a > 0; a-- );
    }
}

static void wait ( void ) { /* ��д��飬ȷ��STA7Ϊ0 */
    P0 = 0xFF;

    do {
        RS = 0;
        RW = 1;
        EN = 1;
    } while ( STA7 == 1 );
}

static void w_dat ( uint8 dat ) { /* д���� */
    wait();
    RS = 1;
    RW = 0;
    P0 = dat;
    EN = 0;
    EN = 1;
    EN = 0;
}

static void w_cmd ( uint8 cmd ) { /* д���� */
    wait();
    P0 = cmd;
    RS = 0;
    RW = 0;
    EN = 0;
    EN = 1;
    EN = 0;
}

void w_string ( uint8 addr_start, uint8 *p ) { /* �����ַ�����LCD */
    w_cmd ( addr_start );

    while ( *p != '\0' ) {
        w_dat ( *p++ );
    }
}

void Init_LCD1602 ( void ) {
    delay1ms ( 15 );
    w_cmd ( 0x38 ); /* ��16*2����ʾ����5*7������8λ���ݽӿ� */
    delay1ms ( 5 );
    w_cmd ( 0x38 );
    delay1ms ( 5 );
    w_cmd ( 0x38 );
    w_cmd ( 0x38 );
    w_cmd ( 0x08 );
    w_cmd ( 0x01 ); /* ��ʾ���� */
    w_cmd ( 0x06 ); /* ���ֲ���������Զ����� */
    w_cmd ( 0x0C ); /* ����ʾ������ʾ��ꡢ��겻��˸ */
}