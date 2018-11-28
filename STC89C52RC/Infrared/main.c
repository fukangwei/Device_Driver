#include <reg52.h>

sbit IR = P3 ^ 2; /* 红外接口标志 */

#define DataPort P0 /* 定义数据端口 */
sbit LATCH1 = P2 ^ 6; /* 定义锁存使能端口，段锁存 */
sbit LATCH2 = P2 ^ 7; /* 定义锁存使能端口，位锁存 */

unsigned char code dofly_DuanMa[10] = {0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7d, 0x07, 0x7f, 0x6f}; /* 显示段码值0至9 */
unsigned char irtime; /* 红外用全局变量 */

bit irpro_ok, irok;
unsigned char IRcord[4];
unsigned char irdata[33];

void Ir_work ( void );
void Ircordpro ( void );

void TIM0init ( void ) { /* 定时器0初始化 */
    TMOD = 0x02; /* 定时器0工作方式2，TH0是重装值，TL0是初值 */
    TH0  = 0x00; /* 重载值 */
    TL0  = 0x00; /* 初始化值 */
    ET0  = 1; /* 开中断 */
    TR0  = 1;
}

void EX0init ( void ) { /* 外部中断0初始化 */
    IT0 = 1; /* 指定外部中断0下降沿触发，INT0(P3.2) */
    EX0 = 1; /* 使能外部中断 */
    EA = 1; /* 开总中断 */
}

void tim0_isr ( void ) interrupt 1 using 1 { /* 定时器0中断处理 */
    irtime++; /* 用于计数2个下降沿之间的时间 */
}

void EX0_ISR ( void ) interrupt 0 { /* 外部中断0中断处理 */
    static unsigned char i = 0; /* 接收红外信号处理 */
    static bit startflag = 0; /* 是否开始处理标志位 */

    if ( startflag ) {
        /* 引导码即TC9012的头码，9ms高电平加上4.5ms低电平，实际上判断低电平有没有达到4.5ms(外部中断0设置为下降沿触发)。若达到，则接收开始 */
        if ( irtime < 63 && irtime >= 40 ) {
            i = 0;
        }

        irdata[i] = irtime; /* 存储每个电平的持续时间，用于以后判断是0还是1 */
        irtime = 0;
        i++;

        if ( i == 33 ) {
            irok = 1;
            i = 0;
        }
    } else {
        irtime = 0;
        startflag = 1;
    }
}

void Ir_work ( void ) { /* 红外键值散转程序 */
    switch ( IRcord[2] ) { /* 判断第三个数码值 */
        /* 显示相应的按键值 */
        case 0x0c:
            DataPort = dofly_DuanMa[1];
            break;

        case 0x18:
            DataPort = dofly_DuanMa[2];
            break;

        case 0x5e:
            DataPort = dofly_DuanMa[3];
            break;

        case 0x08:
            DataPort = dofly_DuanMa[4];
            break;

        case 0x1c:
            DataPort = dofly_DuanMa[5];
            break;

        case 0x5a:
            DataPort = dofly_DuanMa[6];
            break;

        case 0x42:
            DataPort = dofly_DuanMa[7];
            break;

        case 0x52:
            DataPort = dofly_DuanMa[8];
            break;

        case 0x4a:
            DataPort = dofly_DuanMa[9];
            break;

        default:
            break;
    }
}

void Ircordpro ( void ) { /* 红外码值处理 */
    unsigned char i, j, k;
    unsigned char cord, value = 0;
    k = 1;

    for ( i = 0; i < 4; i++ ) { /* 处理4个字节 */
        for ( j = 1; j <= 8; j++ ) { /* 处理1个字节的8位 */
            cord = irdata[k];

            if ( cord > 7 ) { /* 大于某值时为1，这个和晶振有关系，这里使用12M计算，此值可以有一定误差 */
                value |= 0x80;
            }

            if ( j < 8 ) {
                value >>= 1;
            }

            k++;
        }

        IRcord[i] = value;
        value = 0;
    }

    irpro_ok = 1; /* 处理完毕标志位置1 */
}

void main ( void ) {
    EX0init();  /* 初始化外部中断 */
    TIM0init(); /* 初始化定时器 */
    DataPort = 0xfe; /* 取位码，第一位数码管选通，即二进制1111_1110 */
    LATCH2 = 1;
    LATCH2 = 0;

    while ( 1 ) {
        if ( irok ) { /* 如果接收好了就进行红外处理 */
            Ircordpro();
            irok = 0;
        }

        if ( irpro_ok ) { /* 如果红外处理好后就进行工作处理，如按对应的按键后显示对应的数字等 */
            Ir_work();
            irpro_ok = 0; /* 处理完成标志 */
        }
    }
}