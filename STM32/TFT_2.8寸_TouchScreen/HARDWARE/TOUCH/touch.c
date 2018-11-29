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
        TCLK = 0; /* ��������Ч */
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
        TCLK = 0; /* �½�����Ч */

        if ( DOUT ) {
            Num++;
        }
    }

    Num >>= 3; /* ֻ�и�12λ��Ч */
    TCS = 1;
    return ( Num );
}

/* ������ȡREAD_TIMES�����ݣ�����Щ�����������У�Ȼ��ȥ����ͺ����LOST_VAL������ȡƽ��ֵ */
#define READ_TIMES 15 /* ��ȡ���� */
#define LOST_VAL   5  /* ����ֵ */

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

u8 Read_ADS ( u16 *x, u16 *y ) { /* ���˲��������ȡ */
    u16 xtemp, ytemp;
    xtemp = ADS_Read_XY ( CMD_RDX );
    ytemp = ADS_Read_XY ( CMD_RDY );

    if ( xtemp < 100 || ytemp < 100 ) {
        return 0; /* ����ʧ�� */
    }

    *x = xtemp;
    *y = ytemp;
    return 1; /* �����ɹ� */
}

#define ERR_RANGE 50 /* ��Χ */

/* ����2�ζ�ȡADS7846���������ε�ƫ��ܳ���ERR_RANGE��������������Ϊ������ȷ������������� */
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

    if ( ( ( x2 <= x1 && x1 < x2 + ERR_RANGE ) || ( x1 <= x2 && x2 < x1 + ERR_RANGE ) ) /* ǰ�����β����ڡ�+/-50���� */
         && ( ( y2 <= y1 && y1 < y2 + ERR_RANGE ) || ( y1 <= y2 && y2 < y1 + ERR_RANGE ) ) ) {
        *x = ( x1 + x2 ) / 2;
        *y = ( y1 + y2 ) / 2;
        return 1;
    } else {
        return 0;
    }
}

u8 Read_TP_Once ( void ) { /* ��ȡһ������ֵ */
    u8 t = 0;
    Pen_Int_Set ( 0 ); /* �ر��ж� */
    Pen_Point.Key_Sta = Key_Up;
    Read_ADS2 ( &Pen_Point.X, &Pen_Point.Y );

    while ( PEN == 0 && t <= 250 ) {
        t++;
        delay_ms ( 10 );
    };

    Pen_Int_Set ( 1 ); /* �����ж� */

    if ( t >= 250 ) {
        return 0;
    } else {
        return 1;
    }
}

void Drow_Touch_Point ( u8 x, u16 y ) { /* ��һ�������� */
    LCD_DrawLine ( x - 12, y, x + 13, y ); /* ���� */
    LCD_DrawLine ( x, y - 12, x, y + 13 ); /* ���� */
    LCD_DrawPoint ( x + 1, y + 1 );
    LCD_DrawPoint ( x - 1, y + 1 );
    LCD_DrawPoint ( x + 1, y - 1 );
    LCD_DrawPoint ( x - 1, y - 1 );
    Draw_Circle ( x, y, 6 ); /* ������Ȧ */
}

void Draw_Big_Point ( u8 x, u16 y ) {
    LCD_DrawPoint ( x, y );
    LCD_DrawPoint ( x + 1, y );
    LCD_DrawPoint ( x, y + 1 );
    LCD_DrawPoint ( x + 1, y + 1 );
}

void Convert_Pos ( void ) { /* ���ݴ�������У׼����������ת����Ľ����������X0��Y0�� */
    if ( Read_ADS2 ( &Pen_Point.X, &Pen_Point.Y ) ) {
        Pen_Point.X0 = Pen_Point.xfac * Pen_Point.X + Pen_Point.xoff;
        Pen_Point.Y0 = Pen_Point.yfac * Pen_Point.Y + Pen_Point.yoff;
    }
}

void EXTI1_IRQHandler ( void ) { /* �жϴ���������⵽PEN�ŵ�һ���½��� */
    Pen_Point.Key_Sta = Key_Down; /* �������� */
    EXTI->PR = 1 << 1; /* ���LINE1�ϵ��жϱ�־λ */
}

void Pen_Int_Set ( u8 en ) { /* PEN�ж����� */
    if ( en ) {
        EXTI->IMR |= 1 << 1; /* ����line1�ϵ��ж� */
    } else {
        EXTI->IMR &= ~ ( 1 << 1 ); /* �ر�line1�ϵ��ж� */
    }
}

#ifdef ADJ_SAVE_ENABLE /* �˲����漰��ʹ���ⲿEEPROM�����û���ⲿEEPROM�����δ˲��ּ��� */
#define SAVE_ADDR_BASE 40 /* ������EEPROM����ĵ�ַ�����ַ��ռ��13���ֽ�(RANGE��SAVE_ADDR_BASE��SAVE_ADDR_BASE+12) */

