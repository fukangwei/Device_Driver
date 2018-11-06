#define GLOBAL_CLK  1
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

#define LCD_N35 /* NEC/256K色/“240*320”/3.5英寸TFT真彩液晶屏，每一条水平线上包含240个像素点，共有320条这样的线 */

#if defined(LCD_N35)
    #define LCD_WIDTH 240
    #define LCD_HEIGHT 320
    #define LCD_PIXCLOCK 4

    #define LCD_RIGHT_MARGIN 39
    #define LCD_LEFT_MARGIN 16
    #define LCD_HSYNC_LEN 5

    #define LCD_UPPER_MARGIN 1
    #define LCD_LOWER_MARGIN 5
    #define LCD_VSYNC_LEN 1
#endif

void TFT_LCD_Test ( void );

#define LCD_XSIZE  LCD_WIDTH
#define LCD_YSIZE  LCD_HEIGHT
#define SCR_XSIZE  LCD_WIDTH
#define SCR_YSIZE  LCD_HEIGHT

volatile static unsigned short LCD_BUFFER[SCR_YSIZE][SCR_XSIZE]; /* 定义320行，240列的数组，用于存放显示数据 */

extern unsigned char sunflower_240x320[];

#define M5D(n)  ((n)&0x1fffff)
#define LCD_ADDR ((U32)LCD_BUFFER)

#define ADC_FREQ 2500000

volatile U32 preScaler;

static void cal_cpu_bus_clk ( void );
void Set_Clk ( void );

void delay ( int times ) {
    int i, j;

    for ( i = 0; i < times; i++ )
        for ( j = 0; j < 400; j++ );
}

static void Pait_Bmp ( int x0, int y0, int h, int l, const unsigned char *bmp ) { /* 在屏幕上画图 */
    int x, y;
    U32 c;
    int p = 0;

    for ( y = 0; y < l; y++ ) {
        for ( x = 0; x < h; x++ ) {
            c = bmp[p + 1] | ( bmp[p] << 8 );

            if ( ( ( x0 + x ) < SCR_XSIZE ) && ( ( y0 + y ) < SCR_YSIZE ) ) {
                LCD_BUFFER[y0 + y][x0 + x] = c;
            }

            p = p + 2;
        }
    }
}

static void Lcd_ClearScr ( U16 c ) { /* 填充全屏为某一颜色 */
    unsigned int x, y ;

    for ( y = 0 ; y < SCR_YSIZE ; y++ ) {
        for ( x = 0 ; x < SCR_XSIZE ; x++ ) {
            LCD_BUFFER[y][x] = c ;
        }
    }
}

static void Lcd_EnvidOnOff ( int onoff ) { /* LCD开关 */
    if ( onoff == 1 ) {
        rLCDCON1 |= 1; /* ENVID = ON */
    } else {
        rLCDCON1 = rLCDCON1 & 0x3fffe; /* ENVID = Off */
    }
}

static void Lcd_Port_Init ( void ) { /* 端口初始化 */
    rGPCUP = 0xffffffff; /* Disable Pull-up register */
    rGPCCON = 0xaaaa02a8; /* Initialize VD[7:0], VM, VFRAME, VLINE, VCLK */
    rGPDUP = 0xffffffff; /* Disable Pull-up register */
    rGPDCON = 0xaaaaaaaa; /* Initialize VD[15:8] */
}

