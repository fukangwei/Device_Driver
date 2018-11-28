#include "nokia_5110.h"
#include "english_6x8_pixel.h"
#include "write_chinese_string_pixel.h"

void delay_1us ( void ) {
}

void LCD_init ( void ) { /* ��ʼ��LCD5110 */
    LCD_RST = 0; /* ����һ����LCD��λ�ĵ͵�ƽ���� */
    delay_1us();
    LCD_RST = 1;
    /* LCD�������ã�оƬ�ǻ�ġ�ˮƽѰַ��ʹ����չָ� */
    LCD_write_byte ( 0x21, 0 );
    LCD_write_byte ( 0xc8, 0 ); /* ����ƫ�õ�ѹ��дVOP���Ĵ��� */
    LCD_write_byte ( 0x06, 0 ); /* �¶�У��������VLCD�¶�ϵ��2 */
    LCD_write_byte ( 0x13, 0 ); /* ����ƫ��ϵͳ */
    /* LCD�������ã�оƬ�ǻ�ġ�ˮƽѰַ��ʹ�û������� */
    LCD_write_byte ( 0x20, 0 );
    LCD_clear();
    LCD_write_byte ( 0x0c, 0 ); /* ����Ϊ��ͨģʽ */
    LCD_CE = 0;
}

void LCD_clear ( void ) { /* LCD5110�������� */
    unsigned int i;
    LCD_write_byte ( 0x0c, 0 ); /* ����Ϊ��ͨģʽ */
    LCD_write_byte ( 0x80, 0 ); /* ����RAM��X��ַ */
    LCD_write_byte ( 0x40, 0 ); /* ����RAM��Y��ַ */

    for ( i = 0; i <= 503; i++ ) {
        LCD_write_byte ( 0, 1 ); /* д���ݵ���ʾRAM */
    }
}

void LCD_set_XY ( unsigned char X, unsigned char Y ) { /* ����LCD���꺯��������X�ķ�Χ��0��83��Y�ķ�Χ��0��5 */
    LCD_write_byte ( 0x40 | Y, 0 ); /* ����RAM��Y��ַ */
    LCD_write_byte ( 0x80 | X, 0 ); /* ����RAM��X��ַ */
}

void LCD_write_char ( unsigned char c ) { /* ��ʾӢ���ַ�������c����ʾ���ַ� */
    unsigned char line;
    c -= 32;

    for ( line = 0; line < 6; line++ ) {
        LCD_write_byte ( font6x8[c][line], 1 );
    }
}

/* Ӣ���ַ�����ʾ����������s��Ӣ���ַ���ָ�룻X��Y����ʾ�ַ�����λ�ã�xΪ0��83��yΪ0��5 */
void LCD_write_english_string ( unsigned char X, unsigned char Y, char *s ) {
    LCD_set_XY ( X, Y );

    while ( *s ) {
        LCD_write_char ( *s );
        s++;
    }
}

/*----------------------------------------------------------------------------------------------------------------------------
 * Ŀ�ģ���LCD����ʾ����
 * ������X��Y����ʾ���ֵ���ʼX��Y���꣬ch_with�Ǻ��ֵ���Ŀ�ȣ�num����ʾ���ֵĸ�����line�Ǻ��ֵ��������е���ʼ������row�Ǻ�����ʾ���м��
 *---------------------------------------------------------------------------------------------------------------------------*/
void LCD_write_chinese_string ( unsigned char X, unsigned char Y, unsigned char ch_with, 
                                unsigned char num, unsigned char line, unsigned char row ) {
    unsigned char i, n;
    LCD_set_XY ( X, Y );

    for ( i = 0; i < num; ) {
        for ( n = 0; n < ch_with * 2; n++ ) { /* дһ������ */
            if ( n == ch_with ) { /* д���ֵ��°벿�� */
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
 * Ŀ�ģ�λͼ���ƺ���
 * ������X��Y��λͼ���Ƶ���ʼX��Y���ꣻmap��λͼ�������ݣ�Pix_x��λͼ����(��)��Pix_y��λͼ����(��)
 * ---------------------------------------------------------------------------------------*/
void LCD_draw_bmp_pixel ( unsigned char X, unsigned char Y, unsigned char *map, unsigned char Pix_x, unsigned char Pix_y ) {
    unsigned int i, n;
    unsigned char row;

    if ( Pix_y % 8 == 0 ) {
        row = Pix_y / 8; /* ����λͼ��ռ����(��ָ����������) */
    } else {
        row = Pix_y / 8 + 1;
    }

    for ( n = 0; n < row; n++ ) {
        LCD_set_XY ( X, Y );

        for ( i = 0; i < Pix_x; i++ ) {
            LCD_write_byte ( map[i + n * Pix_x], 1 );
        }

        Y++; /* ���� */
    }
}

void LCD_write_byte ( unsigned char dat, unsigned char command ) { /* д���ݵ�LCD5110������data��д������ݣ�command��д����/����ѡ�� */
    unsigned char i;
    LCD_CE = 0; /* CE�ϵĸ���Եʹ�ܴ��нӿڣ���ָʾ��ʼ���ݴ��� */

    if ( command == 0 ) {
        LCD_DC = 0;
    } else {
        LCD_DC = 1;
    }

    for ( i = 0; i < 8; i++ ) {
        if ( dat & 0x80 ) { /* ���ȴ��͵����ֽڵ�MSB(��λ) */
            SDIN = 1;
        } else {
            SDIN = 0;
        }

        SCLK = 0; /* SDIN��SCLK������Եȡ�� */
        dat  = dat << 1;
        SCLK = 1;
    }

    LCD_CE = 1;
}