void Save_Adjdata ( void ) { /* ����У׼���� */
    s32 temp;
    /* ����У����� */
    temp = Pen_Point.xfac * 100000000; /* ����xУ������ */
    AT24CXX_WriteLenByte ( SAVE_ADDR_BASE, temp, 4 );
    temp = Pen_Point.yfac * 100000000; /* ����yУ������ */
    AT24CXX_WriteLenByte ( SAVE_ADDR_BASE + 4, temp, 4 );
    AT24CXX_WriteLenByte ( SAVE_ADDR_BASE + 8, Pen_Point.xoff, 2 ); /* ����xƫ���� */
    AT24CXX_WriteLenByte ( SAVE_ADDR_BASE + 10, Pen_Point.yoff, 2 ); /* ����yƫ���� */
    AT24CXX_WriteOneByte ( SAVE_ADDR_BASE + 12, Pen_Point.touchtype ); /* ���津������ */
    temp = 0X0A; /* ���У׼���� */
    AT24CXX_WriteOneByte ( SAVE_ADDR_BASE + 13, temp );
}

u8 Get_Adjdata ( void ) { /* �õ�������EEPROM�����У׼ֵ������1��ʾ�ɹ���ȡ���ݣ�0��ʾ��ȡʧ�ܣ�Ҫ����У׼ */
    s32 tempfac;
    tempfac = AT24CXX_ReadOneByte ( SAVE_ADDR_BASE + 13 ); /* ��ȡ����֣����Ƿ�У׼�� */

    if ( tempfac == 0X0A ) { /* �������Ѿ�У׼���� */
        tempfac = AT24CXX_ReadLenByte ( SAVE_ADDR_BASE, 4 );
        Pen_Point.xfac = ( float ) tempfac / 100000000; /* �õ�xУ׼���� */
        tempfac = AT24CXX_ReadLenByte ( SAVE_ADDR_BASE + 4, 4 );
        Pen_Point.yfac = ( float ) tempfac / 100000000; /* �õ�yУ׼���� */
        Pen_Point.xoff = AT24CXX_ReadLenByte ( SAVE_ADDR_BASE + 8, 2 ); /* �õ�xƫ���� */
        Pen_Point.yoff = AT24CXX_ReadLenByte ( SAVE_ADDR_BASE + 10, 2 ); /* �õ�yƫ���� */
        Pen_Point.touchtype = AT24CXX_ReadOneByte ( SAVE_ADDR_BASE + 12 ); /* ��ȡ�������ͱ�� */

        if ( Pen_Point.touchtype ) { /* X��Y��������Ļ�෴ */
            CMD_RDX = 0X90;
            CMD_RDY = 0XD0;
        } else { /* X��Y��������Ļ��ͬ */
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

void Touch_Adjust ( void ) { /* ������У׼���룬�õ��ĸ�У׼���� */
    signed short pos_temp[4][2]; /* ���껺��ֵ */
    u8  cnt = 0;
    u16 d1, d2;
    u32 tem1, tem2;
    float fac;
    cnt = 0;
    POINT_COLOR = BLUE;
    BACK_COLOR = WHITE;
    LCD_Clear ( WHITE ); /* ���� */
    POINT_COLOR = RED; /* ��ɫ */
    LCD_Clear ( WHITE ); /* ���� */
    Drow_Touch_Point ( 20, 20 ); /* ����1 */
    Pen_Point.Key_Sta = Key_Up; /* ���������ź� */
    Pen_Point.xfac = 0; /* xfac��������Ƿ�У׼��������У׼֮ǰ���������������� */

    while ( 1 ) {
        if ( Pen_Point.Key_Sta == Key_Down ) { /* ���������� */
            if ( Read_TP_Once() ) { /* �õ����ΰ���ֵ */
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

                case 4: /* ȫ���ĸ����Ѿ��õ� */
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

void Touch_Init ( void ) { /* �ⲿ�жϳ�ʼ������ */
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
    Read_ADS ( &Pen_Point.X, &Pen_Point.Y ); /* ��һ�ζ�ȡ��ʼ�� */
    NVIC_InitStructure.NVIC_IRQChannel = EXTI1_IRQn; /* ʹ�ܰ������ڵ��ⲿ�ж�ͨ�� */
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2; /* ��ռ���ȼ�2�� */
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0; /* �����ȼ�0�� */
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; /* ʹ���ⲿ�ж�ͨ�� */
    NVIC_Init ( &NVIC_InitStructure ); /* ����NVIC_InitStruct��ָ���Ĳ�����ʼ������NVIC�Ĵ��� */
    RCC_APB2PeriphClockCmd ( RCC_APB2Periph_AFIO, ENABLE );
    GPIO_EXTILineConfig ( GPIO_PortSourceGPIOC, GPIO_PinSource1 );
    EXTI_InitStructure.EXTI_Line = EXTI_Line1; /* �ⲿ��·EXIT1 */
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt; /* �����ⲿ�ж�ģʽ��EXTI��·Ϊ�ж����� */
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling; /* �ⲿ�жϴ�����ѡ������������·�½���Ϊ�ж����� */
    EXTI_InitStructure.EXTI_LineCmd = ENABLE; /* ʹ���ⲿ�ж���״̬ */
    EXTI_Init ( &EXTI_InitStructure ); /* ����EXTI_InitStruct��ָ���Ĳ�����ʼ������EXTI�Ĵ��� */
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