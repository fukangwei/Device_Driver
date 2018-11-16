#include "LCD1602.h"

static void delay1ms ( uint8 i ) {
    while ( i-- ) {
        unsigned char a, b, c;

        for ( c = 1; c > 0; c-- )
            for ( b = 4; b > 0; b-- )
                for ( a = 109; a > 0; a-- );
    }
}

/*-------------------------------------------------
 * 目的：读写检查，确保STA7为0
 * 读状态：输入“RS = L”，“RW = H”和“E = H”
 * 对控制器每次进行读写操作之前，都必须进行读写检查
 -------------------------------------------------*/
static void wait ( void ) {
    P0 = 0xFF;

    do {
        RS = 0;
        RW = 1;
        EN = 1;
    } while ( STA7 == 1 );
}

/* 写数据。输入“RS = H”，“RW = L”，“D0至D7 = 数据”，“E = 高脉冲” */
static void w_dat ( uint8 dat ) {
    wait();
    RS = 1;
    RW = 0;
    P0 = dat;
    EN = 0;
    EN = 1;
    EN = 0;
}

/* 写命令。输入“RS = L”，“RW = L”，D0至D7为指令码，“E = 高脉冲” */
static void w_cmd ( uint8 cmd ) {
    wait();
    P0 = cmd;
    RS = 0;
    RW = 0;
    EN = 0;
    EN = 1;
    EN = 0;
}

void w_string ( uint8 addr_start, uint8 *p ) { /* 发送字符串到LCD */
    w_cmd ( addr_start );

    while ( *p != '\0' ) {
        w_dat ( *p++ );
    }
}

void Init_LCD1602 ( void ) {
    delay1ms ( 15 );
    w_cmd ( 0x38 ); /* “16*2”显示，“5*7”点阵，8位数据接口 */
    delay1ms ( 5 );
    w_cmd ( 0x38 );
    delay1ms ( 5 );
    w_cmd ( 0x38 );
    w_cmd ( 0x38 );
    w_cmd ( 0x08 );
    w_cmd ( 0x01 ); /* 显示清屏 */
    w_cmd ( 0x06 ); /* 文字不动，光标自动右移 */
    w_cmd ( 0x0C ); /* 开显示、不显示光标、光标不闪烁 */
}