static void LCD_Init ( void ) { /* LCD初始化 */
    Lcd_Port_Init();
    /* 显示模式初始化 */
    /* bit[17:8](4:CLKVAL); bit[6:5](11:TFT LCD panel); bit[4:1](1100:16 bpp for TFT) */
    rLCDCON1 = ( LCD_PIXCLOCK << 8 ) | ( 3 <<  5 ) | ( 12 << 1 );
    /* bit[31:24](1:VBPD); bit[23:14](320-1:行数); bit[13:6](5:VFPD); bit[5:0](1:VSPW) */
    rLCDCON2 = ( LCD_UPPER_MARGIN << 24 ) | ( ( LCD_HEIGHT - 1 ) << 14 ) | ( LCD_LOWER_MARGIN << 6 ) | ( LCD_VSYNC_LEN << 0 );
    /* bit[25:19](36:HBPD); bit[18:8](240-1:列数); bit[7:0](19:HFPD) */
    rLCDCON3 = ( LCD_RIGHT_MARGIN << 19 ) | ( ( LCD_WIDTH  - 1 ) <<  8 ) | ( LCD_LEFT_MARGIN << 0 );
    /* bit[15:8](13:MVAL，只有当“LCDCON1 bit[7](MMODE) = 1”才有效); bit[7:0](5:HSPW) */
    rLCDCON4 = ( 13 <<  8 ) | ( LCD_HSYNC_LEN << 0 );
    /* bit[11](5:6:5 Format); bit[9](VLINE/HSYNC polarity inverted); bit[8](VFRAME/VSYNC inverted); bit[3](Enalbe PWERN signal); bit[0](half-word swap control bit) */
    rLCDCON5   =  ( 1 << 11 ) | ( 1 << 9 ) | ( 1 << 8 ) | ( 1 << 3 ) | ( 1 << 0 );
    /* 帧缓冲地址初始化 */
    /* LCDBANK：视频帧缓冲区内存地址30-22位；LCDBASEU：视频帧缓冲区的开始地址21-1位；LCDBASEL：视频帧缓冲区的结束地址21-1位 */
    rLCDSADDR1 = ( ( LCD_ADDR >> 22 ) << 21 ) | ( ( M5D ( LCD_ADDR >> 1 ) ) <<  0 ); /* bit[29:21]:LCDBANK, bit[20:0]:LCDBASEU */
    rLCDSADDR2 = M5D ( ( LCD_ADDR + LCD_WIDTH * LCD_HEIGHT * 2 ) >> 1 ); /* bit[20:0]:LCDBASEL */
    /* PAGEWIDTH：虚拟屏幕一行的字节数，如果不使用虚拟屏幕，设置为实际屏幕的行字节数；OFFSIZE:虚拟屏幕左侧偏移的字节数，如果不使用虚拟屏幕，设置为0 */
    rLCDSADDR3 = LCD_WIDTH; /* bit[21:11]:OFFSIZE; bit[10:0]:PAGEWIDTH */
    rLCDINTMSK |= 3; /* 屏蔽中断 */
    rTPAL = 0x0; /* disable调色板 */
    rTCONSEL &= ~ ( ( 1 << 4 ) | 1 ); /* 禁止LPC3600/LCC3600模式 */
    Lcd_EnvidOnOff ( 1 ); /* 打开LCD */
}

void TFT_LCD_Show ( void ) {
    /* 红(255:0:0)；绿(0:255:0)；蓝(0:0:255)；黑(0:0:0)；白(255,255,255) */
    /* 在屏幕上显示三基色 */
    Lcd_ClearScr ( ( 0x00 << 11 ) | ( 0x00 << 5 ) | ( 0x00 ) ); /* black */
    delay ( 1000 );
    Lcd_ClearScr ( ( 0x1f << 11 ) | ( 0x00 << 5 ) | ( 0x00 ) ); /* red */
    delay ( 1000 );
    Lcd_ClearScr ( ( 0x00 << 11 ) | ( 0x3f << 5 ) | ( 0x00 ) ); /* green */
    delay ( 1000 );
    Lcd_ClearScr ( ( 0x00 << 11 ) | ( 0x00 << 5 ) | ( 0x1f ) ); /* blue */
    delay ( 1000 );
    Lcd_ClearScr ( ( 0x1f << 11 ) | ( 0x3f << 5 ) | ( 0x1f ) ); /* white */
    delay ( 1000 );
    Pait_Bmp ( 0, 0, 240, 320, sunflower_240x320 ); /* 显示一幅图片在屏幕上 */
}

int Main ( void ) {
    PCLK = 50000000;
    LCD_Init();
    TFT_LCD_Show();
    return 0;
}