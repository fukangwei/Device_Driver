#include "touch.h"
#include "lcd.h"
#include "delay.h"
#include "stdlib.h"
#include "math.h"
#include "24cxx.h"

Pen_Holder Pen_Point;

u8 CMD_RDX = 0XD0;
u8 CMD_RDY = 0X90;

void ADS_Write_Byte ( u8 num ) {
    u8 count = 0;

    for ( count = 0; count < 8; count++ ) {
        if ( num & 0x80 ) {
            TDIN = 1;
        } else {
            TDIN = 0;
        }

        num <<= 1;
        TCLK = 0; /* 上升沿有效 */
        TCLK = 1;
    }
}

u16 ADS_Read_AD ( u8 CMD ) {
    u8 count = 0;
    u16 Num = 0;
    TCLK = 0;
    TCS = 0;
    ADS_Write_Byte ( CMD );
    delay_us ( 6 );
    TCLK = 1;
    TCLK = 0;

    for ( count = 0; count < 15; count++ ) {
        Num <<= 1;
        TCLK = 1;
        TCLK = 0; /* 下降沿有效 */

        if ( DOUT ) {
            Num++;
        }
    }

    Num >>= 3; /* 只有高12位有效 */
    TCS = 1;
    return ( Num );
}

/* 连续读取READ_TIMES次数据，对这些数据升序排列，然后去掉最低和最高LOST_VAL个数，取平均值 */
#define READ_TIMES 15 /* 读取次数 */
#define LOST_VAL   5  /* 丢弃值 */

u16 ADS_Read_XY ( u8 xy ) {
    u16 i, j;
    u16 buf[READ_TIMES];
    u16 sum = 0;
    u16 temp;

    for ( i = 0; i < READ_TIMES; i++ ) {
        buf[i] = ADS_Read_AD ( xy );
    }

    for ( i = 0; i < READ_TIMES - 1; i++ ) {
        for ( j = i + 1; j < READ_TIMES; j++ ) {
            if ( buf[i] > buf[j] ) {
                temp = buf[i];
                buf[i] = buf[j];
                buf[j] = temp;
            }
        }
    }

    sum = 0;

    for ( i = LOST_VAL; i < READ_TIMES - LOST_VAL; i++ ) {
        sum += buf[i];
    }

    temp = sum / ( READ_TIMES - 2 * LOST_VAL );
    return temp;
}

u8 Read_ADS ( u16 *x, u16 *y ) { /* 带滤波的坐标读取 */
    u16 xtemp, ytemp;
    xtemp = ADS_Read_XY ( CMD_RDX );
    ytemp = ADS_Read_XY ( CMD_RDY );

    if ( xtemp < 100 || ytemp < 100 ) {
        return 0; /* 读数失败 */
    }

    *x = xtemp;
    *y = ytemp;
    return 1; /* 读数成功 */
}

#define ERR_RANGE 50 /* 误差范围 */

/* 连续2次读取ADS7846，且这两次的偏差不能超过ERR_RANGE。满足条件则认为读数正确，否则读数错误 */
u8 Read_ADS2 ( u16 *x, u16 *y ) {
    u16 x1, y1;
    u16 x2, y2;
    u8 flag;
    flag = Read_ADS ( &x1, &y1 );

    if ( flag == 0 ) {
        return ( 0 );
    }

    flag = Read_ADS ( &x2, &y2 );

    if ( flag == 0 ) {
        return ( 0 );
    }

    if ( ( ( x2 <= x1 && x1 < x2 + ERR_RANGE ) || ( x1 <= x2 && x2 < x1 + ERR_RANGE ) ) /* 前后两次采样在“+/-50”内 */
         && ( ( y2 <= y1 && y1 < y2 + ERR_RANGE ) || ( y1 <= y2 && y2 < y1 + ERR_RANGE ) ) ) {
        *x = ( x1 + x2 ) / 2;
        *y = ( y1 + y2 ) / 2;
        return 1;
    } else {
        return 0;
    }
}

