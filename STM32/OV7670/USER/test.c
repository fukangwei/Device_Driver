#include "sys.h"
#include "usart.h"
#include "delay.h"
#include "led.h"
#include "lcd.h"
#include "ov7670.h"
#include "exti.h"
#include "timer.h"

extern u8 ov_sta; /* ��exit.c���涨�� */
extern u8 ov_frame; /* ��timer.c���涨�� */

void camera_refresh ( void ) { /* ����LCD��ʾ */
    u32 j;
    u16 color;
#ifdef GREY_DEAL /* �����Ҫ�Ҷȴ��� */
    volatile u16 gray;
    u8 gm_red, gm_green, gm_blue;
#endif

    if ( ov_sta == 2 ) {
        LCD_Scan_Dir ( U2D_L2R ); /* ���ϵ��£������� */
        LCD_SetCursor ( 0x00, 0x0000 ); /* ���ù��λ�� */
        LCD_WriteRAM_Prepare(); /* ��ʼд��GRAM */
        OV7670_CS = 0;
        OV7670_RRST = 0; /* ��ʼ��λ��ָ�� */
        OV7670_RCK = 0;
        OV7670_RCK = 1;
        OV7670_RCK = 0;
        OV7670_RRST = 1; /* ��λ��ָ����� */
        OV7670_RCK = 1;

        for ( j = 0; j < 76800; j++ ) {
            GPIOB->CRL = 0X88888888;
            OV7670_RCK = 0;
            color = OV7670_DATA; /* ������ */
            OV7670_RCK = 1;
            color <<= 8;
            OV7670_RCK = 0;
            color |= OV7670_DATA; /* ������ */
            OV7670_RCK = 1;
            GPIOB->CRL = 0X33333333;
#ifdef GREY_DEAL /* �����Ҫ�Ҷȴ��� */
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
        EXTI->PR = 1 << 15; /* ���LINE8�ϵ��жϱ�־λ */
        ov_sta = 0; /* ��ʼ��һ�βɼ� */
        ov_frame++;
        LCD_Scan_Dir ( DFT_SCAN_DIR ); /* �ָ�Ĭ��ɨ�跽�� */
    }
}

int main ( void ) {
    u8 i;
    Stm32_Clock_Init ( 16 ); /* ��STM32���г�Ƶ���� */
    uart_init ( 128, 9600 );
    delay_init ( 128 );
    OV7670_Init();
    LED_Init();
    LCD_Init();

    if ( lcddev.id == 0X6804 || lcddev.id == 0X5310 || lcddev.id == 0X5510 ) { /* ǿ��������Ļ�ֱ���Ϊ��320*240������֧��3.5����� */
        lcddev.width = 240;
        lcddev.height = 320;
    }

    POINT_COLOR = RED; /* ��������Ϊ��ɫ */
    LCD_ShowString ( 60, 50, 200, 200, 16, "Mini STM32" );
    LCD_ShowString ( 60, 70, 200, 200, 16, "OV7670 TEST" );
    LCD_ShowString ( 60, 90, 200, 200, 16, "ATOM@ALIENTEK" );
    LCD_ShowString ( 60, 110, 200, 200, 16, "2014/3/27" );
    LCD_ShowString ( 60, 150, 200, 200, 16, "OV7670 Init..." );

    while ( OV7670_Init() ) { /* ��ʼ��OV7670 */
        LCD_ShowString ( 60, 150, 200, 200, 16, "OV7670 Error!!" );
        delay_ms ( 200 );
        LCD_Fill ( 60, 150, 239, 166, WHITE );
        delay_ms ( 200 );
    }

    LCD_ShowString ( 60, 150, 200, 200, 16, "OV7670 Init OK" );
    delay_ms ( 1500 );
    TIM3_Int_Init ( 10000, 7199 ); /* TIM3��10Khz����Ƶ�ʣ�1�����ж� */
    EXTI15_Init(); /* ʹ�ܶ�ʱ������ */
    OV7670_Window_Set ( 10, 174, 240, 320 ); /* ���ô��� */
    OV7670_CS = 0;

    while ( 1 ) {
        camera_refresh(); /* ������ʾ */

        if ( i != ov_frame ) {
            i = ov_frame;
            LED0 = !LED0;
        }
    }
}