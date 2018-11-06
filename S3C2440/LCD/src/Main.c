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

#define LCD_N35 /* NEC/256Kɫ/��240*320��/3.5Ӣ��TFT���Һ������ÿһ��ˮƽ���ϰ���240�����ص㣬����320���������� */

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

volatile static unsigned short LCD_BUFFER[SCR_YSIZE][SCR_XSIZE]; /* ����320�У�240�е����飬���ڴ����ʾ���� */

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

static void Pait_Bmp ( int x0, int y0, int h, int l, const unsigned char *bmp ) { /* ����Ļ�ϻ�ͼ */
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

static void Lcd_ClearScr ( U16 c ) { /* ���ȫ��Ϊĳһ��ɫ */
    unsigned int x, y ;

    for ( y = 0 ; y < SCR_YSIZE ; y++ ) {
        for ( x = 0 ; x < SCR_XSIZE ; x++ ) {
            LCD_BUFFER[y][x] = c ;
        }
    }
}

static void Lcd_EnvidOnOff ( int onoff ) { /* LCD���� */
    if ( onoff == 1 ) {
        rLCDCON1 |= 1; /* ENVID = ON */
    } else {
        rLCDCON1 = rLCDCON1 & 0x3fffe; /* ENVID = Off */
    }
}

static void Lcd_Port_Init ( void ) { /* �˿ڳ�ʼ�� */
    rGPCUP = 0xffffffff; /* Disable Pull-up register */
    rGPCCON = 0xaaaa02a8; /* Initialize VD[7:0], VM, VFRAME, VLINE, VCLK */
    rGPDUP = 0xffffffff; /* Disable Pull-up register */
    rGPDCON = 0xaaaaaaaa; /* Initialize VD[15:8] */
}

static void LCD_Init ( void ) { /* LCD��ʼ�� */
    Lcd_Port_Init();
    /* ��ʾģʽ��ʼ�� */
    /* bit[17:8](4:CLKVAL); bit[6:5](11:TFT LCD panel); bit[4:1](1100:16 bpp for TFT) */
    rLCDCON1 = ( LCD_PIXCLOCK << 8 ) | ( 3 <<  5 ) | ( 12 << 1 );
    /* bit[31:24](1:VBPD); bit[23:14](320-1:����); bit[13:6](5:VFPD); bit[5:0](1:VSPW) */
    rLCDCON2 = ( LCD_UPPER_MARGIN << 24 ) | ( ( LCD_HEIGHT - 1 ) << 14 ) | ( LCD_LOWER_MARGIN << 6 ) | ( LCD_VSYNC_LEN << 0 );
    /* bit[25:19](36:HBPD); bit[18:8](240-1:����); bit[7:0](19:HFPD) */
    rLCDCON3 = ( LCD_RIGHT_MARGIN << 19 ) | ( ( LCD_WIDTH  - 1 ) <<  8 ) | ( LCD_LEFT_MARGIN << 0 );
    /* bit[15:8](13:MVAL��ֻ�е���LCDCON1 bit[7](MMODE) = 1������Ч); bit[7:0](5:HSPW) */
    rLCDCON4 = ( 13 <<  8 ) | ( LCD_HSYNC_LEN << 0 );
    /* bit[11](5:6:5 Format); bit[9](VLINE/HSYNC polarity inverted); bit[8](VFRAME/VSYNC inverted); bit[3](Enalbe PWERN signal); bit[0](half-word swap control bit) */
    rLCDCON5   =  ( 1 << 11 ) | ( 1 << 9 ) | ( 1 << 8 ) | ( 1 << 3 ) | ( 1 << 0 );
    /* ֡�����ַ��ʼ�� */
    /* LCDBANK����Ƶ֡�������ڴ��ַ30-22λ��LCDBASEU����Ƶ֡�������Ŀ�ʼ��ַ21-1λ��LCDBASEL����Ƶ֡�������Ľ�����ַ21-1λ */
    rLCDSADDR1 = ( ( LCD_ADDR >> 22 ) << 21 ) | ( ( M5D ( LCD_ADDR >> 1 ) ) <<  0 ); /* bit[29:21]:LCDBANK, bit[20:0]:LCDBASEU */
    rLCDSADDR2 = M5D ( ( LCD_ADDR + LCD_WIDTH * LCD_HEIGHT * 2 ) >> 1 ); /* bit[20:0]:LCDBASEL */
    /* PAGEWIDTH��������Ļһ�е��ֽ����������ʹ��������Ļ������Ϊʵ����Ļ�����ֽ�����OFFSIZE:������Ļ���ƫ�Ƶ��ֽ����������ʹ��������Ļ������Ϊ0 */
    rLCDSADDR3 = LCD_WIDTH; /* bit[21:11]:OFFSIZE; bit[10:0]:PAGEWIDTH */
    rLCDINTMSK |= 3; /* �����ж� */
    rTPAL = 0x0; /* disable��ɫ�� */
    rTCONSEL &= ~ ( ( 1 << 4 ) | 1 ); /* ��ֹLPC3600/LCC3600ģʽ */
    Lcd_EnvidOnOff ( 1 ); /* ��LCD */
}

void TFT_LCD_Show ( void ) {
    /* ��(255:0:0)����(0:255:0)����(0:0:255)����(0:0:0)����(255,255,255) */
    /* ����Ļ����ʾ����ɫ */
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
    Pait_Bmp ( 0, 0, 240, 320, sunflower_240x320 ); /* ��ʾһ��ͼƬ����Ļ�� */
}

int Main ( void ) {
    PCLK = 50000000;
    LCD_Init();
    TFT_LCD_Show();
    return 0;
}