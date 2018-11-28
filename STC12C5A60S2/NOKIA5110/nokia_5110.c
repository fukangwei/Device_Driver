#include "nokia_5110.h"
#include "english_6x8_pixel.h"
#include "write_chinese_string_pixel.h"

void delay_1us ( void ) {
}

void LCD_init ( void ) { /* 初始化LCD5110 */
    LCD_RST = 0; /* 产生一个让LCD复位的低电平脉冲 */
    delay_1us();
    LCD_RST = 1;
    /* LCD功能设置：芯片是活动的、水平寻址、使用扩展指令集 */
    LCD_write_byte ( 0x21, 0 );
    LCD_write_byte ( 0xc8, 0 ); /* 设置偏置电压，写VOP到寄存器 */
    LCD_write_byte ( 0x06, 0 ); /* 温度校正，采用VLCD温度系数2 */
    LCD_write_byte ( 0x13, 0 ); /* 设置偏置系统 */
    /* LCD功能设置：芯片是活动的、水平寻址、使用基本命令 */
    LCD_write_byte ( 0x20, 0 );
    LCD_clear();
    LCD_write_byte ( 0x0c, 0 ); /* 设置为普通模式 */
    LCD_CE = 0;
}

void LCD_clear ( void ) { /* LCD5110清屏函数 */
    unsigned int i;
    LCD_write_byte ( 0x0c, 0 ); /* 设置为普通模式 */
    LCD_write_byte ( 0x80, 0 ); /* 设置RAM的X地址 */
    LCD_write_byte ( 0x40, 0 ); /* 设置RAM的Y地址 */

    for ( i = 0; i <= 503; i++ ) {
        LCD_write_byte ( 0, 1 ); /* 写数据到显示RAM */
    }
}

void LCD_set_XY ( unsigned char X, unsigned char Y ) { /* 设置LCD坐标函数。参数X的范围是0至83，Y的范围是0至5 */
    LCD_write_byte ( 0x40 | Y, 0 ); /* 设置RAM的Y地址 */
    LCD_write_byte ( 0x80 | X, 0 ); /* 设置RAM的X地址 */
}

void LCD_write_char ( unsigned char c ) { /* 显示英文字符，参数c是显示的字符 */
    unsigned char line;
    c -= 32;

    for ( line = 0; line < 6; line++ ) {
        LCD_write_byte ( font6x8[c][line], 1 );
    }
}

/* 英文字符串显示函数。参数s是英文字符串指针；X、Y是显示字符串的位置，x为0至83，y为0至5 */
void LCD_write_english_string ( unsigned char X, unsigned char Y, char *s ) {
    LCD_set_XY ( X, Y );

    while ( *s ) {
        LCD_write_char ( *s );
        s++;
    }
}

/*----------------------------------------------------------------------------------------------------------------------------
 * 目的：在LCD上显示汉字
 * 参数：X、Y是显示汉字的起始X、Y坐标，ch_with是汉字点阵的宽度，num是显示汉字的个数，line是汉字点阵数组中的起始行数，row是汉字显示的行间距
 *---------------------------------------------------------------------------------------------------------------------------*/
void LCD_write_chinese_string ( unsigned char X, unsigned char Y, unsigned char ch_with, 
                                unsigned char num, unsigned char line, unsigned char row ) {
    unsigned char i, n;
    LCD_set_XY ( X, Y );

    for ( i = 0; i < num; ) {
        for ( n = 0; n < ch_with * 2; n++ ) { /* 写一个汉字 */
            if ( n == ch_with ) { /* 写汉字的下半部分 */
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

/* ---------------------------------------------------------------------------------------
 * 目的：位图绘制函数
 * 参数：X、Y是位图绘制的起始X、Y坐标；map是位图点阵数据；Pix_x是位图像素(长)，Pix_y是位图像素(宽)
 * ---------------------------------------------------------------------------------------*/
void LCD_draw_bmp_pixel ( unsigned char X, unsigned char Y, unsigned char *map, unsigned char Pix_x, unsigned char Pix_y ) {
    unsigned int i, n;
    unsigned char row;

    if ( Pix_y % 8 == 0 ) {
        row = Pix_y / 8; /* 计算位图所占行数(是指缓冲块的行数) */
    } else {
        row = Pix_y / 8 + 1;
    }

    for ( n = 0; n < row; n++ ) {
        LCD_set_XY ( X, Y );

        for ( i = 0; i < Pix_x; i++ ) {
            LCD_write_byte ( map[i + n * Pix_x], 1 );
        }

        Y++; /* 换行 */
    }
}

void LCD_write_byte ( unsigned char dat, unsigned char command ) { /* 写数据到LCD5110。参数data是写入的数据，command是写数据/命令选择 */
    unsigned char i;
    LCD_CE = 0; /* CE上的负边缘使能串行接口，并指示开始数据传输 */

    if ( command == 0 ) {
        LCD_DC = 0;
    } else {
        LCD_DC = 1;
    }

    for ( i = 0; i < 8; i++ ) {
        if ( dat & 0x80 ) { /* 首先传送的是字节的MSB(高位) */
            SDIN = 1;
        } else {
            SDIN = 0;
        }

        SCLK = 0; /* SDIN在SCLK的正边缘取样 */
        dat  = dat << 1;
        SCLK = 1;
    }

    LCD_CE = 1;
}