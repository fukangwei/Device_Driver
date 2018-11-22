#include "lcd.h"
#include "stdlib.h"
#include "font.h"
#include "usart.h"
#include "delay.h"

u16 POINT_COLOR = 0x0000, BACK_COLOR = 0xFFFF;
u16 DeviceCode;

#if LCD_FAST_IO == 1

void LCD_WR_REG ( u8 data ) {
    LCD_RS_CLR;
    LCD_CS_CLR;
    DATAOUT ( data );
    LCD_WR_CLR;
    LCD_WR_SET;
    LCD_CS_SET;
}
#else
void LCD_WR_REG ( u8 data ) {
    LCD_RS = 0;
    LCD_CS = 0;
    DATAOUT ( data );
    LCD_WR = 0;
    LCD_WR = 1;
    LCD_CS = 1;
}
#endif

u16 LCD_RD_DATA ( void ) {
    u16 t;
    GPIOB->CRL = 0X88888888;
    GPIOB->CRH = 0X88888888;
    GPIOB->ODR = 0X0000;
#if LCD_FAST_IO == 1
    LCD_RS_SET;
    LCD_CS_CLR;
    LCD_RD_CLR;

    if ( DeviceCode == 0X8989 ) {
        delay_us ( 2 );
    }

    LCD_RD_SET;
    t = DATAIN;
    LCD_CS_SET;
#else
    LCD_RS = 1;
    LCD_CS = 0;
    LCD_RD = 0;
    LCD_RD = 1;
    t = DATAIN;
    LCD_CS = 1;
#endif
    GPIOB->CRL = 0X33333333;
    GPIOB->CRH = 0X33333333;
    GPIOB->ODR = 0XFFFF;
    return t;
}

void LCD_WriteReg ( u8 LCD_Reg, u16 LCD_RegValue ) {
    LCD_WR_REG ( LCD_Reg );
    LCD_WR_DATA ( LCD_RegValue );
}

u16 LCD_ReadReg ( u8 LCD_Reg ) {
    LCD_WR_REG ( LCD_Reg );
    return LCD_RD_DATA();
}

void LCD_WriteRAM_Prepare ( void ) {
    if ( DeviceCode == 0X9341 ) {
        LCD_WR_REG ( 0x2C );
    } else {
        LCD_WR_REG ( R34 );
    }
}

void LCD_WriteRAM ( u16 RGB_Code ) {
    LCD_WR_DATA ( RGB_Code );
}

u16 LCD_BGR2RGB ( u16 c ) {
    u16  r, g, b, rgb;
    b = ( c >> 0 ) & 0x1f;
    g = ( c >> 5 ) & 0x3f;
    r = ( c >> 11 ) & 0x1f;
    rgb = ( b << 11 ) + ( g << 5 ) + ( r << 0 );
    return ( rgb );
}

u16 LCD_ReadPoint ( u16 x, u16 y ) {
    u16 r, g, b;

    if ( x >= LCD_W || y >= LCD_H ) {
        return 0;
    }

    LCD_SetCursor ( x, y );

    if ( DeviceCode == 0X9341 ) {
        LCD_WR_REG ( 0X2E );
    } else {
        LCD_WR_REG ( R34 );
    }

    GPIOB->CRL = 0X88888888;
    GPIOB->CRH = 0X88888888;
    GPIOB->ODR = 0XFFFF;
#if LCD_FAST_IO == 1
    LCD_RS_SET;
    LCD_CS_CLR;
    LCD_RD_CLR;
    LCD_RD_SET;
    LCD_RD_CLR;

    if ( DeviceCode == 0X8989 ) {
        delay_us ( 2 );
    }

    LCD_RD_SET;
    r = DATAIN;

    if ( DeviceCode == 0X9341 ) {
        LCD_RD_CLR;
        LCD_RD_SET;
        b = DATAIN;
        g = r & 0XFF;
        g <<= 8;
    }

    LCD_CS_SET;
#else
    LCD_RS = 1;
    LCD_CS = 0;
    LCD_RD = 0;
    LCD_RD = 1;
    LCD_RD = 0;
    LCD_RD = 1;
    r = DATAIN;

    if ( DeviceCode == 0X9341 ) {
        LCD_RD = 0;
        LCD_RD = 1;
        b = DATAIN;
        g = r & 0XFF;
        g <<= 8;
    }

    LCD_CS = 1;
#endif
    GPIOB->CRL = 0X33333333;
    GPIOB->CRH = 0X33333333;
    GPIOB->ODR = 0XFFFF;

    if ( DeviceCode == 0X4535 || DeviceCode == 0X4531 || DeviceCode == 0X8989 || DeviceCode == 0XB505 ) {
        return r;
    } else if ( DeviceCode == 0X9341 ) {
        return ( ( ( r >> 11 ) << 11 ) | ( ( g >> 10 ) << 5 ) | ( b >> 11 ) );
    } else {
        return LCD_BGR2RGB ( r );
    }
}

void LCD_DisplayOn ( void ) {
    if ( DeviceCode == 0X9341 ) {
        LCD_WR_REG ( 0X29 );
    } else {
        LCD_WriteReg ( R7, 0x0173 );
    }
}

void LCD_DisplayOff ( void ) {
    if ( DeviceCode == 0X9341 ) {
        LCD_WR_REG ( 0X28 );
    } else {
        LCD_WriteReg ( R7, 0x0 );
    }
}

__inline void LCD_SetCursor ( u16 Xpos, u16 Ypos ) {
#if USE_HORIZONTAL==1

    if ( DeviceCode == 0X8989 ) {
        Xpos = 319 - Xpos;
        LCD_WriteReg ( 0X4E, Ypos );
        LCD_WriteReg ( 0X4F, Xpos );
    } else if ( DeviceCode == 0X9341 ) {
        LCD_WR_REG ( 0x2B );
        LCD_WR_DATA ( Ypos >> 8 );
        LCD_WR_DATA ( Ypos & 0XFF );
        LCD_WR_REG ( 0x2A );
        LCD_WR_DATA ( Xpos >> 8 );
        LCD_WR_DATA ( Xpos & 0XFF );
    } else {
        Xpos = 319 - Xpos;
        LCD_WriteReg ( R32, Ypos );
        LCD_WriteReg ( R33, Xpos );
    }

#else

    if ( DeviceCode == 0X8989 ) {
        LCD_WriteReg ( 0X4E, Xpos );
        LCD_WriteReg ( 0X4F, Ypos );
    } else if ( DeviceCode == 0X9341 ) {
        LCD_WR_REG ( 0x2A );
        LCD_WR_DATA ( Xpos >> 8 );
        LCD_WR_DATA ( Xpos & 0XFF );
        LCD_WR_REG ( 0x2B );
        LCD_WR_DATA ( Ypos >> 8 );
        LCD_WR_DATA ( Ypos & 0XFF );
    } else {
        LCD_WriteReg ( R32, Xpos );
        LCD_WriteReg ( R33, Ypos );
    }

#endif
}

