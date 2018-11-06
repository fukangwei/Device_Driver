#include "STC12C5A.H"
#include "type.h"

sbit DS = P0 ^ 2; /* 定义DS18B20的总线接口 */

static uint temp; /* 存储温度值 */

static void delay1us ( void ) { /* 延时1微秒(较精确，约1.2微秒) */
}

static void dsreset ( void ) { /* DS18B20复位(初始化)函数 */
    uint i;
    DS = 0;
    i = 161;

    while ( i-- > 0 ) {
        delay1us(); /* 480微秒的复位脉冲 */
    }

    DS = 1;
    i = 140;

    while ( i-- > 0 ) {
        delay1us(); /* 等待DS18B20的存在脉冲(480微秒) */
    }
}

static bit tmpreadbit ( void ) { /* 读DS18B20一位数字 */
    uint i;
    bit dat;
    DS = 0; /* 下降沿导致读时序开始 */
    delay1us();
    DS = 1; /* 数据线必须至少保持1微秒，然后总线被释放 */
    i = 10;

    while ( i-- > 0 ) {
        delay1us();
    }

    dat = DS; /* 把控制器采样时间放在15微秒周期的最后 */
    i = 40;

    while ( i-- > 0 ) {
        delay1us(); /* 读时序必须至少为60微秒 */
    }

    return ( dat );
}

static uchar tmpread ( void ) { /* 从DS18B20读一个字节 */
    uchar i, j, dat;
    dat = 0;

    for ( i = 1; i <= 8; i++ ) {
        j = tmpreadbit();
        dat = ( j << 7 ) | ( dat >> 1 );
    }

    return ( dat );
}

static void tmpwritebyte ( uchar dat ) { /* 向DS18B20写一个字节 */
    uint i;
    uchar j;
    bit testb;

    for ( j = 1; j <= 8; j++ ) {
        testb = dat & 0x01;
        dat = dat >> 1;

        if ( testb ) { /* 如果写入的位为1 */
            DS = 0;
            i = 1; /* 至少1微秒的恢复时间 */

            while ( i-- > 0 ) {
                delay1us();
            }

            DS = 1;
            i = 50;

            while ( i-- > 0 ) {
                delay1us(); /* 写时序必须至少持续60微秒 */
            }
        } else { /* 如果写入的位为0 */
            DS = 0;
            i = 50;

            while ( i-- > 0 ) {
                delay1us();
            }

            DS = 1;
            i = 1;

            while ( i-- > 0 ) {
                delay1us();
            }
        }
    }
}

void tmpchange ( void ) { /* DS18B20温度改变 */
    dsreset();
    tmpwritebyte ( 0xcc ); /* 忽略ROM指令 */
    tmpwritebyte ( 0x44 ); /* 温度转换指令 */
}

uint tmp ( void ) { /* 得到DS18B20温度值 */
    float tt;
    uchar a, b;
    dsreset();
    tmpwritebyte ( 0xcc );
    tmpwritebyte ( 0xbe );
    a = tmpread();
    b = tmpread();
    temp = b;
    temp <<= 8; /* 两个字节组成完整的数值 */
    temp = temp | a;
    tt = temp * 0.0625;
    temp = tt * 10 + 0.5; /* 用0.5补偿一下温度 */
    return temp;
}