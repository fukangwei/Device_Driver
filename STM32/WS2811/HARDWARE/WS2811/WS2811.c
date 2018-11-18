#include "sys.h"
#include "WS2811.h"
#include "delay.h"

unsigned long WsDat[nWs]; /* 显存 */

void WS_Init() {
    GPIO_InitTypeDef  GPIO_InitStructure;
    RCC_APB2PeriphClockCmd ( RCC_APB2Periph_GPIOA, ENABLE );
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init ( GPIOA, &GPIO_InitStructure );
}

void delay2us() {
    unsigned char i;

    for ( i = 0; i < 12; i++ );
}

void delay05us() {
    unsigned char i;

    for ( i = 0; i < 1; i++ );
}

void TX0() { /* 发送一比特(发送0) */
    PAout ( 0 ) = 1;
    delay05us();
    PAout ( 0 ) = 0;
    delay2us();
}

void TX1() { /* 发送一比特(发送1) */
    PAout ( 0 ) = 1;
    delay2us();
    PAout ( 0 ) = 0;
    delay05us();
}

void WS_Reset() {
    PAout ( 0 ) = 0;
    delay_us ( 60 );
    PAout ( 0 ) = 1;
    PAout ( 0 ) = 0;
}

void WS_Set1 ( unsigned long dat ) { /* 发送一字节 */
    unsigned char i;

    for ( i = 0; i < 24; i++ ) {
        if ( 0x800000 == ( dat & 0x800000 ) ) {
            TX1();
        } else {
            TX0();
        }

        dat <<= 1; /* 左移一位 */
    }
}

void WS_SetAll() { /* 发送所有字节 */
    unsigned char j;

    for ( j = 0; j < nWs; j++ ) {
        WS_Set1 ( WsDat[0] );
    }

    WS_Reset();
}

unsigned char abs0 ( int num ) { /* 求绝对值 */
    if ( num > 0 ) {
        return num;
    }

    num = -num;
    return ( unsigned char ) num;
}

u32 ColorToColor ( unsigned long color0, unsigned long color1 ) { /* 颜色渐变算法，误差 <= 2 */
    unsigned char Red0, Green0, Blue0; /* 起始三原色 */
    unsigned char Red1, Green1, Blue1; /* 结果三原色 */
    int RedMinus, GreenMinus, BlueMinus; /* 颜色差(color1 - color0) */
    unsigned char NStep; /* 需要几步 */
    float RedStep, GreenStep, BlueStep; /* 各色步进值 */
    unsigned long color; /* 结果色 */
    unsigned char i;
    /* 红、绿、蓝三原色分解 */
    Red0   = color0 >> 8;
    Green0 = color0 >> 16;
    Blue0  = color0;
    Red1   = color1 >> 8;
    Green1 = color1 >> 16;
    Blue1  = color1;
    /* 计算需要多少步(取差值的最大值) */
    RedMinus   = Red1 - Red0;
    GreenMinus = Green1 - Green0;
    BlueMinus  = Blue1 - Blue0;
    NStep = ( abs0 ( RedMinus ) > abs0 ( GreenMinus ) ) ? abs0 ( RedMinus ) : abs0 ( GreenMinus );
    NStep = ( NStep > abs0 ( BlueMinus ) ) ? NStep : abs0 ( BlueMinus );
    /* 计算出各色步进值 */
    RedStep   = ( float ) RedMinus   / NStep;
    GreenStep = ( float ) GreenMinus / NStep;
    BlueStep  = ( float ) BlueMinus  / NStep;

    for ( i = 0; i < NStep; i++ ) { /* 渐变开始 */
        Red1   = Red0   + ( int ) ( RedStep   * i );
        Green1 = Green0 + ( int ) ( GreenStep * i );
        Blue1  = Blue0  + ( int ) ( BlueStep  * i );
        color  = Green1 << 16 | Red1 << 8 | Blue1; /* 合成绿红蓝 */
        WsDat[0] = color;
        WS_SetAll(); /* 输出 */
        delay_ms ( 5 ); /* 渐变速度 */
    }

    return color; /* 渐变结束 */
}