void LCD_Scan_Dir ( u8 dir ) {
    u16 regval = 0;
    u8 dirreg = 0;
#if USE_HORIZONTAL

    switch ( dir ) {
        case 0:
            dir = 6;
            break;

        case 1:
            dir = 7;
            break;

        case 2:
            dir = 4;
            break;

        case 3:
            dir = 5;
            break;

        case 4:
            dir = 1;
            break;

        case 5:
            dir = 0;
            break;

        case 6:
            dir = 3;
            break;

        case 7:
            dir = 2;
            break;
    }

#endif

    if ( DeviceCode == 0x9341 ) {
        switch ( dir ) {
            case L2R_U2D:
                regval |= ( 0 << 7 ) | ( 0 << 6 ) | ( 0 << 5 );
                break;

            case L2R_D2U:
                regval |= ( 1 << 7 ) | ( 0 << 6 ) | ( 0 << 5 );
                break;

            case R2L_U2D:
                regval |= ( 0 << 7 ) | ( 1 << 6 ) | ( 0 << 5 );
                break;

            case R2L_D2U:
                regval |= ( 1 << 7 ) | ( 1 << 6 ) | ( 0 << 5 );
                break;

            case U2D_L2R:
                regval |= ( 0 << 7 ) | ( 0 << 6 ) | ( 1 << 5 );
                break;

            case U2D_R2L:
                regval |= ( 0 << 7 ) | ( 1 << 6 ) | ( 1 << 5 );
                break;

            case D2U_L2R:
                regval |= ( 1 << 7 ) | ( 0 << 6 ) | ( 1 << 5 );
                break;

            case D2U_R2L:
                regval |= ( 1 << 7 ) | ( 1 << 6 ) | ( 1 << 5 );
                break;
        }

        dirreg = 0X36;
        regval |= 0X08;
        LCD_WriteReg ( dirreg, regval );

        if ( regval & 0X20 ) {
            LCD_WR_REG ( 0x2A );
            LCD_WR_DATA ( 0 );
            LCD_WR_DATA ( 0 );
            LCD_WR_DATA ( 319 >> 8 );
            LCD_WR_DATA ( 319 & 0XFF );
            LCD_WR_REG ( 0x2B );
            LCD_WR_DATA ( 0 );
            LCD_WR_DATA ( 0 );
            LCD_WR_DATA ( 239 >> 8 );
            LCD_WR_DATA ( 239 & 0XFF );
        } else {
            LCD_WR_REG ( 0x2A );
            LCD_WR_DATA ( 0 );
            LCD_WR_DATA ( 0 );
            LCD_WR_DATA ( 239 >> 8 );
            LCD_WR_DATA ( 239 & 0XFF );
            LCD_WR_REG ( 0x2B );
            LCD_WR_DATA ( 0 );
            LCD_WR_DATA ( 0 );
            LCD_WR_DATA ( 319 >> 8 );
            LCD_WR_DATA ( 319 & 0XFF );
        }
    } else {
        switch ( dir ) {
            case L2R_U2D:
                regval |= ( 1 << 5 ) | ( 1 << 4 ) | ( 0 << 3 );
                break;

            case L2R_D2U:
                regval |= ( 0 << 5 ) | ( 1 << 4 ) | ( 0 << 3 );
                break;

            case R2L_U2D:
                regval |= ( 1 << 5 ) | ( 0 << 4 ) | ( 0 << 3 );
                break;

            case R2L_D2U:
                regval |= ( 0 << 5 ) | ( 0 << 4 ) | ( 0 << 3 );
                break;

            case U2D_L2R:
                regval |= ( 1 << 5 ) | ( 1 << 4 ) | ( 1 << 3 );
                break;

            case U2D_R2L:
                regval |= ( 1 << 5 ) | ( 0 << 4 ) | ( 1 << 3 );
                break;

            case D2U_L2R:
                regval |= ( 0 << 5 ) | ( 1 << 4 ) | ( 1 << 3 );
                break;

            case D2U_R2L:
                regval |= ( 0 << 5 ) | ( 0 << 4 ) | ( 1 << 3 );
                break;
        }

        if ( DeviceCode == 0x8989 ) {
            dirreg = 0X11;
            regval |= 0X6040;
        } else {
            dirreg = 0X03;
            regval |= 1 << 12;
        }

        LCD_WriteReg ( dirreg, regval );
    }
}

void LCD_DrawPoint ( u16 x, u16 y ) {
    LCD_SetCursor ( x, y );
    LCD_WriteRAM_Prepare();
    LCD_WR_DATA ( POINT_COLOR );
}

