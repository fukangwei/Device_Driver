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

#define ADC_FREQ 2500000

volatile U32 preScaler;
volatile U32 adc_value = 0;

void adc_init ( void );
int ReadAdc ( int channel );
static void cal_cpu_bus_clk ( void );
void Set_Clk ( void );

void delay ( int times ) {
    int i, j;

    for ( i = 0; i < times; i++ )
        for ( j = 0; j < 400; j++ );
}

void Main ( void ) {
    Set_Clk(); /* 时钟初始化 */
    adc_init(); /* adc初始化 */

    while ( 1 ) {
        adc_value = ReadAdc ( 0 ); /* 读取转换值 */
        delay ( 1000 );
    }
}

void adc_init ( void ) { /* adc初始化 */
    int channel = 0; /* 选择输入通道，即AIN0 */
    preScaler = ADC_FREQ; /* 设置分频时钟 */
    preScaler = 50000000 / ADC_FREQ - 1; /* PCLK = 50M */
    rADCCON = ( 1 << 14 ) | ( preScaler << 6 ) | ( channel << 3 ); /* AD转换频率设置，最大频率为2.5MHz */
    delay ( 1000 );
}

int ReadAdc ( int channel ) { /* 获取AD转换后的值 */
    rADCCON |= 0x01; /* 开始AD转换 */

    while ( rADCCON & 0x1 ); /* check if Enable_start is low */

    /* 检查转换是否结束 */
    while ( ! ( rADCCON & 0x8000 ) ); /* check if EC(End of Conversion) flag is high */

    return ( ( int ) rADCDAT0 & 0x3ff );
}

void Set_Clk ( void ) {
    int i;
    U8 key;
    U32 mpll_val = 0;
    i = 2;  /* don't use 100M! */

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
    /* (m + 8) * FIN * 2，不要超出32位数! */
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