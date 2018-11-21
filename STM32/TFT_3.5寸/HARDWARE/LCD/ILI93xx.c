#include "lcd.h"
#include "stdlib.h"
#include "font.h"
#include "usart.h"
#include "delay.h"

u16 POINT_COLOR = 0x0000;
u16 BACK_COLOR = 0xFFFF;

_lcd_dev lcddev;

void LCD_WR_REG ( u16 data ) {
    LCD_RS_CLR;
    LCD_CS_CLR;
    DATAOUT ( data );
    LCD_WR_CLR;
    LCD_WR_SET;
    LCD_CS_SET;
}

u16 LCD_RD_DATA ( void ) {
    u16 t;
    GPIOB->CRL = 0X88888888;
    GPIOB->CRH = 0X88888888;
    GPIOB->ODR = 0X0000;
    LCD_RS_SET;
    LCD_CS_CLR;
    LCD_RD_CLR;

    if ( lcddev.id == 0X8989 ) {
        delay_us ( 2 );
    }

    t = DATAIN;
    LCD_RD_SET;
    LCD_CS_SET;
    GPIOB->CRL = 0X33333333;
    GPIOB->CRH = 0X33333333;
    GPIOB->ODR = 0XFFFF;
    return t;
}

void LCD_WriteReg ( u16 LCD_Reg, u16 LCD_RegValue ) {
    LCD_WR_REG ( LCD_Reg );
    LCD_WR_DATA ( LCD_RegValue );
}

u16 LCD_ReadReg ( u16 LCD_Reg ) {
    LCD_WR_REG ( LCD_Reg );
    return LCD_RD_DATA();
}

void LCD_WriteRAM_Prepare ( void ) {
    LCD_WR_REG ( lcddev.wramcmd );
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

    if ( x >= lcddev.width || y >= lcddev.height ) {
        return 0;
    }

    LCD_SetCursor ( x, y );

    if ( lcddev.id == 0X9341 || lcddev.id == 0X6804 || lcddev.id == 0X5310 ) {
        LCD_WR_REG ( 0X2E );
    } else if ( lcddev.id == 0X5510 ) {
        LCD_WR_REG ( 0X2E00 );
    } else {
        LCD_WR_REG ( R34 );
    }

    GPIOB->CRL = 0X88888888;
    GPIOB->CRH = 0X88888888;
    GPIOB->ODR = 0XFFFF;
    LCD_RS_SET;
    LCD_CS_CLR;
    LCD_RD_CLR;
    delay_us ( 1 );
    LCD_RD_SET;
    LCD_RD_CLR;
    delay_us ( 1 );
    LCD_RD_SET;
    r = DATAIN;

    if ( lcddev.id == 0X9341 || lcddev.id == 0X5310 || lcddev.id == 0X5510 ) {
        LCD_RD_CLR;
        LCD_RD_SET;
        b = DATAIN;
        g = r & 0XFF;
        g <<= 8;
    } else if ( lcddev.id == 0X6804 ) {
        LCD_RD_CLR;
        LCD_RD_SET;
        r = DATAIN;
    }

    LCD_CS_SET;
    GPIOB->CRL = 0X33333333;
    GPIOB->CRH = 0X33333333;
    GPIOB->ODR = 0XFFFF;

    if ( lcddev.id == 0X9325 || lcddev.id == 0X4535 || lcddev.id == 0X4531 || lcddev.id == 0X8989 || lcddev.id == 0XB505 ) {
        return r;
    } else if ( lcddev.id == 0X9341 || lcddev.id == 0X5310 || lcddev.id == 0X5510 ) {
        return ( ( ( r >> 11 ) << 11 ) | ( ( g >> 10 ) << 5 ) | ( b >> 11 ) );
    } else {
        return LCD_BGR2RGB ( r );
    }
}

void LCD_DisplayOn ( void ) {
    if ( lcddev.id == 0X9341 || lcddev.id == 0X6804 || lcddev.id == 0X5310 ) {
        LCD_WR_REG ( 0X29 );
    } else if ( lcddev.id == 0X5510 ) {
        LCD_WR_REG ( 0X2900 );
    } else {
        LCD_WriteReg ( R7, 0x0173 );
    }
}

void LCD_DisplayOff ( void ) {
    if ( lcddev.id == 0X9341 || lcddev.id == 0X6804 || lcddev.id == 0X5310 ) {
        LCD_WR_REG ( 0X28 );
    } else if ( lcddev.id == 0X5510 ) {
        LCD_WR_REG ( 0X2800 );
    } else {
        LCD_WriteReg ( R7, 0x0 );
    }
}

void LCD_SetCursor ( u16 Xpos, u16 Ypos ) {
    if ( lcddev.id == 0X9341 || lcddev.id == 0X5310 ) {
        LCD_WR_REG ( lcddev.setxcmd );
        LCD_WR_DATA ( Xpos >> 8 );
        LCD_WR_DATA ( Xpos & 0XFF );
        LCD_WR_REG ( lcddev.setycmd );
        LCD_WR_DATA ( Ypos >> 8 );
        LCD_WR_DATA ( Ypos & 0XFF );
    } else if ( lcddev.id == 0X6804 ) {
        if ( lcddev.dir == 1 ) {
            Xpos = lcddev.width - 1 - Xpos;
        }

        LCD_WR_REG ( lcddev.setxcmd );
        LCD_WR_DATA ( Xpos >> 8 );
        LCD_WR_DATA ( Xpos & 0XFF );
        LCD_WR_REG ( lcddev.setycmd );
        LCD_WR_DATA ( Ypos >> 8 );
        LCD_WR_DATA ( Ypos & 0XFF );
    } else if ( lcddev.id == 0X5510 ) {
        LCD_WR_REG ( lcddev.setxcmd );
        LCD_WR_DATA ( Xpos >> 8 );
        LCD_WR_REG ( lcddev.setxcmd + 1 );
        LCD_WR_DATA ( Xpos & 0XFF );
        LCD_WR_REG ( lcddev.setycmd );
        LCD_WR_DATA ( Ypos >> 8 );
        LCD_WR_REG ( lcddev.setycmd + 1 );
        LCD_WR_DATA ( Ypos & 0XFF );
    } else {
        if ( lcddev.dir == 1 ) {
            Xpos = lcddev.width - 1 - Xpos;
        }

        LCD_WriteReg ( lcddev.setxcmd, Xpos );
        LCD_WriteReg ( lcddev.setycmd, Ypos );
    }
}