void LCD_Init ( void ) {
    GPIO_InitTypeDef GPIO_InitStructure;
    RCC_APB2PeriphClockCmd ( RCC_APB2Periph_GPIOC | RCC_APB2Periph_GPIOB | RCC_APB2Periph_AFIO, ENABLE );
    GPIO_PinRemapConfig ( GPIO_Remap_SWJ_JTAGDisable , ENABLE );
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10 | GPIO_Pin_9 | GPIO_Pin_8 | GPIO_Pin_7 | GPIO_Pin_6;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init ( GPIOC, &GPIO_InitStructure );
    GPIO_SetBits ( GPIOC, GPIO_Pin_10 | GPIO_Pin_9 | GPIO_Pin_8 | GPIO_Pin_7 | GPIO_Pin_6 );
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_All;
    GPIO_Init ( GPIOB, &GPIO_InitStructure );
    GPIO_SetBits ( GPIOB, GPIO_Pin_All );
    delay_ms ( 50 );
    LCD_WriteReg ( 0x0000, 0x0001 );
    delay_ms ( 50 );
    DeviceCode = LCD_ReadReg ( 0x0000 );

    if ( DeviceCode == 0 || DeviceCode == 0XFFFF ) {
        LCD_WR_REG ( 0XD3 );
        LCD_RD_DATA();
        LCD_RD_DATA();
        DeviceCode = LCD_RD_DATA() & 0XBF;
        DeviceCode <<= 8;
        DeviceCode |= LCD_RD_DATA();
    }

    printf ( " LCD ID:%x\r\n", DeviceCode );

    if ( DeviceCode == 0X9341 ) {
        LCD_WR_REG ( 0xCF );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0xC1 );
        LCD_WR_DATA ( 0X30 );
        LCD_WR_REG ( 0xED );
        LCD_WR_DATA ( 0x64 );
        LCD_WR_DATA ( 0x03 );
        LCD_WR_DATA ( 0X12 );
        LCD_WR_DATA ( 0X81 );
        LCD_WR_REG ( 0xE8 );
        LCD_WR_DATA ( 0x85 );
        LCD_WR_DATA ( 0x10 );
        LCD_WR_DATA ( 0x7A );
        LCD_WR_REG ( 0xCB );
        LCD_WR_DATA ( 0x39 );
        LCD_WR_DATA ( 0x2C );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0x34 );
        LCD_WR_DATA ( 0x02 );
        LCD_WR_REG ( 0xF7 );
        LCD_WR_DATA ( 0x20 );
        LCD_WR_REG ( 0xEA );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_REG ( 0xC0 );
        LCD_WR_DATA ( 0x1B );
        LCD_WR_REG ( 0xC1 );
        LCD_WR_DATA ( 0x01 );
        LCD_WR_REG ( 0xC5 );
        LCD_WR_DATA ( 0x30 );
        LCD_WR_DATA ( 0x30 );
        LCD_WR_REG ( 0xC7 );
        LCD_WR_DATA ( 0XB7 );
        LCD_WR_REG ( 0x36 );
        LCD_WR_DATA ( 0x48 );
        LCD_WR_REG ( 0x3A );
        LCD_WR_DATA ( 0x55 );
        LCD_WR_REG ( 0xB1 );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0x1A );
        LCD_WR_REG ( 0xB6 );
        LCD_WR_DATA ( 0x0A );
        LCD_WR_DATA ( 0xA2 );
        LCD_WR_REG ( 0xF2 );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_REG ( 0x26 );
        LCD_WR_DATA ( 0x01 );
        LCD_WR_REG ( 0xE0 );
        LCD_WR_DATA ( 0x0F );
        LCD_WR_DATA ( 0x2A );
        LCD_WR_DATA ( 0x28 );
        LCD_WR_DATA ( 0x08 );
        LCD_WR_DATA ( 0x0E );
        LCD_WR_DATA ( 0x08 );
        LCD_WR_DATA ( 0x54 );
        LCD_WR_DATA ( 0XA9 );
        LCD_WR_DATA ( 0x43 );
        LCD_WR_DATA ( 0x0A );
        LCD_WR_DATA ( 0x0F );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_REG ( 0XE1 );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0x15 );
        LCD_WR_DATA ( 0x17 );
        LCD_WR_DATA ( 0x07 );
        LCD_WR_DATA ( 0x11 );
        LCD_WR_DATA ( 0x06 );
        LCD_WR_DATA ( 0x2B );
        LCD_WR_DATA ( 0x56 );
        LCD_WR_DATA ( 0x3C );
        LCD_WR_DATA ( 0x05 );
        LCD_WR_DATA ( 0x10 );
        LCD_WR_DATA ( 0x0F );
        LCD_WR_DATA ( 0x3F );
        LCD_WR_DATA ( 0x3F );
        LCD_WR_DATA ( 0x0F );
        LCD_WR_REG ( 0x2B );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0x01 );
        LCD_WR_DATA ( 0x3f );
        LCD_WR_REG ( 0x2A );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0xef );
        LCD_WR_REG ( 0x11 );
        delay_ms ( 120 );
        LCD_WR_REG ( 0x29 );
    } else if ( DeviceCode == 0x9325 ) {
        LCD_WriteReg ( 0x00E5, 0x78F0 );
        LCD_WriteReg ( 0x0001, 0x0100 );
        LCD_WriteReg ( 0x0002, 0x0700 );
        LCD_WriteReg ( 0x0003, 0x1030 );
        LCD_WriteReg ( 0x0004, 0x0000 );
        LCD_WriteReg ( 0x0008, 0x0202 );
        LCD_WriteReg ( 0x0009, 0x0000 );
        LCD_WriteReg ( 0x000A, 0x0000 );
        LCD_WriteReg ( 0x000C, 0x0000 );
        LCD_WriteReg ( 0x000D, 0x0000 );
        LCD_WriteReg ( 0x000F, 0x0000 );
        LCD_WriteReg ( 0x0010, 0x0000 );
        LCD_WriteReg ( 0x0011, 0x0007 );
        LCD_WriteReg ( 0x0012, 0x0000 );
        LCD_WriteReg ( 0x0013, 0x0000 );
        LCD_WriteReg ( 0x0007, 0x0000 );
        LCD_WriteReg ( 0x0010, 0x1690 );
        LCD_WriteReg ( 0x0011, 0x0227 );
        LCD_WriteReg ( 0x0012, 0x009D );
        LCD_WriteReg ( 0x0013, 0x1900 );
        LCD_WriteReg ( 0x0029, 0x0025 );
        LCD_WriteReg ( 0x002B, 0x000D );
        LCD_WriteReg ( 0x0030, 0x0007 );
        LCD_WriteReg ( 0x0031, 0x0303 );
        LCD_WriteReg ( 0x0032, 0x0003 );
        LCD_WriteReg ( 0x0035, 0x0206 );
        LCD_WriteReg ( 0x0036, 0x0008 );
        LCD_WriteReg ( 0x0037, 0x0406 );
        LCD_WriteReg ( 0x0038, 0x0304 );
        LCD_WriteReg ( 0x0039, 0x0007 );
        LCD_WriteReg ( 0x003C, 0x0602 );
        LCD_WriteReg ( 0x003D, 0x0008 );
        LCD_WriteReg ( 0x0050, 0x0000 );
        LCD_WriteReg ( 0x0051, 0x00EF );
        LCD_WriteReg ( 0x0052, 0x0000 );
        LCD_WriteReg ( 0x0053, 0x013F );
        LCD_WriteReg ( 0x0060, 0xA700 );
        LCD_WriteReg ( 0x0061, 0x0001 );
        LCD_WriteReg ( 0x006A, 0x0000 );
        LCD_WriteReg ( 0x0080, 0x0000 );
        LCD_WriteReg ( 0x0081, 0x0000 );
        LCD_WriteReg ( 0x0082, 0x0000 );
        LCD_WriteReg ( 0x0083, 0x0000 );
        LCD_WriteReg ( 0x0084, 0x0000 );
        LCD_WriteReg ( 0x0085, 0x0000 );
        LCD_WriteReg ( 0x0090, 0x0010 );
        LCD_WriteReg ( 0x0092, 0x0600 );
        LCD_WriteReg ( 0x0007, 0x0133 );
        LCD_WriteReg ( 0x00, 0x0022 );
    } else if ( DeviceCode == 0x9328 ) {
        LCD_WriteReg ( 0x00e7, 0x0010 );
        LCD_WriteReg ( 0x0000, 0x0001 );
        LCD_WriteReg ( 0x0001, 0x0100 );
        LCD_WriteReg ( 0x0002, 0x0700 );
        LCD_WriteReg ( 0x0003, ( 1 << 12 ) | ( 3 << 4 ) | ( 0 << 3 ) );
        LCD_WriteReg ( 0x0004, 0x0000 );
        LCD_WriteReg ( 0x0008, 0x0207 );
        LCD_WriteReg ( 0x0009, 0x0000 );
        LCD_WriteReg ( 0x000a, 0x0000 );
        LCD_WriteReg ( 0x000c, 0x0001 );
        LCD_WriteReg ( 0x000d, 0x0000 );
        LCD_WriteReg ( 0x000f, 0x0000 );
        LCD_WriteReg ( 0x0010, 0x0000 );
        LCD_WriteReg ( 0x0011, 0x0007 );
        LCD_WriteReg ( 0x0012, 0x0000 );
        LCD_WriteReg ( 0x0013, 0x0000 );
        delay_ms ( 50 );
        LCD_WriteReg ( 0x0010, 0x1590 );
        LCD_WriteReg ( 0x0011, 0x0227 );
        delay_ms ( 50 );
        LCD_WriteReg ( 0x0012, 0x009c );
        delay_ms ( 50 );
        LCD_WriteReg ( 0x0013, 0x1900 );
        LCD_WriteReg ( 0x0029, 0x0023 );
        LCD_WriteReg ( 0x002b, 0x000e );
        delay_ms ( 50 );
        LCD_WriteReg ( 0x0020, 0x0000 );
        LCD_WriteReg ( 0x0021, 0x013f );
        delay_ms ( 50 );
        LCD_WriteReg ( 0x0030, 0x0007 );
        LCD_WriteReg ( 0x0031, 0x0707 );
        LCD_WriteReg ( 0x0032, 0x0006 );
        LCD_WriteReg ( 0x0035, 0x0704 );
        LCD_WriteReg ( 0x0036, 0x1f04 );
        LCD_WriteReg ( 0x0037, 0x0004 );
        LCD_WriteReg ( 0x0038, 0x0000 );
        LCD_WriteReg ( 0x0039, 0x0706 );
        LCD_WriteReg ( 0x003c, 0x0701 );
        LCD_WriteReg ( 0x003d, 0x000f );
        delay_ms ( 50 );
        LCD_WriteReg ( 0x0050, 0x0000 );
        LCD_WriteReg ( 0x0051, 0x00ef );
        LCD_WriteReg ( 0x0052, 0x0000 );
        LCD_WriteReg ( 0x0053, 0x013f );
        LCD_WriteReg ( 0x0060, 0xa700 );
        LCD_WriteReg ( 0x0061, 0x0001 );
        LCD_WriteReg ( 0x006a, 0x0000 );
        LCD_WriteReg ( 0x0080, 0x0000 );
        LCD_WriteReg ( 0x0081, 0x0000 );
        LCD_WriteReg ( 0x0082, 0x0000 );
        LCD_WriteReg ( 0x0083, 0x0000 );
        LCD_WriteReg ( 0x0084, 0x0000 );
        LCD_WriteReg ( 0x0085, 0x0000 );
        LCD_WriteReg ( 0x0090, 0x0010 );
        LCD_WriteReg ( 0x0092, 0x0000 );
        LCD_WriteReg ( 0x0093, 0x0003 );
        LCD_WriteReg ( 0x0095, 0x0110 );
        LCD_WriteReg ( 0x0097, 0x0000 );
        LCD_WriteReg ( 0x0098, 0x0000 );
        LCD_WriteReg ( 0x0007, 0x0133 );
        LCD_WriteReg ( 0x0020, 0x0000 );
        LCD_WriteReg ( 0x0021, 0x013f );
    } else if ( DeviceCode == 0x9320 || DeviceCode == 0x9300 ) {
        LCD_WriteReg ( 0x00, 0x0000 );
        LCD_WriteReg ( 0x01, 0x0100 );
        LCD_WriteReg ( 0x02, 0x0700 );
        LCD_WriteReg ( 0x03, 0x1030 );
        LCD_WriteReg ( 0x04, 0x0000 );
        LCD_WriteReg ( 0x08, 0x0202 );
        LCD_WriteReg ( 0x09, 0x0000 );
        LCD_WriteReg ( 0x0a, 0x0000 );
        LCD_WriteReg ( 0x0c, ( 1 << 0 ) );
        LCD_WriteReg ( 0x0d, 0x0000 );
        LCD_WriteReg ( 0x0f, 0x0000 );
        delay_ms ( 50 );
        LCD_WriteReg ( 0x07, 0x0101 );
        delay_ms ( 50 );
        LCD_WriteReg ( 0x10, ( 1 << 12 ) | ( 0 << 8 ) | ( 1 << 7 ) | ( 1 << 6 ) | ( 0 << 4 ) );
        LCD_WriteReg ( 0x11, 0x0007 );
        LCD_WriteReg ( 0x12, ( 1 << 8 ) | ( 1 << 4 ) | ( 0 << 0 ) );
        LCD_WriteReg ( 0x13, 0x0b00 );
        LCD_WriteReg ( 0x29, 0x0000 );
        LCD_WriteReg ( 0x2b, ( 1 << 14 ) | ( 1 << 4 ) );
        LCD_WriteReg ( 0x50, 0 );
        LCD_WriteReg ( 0x51, 239 );
        LCD_WriteReg ( 0x52, 0 );
        LCD_WriteReg ( 0x53, 319 );
        LCD_WriteReg ( 0x60, 0x2700 );
        LCD_WriteReg ( 0x61, 0x0001 );
        LCD_WriteReg ( 0x6a, 0x0000 );
        LCD_WriteReg ( 0x80, 0x0000 );
        LCD_WriteReg ( 0x81, 0x0000 );
        LCD_WriteReg ( 0x82, 0x0000 );
        LCD_WriteReg ( 0x83, 0x0000 );
        LCD_WriteReg ( 0x84, 0x0000 );
        LCD_WriteReg ( 0x85, 0x0000 );
        LCD_WriteReg ( 0x90, ( 0 << 7 ) | ( 16 << 0 ) );
        LCD_WriteReg ( 0x92, 0x0000 );
        LCD_WriteReg ( 0x93, 0x0001 );
        LCD_WriteReg ( 0x95, 0x0110 );
        LCD_WriteReg ( 0x97, ( 0 << 8 ) );
        LCD_WriteReg ( 0x98, 0x0000 );
        LCD_WriteReg ( 0x07, 0x0173 );
    } else if ( DeviceCode == 0x5408 ) {
        LCD_WriteReg ( 0x01, 0x0100 );
        LCD_WriteReg ( 0x02, 0x0700 );
        LCD_WriteReg ( 0x03, 0x1030 );
        LCD_WriteReg ( 0x04, 0x0000 );
        LCD_WriteReg ( 0x08, 0x0207 );
        LCD_WriteReg ( 0x09, 0x0000 );
        LCD_WriteReg ( 0x0A, 0x0000 );
        LCD_WriteReg ( 0x0C, 0x0000 );
        LCD_WriteReg ( 0x0D, 0x0000 );
        LCD_WriteReg ( 0x0F, 0x0000 );
        delay_ms ( 20 );
        LCD_WriteReg ( 0x10, 0x16B0 );
        LCD_WriteReg ( 0x11, 0x0001 );
        LCD_WriteReg ( 0x17, 0x0001 );
        LCD_WriteReg ( 0x12, 0x0138 );
        LCD_WriteReg ( 0x13, 0x0800 );
        LCD_WriteReg ( 0x29, 0x0009 );
        LCD_WriteReg ( 0x2a, 0x0009 );
        LCD_WriteReg ( 0xa4, 0x0000 );
        LCD_WriteReg ( 0x50, 0x0000 );
        LCD_WriteReg ( 0x51, 0x00EF );
        LCD_WriteReg ( 0x52, 0x0000 );
        LCD_WriteReg ( 0x53, 0x013F );
        LCD_WriteReg ( 0x60, 0x2700 );
        LCD_WriteReg ( 0x61, 0x0001 );
        LCD_WriteReg ( 0x6A, 0x0000 );
        LCD_WriteReg ( 0x80, 0x0000 );
        LCD_WriteReg ( 0x81, 0x0000 );
        LCD_WriteReg ( 0x82, 0x0000 );
        LCD_WriteReg ( 0x83, 0x0000 );
        LCD_WriteReg ( 0x84, 0x0000 );
        LCD_WriteReg ( 0x85, 0x0000 );
        LCD_WriteReg ( 0x90, 0x0013 );
        LCD_WriteReg ( 0x92, 0x0000 );
        LCD_WriteReg ( 0x93, 0x0003 );
        LCD_WriteReg ( 0x95, 0x0110 );
        LCD_WriteReg ( 0x07, 0x0173 );
        delay_ms ( 50 );
    } else if ( DeviceCode == 0x1505 ) {
        LCD_WriteReg ( 0x0007, 0x0000 );
        delay_ms ( 50 );
        LCD_WriteReg ( 0x0012, 0x011C );
        LCD_WriteReg ( 0x00A4, 0x0001 );
        LCD_WriteReg ( 0x0008, 0x000F );
        LCD_WriteReg ( 0x000A, 0x0008 );
        LCD_WriteReg ( 0x000D, 0x0008 );
        LCD_WriteReg ( 0x0030, 0x0707 );
        LCD_WriteReg ( 0x0031, 0x0007 );
        LCD_WriteReg ( 0x0032, 0x0603 );
        LCD_WriteReg ( 0x0033, 0x0700 );
        LCD_WriteReg ( 0x0034, 0x0202 );
        LCD_WriteReg ( 0x0035, 0x0002 );
        LCD_WriteReg ( 0x0036, 0x1F0F );
        LCD_WriteReg ( 0x0037, 0x0707 );
        LCD_WriteReg ( 0x0038, 0x0000 );
        LCD_WriteReg ( 0x0039, 0x0000 );
        LCD_WriteReg ( 0x003A, 0x0707 );
        LCD_WriteReg ( 0x003B, 0x0000 );
        LCD_WriteReg ( 0x003C, 0x0007 );
        LCD_WriteReg ( 0x003D, 0x0000 );
        delay_ms ( 50 );
        LCD_WriteReg ( 0x0007, 0x0001 );
        LCD_WriteReg ( 0x0017, 0x0001 );
        delay_ms ( 50 );
        LCD_WriteReg ( 0x0010, 0x17A0 );
        LCD_WriteReg ( 0x0011, 0x0217 );
        LCD_WriteReg ( 0x0012, 0x011E );
        LCD_WriteReg ( 0x0013, 0x0F00 );
        LCD_WriteReg ( 0x002A, 0x0000 );
        LCD_WriteReg ( 0x0029, 0x000A );
        LCD_WriteReg ( 0x0012, 0x013E );
        LCD_WriteReg ( 0x0050, 0x0000 );
        LCD_WriteReg ( 0x0051, 0x00EF );
        LCD_WriteReg ( 0x0052, 0x0000 );
        LCD_WriteReg ( 0x0053, 0x013F );
        LCD_WriteReg ( 0x0060, 0x2700 );
        LCD_WriteReg ( 0x0061, 0x0001 );
        LCD_WriteReg ( 0x006A, 0x0000 );
        LCD_WriteReg ( 0x0080, 0x0000 );
        LCD_WriteReg ( 0x0081, 0x0000 );
        LCD_WriteReg ( 0x0082, 0x0000 );
        LCD_WriteReg ( 0x0083, 0x0000 );
        LCD_WriteReg ( 0x0084, 0x0000 );
        LCD_WriteReg ( 0x0085, 0x0000 );
        LCD_WriteReg ( 0x0090, 0x0013 );
        LCD_WriteReg ( 0x0092, 0x0300 );
        LCD_WriteReg ( 0x0093, 0x0005 );
        LCD_WriteReg ( 0x0095, 0x0000 );
        LCD_WriteReg ( 0x0097, 0x0000 );
        LCD_WriteReg ( 0x0098, 0x0000 );
        LCD_WriteReg ( 0x0001, 0x0100 );
        LCD_WriteReg ( 0x0002, 0x0700 );
        LCD_WriteReg ( 0x0003, 0x1030 );
        LCD_WriteReg ( 0x0004, 0x0000 );
        LCD_WriteReg ( 0x000C, 0x0000 );
        LCD_WriteReg ( 0x000F, 0x0000 );
        LCD_WriteReg ( 0x0020, 0x0000 );
        LCD_WriteReg ( 0x0021, 0x0000 );
        LCD_WriteReg ( 0x0007, 0x0021 );
        delay_ms ( 20 );
        LCD_WriteReg ( 0x0007, 0x0061 );
        delay_ms ( 20 );
        LCD_WriteReg ( 0x0007, 0x0173 );
        delay_ms ( 20 );
    } else if ( DeviceCode == 0xB505 ) {
        LCD_WriteReg ( 0x0000, 0x0000 );
        LCD_WriteReg ( 0x0000, 0x0000 );
        LCD_WriteReg ( 0x0000, 0x0000 );
        LCD_WriteReg ( 0x0000, 0x0000 );
        LCD_WriteReg ( 0x00a4, 0x0001 );
        delay_ms ( 20 );
        LCD_WriteReg ( 0x0060, 0x2700 );
        LCD_WriteReg ( 0x0008, 0x0202 );
        LCD_WriteReg ( 0x0030, 0x0214 );
        LCD_WriteReg ( 0x0031, 0x3715 );
        LCD_WriteReg ( 0x0032, 0x0604 );
        LCD_WriteReg ( 0x0033, 0x0e16 );
        LCD_WriteReg ( 0x0034, 0x2211 );
        LCD_WriteReg ( 0x0035, 0x1500 );
        LCD_WriteReg ( 0x0036, 0x8507 );
        LCD_WriteReg ( 0x0037, 0x1407 );
        LCD_WriteReg ( 0x0038, 0x1403 );
        LCD_WriteReg ( 0x0039, 0x0020 );
        LCD_WriteReg ( 0x0090, 0x001a );
        LCD_WriteReg ( 0x0010, 0x0000 );
        LCD_WriteReg ( 0x0011, 0x0007 );
        LCD_WriteReg ( 0x0012, 0x0000 );
        LCD_WriteReg ( 0x0013, 0x0000 );
        delay_ms ( 20 );
        LCD_WriteReg ( 0x0010, 0x0730 );
        LCD_WriteReg ( 0x0011, 0x0137 );
        delay_ms ( 20 );
        LCD_WriteReg ( 0x0012, 0x01b8 );
        delay_ms ( 20 );
        LCD_WriteReg ( 0x0013, 0x0f00 );
        LCD_WriteReg ( 0x002a, 0x0080 );
        LCD_WriteReg ( 0x0029, 0x0048 );
        delay_ms ( 20 );
        LCD_WriteReg ( 0x0001, 0x0100 );
        LCD_WriteReg ( 0x0002, 0x0700 );
        LCD_WriteReg ( 0x0003, 0x1230 );
        LCD_WriteReg ( 0x0008, 0x0202 );
        LCD_WriteReg ( 0x000a, 0x0000 );
        LCD_WriteReg ( 0x000c, 0x0000 );
        LCD_WriteReg ( 0x000d, 0x0000 );
        LCD_WriteReg ( 0x000e, 0x0030 );
        LCD_WriteReg ( 0x0050, 0x0000 );
        LCD_WriteReg ( 0x0051, 0x00ef );
        LCD_WriteReg ( 0x0052, 0x0000 );
        LCD_WriteReg ( 0x0053, 0x013f );
        LCD_WriteReg ( 0x0060, 0x2700 );
        LCD_WriteReg ( 0x0061, 0x0001 );
        LCD_WriteReg ( 0x006a, 0x0000 );
        LCD_WriteReg ( 0x0090, 0X0011 );
        LCD_WriteReg ( 0x0092, 0x0600 );
        LCD_WriteReg ( 0x0093, 0x0402 );
        LCD_WriteReg ( 0x0094, 0x0002 );
        delay_ms ( 20 );
        LCD_WriteReg ( 0x0007, 0x0001 );
        delay_ms ( 20 );
        LCD_WriteReg ( 0x0007, 0x0061 );
        LCD_WriteReg ( 0x0007, 0x0173 );
        LCD_WriteReg ( 0x0020, 0x0000 );
        LCD_WriteReg ( 0x0021, 0x0000 );
        LCD_WriteReg ( 0x00, 0x22 );
    } else if ( DeviceCode == 0xC505 ) {
        LCD_WriteReg ( 0x0000, 0x0000 );
        LCD_WriteReg ( 0x0000, 0x0000 );
        delay_ms ( 20 );
        LCD_WriteReg ( 0x0000, 0x0000 );
        LCD_WriteReg ( 0x0000, 0x0000 );
        LCD_WriteReg ( 0x0000, 0x0000 );
        LCD_WriteReg ( 0x0000, 0x0000 );
        LCD_WriteReg ( 0x00a4, 0x0001 );
        delay_ms ( 20 );
        LCD_WriteReg ( 0x0060, 0x2700 );
        LCD_WriteReg ( 0x0008, 0x0806 );
        LCD_WriteReg ( 0x0030, 0x0703 );
        LCD_WriteReg ( 0x0031, 0x0001 );
        LCD_WriteReg ( 0x0032, 0x0004 );
        LCD_WriteReg ( 0x0033, 0x0102 );
        LCD_WriteReg ( 0x0034, 0x0300 );
        LCD_WriteReg ( 0x0035, 0x0103 );
        LCD_WriteReg ( 0x0036, 0x001F );
        LCD_WriteReg ( 0x0037, 0x0703 );
        LCD_WriteReg ( 0x0038, 0x0001 );
        LCD_WriteReg ( 0x0039, 0x0004 );
        LCD_WriteReg ( 0x0090, 0x0015 );
        LCD_WriteReg ( 0x0010, 0X0410 );
        LCD_WriteReg ( 0x0011, 0x0247 );
        LCD_WriteReg ( 0x0012, 0x01BC );
        LCD_WriteReg ( 0x0013, 0x0e00 );
        delay_ms ( 120 );
        LCD_WriteReg ( 0x0001, 0x0100 );
        LCD_WriteReg ( 0x0002, 0x0200 );
        LCD_WriteReg ( 0x0003, 0x1030 );
        LCD_WriteReg ( 0x000A, 0x0008 );
        LCD_WriteReg ( 0x000C, 0x0000 );
        LCD_WriteReg ( 0x000E, 0x0020 );
        LCD_WriteReg ( 0x000F, 0x0000 );
        LCD_WriteReg ( 0x0020, 0x0000 );
        LCD_WriteReg ( 0x0021, 0x0000 );
        LCD_WriteReg ( 0x002A, 0x003D );
        delay_ms ( 20 );
        LCD_WriteReg ( 0x0029, 0x002d );
        LCD_WriteReg ( 0x0050, 0x0000 );
        LCD_WriteReg ( 0x0051, 0xD0EF );
        LCD_WriteReg ( 0x0052, 0x0000 );
        LCD_WriteReg ( 0x0053, 0x013F );
        LCD_WriteReg ( 0x0061, 0x0000 );
        LCD_WriteReg ( 0x006A, 0x0000 );
        LCD_WriteReg ( 0x0092, 0x0300 );
        LCD_WriteReg ( 0x0093, 0x0005 );
        LCD_WriteReg ( 0x0007, 0x0100 );
    } else if ( DeviceCode == 0x8989 ) {
        LCD_WriteReg ( 0x0000, 0x0001 );
        LCD_WriteReg ( 0x0003, 0xA8A4 );
        LCD_WriteReg ( 0x000C, 0x0000 );
        LCD_WriteReg ( 0x000D, 0x080C );
        LCD_WriteReg ( 0x000E, 0x2B00 );
        LCD_WriteReg ( 0x001E, 0x00B0 );
        LCD_WriteReg ( 0x0001, 0x2B3F );
        LCD_WriteReg ( 0x0002, 0x0600 );
        LCD_WriteReg ( 0x0010, 0x0000 );
        LCD_WriteReg ( 0x0011, 0x6070 );
        LCD_WriteReg ( 0x0005, 0x0000 );
        LCD_WriteReg ( 0x0006, 0x0000 );
        LCD_WriteReg ( 0x0016, 0xEF1C );
        LCD_WriteReg ( 0x0017, 0x0003 );
        LCD_WriteReg ( 0x0007, 0x0233 );
        LCD_WriteReg ( 0x000B, 0x0000 );
        LCD_WriteReg ( 0x000F, 0x0000 );
        LCD_WriteReg ( 0x0041, 0x0000 );
        LCD_WriteReg ( 0x0042, 0x0000 );
        LCD_WriteReg ( 0x0048, 0x0000 );
        LCD_WriteReg ( 0x0049, 0x013F );
        LCD_WriteReg ( 0x004A, 0x0000 );
        LCD_WriteReg ( 0x004B, 0x0000 );
        LCD_WriteReg ( 0x0044, 0xEF00 );
        LCD_WriteReg ( 0x0045, 0x0000 );
        LCD_WriteReg ( 0x0046, 0x013F );
        LCD_WriteReg ( 0x0030, 0x0707 );
        LCD_WriteReg ( 0x0031, 0x0204 );
        LCD_WriteReg ( 0x0032, 0x0204 );
        LCD_WriteReg ( 0x0033, 0x0502 );
        LCD_WriteReg ( 0x0034, 0x0507 );
        LCD_WriteReg ( 0x0035, 0x0204 );
        LCD_WriteReg ( 0x0036, 0x0204 );
        LCD_WriteReg ( 0x0037, 0x0502 );
        LCD_WriteReg ( 0x003A, 0x0302 );
        LCD_WriteReg ( 0x003B, 0x0302 );
        LCD_WriteReg ( 0x0023, 0x0000 );
        LCD_WriteReg ( 0x0024, 0x0000 );
        LCD_WriteReg ( 0x0025, 0x8000 );
        LCD_WriteReg ( 0x004f, 0 );
        LCD_WriteReg ( 0x004e, 0 );
    } else if ( DeviceCode == 0x4531 ) {
        LCD_WriteReg ( 0X00, 0X0001 );
        delay_ms ( 10 );
        LCD_WriteReg ( 0X10, 0X1628 );
        LCD_WriteReg ( 0X12, 0X000e );
        LCD_WriteReg ( 0X13, 0X0A39 );
        delay_ms ( 10 );
        LCD_WriteReg ( 0X11, 0X0040 );
        LCD_WriteReg ( 0X15, 0X0050 );
        delay_ms ( 10 );
        LCD_WriteReg ( 0X12, 0X001e );
        delay_ms ( 10 );
        LCD_WriteReg ( 0X10, 0X1620 );
        LCD_WriteReg ( 0X13, 0X2A39 );
        delay_ms ( 10 );
        LCD_WriteReg ( 0X01, 0X0100 );
        LCD_WriteReg ( 0X02, 0X0300 );
        LCD_WriteReg ( 0X03, 0X1030 );
        LCD_WriteReg ( 0X08, 0X0202 );
        LCD_WriteReg ( 0X0A, 0X0008 );
        LCD_WriteReg ( 0X30, 0X0000 );
        LCD_WriteReg ( 0X31, 0X0402 );
        LCD_WriteReg ( 0X32, 0X0106 );
        LCD_WriteReg ( 0X33, 0X0503 );
        LCD_WriteReg ( 0X34, 0X0104 );
        LCD_WriteReg ( 0X35, 0X0301 );
        LCD_WriteReg ( 0X36, 0X0707 );
        LCD_WriteReg ( 0X37, 0X0305 );
        LCD_WriteReg ( 0X38, 0X0208 );
        LCD_WriteReg ( 0X39, 0X0F0B );
        LCD_WriteReg ( 0X41, 0X0002 );
        LCD_WriteReg ( 0X60, 0X2700 );
        LCD_WriteReg ( 0X61, 0X0001 );
        LCD_WriteReg ( 0X90, 0X0210 );
        LCD_WriteReg ( 0X92, 0X010A );
        LCD_WriteReg ( 0X93, 0X0004 );
        LCD_WriteReg ( 0XA0, 0X0100 );
        LCD_WriteReg ( 0X07, 0X0001 );
        LCD_WriteReg ( 0X07, 0X0021 );
        LCD_WriteReg ( 0X07, 0X0023 );
        LCD_WriteReg ( 0X07, 0X0033 );
        LCD_WriteReg ( 0X07, 0X0133 );
        LCD_WriteReg ( 0XA0, 0X0000 );
    } else if ( DeviceCode == 0x4535 ) {
        LCD_WriteReg ( 0X15, 0X0030 );
        LCD_WriteReg ( 0X9A, 0X0010 );
        LCD_WriteReg ( 0X11, 0X0020 );
        LCD_WriteReg ( 0X10, 0X3428 );
        LCD_WriteReg ( 0X12, 0X0002 );
        LCD_WriteReg ( 0X13, 0X1038 );
        delay_ms ( 40 );
        LCD_WriteReg ( 0X12, 0X0012 );
        delay_ms ( 40 );
        LCD_WriteReg ( 0X10, 0X3420 );
        LCD_WriteReg ( 0X13, 0X3038 );
        delay_ms ( 70 );
        LCD_WriteReg ( 0X30, 0X0000 );
        LCD_WriteReg ( 0X31, 0X0402 );
        LCD_WriteReg ( 0X32, 0X0307 );
        LCD_WriteReg ( 0X33, 0X0304 );
        LCD_WriteReg ( 0X34, 0X0004 );
        LCD_WriteReg ( 0X35, 0X0401 );
        LCD_WriteReg ( 0X36, 0X0707 );
        LCD_WriteReg ( 0X37, 0X0305 );
        LCD_WriteReg ( 0X38, 0X0610 );
        LCD_WriteReg ( 0X39, 0X0610 );
        LCD_WriteReg ( 0X01, 0X0100 );
        LCD_WriteReg ( 0X02, 0X0300 );
        LCD_WriteReg ( 0X03, 0X1030 );
        LCD_WriteReg ( 0X08, 0X0808 );
        LCD_WriteReg ( 0X0A, 0X0008 );
        LCD_WriteReg ( 0X60, 0X2700 );
        LCD_WriteReg ( 0X61, 0X0001 );
        LCD_WriteReg ( 0X90, 0X013E );
        LCD_WriteReg ( 0X92, 0X0100 );
        LCD_WriteReg ( 0X93, 0X0100 );
        LCD_WriteReg ( 0XA0, 0X3000 );
        LCD_WriteReg ( 0XA3, 0X0010 );
        LCD_WriteReg ( 0X07, 0X0001 );
        LCD_WriteReg ( 0X07, 0X0021 );
        LCD_WriteReg ( 0X07, 0X0023 );
        LCD_WriteReg ( 0X07, 0X0033 );
        LCD_WriteReg ( 0X07, 0X0133 );
    }

    LCD_Scan_Dir ( DFT_SCAN_DIR );
    LCD_LED = 1;
    LCD_Clear ( WHITE );
}

