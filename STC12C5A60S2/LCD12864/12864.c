#include "12864.h"

/*---------------------------------------------------------------------------
���ܣ���LCD12864д�������������
������ucData��Ҫд��Һ�������ݻ������������
      bComOrData������������ݵı�־λѡ������1����д��������ݣ�0����д���������
----------------------------------------------------------------------------*/
void LCD12864_WriteInfomation ( unsigned char ucData, bit bComOrData ) {
    LCD12864_CheckBusy(); /* æ��� */
    LCD12864_RW = 0; /* ����RW */
    LCD12864_RS = bComOrData; /* ���ݱ�־λ�ж�д�������������� */
    Delay ( 150 );
    LCDPORT = ucData; /* �������������ݶ˿� */
    LCD12864_EN = 1; /* ʹ���ź� */
    Delay ( 150 );
    LCD12864_EN = 0;
    Delay ( 150 );
}

void LCD12864_Init ( void ) { /* LCD12864Һ����ʼ�� */
    Delay ( 400 );
    LCD12864_PSB = 1; /* 8λ���ڹ���ģʽ */
    Delay ( 150 );
    LCD12864_WriteInfomation ( 0x30, 0 ); /* ����ָ� */
    Delay ( 150 );
    LCD12864_WriteInfomation ( 0x08, 0 ); /* ��ʾ���� */
    Delay ( 150 );
    LCD12864_WriteInfomation ( 0x10, 0 ); /* ������� */
    Delay ( 150 );
    LCD12864_WriteInfomation ( 0x0c, 0 ); /* �α����� */
    Delay ( 150 );
    LCD12864_WriteInfomation ( 0x01, 0 ); /* ���� */
    Delay ( 150 );
    LCD12864_WriteInfomation ( 0x06, 0 ); /* ������趨 */
    Delay ( 150 );
}

void LCD12864_CheckBusy ( void ) { /* LCD12864æ��� */
    unsigned char i = 250;
    LCD12864_RS = 0; /* ���� */
    LCD12864_RW = 1; /* ���� */
    LCD12864_EN = 1; /* ʹ�� */

    while ( ( i > 0 ) && ( P0 & 0x80 ) ) { /* ʹ�ñ���i����ʱ������Һ������ѭ����ͣ�� */
        i--;
    }

    LCD12864_EN = 0; /* �ͷ� */
}

/*--------------------------------------------------------------------------------
���ܣ���ʾһ�к���(8�����ֻ���16��Ӣ���ַ�)
������position��Ҫ��ʾ���е��׵�ַ����ѡֵ0x80��0x88��0x90��0x98������0x80��ʾҺ���ĵ�һ�У�
      0x88��ʾҺ���ĵ����У�0x90��ʾҺ���ĵڶ��У�0x98��ʾҺ���ĵ����С�
      p��Ҫ��ʾ�����ݵ��׵�ַ
---------------------------------------------------------------------------------*/
void LCD12864_DisplayOneLine ( unsigned char position, unsigned char *p ) {
    unsigned char i;
    LCD12864_WriteInfomation ( position, 0 ); /* д��Ҫ��ʾ���ֵ��е��׵�ַ */
    Delay ( 150 );

    for ( i = 0; i < 16; i++ ) { /* ����ִ��д����� */
        LCD12864_WriteInfomation ( *p, 1 );
        p++;
    }
}

void Delay ( unsigned int uiCount ) {
    while ( uiCount-- );
}