void LCD_Scan_Dir ( u8 dir ) {
    u16 regval = 0;
    u16 dirreg = 0;
    u16 temp;

    if ( lcddev.dir == 1 && lcddev.id != 0X6804 ) {
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
    }

    if ( lcddev.id == 0x9341 || lcddev.id == 0X6804 || lcddev.id == 0X5310 || lcddev.id == 0X5510 ) {
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

        if ( lcddev.id == 0X5510 ) {
            dirreg = 0X3600;
        } else {
            dirreg = 0X36;
        }

        if ( ( lcddev.id != 0X5310 ) && ( lcddev.id != 0X5510 ) ) {
            regval |= 0X08;
        }

        if ( lcddev.id == 0X6804 ) {
            regval |= 0x02;
        }

        LCD_WriteReg ( dirreg, regval );

        if ( ( regval & 0X20 ) || lcddev.dir == 1 ) {
            if ( lcddev.width < lcddev.height ) {
                temp = lcddev.width;
                lcddev.width = lcddev.height;
                lcddev.height = temp;
            }
        } else {
            if ( lcddev.width > lcddev.height ) {
                temp = lcddev.width;
                lcddev.width = lcddev.height;
                lcddev.height = temp;
            }
        }

        if ( lcddev.id == 0X5510 ) {
            LCD_WR_REG ( lcddev.setxcmd );
            LCD_WR_DATA ( 0 );
            LCD_WR_REG ( lcddev.setxcmd + 1 );
            LCD_WR_DATA ( 0 );
            LCD_WR_REG ( lcddev.setxcmd + 2 );
            LCD_WR_DATA ( ( lcddev.width - 1 ) >> 8 );
            LCD_WR_REG ( lcddev.setxcmd + 3 );
            LCD_WR_DATA ( ( lcddev.width - 1 ) & 0XFF );
            LCD_WR_REG ( lcddev.setycmd );
            LCD_WR_DATA ( 0 );
            LCD_WR_REG ( lcddev.setycmd + 1 );
            LCD_WR_DATA ( 0 );
            LCD_WR_REG ( lcddev.setycmd + 2 );
            LCD_WR_DATA ( ( lcddev.height - 1 ) >> 8 );
            LCD_WR_REG ( lcddev.setycmd + 3 );
            LCD_WR_DATA ( ( lcddev.height - 1 ) & 0XFF );
        } else {
            LCD_WR_REG ( lcddev.setxcmd );
            LCD_WR_DATA ( 0 );
            LCD_WR_DATA ( 0 );
            LCD_WR_DATA ( ( lcddev.width - 1 ) >> 8 );
            LCD_WR_DATA ( ( lcddev.width - 1 ) & 0XFF );
            LCD_WR_REG ( lcddev.setycmd );
            LCD_WR_DATA ( 0 );
            LCD_WR_DATA ( 0 );
            LCD_WR_DATA ( ( lcddev.height - 1 ) >> 8 );
            LCD_WR_DATA ( ( lcddev.height - 1 ) & 0XFF );
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

        if ( lcddev.id == 0x8989 ) {
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

void LCD_Fast_DrawPoint ( u16 x, u16 y, u16 color ) {
    if ( lcddev.id == 0X9341 || lcddev.id == 0X5310 ) {
        LCD_WR_REG ( lcddev.setxcmd );
        LCD_WR_DATA ( x >> 8 );
        LCD_WR_DATA ( x & 0XFF );
        LCD_WR_REG ( lcddev.setycmd );
        LCD_WR_DATA ( y >> 8 );
        LCD_WR_DATA ( y & 0XFF );
    } else if ( lcddev.id == 0X5510 ) {
        LCD_WR_REG ( lcddev.setxcmd );
        LCD_WR_DATA ( x >> 8 );
        LCD_WR_REG ( lcddev.setxcmd + 1 );
        LCD_WR_DATA ( x & 0XFF );
        LCD_WR_REG ( lcddev.setycmd );
        LCD_WR_DATA ( y >> 8 );
        LCD_WR_REG ( lcddev.setycmd + 1 );
        LCD_WR_DATA ( y & 0XFF );
    } else if ( lcddev.id == 0X6804 ) {
        if ( lcddev.dir == 1 ) {
            x = lcddev.width - 1 - x;
        }

        LCD_WR_REG ( lcddev.setxcmd );
        LCD_WR_DATA ( x >> 8 );
        LCD_WR_DATA ( x & 0XFF );
        LCD_WR_REG ( lcddev.setycmd );
        LCD_WR_DATA ( y >> 8 );
        LCD_WR_DATA ( y & 0XFF );
    } else {
        if ( lcddev.dir == 1 ) {
            x = lcddev.width - 1 - x;
        }

        LCD_WriteReg ( lcddev.setxcmd, x );
        LCD_WriteReg ( lcddev.setycmd, y );
    }

    LCD_WR_REG ( lcddev.wramcmd );
    LCD_WR_DATA ( color );
}

void LCD_Display_Dir ( u8 dir ) {
    if ( dir == 0 ) {
        lcddev.dir = 0;
        lcddev.width = 240;
        lcddev.height = 320;

        if ( lcddev.id == 0X9341 || lcddev.id == 0X6804 || lcddev.id == 0X5310 ) {
            lcddev.wramcmd = 0X2C;
            lcddev.setxcmd = 0X2A;
            lcddev.setycmd = 0X2B;

            if ( lcddev.id == 0X6804 || lcddev.id == 0X5310 ) {
                lcddev.width = 320;
                lcddev.height = 480;
            }
        } else if ( lcddev.id == 0X8989 ) {
            lcddev.wramcmd = R34;
            lcddev.setxcmd = 0X4E;
            lcddev.setycmd = 0X4F;
        } else if ( lcddev.id == 0x5510 ) {
            lcddev.wramcmd = 0X2C00;
            lcddev.setxcmd = 0X2A00;
            lcddev.setycmd = 0X2B00;
            lcddev.width = 480;
            lcddev.height = 800;
        } else {
            lcddev.wramcmd = R34;
            lcddev.setxcmd = R32;
            lcddev.setycmd = R33;
        }
    } else {
        lcddev.dir = 1;
        lcddev.width = 320;
        lcddev.height = 240;

        if ( lcddev.id == 0X9341 || lcddev.id == 0X5310 ) {
            lcddev.wramcmd = 0X2C;
            lcddev.setxcmd = 0X2A;
            lcddev.setycmd = 0X2B;
        } else if ( lcddev.id == 0X6804 ) {
            lcddev.wramcmd = 0X2C;
            lcddev.setxcmd = 0X2B;
            lcddev.setycmd = 0X2A;
        } else if ( lcddev.id == 0X8989 ) {
            lcddev.wramcmd = R34;
            lcddev.setxcmd = 0X4F;
            lcddev.setycmd = 0X4E;
        } else if ( lcddev.id == 0x5510 ) {
            lcddev.wramcmd = 0X2C00;
            lcddev.setxcmd = 0X2A00;
            lcddev.setycmd = 0X2B00;
            lcddev.width = 800;
            lcddev.height = 480;
        } else {
            lcddev.wramcmd = R34;
            lcddev.setxcmd = R33;
            lcddev.setycmd = R32;
        }

        if ( lcddev.id == 0X6804 || lcddev.id == 0X5310 ) {
            lcddev.width = 480;
            lcddev.height = 320;
        }
    }

    LCD_Scan_Dir ( DFT_SCAN_DIR );
}

void LCD_Set_Window ( u16 sx, u16 sy, u16 width, u16 height ) {
    u8 hsareg, heareg, vsareg, veareg;
    u16 hsaval, heaval, vsaval, veaval;
    width = sx + width - 1;
    height = sy + height - 1;

    if ( lcddev.id == 0X9341 || lcddev.id == 0X5310 || lcddev.id == 0X6804 ) {
        LCD_WR_REG ( lcddev.setxcmd );
        LCD_WR_DATA ( sx >> 8 );
        LCD_WR_DATA ( sx & 0XFF );
        LCD_WR_DATA ( width >> 8 );
        LCD_WR_DATA ( width & 0XFF );
        LCD_WR_REG ( lcddev.setycmd );
        LCD_WR_DATA ( sy >> 8 );
        LCD_WR_DATA ( sy & 0XFF );
        LCD_WR_DATA ( height >> 8 );
        LCD_WR_DATA ( height & 0XFF );
    } else if ( lcddev.id == 0X5510 ) {
        LCD_WR_REG ( lcddev.setxcmd );
        LCD_WR_DATA ( sx >> 8 );
        LCD_WR_REG ( lcddev.setxcmd + 1 );
        LCD_WR_DATA ( sx & 0XFF );
        LCD_WR_REG ( lcddev.setxcmd + 2 );
        LCD_WR_DATA ( width >> 8 );
        LCD_WR_REG ( lcddev.setxcmd + 3 );
        LCD_WR_DATA ( width & 0XFF );
        LCD_WR_REG ( lcddev.setycmd );
        LCD_WR_DATA ( sy >> 8 );
        LCD_WR_REG ( lcddev.setycmd + 1 );
        LCD_WR_DATA ( sy & 0XFF );
        LCD_WR_REG ( lcddev.setycmd + 2 );
        LCD_WR_DATA ( height >> 8 );
        LCD_WR_REG ( lcddev.setycmd + 3 );
        LCD_WR_DATA ( height & 0XFF );
    } else {
        if ( lcddev.dir == 1 ) {
            hsaval = sy;
            heaval = height;
            vsaval = lcddev.width - width - 1;
            veaval = lcddev.width - sx - 1;
        } else {
            hsaval = sx;
            heaval = width;
            vsaval = sy;
            veaval = height;
        }

        if ( lcddev.id == 0X8989 ) {
            hsareg = 0X44;
            heareg = 0X44;
            hsaval |= ( heaval << 8 );
            heaval = hsaval;
            vsareg = 0X45;
            veareg = 0X46;
        } else {
            hsareg = 0X50;
            heareg = 0X51;
            vsareg = 0X52;
            veareg = 0X53;
        }

        LCD_WriteReg ( hsareg, hsaval );
        LCD_WriteReg ( heareg, heaval );
        LCD_WriteReg ( vsareg, vsaval );
        LCD_WriteReg ( veareg, veaval );
        LCD_SetCursor ( sx, sy );
    }
}

void LCD_Init ( void ) {
    RCC->APB2ENR |= 1 << 3;
    RCC->APB2ENR |= 1 << 4;
    RCC->APB2ENR |= 1 << 0;
    GPIOC->CRH &= 0XFFFFF000;
    GPIOC->CRH |= 0X00000333;
    GPIOC->CRL &= 0X00FFFFFF;
    GPIOC->CRL |= 0X33000000;
    GPIOC->ODR |= 0X07C0;
    GPIOB->CRH = 0X33333333;
    GPIOB->CRL = 0X33333333;
    GPIOB->ODR = 0XFFFF;
    delay_ms ( 50 );
    LCD_WriteReg ( 0x0000, 0x0001 );
    delay_ms ( 50 );
    lcddev.id = LCD_ReadReg ( 0x0000 );

    if ( lcddev.id < 0XFF || lcddev.id == 0XFFFF || lcddev.id == 0X9300 ) {
        LCD_WR_REG ( 0XD3 );
        LCD_RD_DATA();
        LCD_RD_DATA();
        lcddev.id = LCD_RD_DATA();
        lcddev.id <<= 8;
        lcddev.id |= LCD_RD_DATA();

        if ( lcddev.id != 0X9341 ) {
            LCD_WR_REG ( 0XBF );
            LCD_RD_DATA();
            LCD_RD_DATA();
            LCD_RD_DATA();
            lcddev.id = LCD_RD_DATA();
            lcddev.id <<= 8;
            lcddev.id |= LCD_RD_DATA();

            if ( lcddev.id != 0X6804 ) {
                LCD_WR_REG ( 0XD4 );
                LCD_RD_DATA();
                LCD_RD_DATA();
                lcddev.id = LCD_RD_DATA();
                lcddev.id <<= 8;
                lcddev.id |= LCD_RD_DATA();

                if ( lcddev.id != 0X5310 ) {
                    LCD_WR_REG ( 0XDA00 );
                    LCD_RD_DATA();
                    LCD_WR_REG ( 0XDB00 );
                    lcddev.id = LCD_RD_DATA();
                    lcddev.id <<= 8;
                    LCD_WR_REG ( 0XDC00 );
                    lcddev.id |= LCD_RD_DATA();

                    if ( lcddev.id == 0x8000 ) {
                        lcddev.id = 0x5510;
                    }
                }
            }
        }
    }

    printf ( " LCD ID:%x\r\n", lcddev.id );

    if ( lcddev.id == 0X9341 ) {
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
    } else if ( lcddev.id == 0x6804 ) {
        LCD_WR_REG ( 0X11 );
        delay_ms ( 20 );
        LCD_WR_REG ( 0XD0 );
        LCD_WR_DATA ( 0X07 );
        LCD_WR_DATA ( 0X42 );
        LCD_WR_DATA ( 0X1D );
        LCD_WR_REG ( 0XD1 );
        LCD_WR_DATA ( 0X00 );
        LCD_WR_DATA ( 0X1a );
        LCD_WR_DATA ( 0X09 );
        LCD_WR_REG ( 0XD2 );
        LCD_WR_DATA ( 0X01 );
        LCD_WR_DATA ( 0X22 );
        LCD_WR_REG ( 0XC0 );
        LCD_WR_DATA ( 0X10 );
        LCD_WR_DATA ( 0X3B );
        LCD_WR_DATA ( 0X00 );
        LCD_WR_DATA ( 0X02 );
        LCD_WR_DATA ( 0X11 );
        LCD_WR_REG ( 0XC5 );
        LCD_WR_DATA ( 0X03 );
        LCD_WR_REG ( 0XC8 );
        LCD_WR_DATA ( 0X00 );
        LCD_WR_DATA ( 0X25 );
        LCD_WR_DATA ( 0X21 );
        LCD_WR_DATA ( 0X05 );
        LCD_WR_DATA ( 0X00 );
        LCD_WR_DATA ( 0X0a );
        LCD_WR_DATA ( 0X65 );
        LCD_WR_DATA ( 0X25 );
        LCD_WR_DATA ( 0X77 );
        LCD_WR_DATA ( 0X50 );
        LCD_WR_DATA ( 0X0f );
        LCD_WR_DATA ( 0X00 );
        LCD_WR_REG ( 0XF8 );
        LCD_WR_DATA ( 0X01 );
        LCD_WR_REG ( 0XFE );
        LCD_WR_DATA ( 0X00 );
        LCD_WR_DATA ( 0X02 );
        LCD_WR_REG ( 0X20 );
        LCD_WR_REG ( 0X36 );
        LCD_WR_DATA ( 0X08 );
        LCD_WR_REG ( 0X3A );
        LCD_WR_DATA ( 0X55 );
        LCD_WR_REG ( 0X2B );
        LCD_WR_DATA ( 0X00 );
        LCD_WR_DATA ( 0X00 );
        LCD_WR_DATA ( 0X01 );
        LCD_WR_DATA ( 0X3F );
        LCD_WR_REG ( 0X2A );
        LCD_WR_DATA ( 0X00 );
        LCD_WR_DATA ( 0X00 );
        LCD_WR_DATA ( 0X01 );
        LCD_WR_DATA ( 0XDF );
        delay_ms ( 120 );
        LCD_WR_REG ( 0X29 );
    } else if ( lcddev.id == 0x5310 ) {
        LCD_WR_REG ( 0xED );
        LCD_WR_DATA ( 0x01 );
        LCD_WR_DATA ( 0xFE );
        LCD_WR_REG ( 0xEE );
        LCD_WR_DATA ( 0xDE );
        LCD_WR_DATA ( 0x21 );
        LCD_WR_REG ( 0xF1 );
        LCD_WR_DATA ( 0x01 );
        LCD_WR_REG ( 0xDF );
        LCD_WR_DATA ( 0x10 );
        LCD_WR_REG ( 0xC4 );
        LCD_WR_DATA ( 0x8F );
        LCD_WR_REG ( 0xC6 );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0xE2 );
        LCD_WR_DATA ( 0xE2 );
        LCD_WR_DATA ( 0xE2 );
        LCD_WR_REG ( 0xBF );
        LCD_WR_DATA ( 0xAA );
        LCD_WR_REG ( 0xB0 );
        LCD_WR_DATA ( 0x0D );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0x0D );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0x11 );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0x19 );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0x21 );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0x2D );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0x3D );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0x5D );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0x5D );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_REG ( 0xB1 );
        LCD_WR_DATA ( 0x80 );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0x8B );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0x96 );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_REG ( 0xB2 );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0x02 );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0x03 );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_REG ( 0xB3 );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_REG ( 0xB4 );
        LCD_WR_DATA ( 0x8B );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0x96 );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0xA1 );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_REG ( 0xB5 );
        LCD_WR_DATA ( 0x02 );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0x03 );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0x04 );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_REG ( 0xB6 );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_REG ( 0xB7 );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0x3F );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0x5E );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0x64 );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0x8C );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0xAC );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0xDC );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0x70 );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0x90 );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0xEB );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0xDC );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_REG ( 0xB8 );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_REG ( 0xBA );
        LCD_WR_DATA ( 0x24 );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_REG ( 0xC1 );
        LCD_WR_DATA ( 0x20 );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0x54 );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0xFF );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_REG ( 0xC2 );
        LCD_WR_DATA ( 0x0A );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0x04 );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_REG ( 0xC3 );
        LCD_WR_DATA ( 0x3C );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0x3A );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0x39 );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0x37 );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0x3C );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0x36 );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0x32 );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0x2F );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0x2C );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0x29 );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0x26 );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0x24 );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0x24 );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0x23 );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0x3C );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0x36 );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0x32 );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0x2F );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0x2C );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0x29 );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0x26 );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0x24 );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0x24 );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0x23 );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_REG ( 0xC4 );
        LCD_WR_DATA ( 0x62 );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0x05 );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0x84 );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0xF0 );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0x18 );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0xA4 );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0x18 );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0x50 );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0x0C );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0x17 );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0x95 );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0xF3 );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0xE6 );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_REG ( 0xC5 );
        LCD_WR_DATA ( 0x32 );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0x44 );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0x65 );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0x76 );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0x88 );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_REG ( 0xC6 );
        LCD_WR_DATA ( 0x20 );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0x17 );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0x01 );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_REG ( 0xC7 );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_REG ( 0xC8 );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_REG ( 0xC9 );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_REG ( 0xE0 );
        LCD_WR_DATA ( 0x16 );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0x1C );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0x21 );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0x36 );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0x46 );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0x52 );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0x64 );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0x7A );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0x8B );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0x99 );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0xA8 );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0xB9 );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0xC4 );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0xCA );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0xD2 );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0xD9 );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0xE0 );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0xF3 );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_REG ( 0xE1 );
        LCD_WR_DATA ( 0x16 );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0x1C );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0x22 );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0x36 );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0x45 );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0x52 );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0x64 );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0x7A );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0x8B );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0x99 );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0xA8 );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0xB9 );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0xC4 );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0xCA );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0xD2 );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0xD8 );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0xE0 );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0xF3 );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_REG ( 0xE2 );
        LCD_WR_DATA ( 0x05 );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0x0B );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0x1B );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0x34 );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0x44 );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0x4F );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0x61 );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0x79 );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0x88 );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0x97 );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0xA6 );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0xB7 );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0xC2 );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0xC7 );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0xD1 );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0xD6 );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0xDD );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0xF3 );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_REG ( 0xE3 );
        LCD_WR_DATA ( 0x05 );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0xA );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0x1C );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0x33 );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0x44 );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0x50 );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0x62 );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0x78 );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0x88 );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0x97 );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0xA6 );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0xB7 );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0xC2 );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0xC7 );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0xD1 );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0xD5 );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0xDD );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0xF3 );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_REG ( 0xE4 );
        LCD_WR_DATA ( 0x01 );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0x01 );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0x02 );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0x2A );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0x3C );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0x4B );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0x5D );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0x74 );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0x84 );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0x93 );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0xA2 );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0xB3 );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0xBE );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0xC4 );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0xCD );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0xD3 );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0xDD );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0xF3 );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_REG ( 0xE5 );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0x02 );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0x29 );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0x3C );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0x4B );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0x5D );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0x74 );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0x84 );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0x93 );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0xA2 );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0xB3 );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0xBE );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0xC4 );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0xCD );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0xD3 );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0xDC );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0xF3 );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_REG ( 0xE6 );
        LCD_WR_DATA ( 0x11 );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0x34 );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0x56 );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0x76 );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0x77 );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0x66 );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0x88 );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0x99 );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0xBB );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0x99 );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0x66 );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0x55 );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0x55 );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0x45 );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0x43 );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0x44 );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_REG ( 0xE7 );
        LCD_WR_DATA ( 0x32 );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0x55 );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0x76 );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0x66 );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0x67 );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0x67 );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0x87 );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0x99 );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0xBB );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0x99 );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0x77 );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0x44 );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0x56 );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0x23 );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0x33 );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0x45 );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_REG ( 0xE8 );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0x99 );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0x87 );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0x88 );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0x77 );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0x66 );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0x88 );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0xAA );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0xBB );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0x99 );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0x66 );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0x55 );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0x55 );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0x44 );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0x44 );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0x55 );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_REG ( 0xE9 );
        LCD_WR_DATA ( 0xAA );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_REG ( 0x00 );
        LCD_WR_DATA ( 0xAA );
        LCD_WR_REG ( 0xCF );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_REG ( 0xF0 );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0x50 );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_REG ( 0xF3 );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_REG ( 0xF9 );
        LCD_WR_DATA ( 0x06 );
        LCD_WR_DATA ( 0x10 );
        LCD_WR_DATA ( 0x29 );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_REG ( 0x3A );
        LCD_WR_DATA ( 0x55 );
        LCD_WR_REG ( 0x11 );
        delay_ms ( 100 );
        LCD_WR_REG ( 0x29 );
        LCD_WR_REG ( 0x35 );
        LCD_WR_DATA ( 0x00 );
        LCD_WR_REG ( 0x51 );
        LCD_WR_DATA ( 0xFF );
        LCD_WR_REG ( 0x53 );
        LCD_WR_DATA ( 0x2C );
        LCD_WR_REG ( 0x55 );
        LCD_WR_DATA ( 0x82 );
        LCD_WR_REG ( 0x2c );
    } else if ( lcddev.id == 0x5510 ) {
        LCD_WriteReg ( 0xF000, 0x55 );
        LCD_WriteReg ( 0xF001, 0xAA );
        LCD_WriteReg ( 0xF002, 0x52 );
        LCD_WriteReg ( 0xF003, 0x08 );
        LCD_WriteReg ( 0xF004, 0x01 );
        LCD_WriteReg ( 0xB000, 0x0D );
        LCD_WriteReg ( 0xB001, 0x0D );
        LCD_WriteReg ( 0xB002, 0x0D );
        LCD_WriteReg ( 0xB600, 0x34 );
        LCD_WriteReg ( 0xB601, 0x34 );
        LCD_WriteReg ( 0xB602, 0x34 );
        LCD_WriteReg ( 0xB100, 0x0D );
        LCD_WriteReg ( 0xB101, 0x0D );
        LCD_WriteReg ( 0xB102, 0x0D );
        LCD_WriteReg ( 0xB700, 0x34 );
        LCD_WriteReg ( 0xB701, 0x34 );
        LCD_WriteReg ( 0xB702, 0x34 );
        LCD_WriteReg ( 0xB200, 0x00 );
        LCD_WriteReg ( 0xB201, 0x00 );
        LCD_WriteReg ( 0xB202, 0x00 );
        LCD_WriteReg ( 0xB800, 0x24 );
        LCD_WriteReg ( 0xB801, 0x24 );
        LCD_WriteReg ( 0xB802, 0x24 );
        LCD_WriteReg ( 0xBF00, 0x01 );
        LCD_WriteReg ( 0xB300, 0x0F );
        LCD_WriteReg ( 0xB301, 0x0F );
        LCD_WriteReg ( 0xB302, 0x0F );
        LCD_WriteReg ( 0xB900, 0x34 );
        LCD_WriteReg ( 0xB901, 0x34 );
        LCD_WriteReg ( 0xB902, 0x34 );
        LCD_WriteReg ( 0xB500, 0x08 );
        LCD_WriteReg ( 0xB501, 0x08 );
        LCD_WriteReg ( 0xB502, 0x08 );
        LCD_WriteReg ( 0xC200, 0x03 );
        LCD_WriteReg ( 0xBA00, 0x24 );
        LCD_WriteReg ( 0xBA01, 0x24 );
        LCD_WriteReg ( 0xBA02, 0x24 );
        LCD_WriteReg ( 0xBC00, 0x00 );
        LCD_WriteReg ( 0xBC01, 0x78 );
        LCD_WriteReg ( 0xBC02, 0x00 );
        LCD_WriteReg ( 0xBD00, 0x00 );
        LCD_WriteReg ( 0xBD01, 0x78 );
        LCD_WriteReg ( 0xBD02, 0x00 );
        LCD_WriteReg ( 0xBE00, 0x00 );
        LCD_WriteReg ( 0xBE01, 0x64 );
        LCD_WriteReg ( 0xD100, 0x00 );
        LCD_WriteReg ( 0xD101, 0x33 );
        LCD_WriteReg ( 0xD102, 0x00 );
        LCD_WriteReg ( 0xD103, 0x34 );
        LCD_WriteReg ( 0xD104, 0x00 );
        LCD_WriteReg ( 0xD105, 0x3A );
        LCD_WriteReg ( 0xD106, 0x00 );
        LCD_WriteReg ( 0xD107, 0x4A );
        LCD_WriteReg ( 0xD108, 0x00 );
        LCD_WriteReg ( 0xD109, 0x5C );
        LCD_WriteReg ( 0xD10A, 0x00 );
        LCD_WriteReg ( 0xD10B, 0x81 );
        LCD_WriteReg ( 0xD10C, 0x00 );
        LCD_WriteReg ( 0xD10D, 0xA6 );
        LCD_WriteReg ( 0xD10E, 0x00 );
        LCD_WriteReg ( 0xD10F, 0xE5 );
        LCD_WriteReg ( 0xD110, 0x01 );
        LCD_WriteReg ( 0xD111, 0x13 );
        LCD_WriteReg ( 0xD112, 0x01 );
        LCD_WriteReg ( 0xD113, 0x54 );
        LCD_WriteReg ( 0xD114, 0x01 );
        LCD_WriteReg ( 0xD115, 0x82 );
        LCD_WriteReg ( 0xD116, 0x01 );
        LCD_WriteReg ( 0xD117, 0xCA );
        LCD_WriteReg ( 0xD118, 0x02 );
        LCD_WriteReg ( 0xD119, 0x00 );
        LCD_WriteReg ( 0xD11A, 0x02 );
        LCD_WriteReg ( 0xD11B, 0x01 );
        LCD_WriteReg ( 0xD11C, 0x02 );
        LCD_WriteReg ( 0xD11D, 0x34 );
        LCD_WriteReg ( 0xD11E, 0x02 );
        LCD_WriteReg ( 0xD11F, 0x67 );
        LCD_WriteReg ( 0xD120, 0x02 );
        LCD_WriteReg ( 0xD121, 0x84 );
        LCD_WriteReg ( 0xD122, 0x02 );
        LCD_WriteReg ( 0xD123, 0xA4 );
        LCD_WriteReg ( 0xD124, 0x02 );
        LCD_WriteReg ( 0xD125, 0xB7 );
        LCD_WriteReg ( 0xD126, 0x02 );
        LCD_WriteReg ( 0xD127, 0xCF );
        LCD_WriteReg ( 0xD128, 0x02 );
        LCD_WriteReg ( 0xD129, 0xDE );
        LCD_WriteReg ( 0xD12A, 0x02 );
        LCD_WriteReg ( 0xD12B, 0xF2 );
        LCD_WriteReg ( 0xD12C, 0x02 );
        LCD_WriteReg ( 0xD12D, 0xFE );
        LCD_WriteReg ( 0xD12E, 0x03 );
        LCD_WriteReg ( 0xD12F, 0x10 );
        LCD_WriteReg ( 0xD130, 0x03 );
        LCD_WriteReg ( 0xD131, 0x33 );
        LCD_WriteReg ( 0xD132, 0x03 );
        LCD_WriteReg ( 0xD133, 0x6D );
        LCD_WriteReg ( 0xD200, 0x00 );
        LCD_WriteReg ( 0xD201, 0x33 );
        LCD_WriteReg ( 0xD202, 0x00 );
        LCD_WriteReg ( 0xD203, 0x34 );
        LCD_WriteReg ( 0xD204, 0x00 );
        LCD_WriteReg ( 0xD205, 0x3A );
        LCD_WriteReg ( 0xD206, 0x00 );
        LCD_WriteReg ( 0xD207, 0x4A );
        LCD_WriteReg ( 0xD208, 0x00 );
        LCD_WriteReg ( 0xD209, 0x5C );
        LCD_WriteReg ( 0xD20A, 0x00 );
        LCD_WriteReg ( 0xD20B, 0x81 );
        LCD_WriteReg ( 0xD20C, 0x00 );
        LCD_WriteReg ( 0xD20D, 0xA6 );
        LCD_WriteReg ( 0xD20E, 0x00 );
        LCD_WriteReg ( 0xD20F, 0xE5 );
        LCD_WriteReg ( 0xD210, 0x01 );
        LCD_WriteReg ( 0xD211, 0x13 );
        LCD_WriteReg ( 0xD212, 0x01 );
        LCD_WriteReg ( 0xD213, 0x54 );
        LCD_WriteReg ( 0xD214, 0x01 );
        LCD_WriteReg ( 0xD215, 0x82 );
        LCD_WriteReg ( 0xD216, 0x01 );
        LCD_WriteReg ( 0xD217, 0xCA );
        LCD_WriteReg ( 0xD218, 0x02 );
        LCD_WriteReg ( 0xD219, 0x00 );
        LCD_WriteReg ( 0xD21A, 0x02 );
        LCD_WriteReg ( 0xD21B, 0x01 );
        LCD_WriteReg ( 0xD21C, 0x02 );
        LCD_WriteReg ( 0xD21D, 0x34 );
        LCD_WriteReg ( 0xD21E, 0x02 );
        LCD_WriteReg ( 0xD21F, 0x67 );
        LCD_WriteReg ( 0xD220, 0x02 );
        LCD_WriteReg ( 0xD221, 0x84 );
        LCD_WriteReg ( 0xD222, 0x02 );
        LCD_WriteReg ( 0xD223, 0xA4 );
        LCD_WriteReg ( 0xD224, 0x02 );
        LCD_WriteReg ( 0xD225, 0xB7 );
        LCD_WriteReg ( 0xD226, 0x02 );
        LCD_WriteReg ( 0xD227, 0xCF );
        LCD_WriteReg ( 0xD228, 0x02 );
        LCD_WriteReg ( 0xD229, 0xDE );
        LCD_WriteReg ( 0xD22A, 0x02 );
        LCD_WriteReg ( 0xD22B, 0xF2 );
        LCD_WriteReg ( 0xD22C, 0x02 );
        LCD_WriteReg ( 0xD22D, 0xFE );
        LCD_WriteReg ( 0xD22E, 0x03 );
        LCD_WriteReg ( 0xD22F, 0x10 );
        LCD_WriteReg ( 0xD230, 0x03 );
        LCD_WriteReg ( 0xD231, 0x33 );
        LCD_WriteReg ( 0xD232, 0x03 );
        LCD_WriteReg ( 0xD233, 0x6D );
        LCD_WriteReg ( 0xD300, 0x00 );
        LCD_WriteReg ( 0xD301, 0x33 );
        LCD_WriteReg ( 0xD302, 0x00 );
        LCD_WriteReg ( 0xD303, 0x34 );
        LCD_WriteReg ( 0xD304, 0x00 );
        LCD_WriteReg ( 0xD305, 0x3A );
        LCD_WriteReg ( 0xD306, 0x00 );
        LCD_WriteReg ( 0xD307, 0x4A );
        LCD_WriteReg ( 0xD308, 0x00 );
        LCD_WriteReg ( 0xD309, 0x5C );
        LCD_WriteReg ( 0xD30A, 0x00 );
        LCD_WriteReg ( 0xD30B, 0x81 );
        LCD_WriteReg ( 0xD30C, 0x00 );
        LCD_WriteReg ( 0xD30D, 0xA6 );
        LCD_WriteReg ( 0xD30E, 0x00 );
        LCD_WriteReg ( 0xD30F, 0xE5 );
        LCD_WriteReg ( 0xD310, 0x01 );
        LCD_WriteReg ( 0xD311, 0x13 );
        LCD_WriteReg ( 0xD312, 0x01 );
        LCD_WriteReg ( 0xD313, 0x54 );
        LCD_WriteReg ( 0xD314, 0x01 );
        LCD_WriteReg ( 0xD315, 0x82 );
        LCD_WriteReg ( 0xD316, 0x01 );
        LCD_WriteReg ( 0xD317, 0xCA );
        LCD_WriteReg ( 0xD318, 0x02 );
        LCD_WriteReg ( 0xD319, 0x00 );
        LCD_WriteReg ( 0xD31A, 0x02 );
        LCD_WriteReg ( 0xD31B, 0x01 );
        LCD_WriteReg ( 0xD31C, 0x02 );
        LCD_WriteReg ( 0xD31D, 0x34 );
        LCD_WriteReg ( 0xD31E, 0x02 );
        LCD_WriteReg ( 0xD31F, 0x67 );
        LCD_WriteReg ( 0xD320, 0x02 );
        LCD_WriteReg ( 0xD321, 0x84 );
        LCD_WriteReg ( 0xD322, 0x02 );
        LCD_WriteReg ( 0xD323, 0xA4 );
        LCD_WriteReg ( 0xD324, 0x02 );
        LCD_WriteReg ( 0xD325, 0xB7 );
        LCD_WriteReg ( 0xD326, 0x02 );
        LCD_WriteReg ( 0xD327, 0xCF );
        LCD_WriteReg ( 0xD328, 0x02 );
        LCD_WriteReg ( 0xD329, 0xDE );
        LCD_WriteReg ( 0xD32A, 0x02 );
        LCD_WriteReg ( 0xD32B, 0xF2 );
        LCD_WriteReg ( 0xD32C, 0x02 );
        LCD_WriteReg ( 0xD32D, 0xFE );
        LCD_WriteReg ( 0xD32E, 0x03 );
        LCD_WriteReg ( 0xD32F, 0x10 );
        LCD_WriteReg ( 0xD330, 0x03 );
        LCD_WriteReg ( 0xD331, 0x33 );
        LCD_WriteReg ( 0xD332, 0x03 );
        LCD_WriteReg ( 0xD333, 0x6D );
        LCD_WriteReg ( 0xD400, 0x00 );
        LCD_WriteReg ( 0xD401, 0x33 );
        LCD_WriteReg ( 0xD402, 0x00 );
        LCD_WriteReg ( 0xD403, 0x34 );
        LCD_WriteReg ( 0xD404, 0x00 );
        LCD_WriteReg ( 0xD405, 0x3A );
        LCD_WriteReg ( 0xD406, 0x00 );
        LCD_WriteReg ( 0xD407, 0x4A );
        LCD_WriteReg ( 0xD408, 0x00 );
        LCD_WriteReg ( 0xD409, 0x5C );
        LCD_WriteReg ( 0xD40A, 0x00 );
        LCD_WriteReg ( 0xD40B, 0x81 );
        LCD_WriteReg ( 0xD40C, 0x00 );
        LCD_WriteReg ( 0xD40D, 0xA6 );
        LCD_WriteReg ( 0xD40E, 0x00 );
        LCD_WriteReg ( 0xD40F, 0xE5 );
        LCD_WriteReg ( 0xD410, 0x01 );
        LCD_WriteReg ( 0xD411, 0x13 );
        LCD_WriteReg ( 0xD412, 0x01 );
        LCD_WriteReg ( 0xD413, 0x54 );
        LCD_WriteReg ( 0xD414, 0x01 );
        LCD_WriteReg ( 0xD415, 0x82 );
        LCD_WriteReg ( 0xD416, 0x01 );
        LCD_WriteReg ( 0xD417, 0xCA );
        LCD_WriteReg ( 0xD418, 0x02 );
        LCD_WriteReg ( 0xD419, 0x00 );
        LCD_WriteReg ( 0xD41A, 0x02 );
        LCD_WriteReg ( 0xD41B, 0x01 );
        LCD_WriteReg ( 0xD41C, 0x02 );
        LCD_WriteReg ( 0xD41D, 0x34 );
        LCD_WriteReg ( 0xD41E, 0x02 );
        LCD_WriteReg ( 0xD41F, 0x67 );
        LCD_WriteReg ( 0xD420, 0x02 );
        LCD_WriteReg ( 0xD421, 0x84 );
        LCD_WriteReg ( 0xD422, 0x02 );
        LCD_WriteReg ( 0xD423, 0xA4 );
        LCD_WriteReg ( 0xD424, 0x02 );
        LCD_WriteReg ( 0xD425, 0xB7 );
        LCD_WriteReg ( 0xD426, 0x02 );
        LCD_WriteReg ( 0xD427, 0xCF );
        LCD_WriteReg ( 0xD428, 0x02 );
        LCD_WriteReg ( 0xD429, 0xDE );
        LCD_WriteReg ( 0xD42A, 0x02 );
        LCD_WriteReg ( 0xD42B, 0xF2 );
        LCD_WriteReg ( 0xD42C, 0x02 );
        LCD_WriteReg ( 0xD42D, 0xFE );
        LCD_WriteReg ( 0xD42E, 0x03 );
        LCD_WriteReg ( 0xD42F, 0x10 );
        LCD_WriteReg ( 0xD430, 0x03 );
        LCD_WriteReg ( 0xD431, 0x33 );
        LCD_WriteReg ( 0xD432, 0x03 );
        LCD_WriteReg ( 0xD433, 0x6D );
        LCD_WriteReg ( 0xD500, 0x00 );
        LCD_WriteReg ( 0xD501, 0x33 );
        LCD_WriteReg ( 0xD502, 0x00 );
        LCD_WriteReg ( 0xD503, 0x34 );
        LCD_WriteReg ( 0xD504, 0x00 );
        LCD_WriteReg ( 0xD505, 0x3A );
        LCD_WriteReg ( 0xD506, 0x00 );
        LCD_WriteReg ( 0xD507, 0x4A );
        LCD_WriteReg ( 0xD508, 0x00 );
        LCD_WriteReg ( 0xD509, 0x5C );
        LCD_WriteReg ( 0xD50A, 0x00 );
        LCD_WriteReg ( 0xD50B, 0x81 );
        LCD_WriteReg ( 0xD50C, 0x00 );
        LCD_WriteReg ( 0xD50D, 0xA6 );
        LCD_WriteReg ( 0xD50E, 0x00 );
        LCD_WriteReg ( 0xD50F, 0xE5 );
        LCD_WriteReg ( 0xD510, 0x01 );
        LCD_WriteReg ( 0xD511, 0x13 );
        LCD_WriteReg ( 0xD512, 0x01 );
        LCD_WriteReg ( 0xD513, 0x54 );
        LCD_WriteReg ( 0xD514, 0x01 );
        LCD_WriteReg ( 0xD515, 0x82 );
        LCD_WriteReg ( 0xD516, 0x01 );
        LCD_WriteReg ( 0xD517, 0xCA );
        LCD_WriteReg ( 0xD518, 0x02 );
        LCD_WriteReg ( 0xD519, 0x00 );
        LCD_WriteReg ( 0xD51A, 0x02 );
        LCD_WriteReg ( 0xD51B, 0x01 );
        LCD_WriteReg ( 0xD51C, 0x02 );
        LCD_WriteReg ( 0xD51D, 0x34 );
        LCD_WriteReg ( 0xD51E, 0x02 );
        LCD_WriteReg ( 0xD51F, 0x67 );
        LCD_WriteReg ( 0xD520, 0x02 );
        LCD_WriteReg ( 0xD521, 0x84 );
        LCD_WriteReg ( 0xD522, 0x02 );
        LCD_WriteReg ( 0xD523, 0xA4 );
        LCD_WriteReg ( 0xD524, 0x02 );
        LCD_WriteReg ( 0xD525, 0xB7 );
        LCD_WriteReg ( 0xD526, 0x02 );
        LCD_WriteReg ( 0xD527, 0xCF );
        LCD_WriteReg ( 0xD528, 0x02 );
        LCD_WriteReg ( 0xD529, 0xDE );
        LCD_WriteReg ( 0xD52A, 0x02 );
        LCD_WriteReg ( 0xD52B, 0xF2 );
        LCD_WriteReg ( 0xD52C, 0x02 );
        LCD_WriteReg ( 0xD52D, 0xFE );
        LCD_WriteReg ( 0xD52E, 0x03 );
        LCD_WriteReg ( 0xD52F, 0x10 );
        LCD_WriteReg ( 0xD530, 0x03 );
        LCD_WriteReg ( 0xD531, 0x33 );
        LCD_WriteReg ( 0xD532, 0x03 );
        LCD_WriteReg ( 0xD533, 0x6D );
        LCD_WriteReg ( 0xD600, 0x00 );
        LCD_WriteReg ( 0xD601, 0x33 );
        LCD_WriteReg ( 0xD602, 0x00 );
        LCD_WriteReg ( 0xD603, 0x34 );
        LCD_WriteReg ( 0xD604, 0x00 );
        LCD_WriteReg ( 0xD605, 0x3A );
        LCD_WriteReg ( 0xD606, 0x00 );
        LCD_WriteReg ( 0xD607, 0x4A );
        LCD_WriteReg ( 0xD608, 0x00 );
        LCD_WriteReg ( 0xD609, 0x5C );
        LCD_WriteReg ( 0xD60A, 0x00 );
        LCD_WriteReg ( 0xD60B, 0x81 );
        LCD_WriteReg ( 0xD60C, 0x00 );
        LCD_WriteReg ( 0xD60D, 0xA6 );
        LCD_WriteReg ( 0xD60E, 0x00 );
        LCD_WriteReg ( 0xD60F, 0xE5 );
        LCD_WriteReg ( 0xD610, 0x01 );
        LCD_WriteReg ( 0xD611, 0x13 );
        LCD_WriteReg ( 0xD612, 0x01 );
        LCD_WriteReg ( 0xD613, 0x54 );
        LCD_WriteReg ( 0xD614, 0x01 );
        LCD_WriteReg ( 0xD615, 0x82 );
        LCD_WriteReg ( 0xD616, 0x01 );
        LCD_WriteReg ( 0xD617, 0xCA );
        LCD_WriteReg ( 0xD618, 0x02 );
        LCD_WriteReg ( 0xD619, 0x00 );
        LCD_WriteReg ( 0xD61A, 0x02 );
        LCD_WriteReg ( 0xD61B, 0x01 );
        LCD_WriteReg ( 0xD61C, 0x02 );
        LCD_WriteReg ( 0xD61D, 0x34 );
        LCD_WriteReg ( 0xD61E, 0x02 );
        LCD_WriteReg ( 0xD61F, 0x67 );
        LCD_WriteReg ( 0xD620, 0x02 );
        LCD_WriteReg ( 0xD621, 0x84 );
        LCD_WriteReg ( 0xD622, 0x02 );
        LCD_WriteReg ( 0xD623, 0xA4 );
        LCD_WriteReg ( 0xD624, 0x02 );
        LCD_WriteReg ( 0xD625, 0xB7 );
        LCD_WriteReg ( 0xD626, 0x02 );
        LCD_WriteReg ( 0xD627, 0xCF );
        LCD_WriteReg ( 0xD628, 0x02 );
        LCD_WriteReg ( 0xD629, 0xDE );
        LCD_WriteReg ( 0xD62A, 0x02 );
        LCD_WriteReg ( 0xD62B, 0xF2 );
        LCD_WriteReg ( 0xD62C, 0x02 );
        LCD_WriteReg ( 0xD62D, 0xFE );
        LCD_WriteReg ( 0xD62E, 0x03 );
        LCD_WriteReg ( 0xD62F, 0x10 );
        LCD_WriteReg ( 0xD630, 0x03 );
        LCD_WriteReg ( 0xD631, 0x33 );
        LCD_WriteReg ( 0xD632, 0x03 );
        LCD_WriteReg ( 0xD633, 0x6D );
        LCD_WriteReg ( 0xF000, 0x55 );
        LCD_WriteReg ( 0xF001, 0xAA );
        LCD_WriteReg ( 0xF002, 0x52 );
        LCD_WriteReg ( 0xF003, 0x08 );
        LCD_WriteReg ( 0xF004, 0x00 );
        LCD_WriteReg ( 0xB100, 0xCC );
        LCD_WriteReg ( 0xB101, 0x00 );
        LCD_WriteReg ( 0xB600, 0x05 );
        LCD_WriteReg ( 0xB700, 0x70 );
        LCD_WriteReg ( 0xB701, 0x70 );
        LCD_WriteReg ( 0xB800, 0x01 );
        LCD_WriteReg ( 0xB801, 0x03 );
        LCD_WriteReg ( 0xB802, 0x03 );
        LCD_WriteReg ( 0xB803, 0x03 );
        LCD_WriteReg ( 0xBC00, 0x02 );
        LCD_WriteReg ( 0xBC01, 0x00 );
        LCD_WriteReg ( 0xBC02, 0x00 );
        LCD_WriteReg ( 0xC900, 0xD0 );
        LCD_WriteReg ( 0xC901, 0x02 );
        LCD_WriteReg ( 0xC902, 0x50 );
        LCD_WriteReg ( 0xC903, 0x50 );
        LCD_WriteReg ( 0xC904, 0x50 );
        LCD_WriteReg ( 0x3500, 0x00 );
        LCD_WriteReg ( 0x3A00, 0x55 );
        LCD_WR_REG ( 0x1100 );
        delay_us ( 120 );
        LCD_WR_REG ( 0x2900 );
    } else if ( lcddev.id == 0x9325 ) {
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
    } else if ( lcddev.id == 0x9328 ) {
        LCD_WriteReg ( 0x00EC, 0x108F );
        LCD_WriteReg ( 0x00EF, 0x1234 );
        LCD_WriteReg ( 0x0001, 0x0100 );
        LCD_WriteReg ( 0x0002, 0x0700 );
        LCD_WriteReg ( 0x0003, ( 1 << 12 ) | ( 3 << 4 ) | ( 0 << 3 ) );
        LCD_WriteReg ( 0x0004, 0x0000 );
        LCD_WriteReg ( 0x0008, 0x0202 );
        LCD_WriteReg ( 0x0009, 0x0000 );
        LCD_WriteReg ( 0x000a, 0x0000 );
        LCD_WriteReg ( 0x000c, 0x0001 );
        LCD_WriteReg ( 0x000d, 0x0000 );
        LCD_WriteReg ( 0x000f, 0x0000 );
        LCD_WriteReg ( 0x0010, 0x0000 );
        LCD_WriteReg ( 0x0011, 0x0007 );
        LCD_WriteReg ( 0x0012, 0x0000 );
        LCD_WriteReg ( 0x0013, 0x0000 );
        LCD_WriteReg ( 0x0007, 0x0001 );
        delay_ms ( 50 );
        LCD_WriteReg ( 0x0010, 0x1490 );
        LCD_WriteReg ( 0x0011, 0x0227 );
        delay_ms ( 50 );
        LCD_WriteReg ( 0x0012, 0x008A );
        delay_ms ( 50 );
        LCD_WriteReg ( 0x0013, 0x1a00 );
        LCD_WriteReg ( 0x0029, 0x0006 );
        LCD_WriteReg ( 0x002b, 0x000d );
        delay_ms ( 50 );
        LCD_WriteReg ( 0x0020, 0x0000 );
        LCD_WriteReg ( 0x0021, 0x0000 );
        delay_ms ( 50 );
        LCD_WriteReg ( 0x0030, 0x0000 );
        LCD_WriteReg ( 0x0031, 0x0604 );
        LCD_WriteReg ( 0x0032, 0x0305 );
        LCD_WriteReg ( 0x0035, 0x0000 );
        LCD_WriteReg ( 0x0036, 0x0C09 );
        LCD_WriteReg ( 0x0037, 0x0204 );
        LCD_WriteReg ( 0x0038, 0x0301 );
        LCD_WriteReg ( 0x0039, 0x0707 );
        LCD_WriteReg ( 0x003c, 0x0000 );
        LCD_WriteReg ( 0x003d, 0x0a0a );
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
        LCD_WriteReg ( 0x0092, 0x0600 );
        LCD_WriteReg ( 0x0007, 0x0133 );
    } else if ( lcddev.id == 0x9320 ) {
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
    } else if ( lcddev.id == 0X9331 ) {
        LCD_WriteReg ( 0x00E7, 0x1014 );
        LCD_WriteReg ( 0x0001, 0x0100 );
        LCD_WriteReg ( 0x0002, 0x0200 );
        LCD_WriteReg ( 0x0003, ( 1 << 12 ) | ( 3 << 4 ) | ( 1 << 3 ) );
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
        delay_ms ( 200 );
        LCD_WriteReg ( 0x0010, 0x1690 );
        LCD_WriteReg ( 0x0011, 0x0227 );
        delay_ms ( 50 );
        LCD_WriteReg ( 0x0012, 0x000C );
        delay_ms ( 50 );
        LCD_WriteReg ( 0x0013, 0x0800 );
        LCD_WriteReg ( 0x0029, 0x0011 );
        LCD_WriteReg ( 0x002B, 0x000B );
        delay_ms ( 50 );
        LCD_WriteReg ( 0x0020, 0x0000 );
        LCD_WriteReg ( 0x0021, 0x013f );
        LCD_WriteReg ( 0x0030, 0x0000 );
        LCD_WriteReg ( 0x0031, 0x0106 );
        LCD_WriteReg ( 0x0032, 0x0000 );
        LCD_WriteReg ( 0x0035, 0x0204 );
        LCD_WriteReg ( 0x0036, 0x160A );
        LCD_WriteReg ( 0x0037, 0x0707 );
        LCD_WriteReg ( 0x0038, 0x0106 );
        LCD_WriteReg ( 0x0039, 0x0707 );
        LCD_WriteReg ( 0x003C, 0x0402 );
        LCD_WriteReg ( 0x003D, 0x0C0F );
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
        LCD_WriteReg ( 0x0090, 0x0010 );
        LCD_WriteReg ( 0x0092, 0x0600 );
        LCD_WriteReg ( 0x0007, 0x0133 );
    } else if ( lcddev.id == 0x5408 ) {
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
    } else if ( lcddev.id == 0x1505 ) {
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
        LCD_WriteReg ( 0x0003, 0x1038 );
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
    } else if ( lcddev.id == 0xB505 ) {
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
        LCD_WriteReg ( 0x0003, 0x1038 );
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
    } else if ( lcddev.id == 0xC505 ) {
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
    } else if ( lcddev.id == 0x8989 ) {
        LCD_WriteReg ( 0x0000, 0x0001 );
        LCD_WriteReg ( 0x0003, 0xA8A4 );
        LCD_WriteReg ( 0x000C, 0x0000 );
        LCD_WriteReg ( 0x000D, 0x080C );
        LCD_WriteReg ( 0x000E, 0x2B00 );
        LCD_WriteReg ( 0x001E, 0x00B0 );
        LCD_WriteReg ( 0x0001, 0x2B3F );
        LCD_WriteReg ( 0x0002, 0x0600 );
        LCD_WriteReg ( 0x0010, 0x0000 );
        LCD_WriteReg ( 0x0011, 0x6078 );
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
    } else if ( lcddev.id == 0x4531 ) {
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
        LCD_WriteReg ( 0X03, 0X1038 );
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
    } else if ( lcddev.id == 0x4535 ) {
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

    LCD_Display_Dir ( 1 );
    LCD_LED = 1;
    LCD_Clear ( WHITE );
}