void LCD_Clear ( u16 Color ) {
    u32 index = 0;
    LCD_SetCursor ( 0x00, 0x0000 );
    LCD_WriteRAM_Prepare();

    for ( index = 0; index < 76800; index++ ) {
        LCD_WR_DATA ( Color );
    }
}

void LCD_Fill ( u16 sx, u16 sy, u16 ex, u16 ey, u16 color ) {
    u16 i, j;
    u16 xlen = 0;
    LCD_Scan_Dir ( L2R_U2D );
#if USE_HORIZONTAL==1
    xlen = ey - sy + 1;

    for ( i = sx; i <= ex; i++ ) {
        LCD_SetCursor ( i, sy );
        LCD_WriteRAM_Prepare();

        for ( j = 0; j < xlen; j++ ) {
            LCD_WR_DATA ( color );
        }
    }

#else
    xlen = ex - sx + 1;

    for ( i = sy; i <= ey; i++ ) {
        LCD_SetCursor ( sx, i );
        LCD_WriteRAM_Prepare();

        for ( j = 0; j < xlen; j++ ) {
            LCD_WR_DATA ( color );
        }
    }

#endif
    LCD_Scan_Dir ( DFT_SCAN_DIR );
}

void LCD_DrawLine ( u16 x1, u16 y1, u16 x2, u16 y2 ) {
    u16 t;
    int xerr = 0, yerr = 0, delta_x, delta_y, distance;
    int incx, incy, uRow, uCol;
    delta_x = x2 - x1;
    delta_y = y2 - y1;
    uRow = x1;
    uCol = y1;

    if ( delta_x > 0 ) {
        incx = 1;
    } else if ( delta_x == 0 ) {
        incx = 0;
    } else {
        incx = -1;
        delta_x = -delta_x;
    }

    if ( delta_y > 0 ) {
        incy = 1;
    } else if ( delta_y == 0 ) {
        incy = 0;
    } else {
        incy = -1;
        delta_y = -delta_y;
    }

    if ( delta_x > delta_y ) {
        distance = delta_x;
    } else {
        distance = delta_y;
    }

    for ( t = 0; t <= distance + 1; t++ ) {
        LCD_DrawPoint ( uRow, uCol );
        xerr += delta_x ;
        yerr += delta_y ;

        if ( xerr > distance ) {
            xerr -= distance;
            uRow += incx;
        }

        if ( yerr > distance ) {
            yerr -= distance;
            uCol += incy;
        }
    }
}

