#include "lcd1602.h"
#include "delay.h"

void GPIO_Configuration ( void ) {
    GPIO_InitTypeDef    GPIO_InitStructure;
    RCC_APB2PeriphClockCmd ( RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOC, ENABLE );
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3 |
                                  GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init ( GPIOA, &GPIO_InitStructure );
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_9;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init ( GPIOC, &GPIO_InitStructure );
}

void LCD1602_Wait_Ready ( void ) {
    u8 sta;
    DATAOUT ( 0xff );
    LCD_RS_Clr();
    LCD_RW_Set();

    do {
        LCD_EN_Set();
        delay_ms ( 5 );
        sta = GPIO_ReadInputDataBit ( GPIOA, GPIO_Pin_7 );
        LCD_EN_Clr();
    } while ( sta & 0x80 ); /* bit7����1��ʾҺ����æ���ظ����ֱ�������0Ϊֹ */
}

void LCD1602_Write_Cmd ( u8 cmd ) {
    LCD1602_Wait_Ready();
    LCD_RS_Clr();
    LCD_RW_Clr();
    DATAOUT ( cmd );
    LCD_EN_Set();
    LCD_EN_Clr();
}

void LCD1602_Write_Dat ( u8 dat ) {
    LCD1602_Wait_Ready();
    LCD_RS_Set();
    LCD_RW_Clr();
    DATAOUT ( dat );
    LCD_EN_Set();
    LCD_EN_Clr();
}

void LCD1602_ClearScreen ( void ) {
    LCD1602_Write_Cmd ( 0x01 );
}

void LCD1602_Set_Cursor ( u8 x, u8 y ) { /* ���ù��λ�� */
    u8 addr;

    if ( y == 0 ) {
        addr = 0x00 + x;
    } else {
        addr = 0x40 + x;
    }

    LCD1602_Write_Cmd ( addr | 0x80 );
}

void LCD1602_Show_Str ( u8 x, u8 y, u8 *str ) { /* ��Һ������ʾ�ַ��� */
    LCD1602_Set_Cursor ( x, y );

    while ( *str != '\0' ) {
        LCD1602_Write_Dat ( *str++ );
    }
}

void LCD1602_Init ( void ) {
    GPIO_Configuration();
    LCD1602_Write_Cmd ( 0x38 ); /* ��16*2����ʾ����5*7������8λ���ݿ� */
    LCD1602_Write_Cmd ( 0x0c ); /* ����ʾ�����ر� */
    LCD1602_Write_Cmd ( 0x06 ); /* ���ֲ�������ַ�Զ���һ */
    LCD1602_Write_Cmd ( 0x01 ); /* ���� */
}