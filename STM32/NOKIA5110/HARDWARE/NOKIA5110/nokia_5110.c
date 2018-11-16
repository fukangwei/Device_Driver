#include "delay.h"
#include "nokia_5110.h"
#include "english_6x8_pixel.h"
#include "write_chinese_string_pixel.h"

static void LCD_write_byte ( unsigned char dat, unsigned char command ) {
    unsigned char i;
    LCD_CE = 0;

    if ( command == 0 ) {
        LCD_DC = 0;
    } else {
        LCD_DC = 1;
    }

    for ( i = 0; i < 8; i++ ) {
        if ( dat & 0x80 ) {
            SDIN = 1;
        } else {
            SDIN = 0;
        }

        SCLK = 0;
        dat  = dat << 1;
        SCLK = 1;
    }

    LCD_CE = 1;
}

static void LCD_set_XY ( unsigned char X, unsigned char Y ) {
    LCD_write_byte ( 0x40 | Y, 0 );
    LCD_write_byte ( 0x80 | X, 0 );
}

static void LCD_clear ( void ) {
    unsigned int i;
    LCD_write_byte ( 0x0c, 0 );
    LCD_write_byte ( 0x80, 0 );
    LCD_write_byte ( 0x40, 0 );

    for ( i = 0; i <= 503; i++ ) {
        LCD_write_byte ( 0, 1 );
    }
}

static void LCD_write_char ( unsigned char c ) {
    unsigned char line;
    c -= 32;

    for ( line = 0; line < 6; line++ ) {
        LCD_write_byte ( font6x8[c][line], 1 );
    }
}

void LCD_init ( void ) {
    GPIO_InitTypeDef  GPIO_InitStructure;
    RCC_APB2PeriphClockCmd ( RCC_APB2Periph_GPIOA, ENABLE );
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_4;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init ( GPIOA, &GPIO_InitStructure );
    LCD_RST = 0;
    delay_us ( 1 );
    LCD_RST = 1;
    LCD_write_byte ( 0x21, 0 );
    LCD_write_byte ( 0xc8, 0 );
    LCD_write_byte ( 0x06, 0 );
    LCD_write_byte ( 0x13, 0 );
    LCD_write_byte ( 0x20, 0 );
    LCD_clear();
    LCD_write_byte ( 0x0c, 0 );
    LCD_CE = 0;
}

void LCD_write_english_string ( unsigned char X, unsigned char Y, char *s ) {
    LCD_set_XY ( X, Y );

    while ( *s ) {
        LCD_write_char ( *s );
        s++;
    }
}

void LCD_write_chinese_string ( unsigned char X, unsigned char Y, unsigned char ch_with,
                                unsigned char num, unsigned char line, unsigned char row ) {
    unsigned char i, n;
    LCD_set_XY ( X, Y );

    for ( i = 0; i < num; ) {
        for ( n = 0; n < ch_with * 2; n++ ) {
            if ( n == ch_with ) {
                if ( i == 0 ) {
                    LCD_set_XY ( X, Y + 1 );
                } else {
                    LCD_set_XY ( ( X + ( ch_with + row ) *i ), Y + 1 );
                }
            }

            LCD_write_byte ( write_chinese[line + i][n], 1 );
        }

        i++;
        LCD_set_XY ( ( X + ( ch_with + row ) *i ), Y );
    }
}

void LCD_draw_bmp_pixel ( unsigned char X, unsigned char Y, unsigned char *map,
                          unsigned char Pix_x, unsigned char Pix_y ) {
    unsigned int i, n;
    unsigned char row;

    if ( Pix_y % 8 == 0 ) {
        row = Pix_y / 8;
    } else {
        row = Pix_y / 8 + 1;
    }

    for ( n = 0; n < row; n++ ) {
        LCD_set_XY ( X, Y );

        for ( i = 0; i < Pix_x; i++ ) {
            LCD_write_byte ( map[i + n * Pix_x], 1 );
        }

        Y++;
    }
}