void LCD_DrawRectangle ( u16 x1, u16 y1, u16 x2, u16 y2 ) {
    LCD_DrawLine ( x1, y1, x2, y1 );
    LCD_DrawLine ( x1, y1, x1, y2 );
    LCD_DrawLine ( x1, y2, x2, y2 );
    LCD_DrawLine ( x2, y1, x2, y2 );
}

void Draw_Circle ( u16 x0, u16 y0, u8 r ) {
    int a, b;
    int di;
    a = 0;
    b = r;
    di = 3 - ( r << 1 );

    while ( a <= b ) {
        LCD_DrawPoint ( x0 - b, y0 - a );
        LCD_DrawPoint ( x0 + b, y0 - a );
        LCD_DrawPoint ( x0 - a, y0 + b );
        LCD_DrawPoint ( x0 - b, y0 - a );
        LCD_DrawPoint ( x0 - a, y0 - b );
        LCD_DrawPoint ( x0 + b, y0 + a );
        LCD_DrawPoint ( x0 + a, y0 - b );
        LCD_DrawPoint ( x0 + a, y0 + b );
        LCD_DrawPoint ( x0 - b, y0 + a );
        a++;

        if ( di < 0 ) {
            di += 4 * a + 6;
        } else {
            di += 10 + 4 * ( a - b );
            b--;
        }

        LCD_DrawPoint ( x0 + a, y0 + b );
    }
}