void LCD_Clear ( u16 color ) {
    u32 index = 0;
    u32 totalpoint = lcddev.width;
    totalpoint *= lcddev.height;

    if ( ( lcddev.id == 0X6804 ) && ( lcddev.dir == 1 ) ) {
        lcddev.dir = 0;
        lcddev.setxcmd = 0X2A;
        lcddev.setycmd = 0X2B;
        LCD_SetCursor ( 0x00, 0x0000 );
        lcddev.dir = 1;
        lcddev.setxcmd = 0X2B;
        lcddev.setycmd = 0X2A;
    } else {
        LCD_SetCursor ( 0x00, 0x0000 );
    }

    LCD_WriteRAM_Prepare();

    for ( index = 0; index < totalpoint; index++ ) {
        LCD_WR_DATA ( color );
    }
}

void LCD_Fill ( u16 sx, u16 sy, u16 ex, u16 ey, u16 color ) {
    u16 i, j;
    u16 xlen = 0;
    u16 temp;

    if ( ( lcddev.id == 0X6804 ) && ( lcddev.dir == 1 ) ) {
        temp = sx;
        sx = sy;
        sy = lcddev.width - ex - 1;
        ex = ey;
        ey = lcddev.width - temp - 1;
        lcddev.dir = 0;
        lcddev.setxcmd = 0X2A;
        lcddev.setycmd = 0X2B;
        LCD_Fill ( sx, sy, ex, ey, color );
        lcddev.dir = 1;
        lcddev.setxcmd = 0X2B;
        lcddev.setycmd = 0X2A;
    } else {
        xlen = ex - sx + 1;

        for ( i = sy; i <= ey; i++ ) {
            LCD_SetCursor ( sx, i );
            LCD_WriteRAM_Prepare();

            for ( j = 0; j < xlen; j++ ) {
                LCD_WR_DATA ( color );
            }
        }
    }
}

