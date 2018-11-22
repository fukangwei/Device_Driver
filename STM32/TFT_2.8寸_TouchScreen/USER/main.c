#include "led.h"
#include "delay.h"
#include "sys.h"
#include "key.h"
#include "usart.h"
#include "lcd.h"
#include "touch.h"

void Load_Drow_Dialog ( void ) {
    LCD_Clear ( WHITE ); /* 清屏 */
    POINT_COLOR = BLUE; /* 设置字体为蓝色 */
    LCD_ShowString ( 216, 0, "RST" ); /* 显示清屏区域 */
    POINT_COLOR = RED; /* 设置画笔蓝色 */
}

int main ( void ) {
    u8 key;
    u8 i = 0;
    SystemInit();
    delay_init ( 72 );
    NVIC_Configuration();
    uart_init ( 9600 );
    LED_Init();
    KEY_Init();
    LCD_Init();
    POINT_COLOR = RED;
    LCD_ShowString ( 60, 50, "Mini STM32" );
    LCD_ShowString ( 60, 70, "TOUCH TEST" );
    LCD_ShowString ( 60, 90, "ATOM@ALIENTEK" );
    LCD_ShowString ( 60, 110, "2011/1/1" );
    LCD_ShowString ( 60, 130, "Press KEY0 to Adjust" );
    Touch_Init();
    delay_ms ( 1500 );
    Load_Drow_Dialog();

    while ( 1 ) {
        key = KEY_Scan();

        if ( Pen_Point.Key_Sta == Key_Down ) { /* 触摸屏被按下 */
            Pen_Int_Set ( 0 ); /* 关闭中断 */

            do {
                Convert_Pos();
                Pen_Point.Key_Sta = Key_Up;

                if ( Pen_Point.X0 > 216 && Pen_Point.Y0 < 16 ) {
                    Load_Drow_Dialog();
                } else {
                    Draw_Big_Point ( Pen_Point.X0, Pen_Point.Y0 );
                    GPIOC->ODR |= 1 << 1;
                }
            } while ( PEN == 0 );

            Pen_Int_Set ( 1 );
        } else {
            delay_ms ( 10 );
        }

        if ( key == 1 ) {
            LCD_Clear ( WHITE );
            Touch_Adjust();
            Save_Adjdata();
            Load_Drow_Dialog();
        }

        i++;

        if ( i == 20 ) {
            i = 0;
            LED0 = !LED0;
        }
    };
}