void LCD_ShowChar ( u16 x, u16 y, u8 num, u8 size, u8 mode ) {
#if USE_HORIZONTAL==1
#define MAX_CHAR_POSX 312
#define MAX_CHAR_POSY 232
#else
#define MAX_CHAR_POSX 232
#define MAX_CHAR_POSY 312
#endif
    u8 temp;
    u8 pos, t;
    u16 x0 = x;
    u16 colortemp = POINT_COLOR;

    if ( x > MAX_CHAR_POSX || y > MAX_CHAR_POSY ) {
        return;
    }

    num = num - ' ';

    if ( !mode ) {
        for ( pos = 0; pos < size; pos++ ) {
            if ( size == 12 ) {
                temp = asc2_1206[num][pos];
            } else {
                temp = asc2_1608[num][pos];
            }

            for ( t = 0; t < size / 2; t++ ) {
                if ( temp & 0x01 ) {
                    POINT_COLOR = colortemp;
                } else {
                    POINT_COLOR = BACK_COLOR;
                }

                LCD_DrawPoint ( x, y );
                temp >>= 1;
                x++;
            }

            x = x0;
            y++;
        }
    } else {
        for ( pos = 0; pos < size; pos++ ) {
            if ( size == 12 ) {
                temp = asc2_1206[num][pos];
            } else {
                temp = asc2_1608[num][pos];
            }

            for ( t = 0; t < size / 2; t++ ) {
                if ( temp & 0x01 ) {
                    LCD_DrawPoint ( x + t, y + pos );
                }

                temp >>= 1;
            }
        }
    }

    POINT_COLOR = colortemp;
}

