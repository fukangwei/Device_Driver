#define GLOBAL_CLK 1
#include <stdlib.h>
#include <string.h>
#include "def.h"
#include "option.h"
#include "2440addr.h"
#include "2440lib.h"
#include "2440slib.h"
#include "mmu.h"
#include "profile.h"
#include "memtest.h"

void Timer0_init ( void );
static void __irq IRQ_Timer0_Handle ( void );
void Set_Clk ( void );
static void cal_cpu_bus_clk ( void );
void Led1_init ( void );
void Led1_run ( void );

void delay ( int times ) {
    int i, j;

    for ( i = 0; i < times; i++ )
        for ( j = 0; j < 400; j++ );
}

void Main ( void ) {
    Set_Clk();
    MMU_Init();
    Led1_init();
    Timer0_init();

    while ( 1 );
}

void Timer0_init ( void ) {
    rTCFG0 = 49; /* “pclk/(49+1)”，即“50MHz/50” */
    rTCFG1 = 0x03;  /* 16分频 = 62500Hz */
    rTCNTB0 = 62500 / 2; /* TCNTB0[15:0] = 计数值 */
    rTCMPB0 = 0;
    rTCON |= ( 1 << 1 ); /* 将计数值装入TCNTB0、TCMPB0 */
    rTCON = 0x09; /* 定时器0自动重载开启，并启动定时器0 */
    ClearPending ( BIT_TIMER0 );
    pISR_TIMER0 = ( U32 ) IRQ_Timer0_Handle;
    EnableIrq ( BIT_TIMER0 );
}

static void __irq IRQ_Timer0_Handle ( void ) { /* 定时器中断处理函数 */
    Led1_run();
    ClearPending ( BIT_TIMER0 );
}

void Set_Clk ( void ) { /* 系统时钟设置函数 */
    int i;
    U8 key;
    U32 mpll_val = 0 ;
    i = 2; /* don't use 100M! */

    switch ( i ) {
        case 0: /* 200 */
            key = 12;
            mpll_val = ( 92 << 12 ) | ( 4 << 4 ) | ( 1 );
            break;

        case 1: /* 300 */
            key = 13;
            mpll_val = ( 67 << 12 ) | ( 1 << 4 ) | ( 1 );
            break;

        case 2: /* 400 */
            key = 14;
            mpll_val = ( 92 << 12 ) | ( 1 << 4 ) | ( 1 );
            break;

        case 3: /* 440 */
            key = 14;
            mpll_val = ( 102 << 12 ) | ( 1 << 4 ) | ( 1 );
            break;

        default:
            key = 14;
            mpll_val = ( 92 << 12 ) | ( 1 << 4 ) | ( 1 );
            break;
    }

    /* init FCLK = 400M, so change MPLL first */
    ChangeMPllValue ( ( mpll_val >> 12 ) & 0xff, ( mpll_val >> 4 ) & 0x3f, mpll_val & 3 ); /* set rMPLLCON */
    ChangeClockDivider ( key, 12 );
    cal_cpu_bus_clk(); /* HCLK = 100M, PCLK = 50M */
}

static void cal_cpu_bus_clk ( void ) {
    static U32 cpu_freq;
    static U32 UPLL;
    U32 val;
    U8 m, p, s;
    val = rMPLLCON;
    m = ( val >> 12 ) & 0xff;
    p = ( val >> 4 ) & 0x3f;
    s = val & 3;
    /* “(m + 8) * FIN * 2”，不要超出32位数! */
    FCLK = ( ( m + 8 ) * ( FIN / 100 ) * 2 ) / ( ( p + 2 ) * ( 1 << s ) ) * 100; /* FCLK = 400M, FIN = 12000000 */
    val = rCLKDIVN;
    m = ( val >> 1 ) & 3;
    p = val & 1;
    val = rCAMDIVN;
    s = val >> 8;

    switch ( m ) {
        case 0:
            HCLK = FCLK;
            break;

        case 1:
            HCLK = FCLK >> 1;
            break;

        case 2:
            if ( s & 2 ) {
                HCLK = FCLK >> 3;
            } else {
                HCLK = FCLK >> 2;
            }

            break;

        case 3:
            if ( s & 1 ) {
                HCLK = FCLK / 6;
            } else {
                HCLK = FCLK / 3;
            }

            break;
    }

    if ( p ) {
        PCLK = HCLK >> 1;
    } else {
        PCLK = HCLK;
    }

    if ( s & 0x10 ) {
        cpu_freq = HCLK;
    } else {
        cpu_freq = FCLK;
    }

    val = rUPLLCON;
    m = ( val >> 12 ) & 0xff;
    p = ( val >> 4 ) & 0x3f;
    s = val & 3;
    UPLL = ( ( m + 8 ) * FIN ) / ( ( p + 2 ) * ( 1 << s ) );
    UCLK = ( rCLKDIVN & 8 ) ? ( UPLL >> 1 ) : UPLL;
}

void Led1_init ( void ) {
    rGPBCON &= ~ ( 0x3 << 10 );
    rGPBCON |=  ( 0x1 << 10 );
}

void Led1_run ( void ) {
    if ( rGPBDAT & ( 1 << 5 ) ) {
        rGPBDAT &= ~ ( 1 << 5 );
    } else {
        rGPBDAT |= ( 1 << 5 );
    }
}