u8 Read_TP_Once ( void ) { /* 读取一次坐标值 */
    u8 t = 0;
    Pen_Int_Set ( 0 ); /* 关闭中断 */
    Pen_Point.Key_Sta = Key_Up;
    Read_ADS2 ( &Pen_Point.X, &Pen_Point.Y );

    while ( PEN == 0 && t <= 250 ) {
        t++;
        delay_ms ( 10 );
    };

    Pen_Int_Set ( 1 ); /* 开启中断 */

    if ( t >= 250 ) {
        return 0;
    } else {
        return 1;
    }
}

void Drow_Touch_Point ( u8 x, u16 y ) { /* 画一个触摸点 */
    LCD_DrawLine ( x - 12, y, x + 13, y ); /* 横线 */
    LCD_DrawLine ( x, y - 12, x, y + 13 ); /* 竖线 */
    LCD_DrawPoint ( x + 1, y + 1 );
    LCD_DrawPoint ( x - 1, y + 1 );
    LCD_DrawPoint ( x + 1, y - 1 );
    LCD_DrawPoint ( x - 1, y - 1 );
    Draw_Circle ( x, y, 6 ); /* 画中心圈 */
}

void Draw_Big_Point ( u8 x, u16 y ) {
    LCD_DrawPoint ( x, y );
    LCD_DrawPoint ( x + 1, y );
    LCD_DrawPoint ( x, y + 1 );
    LCD_DrawPoint ( x + 1, y + 1 );
}

void Convert_Pos ( void ) { /* 根据触摸屏的校准参数来决定转换后的结果，保存在X0、Y0中 */
    if ( Read_ADS2 ( &Pen_Point.X, &Pen_Point.Y ) ) {
        Pen_Point.X0 = Pen_Point.xfac * Pen_Point.X + Pen_Point.xoff;
        Pen_Point.Y0 = Pen_Point.yfac * Pen_Point.Y + Pen_Point.yoff;
    }
}

void EXTI1_IRQHandler ( void ) { /* 中断处理函数，检测到PEN脚的一个下降沿 */
    Pen_Point.Key_Sta = Key_Down; /* 按键按下 */
    EXTI->PR = 1 << 1; /* 清除LINE1上的中断标志位 */
}

void Pen_Int_Set ( u8 en ) { /* PEN中断设置 */
    if ( en ) {
        EXTI->IMR |= 1 << 1; /* 开启line1上的中断 */
    } else {
        EXTI->IMR &= ~ ( 1 << 1 ); /* 关闭line1上的中断 */
    }
}

#ifdef ADJ_SAVE_ENABLE /* 此部分涉及到使用外部EEPROM，如果没有外部EEPROM，屏蔽此部分即可 */
#define SAVE_ADDR_BASE 40 /* 保存在EEPROM里面的地址区间基址，占用13个字节(RANGE：SAVE_ADDR_BASE至SAVE_ADDR_BASE+12) */

void Save_Adjdata ( void ) { /* 保存校准参数 */
    s32 temp;
    /* 保存校正结果 */
    temp = Pen_Point.xfac * 100000000; /* 保存x校正因素 */
    AT24CXX_WriteLenByte ( SAVE_ADDR_BASE, temp, 4 );
    temp = Pen_Point.yfac * 100000000; /* 保存y校正因素 */
    AT24CXX_WriteLenByte ( SAVE_ADDR_BASE + 4, temp, 4 );
    AT24CXX_WriteLenByte ( SAVE_ADDR_BASE + 8, Pen_Point.xoff, 2 ); /* 保存x偏移量 */
    AT24CXX_WriteLenByte ( SAVE_ADDR_BASE + 10, Pen_Point.yoff, 2 ); /* 保存y偏移量 */
    AT24CXX_WriteOneByte ( SAVE_ADDR_BASE + 12, Pen_Point.touchtype ); /* 保存触屏类型 */
    temp = 0X0A; /* 标记校准过了 */
    AT24CXX_WriteOneByte ( SAVE_ADDR_BASE + 13, temp );
}

