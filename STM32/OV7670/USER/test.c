#include "sys.h"
#include "usart.h"
#include "delay.h"
#include "led.h"
#include "lcd.h"
#include "ov7670.h"
#include "exti.h"
#include "timer.h"

extern u8 ov_sta; /* 在exit.c里面定义 */
extern u8 ov_frame; /* 在timer.c里面定义 */

void camera_refresh ( void ) { /* 更新LCD显示 */
    u32 j;
    u16 color;
#ifdef GREY_DEAL /* 如果需要灰度处理 */
    volatile u16 gray;
    u8 gm_red, gm_green, gm_blue;
#endif

    if ( ov_sta == 2 ) {
        LCD_Scan_Dir ( U2D_L2R ); /* 从上到下，从左到右 */
        LCD_SetCursor ( 0x00, 0x0000 ); /* 设置光标位置 */
        LCD_WriteRAM_Prepare(); /* 开始写入GRAM */
        OV7670_CS = 0;
        OV7670_RRST = 0; /* 开始复位读指针 */
        OV7670_RCK = 0;
        OV7670_RCK = 1;
        OV7670_RCK = 0;
        OV7670_RRST = 1; /* 复位读指针结束 */
        OV7670_RCK = 1;

        for ( j = 0; j < 76800; j++ ) {
            GPIOB->CRL = 0X88888888;
            OV7670_RCK = 0;
            color = OV7670_DATA; /* 读数据 */
            OV7670_RCK = 1;
            color <<= 8;
            OV7670_RCK = 0;
            color |= OV7670_DATA; /* 读数据 */
            OV7670_RCK = 1;
            GPIOB->CRL = 0X33333333;
#ifdef GREY_DEAL /* 如果需要灰度处理 */
            gm_red = ( color & 0xF800 ) >> 8;
            gm_green = ( color & 0x07E0 ) >> 3;
            gm_blue = ( color & 0x001F ) << 3;
            gray = ( gm_red * 77 + gm_green * 150 + gm_blue * 29 + 128 ) / 256;
            gray = gray / 8;
            color = ( 0x001f & gray ) << 11;
            color = color | ( ( ( 0x003f ) & ( gray * 2 ) ) << 5 );
            color = color | ( 0x001f & gray );
#endif
            LCD_WR_DATA ( color );
        }

        OV7670_CS = 1;
        OV7670_RCK = 0;
        OV7670_RCK = 1;
        EXTI->PR = 1 << 15; /* 清除LINE8上的中断标志位 */
        ov_sta = 0; /* 开始下一次采集 */
        ov_frame++;
        LCD_Scan_Dir ( DFT_SCAN_DIR ); /* 恢复默认扫描方向 */
    }
}

int main ( void ) {
    u8 i;
    Stm32_Clock_Init ( 16 ); /* 对STM32进行超频处理 */
    uart_init ( 128, 9600 );
    delay_init ( 128 );
    OV7670_Init();
    LED_Init();
    LCD_Init();

    if ( lcddev.id == 0X6804 || lcddev.id == 0X5310 || lcddev.id == 0X5510 ) { /* 强制设置屏幕分辨率为“320*240”，以支持3.5寸大屏 */
        lcddev.width = 240;
        lcddev.height = 320;
    }

    POINT_COLOR = RED; /* 设置字体为红色 */
    LCD_ShowString ( 60, 50, 200, 200, 16, "Mini STM32" );
    LCD_ShowString ( 60, 70, 200, 200, 16, "OV7670 TEST" );
    LCD_ShowString ( 60, 90, 200, 200, 16, "ATOM@ALIENTEK" );
    LCD_ShowString ( 60, 110, 200, 200, 16, "2014/3/27" );
    LCD_ShowString ( 60, 150, 200, 200, 16, "OV7670 Init..." );

    while ( OV7670_Init() ) { /* 初始化OV7670 */
        LCD_ShowString ( 60, 150, 200, 200, 16, "OV7670 Error!!" );
        delay_ms ( 200 );
        LCD_Fill ( 60, 150, 239, 166, WHITE );
        delay_ms ( 200 );
    }

    LCD_ShowString ( 60, 150, 200, 200, 16, "OV7670 Init OK" );
    delay_ms ( 1500 );
    TIM3_Int_Init ( 10000, 7199 ); /* TIM3，10Khz计数频率，1秒钟中断 */
    EXTI15_Init(); /* 使能定时器捕获 */
    OV7670_Window_Set ( 10, 174, 240, 320 ); /* 设置窗口 */
    OV7670_CS = 0;

    while ( 1 ) {
        camera_refresh(); /* 更新显示 */

        if ( i != ov_frame ) {
            i = ov_frame;
            LED0 = !LED0;
        }
    }
}