void LCD_Color_Fill ( u16 sx, u16 sy, u16 ex, u16 ey, u16 *color ) {
    u16 height, width;
    u16 i, j;
    width = ex - sx + 1;
    height = ey - sy + 1;

    for ( i = 0; i < height; i++ ) {
        LCD_SetCursor ( sx, sy + i );
        LCD_WriteRAM_Prepare();

        for ( j = 0; j < width; j++ ) {
            LCD_WR_DATA ( color[i * height + j] );
        }
    }
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
        LCD_DrawPoint ( x0 + a, y0 - b );
        LCD_DrawPoint ( x0 + b, y0 - a );
        LCD_DrawPoint ( x0 + b, y0 + a );
        LCD_DrawPoint ( x0 + a, y0 + b );
        LCD_DrawPoint ( x0 - a, y0 + b );
        LCD_DrawPoint ( x0 - b, y0 + a );
        LCD_DrawPoint ( x0 - a, y0 - b );
        LCD_DrawPoint ( x0 - b, y0 - a );
        a++;

        if ( di < 0 ) {
            di += 4 * a + 6;
        } else {
            di += 10 + 4 * ( a - b );
            b--;
        }
    }
}

void LCD_ShowChar ( u16 x, u16 y, u8 num, u8 size, u8 mode ) {
    u8 temp, t1, t;
    u16 y0 = y;
    u16 colortemp = POINT_COLOR;
    num = num - ' ';

    if ( !mode ) {
        for ( t = 0; t < size; t++ ) {
            if ( size == 12 ) {
                temp = asc2_1206[num][t];
            } else {
                temp = asc2_1608[num][t];
            }

            for ( t1 = 0; t1 < 8; t1++ ) {
                if ( temp & 0x80 ) {
                    POINT_COLOR = colortemp;
                } else {
                    POINT_COLOR = BACK_COLOR;
                }

                LCD_DrawPoint ( x, y );
                temp <<= 1;
                y++;

                if ( x >= lcddev.width ) {
                    POINT_COLOR = colortemp;
                    return;
                }

                if ( ( y - y0 ) == size ) {
                    y = y0;
                    x++;

                    if ( x >= lcddev.width ) {
                        POINT_COLOR = colortemp;
                        return;
                    }

                    break;
                }
            }
        }
    } else {
        for ( t = 0; t < size; t++ ) {
            if ( size == 12 ) {
                temp = asc2_1206[num][t];
            } else {
                temp = asc2_1608[num][t];
            }

            for ( t1 = 0; t1 < 8; t1++ ) {
                if ( temp & 0x80 ) {
                    LCD_DrawPoint ( x, y );
                }

                temp <<= 1;
                y++;

                if ( x >= lcddev.height ) {
                    POINT_COLOR = colortemp;
                    return;
                }

                if ( ( y - y0 ) == size ) {
                    y = y0;
                    x++;

                    if ( x >= lcddev.width ) {
                        POINT_COLOR = colortemp;
                        return;
                    }

                    break;
                }
            }
        }
    }

    POINT_COLOR = colortemp;
}