u8 Get_Adjdata ( void ) { /* 得到保存在EEPROM里面的校准值。返回1表示成功获取数据；0表示获取失败，要重新校准 */
    s32 tempfac;
    tempfac = AT24CXX_ReadOneByte ( SAVE_ADDR_BASE + 13 ); /* 读取标记字，看是否校准过 */

    if ( tempfac == 0X0A ) { /* 触摸屏已经校准过了 */
        tempfac = AT24CXX_ReadLenByte ( SAVE_ADDR_BASE, 4 );
        Pen_Point.xfac = ( float ) tempfac / 100000000; /* 得到x校准参数 */
        tempfac = AT24CXX_ReadLenByte ( SAVE_ADDR_BASE + 4, 4 );
        Pen_Point.yfac = ( float ) tempfac / 100000000; /* 得到y校准参数 */
        Pen_Point.xoff = AT24CXX_ReadLenByte ( SAVE_ADDR_BASE + 8, 2 ); /* 得到x偏移量 */
        Pen_Point.yoff = AT24CXX_ReadLenByte ( SAVE_ADDR_BASE + 10, 2 ); /* 得到y偏移量 */
        Pen_Point.touchtype = AT24CXX_ReadOneByte ( SAVE_ADDR_BASE + 12 ); /* 读取触屏类型标记 */

        if ( Pen_Point.touchtype ) { /* X、Y方向与屏幕相反 */
            CMD_RDX = 0X90;
            CMD_RDY = 0XD0;
        } else { /* X、Y方向与屏幕相同 */
            CMD_RDX = 0XD0;
            CMD_RDY = 0X90;
        }

        return 1;
    }

    return 0;
}
#endif

void ADJ_INFO_SHOW ( u8 *str ) {
    LCD_ShowString ( 40, 40, "x1:       y1:       " );
    LCD_ShowString ( 40, 60, "x2:       y2:       " );
    LCD_ShowString ( 40, 80, "x3:       y3:       " );
    LCD_ShowString ( 40, 100, "x4:       y4:       " );
    LCD_ShowString ( 40, 100, "x4:       y4:       " );
    LCD_ShowString ( 40, 120, str );
}

