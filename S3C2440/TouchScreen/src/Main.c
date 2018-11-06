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

int count = 0;
volatile U32 preScaler;
int xdata, ydata;

void Test_Touchpanel ( void );
static void __irq AdcTsAuto ( void );
static void cal_cpu_bus_clk ( void );
void Set_Clk ( void );

void delay ( int times ) {
    int i, j;

    for ( i = 0; i < times; i++ )
        for ( j = 0; j < 400; j++ );
}

int Main ( void ) {
    int Scom = 0;
    Set_Clk();
    MMU_Init();
    Uart_Init ( 0, 115200 );
    Uart_Select ( Scom );
    Test_Touchpanel();

    while ( 1 );

    return 0;
}

void Test_Touchpanel ( void ) { /* 触摸屏初始化 */
    rADCDLY = 50000; /* Normal conversion mode delay about (1/3.6864M) * 50000 = 13.56ms */
    /* 设置AD转频率 */
    preScaler = ADC_FREQ;
    preScaler = 50000000 / ADC_FREQ - 1; /* PCLK = 50M */
    rADCCON = ( 1 << 14 ) | ( preScaler << 6 ); /* ADCPRS En, PRSCVL */
    /* 设置触摸屏为等待中断模式，等待触摸笔被按下 */
    rADCTSC = 0xd3; /* Wfait, XP_PU, XP_Dis, XM_Dis, YP_Dis, YM_En */
    /* clear irq */
    rSRCPND = 0x80000000;
    rINTPND = 0x80000000;
    ClearSubPending ( BIT_SUB_TC );
    pISR_ADC = ( U32 ) AdcTsAuto;
    /* enable INT_TC irq */
    rINTMSK = 0x7fffffff;
    EnableSubIrq ( BIT_SUB_TC );
}

static void __irq AdcTsAuto ( void ) { /* 中断服务程序 */
    U32 saveAdcdly;

    /* -------------- stylus down ------------------- */
    /* 检测子中断源，判断是否为INT_TC中断且触摸笔按下*/
    if ( rSUBSRCPND & ( BIT_SUB_TC ) ) {
        if ( ! ( rADCDAT0 & 0x8000 ) ) {
            Uart_Printf ( "\nStylus down\n" );
        } else {
            Uart_Printf ( "\nStylus up\n" );
        }
    }

    /* “pull-up disable”，自动连续X和Y坐标转换 */
    rADCTSC = ( 1 << 3 ) | ( 1 << 2 );
    saveAdcdly = rADCDLY;
    rADCDLY = 40000; /* Normal conversion mode delay about (1/50M) * 40000 = 0.8ms */
    rADCCON |= 0x1; /* 开始AD转换 */

    while ( rADCCON & 0x1 ); /* check if Enable_start is low */

    while ( ! ( rADCCON & 0x8000 ) ); /* check if EC(End of Conversion) flag is high, This line is necessary! */

    while ( ! ( rSRCPND & 0x80000000 ) ); /* check if ADC is finished with interrupt bit */

    /* 获取X和Y坐标 */
    xdata = ( rADCDAT0 & 0x3ff );
    ydata = ( rADCDAT1 & 0x3ff );
    ClearSubPending ( BIT_SUB_TC );
    rSRCPND = 0x80000000;
    rINTPND = 0x80000000;
    EnableSubIrq ( BIT_SUB_TC );
    rINTMSK = 0x7fffffff;
    /* --------------- stylus down ------------- */
    /* --------------- stylus up --------------- */
    /* 设置触摸屏为等待中断模式，等待触摸笔抬起 */
    rADCTSC = 0xd3; /* Waiting for interrupt */
    rADCTSC = rADCTSC | ( 1 << 8 ); /* Detect stylus up interrupt signal */

    while ( 1 ) { /* to check Pen-up state */
        if ( rSUBSRCPND & ( BIT_SUB_TC ) ) { /* check if ADC is finished with interrupt bit */
            Uart_Printf ( "Stylus Up Interrupt~!\n" );
            break; /* if Stylus is up state */
        }
    }

    /* --------------- stylus up --------------- */
    Uart_Printf ( "count = %03d  XP = %04d, YP = %04d\n", count++, xdata, ydata );
    rADCDLY = saveAdcdly;
    /* 设置触摸屏为等待中断模式，等待下次触摸笔按下 */
    rADCTSC = 0xd3; /* Waiting for interrupt */
    ClearSubPending ( BIT_SUB_TC );
    rSRCPND = 0x80000000;
    rINTPND = 0x80000000;
    EnableSubIrq ( BIT_SUB_TC );
    rINTMSK = 0x7fffffff;
}

void Set_Clk ( void ) {
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