u32 LCD_Pow ( u8 m, u8 n ) {
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
        temp = ( num / LCD_Pow ( 10, len - t - 1 ) ) % 10;

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

void LCD_ShowxNum ( u16 x, u16 y, u32 num, u8 len, u8 size, u8 mode ) {
    u8 t, temp;
    u8 enshow = 0;

    for ( t = 0; t < len; t++ ) {
        temp = ( num / LCD_Pow ( 10, len - t - 1 ) ) % 10;

        if ( enshow == 0 && t < ( len - 1 ) ) {
            if ( temp == 0 ) {
                if ( mode & 0X80 ) {
                    LCD_ShowChar ( x + ( size / 2 ) *t, y, '0', size, mode & 0X01 );
                } else {
                    LCD_ShowChar ( x + ( size / 2 ) *t, y, ' ', size, mode & 0X01 );
                }

                continue;
            } else {
                enshow = 1;
            }
        }

        LCD_ShowChar ( x + ( size / 2 ) *t, y, temp + '0', size, mode & 0X01 );
    }
}

void LCD_ShowString ( u16 x, u16 y, u16 width, u16 height, u8 size, u8 *p ) {
    u8 x0 = x;
    width += x;
    height += y;

    while ( ( *p <= '~' ) && ( *p >= ' ' ) ) {
        if ( x >= width ) {
            x = x0;
            y += size;
        }

        if ( y >= height ) {
            break;
        }

        LCD_ShowChar ( x, y, *p, size, 0 );
        x += size / 2;
        p++;
    }
}