u32 mypow ( u8 m, u8 n ) {
    u32 result = 1;

    while ( n-- ) {
        result *= m;
    }

    return result;
}

void LCD_ShowNum ( u16 x, u16 y, u32 num, u8 len, u8 size ) {
    u8 t, temp;
    u8 enshow = 0;

    for ( t = 0; t < len; t++ ) {
        temp = ( num / mypow ( 10, len - t - 1 ) ) % 10;

        if ( enshow == 0 && t < ( len - 1 ) ) {
            if ( temp == 0 ) {
                LCD_ShowChar ( x + ( size / 2 ) *t, y, ' ', size, 0 );
                continue;
            } else {
                enshow = 1;
            }
        }

        LCD_ShowChar ( x + ( size / 2 ) *t, y, temp + '0', size, 0 );
    }
}

void LCD_Show2Num ( u16 x, u16 y, u16 num, u8 len, u8 size, u8 mode ) {
    u8 t, temp;

    for ( t = 0; t < len; t++ ) {
        temp = ( num / mypow ( 10, len - t - 1 ) ) % 10;
        LCD_ShowChar ( x + ( size / 2 ) *t, y, temp + '0', size, mode );
    }
}

void LCD_ShowString ( u16 x, u16 y, const u8 *p ) {
    while ( *p != '\0' ) {
        if ( x > MAX_CHAR_POSX ) {
            x = 0;
            y += 16;
        }

        if ( y > MAX_CHAR_POSY ) {
            y = x = 0;
            LCD_Clear ( WHITE );
        }

        LCD_ShowChar ( x, y, *p, 16, 0 );
        x += 8;
        p++;
    }
}