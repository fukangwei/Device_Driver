#include "12864.h"

/*---------------------------------------------------------------------------
功能：向LCD12864写入命令或者数据
参数：ucData：要写入液晶的数据或者命令的内容
      bComOrData：命令或者数据的标志位选择，其中1代表写入的是数据，0代表写入的是命令
----------------------------------------------------------------------------*/
void LCD12864_WriteInfomation ( unsigned char ucData, bit bComOrData ) {
    LCD12864_CheckBusy(); /* 忙检测 */
    LCD12864_RW = 0; /* 拉低RW */
    LCD12864_RS = bComOrData; /* 根据标志位判断写入的是命令还是数据 */
    Delay ( 150 );
    LCDPORT = ucData; /* 将数据送至数据端口 */
    LCD12864_EN = 1; /* 使能信号 */
    Delay ( 150 );
    LCD12864_EN = 0;
    Delay ( 150 );
}

void LCD12864_Init ( void ) { /* LCD12864液晶初始化 */
    Delay ( 400 );
    LCD12864_PSB = 1; /* 8位并口工作模式 */
    Delay ( 150 );
    LCD12864_WriteInfomation ( 0x30, 0 ); /* 基本指令集 */
    Delay ( 150 );
    LCD12864_WriteInfomation ( 0x08, 0 ); /* 显示设置 */
    Delay ( 150 );
    LCD12864_WriteInfomation ( 0x10, 0 ); /* 光标设置 */
    Delay ( 150 );
    LCD12864_WriteInfomation ( 0x0c, 0 ); /* 游标设置 */
    Delay ( 150 );
    LCD12864_WriteInfomation ( 0x01, 0 ); /* 清屏 */
    Delay ( 150 );
    LCD12864_WriteInfomation ( 0x06, 0 ); /* 进入点设定 */
    Delay ( 150 );
}

void LCD12864_CheckBusy ( void ) { /* LCD12864忙检测 */
    unsigned char i = 250;
    LCD12864_RS = 0; /* 拉低 */
    LCD12864_RW = 1; /* 拉高 */
    LCD12864_EN = 1; /* 使能 */

    while ( ( i > 0 ) && ( P0 & 0x80 ) ) { /* 使用变量i做计时，避免液晶在死循环处停滞 */
        i--;
    }

    LCD12864_EN = 0; /* 释放 */
}

/*--------------------------------------------------------------------------------
功能：显示一行汉字(8个汉字或者16个英文字符)
参数：position：要显示的行的首地址，可选值0x80、0x88、0x90、0x98，其中0x80表示液晶的第一行；
      0x88表示液晶的第三行；0x90表示液晶的第二行；0x98表示液晶的第四行。
      p：要显示的内容的首地址
---------------------------------------------------------------------------------*/
void LCD12864_DisplayOneLine ( unsigned char position, unsigned char *p ) {
    unsigned char i;
    LCD12864_WriteInfomation ( position, 0 ); /* 写入要显示文字的行的首地址 */
    Delay ( 150 );

    for ( i = 0; i < 16; i++ ) { /* 依次执行写入操作 */
        LCD12864_WriteInfomation ( *p, 1 );
        p++;
    }
}

void Delay ( unsigned int uiCount ) {
    while ( uiCount-- );
}