void Touch_Adjust ( void ) { /* 触摸屏校准代码，得到四个校准参数 */
    signed short pos_temp[4][2]; /* 坐标缓存值 */
    u8  cnt = 0;
    u16 d1, d2;
    u32 tem1, tem2;
    float fac;
    cnt = 0;
    POINT_COLOR = BLUE;
    BACK_COLOR = WHITE;
    LCD_Clear ( WHITE ); /* 清屏 */
    POINT_COLOR = RED; /* 红色 */
    LCD_Clear ( WHITE ); /* 清屏 */
    Drow_Touch_Point ( 20, 20 ); /* 画点1 */
    Pen_Point.Key_Sta = Key_Up; /* 消除触发信号 */
    Pen_Point.xfac = 0; /* xfac用来标记是否校准过，所以校准之前必须清掉，以免错误 */

    while ( 1 ) {
        if ( Pen_Point.Key_Sta == Key_Down ) { /* 按键按下了 */
            if ( Read_TP_Once() ) { /* 得到单次按键值 */
                pos_temp[cnt][0] = Pen_Point.X;
                pos_temp[cnt][1] = Pen_Point.Y;
                cnt++;
            }

            switch ( cnt ) {
                case 1:
                    LCD_Clear ( WHITE );
                    Drow_Touch_Point ( 220, 20 );
                    break;

                case 2:
                    LCD_Clear ( WHITE );
                    Drow_Touch_Point ( 20, 300 );
                    break;

                case 3:
                    LCD_Clear ( WHITE );
                    Drow_Touch_Point ( 220, 300 );
                    break;

                case 4: /* 全部四个点已经得到 */
                    tem1 = abs ( pos_temp[0][0] - pos_temp[1][0] );
                    tem2 = abs ( pos_temp[0][1] - pos_temp[1][1] );
                    tem1 *= tem1;
                    tem2 *= tem2;
                    d1 = sqrt ( tem1 + tem2 );
                    tem1 = abs ( pos_temp[2][0] - pos_temp[3][0] );
                    tem2 = abs ( pos_temp[2][1] - pos_temp[3][1] );
                    tem1 *= tem1;
                    tem2 *= tem2;
                    d2 = sqrt ( tem1 + tem2 );
                    fac = ( float ) d1 / d2;

                    if ( fac < 0.95 || fac > 1.05 || d1 == 0 || d2 == 0 ) {
                        cnt = 0;
                        LCD_Clear ( WHITE );
                        Drow_Touch_Point ( 20, 20 );
                        ADJ_INFO_SHOW ( "ver fac is:" );
                        LCD_ShowNum ( 40 + 24, 40, pos_temp[0][0], 4, 16 );
                        LCD_ShowNum ( 40 + 24 + 80, 40, pos_temp[0][1], 4, 16 );
                        LCD_ShowNum ( 40 + 24, 60, pos_temp[1][0], 4, 16 );
                        LCD_ShowNum ( 40 + 24 + 80, 60, pos_temp[1][1], 4, 16 );
                        LCD_ShowNum ( 40 + 24, 80, pos_temp[2][0], 4, 16 );
                        LCD_ShowNum ( 40 + 24 + 80, 80, pos_temp[2][1], 4, 16 );
                        LCD_ShowNum ( 40 + 24, 100, pos_temp[3][0], 4, 16 );
                        LCD_ShowNum ( 40 + 24 + 80, 100, pos_temp[3][1], 4, 16 );
                        LCD_ShowNum ( 40, 140, fac * 100, 3, 16 );
                        continue;
                    }

                    tem1 = abs ( pos_temp[0][0] - pos_temp[2][0] );
                    tem2 = abs ( pos_temp[0][1] - pos_temp[2][1] );
                    tem1 *= tem1;
                    tem2 *= tem2;
                    d1 = sqrt ( tem1 + tem2 );
                    tem1 = abs ( pos_temp[1][0] - pos_temp[3][0] );
                    tem2 = abs ( pos_temp[1][1] - pos_temp[3][1] );
                    tem1 *= tem1;
                    tem2 *= tem2;
                    d2 = sqrt ( tem1 + tem2 );
                    fac = ( float ) d1 / d2;

                    if ( fac < 0.95 || fac > 1.05 ) {
                        cnt = 0;
                        LCD_Clear ( WHITE );
                        Drow_Touch_Point ( 20, 20 );
                        ADJ_INFO_SHOW ( "hor fac is:" );
                        LCD_ShowNum ( 40 + 24, 40, pos_temp[0][0], 4, 16 );
                        LCD_ShowNum ( 40 + 24 + 80, 40, pos_temp[0][1], 4, 16 );
                        LCD_ShowNum ( 40 + 24, 60, pos_temp[1][0], 4, 16 );
                        LCD_ShowNum ( 40 + 24 + 80, 60, pos_temp[1][1], 4, 16 );
                        LCD_ShowNum ( 40 + 24, 80, pos_temp[2][0], 4, 16 );
                        LCD_ShowNum ( 40 + 24 + 80, 80, pos_temp[2][1], 4, 16 );
                        LCD_ShowNum ( 40 + 24, 100, pos_temp[3][0], 4, 16 );
                        LCD_ShowNum ( 40 + 24 + 80, 100, pos_temp[3][1], 4, 16 );
                        LCD_ShowNum ( 40, 140, fac * 100, 3, 16 );
                        continue;
                    }

                    tem1 = abs ( pos_temp[1][0] - pos_temp[2][0] );
                    tem2 = abs ( pos_temp[1][1] - pos_temp[2][1] );
                    tem1 *= tem1;
                    tem2 *= tem2;
                    d1 = sqrt ( tem1 + tem2 );
                    tem1 = abs ( pos_temp[0][0] - pos_temp[3][0] );
                    tem2 = abs ( pos_temp[0][1] - pos_temp[3][1] );
                    tem1 *= tem1;
                    tem2 *= tem2;
                    d2 = sqrt ( tem1 + tem2 );
                    fac = ( float ) d1 / d2;

                    if ( fac < 0.95 || fac > 1.05 ) {
                        cnt = 0;
                        LCD_Clear ( WHITE );
                        Drow_Touch_Point ( 20, 20 );
                        ADJ_INFO_SHOW ( "dia fac is:" );
                        LCD_ShowNum ( 40 + 24, 40, pos_temp[0][0], 4, 16 );
                        LCD_ShowNum ( 40 + 24 + 80, 40, pos_temp[0][1], 4, 16 );
                        LCD_ShowNum ( 40 + 24, 60, pos_temp[1][0], 4, 16 );
                        LCD_ShowNum ( 40 + 24 + 80, 60, pos_temp[1][1], 4, 16 );
                        LCD_ShowNum ( 40 + 24, 80, pos_temp[2][0], 4, 16 );
                        LCD_ShowNum ( 40 + 24 + 80, 80, pos_temp[2][1], 4, 16 );
                        LCD_ShowNum ( 40 + 24, 100, pos_temp[3][0], 4, 16 );
                        LCD_ShowNum ( 40 + 24 + 80, 100, pos_temp[3][1], 4, 16 );
                        LCD_ShowNum ( 40, 140, fac * 100, 3, 16 );
                        continue;
                    }

                    Pen_Point.xfac = ( float ) 200 / ( pos_temp[1][0] - pos_temp[0][0] );
                    Pen_Point.xoff = ( 240 - Pen_Point.xfac * ( pos_temp[1][0] + pos_temp[0][0] ) ) / 2;
                    Pen_Point.yfac = ( float ) 280 / ( pos_temp[2][1] - pos_temp[0][1] );
                    Pen_Point.yoff = ( 320 - Pen_Point.yfac * ( pos_temp[2][1] + pos_temp[0][1] ) ) / 2;

                    if ( abs ( Pen_Point.xfac ) > 2 || abs ( Pen_Point.yfac ) > 2 ) {
                        cnt = 0;
                        LCD_Clear ( WHITE );
                        Drow_Touch_Point ( 20, 20 );
                        LCD_ShowString ( 35, 110, "TP Need readjust!" );
                        Pen_Point.touchtype = !Pen_Point.touchtype;

                        if ( Pen_Point.touchtype ) {
                            CMD_RDX = 0X90;
                            CMD_RDY = 0XD0;
                        } else {
                            CMD_RDX = 0XD0;
                            CMD_RDY = 0X90;
                        }

                        delay_ms ( 500 );
                        continue;
                    }

                    POINT_COLOR = BLUE;
                    LCD_Clear ( WHITE );
                    LCD_ShowString ( 35, 110, "Touch Screen Adjust OK!" );
                    delay_ms ( 500 );
                    LCD_Clear ( WHITE );
                    return;
            }
        }
    }
}

