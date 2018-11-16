#include <reg52.h>
#include "1602.h"

#define VELOCITY_30C 3495 /* 30摄氏度时的声速，声速“V = 331.5 + 0.6 * 温度” */
#define VELOCITY_23C 3453 /* 23摄氏度时的声速，声速“V = 331.5 + 0.6 * 温度” */

sbit INPUT  = P3 ^ 2; /* 回声接收端口 */
sbit OUTPUT = P1 ^ 5; /* 超声触发端口 */
sbit Beep   = P2 ^ 3; /* 蜂鸣器 */

long int distance = 0; /* 距离变量 */
uchar count = 0;
uchar table3[] = "Distance:";

extern void initLCD();
extern void write_date ( uchar date );
extern void write_com ( uchar com );
extern void delay ( uint x );

void delayt ( uint x ) {
    uchar j;

    while ( x-- > 0 ) {
        for ( j = 0; j < 125; j++ ) {
            ;
        }
    }
}

void Init_MCU ( void ) {
    TMOD = 0x01; /* 定时器0初始化，设置为16位自动重装模式 */
    TL0  = 0x66; /* 定时时间为1ms */
    TH0  = 0xfc;
    ET0  = 1; /* 允许定时器0中断 */
    EA   = 1; /* 总中断使能 */
}

void Init_Parameter ( void ) {
    OUTPUT = 1;
    INPUT  = 1;
    count  = 0;
    distance = 0;
}

void display_char ( uchar *point, uchar address ) {
    uchar i;
    write_com ( 0x80 + address );

    for ( i = 0; i < 16; i++ ) {
        write_date ( *point );
        point++;
    }
}

void display ( int number, uchar address ) {
    uchar b, c, d, e;
    b = ( number / 1000 );
    c = ( number / 100 ) % 10;
    d = ( number / 10 ) % 10;
    e = number % 10;
    write_com ( 0x80 + address );
    write_date ( b + 48 );
    write_date ( c + 48 );
    write_date ( d + 48 );
    write_date ( 46 ); /* 小数点的ASCII */
    write_date ( e + 48 );
    write_date ( 99 ); /* “c”的ASCII */
    write_date ( 109 ); /* “m”的ASCII */
}

void Trig_SuperSonic ( void ) { /* 发出声波函数 */
    /* 采用IO口TRIG触发测距，提供至少10us的高电平信号 */
    OUTPUT = 1;
    delayt ( 1 );
    OUTPUT = 0;
}

void Measure_Distance ( void ) { /* 计算距离函数 */
    uchar l;
    uint h, y;
    TR0 = 1; /* “TR0 = 1”表示开定时器0 */

    while ( INPUT ) {
        ;
    }

    TR0 = 0; /* “TR0 = 0”表示关定时器0 */
    l = TL0;
    h = TH0;
    y = ( h << 8 ) + l;
    y = y - 0xfc66; /* us部分 */
    distance = y + 1000 * count; /* 计算总时间 */
    TL0 = 0x66;
    TH0 = 0xfc;
    delayt ( 30 );
    distance = VELOCITY_30C * distance / 20000;
}

void main ( void ) {
    initLCD();
    Init_MCU();
    Init_Parameter();

    while ( 1 ) {
        Trig_SuperSonic(); /* 触发超声波发射 */

        while ( INPUT == 0 ) { /* 等待回声 */
            ;
        }

        Measure_Distance(); /* 计算脉宽并转换为距离 */
        display_char ( table3, 0x40 );
        display ( distance, 0x49 ); /* 显示距离 */
        Init_Parameter(); /* 参数重新初始化 */
        delayt ( 100 ); /* 延时，两次发射之间至少要有10ms间隔 */
    }
}

void timer0 ( void ) interrupt 1 {
    TL0 = 0x66;
    TH0 = 0xfc;
    count++;

    if ( count == 18 ) { /* 超声波回声脉宽最多18ms */
        TL0 = 0x66;
        TH0 = 0xfc;
        count = 0;
    }
}