void Touch_Init ( void ) { /* 外部中断初始化函数 */
    NVIC_InitTypeDef NVIC_InitStructure;
    GPIO_InitTypeDef GPIO_InitStructure;
    EXTI_InitTypeDef EXTI_InitStructure;
    RCC_APB2PeriphClockCmd ( RCC_APB2Periph_GPIOC  | RCC_APB2Periph_AFIO, ENABLE );
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3 | GPIO_Pin_0 | GPIO_Pin_13;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init ( GPIOC, &GPIO_InitStructure );
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1 | GPIO_Pin_2;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU ;
    GPIO_Init ( GPIOC, &GPIO_InitStructure );
    Read_ADS ( &Pen_Point.X, &Pen_Point.Y ); /* 第一次读取初始化 */
    NVIC_InitStructure.NVIC_IRQChannel = EXTI1_IRQn; /* 使能按键所在的外部中断通道 */
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2; /* 先占优先级2级 */
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0; /* 从优先级0级 */
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; /* 使能外部中断通道 */
    NVIC_Init ( &NVIC_InitStructure ); /* 根据NVIC_InitStruct中指定的参数初始化外设NVIC寄存器 */
    RCC_APB2PeriphClockCmd ( RCC_APB2Periph_AFIO, ENABLE );
    GPIO_EXTILineConfig ( GPIO_PortSourceGPIOC, GPIO_PinSource1 );
    EXTI_InitStructure.EXTI_Line = EXTI_Line1; /* 外部线路EXIT1 */
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt; /* 设外外部中断模式：EXTI线路为中断请求 */
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling; /* 外部中断触发沿选择：设置输入线路下降沿为中断请求 */
    EXTI_InitStructure.EXTI_LineCmd = ENABLE; /* 使能外部中断新状态 */
    EXTI_Init ( &EXTI_InitStructure ); /* 根据EXTI_InitStruct中指定的参数初始化外设EXTI寄存器 */
#ifdef ADJ_SAVE_ENABLE
    AT24CXX_Init();

    if ( Get_Adjdata() ) {
        return;
    } else {
        LCD_Clear ( WHITE );
        Touch_Adjust();
        Save_Adjdata();
    }

    Get_Adjdata();
#else
    LCD_Clear ( WHITE );
    Touch_Adjust();
#endif
}