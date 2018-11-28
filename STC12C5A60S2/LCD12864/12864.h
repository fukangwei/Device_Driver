#ifndef LCD12864_H
#define LCD12864_H
#include "STC12C5A.h"

sbit LCD12864_RS  = P1 ^ 0; /* RS�������� */
sbit LCD12864_RW  = P1 ^ 1; /* RW�������� */
sbit LCD12864_EN  = P2 ^ 5; /* EN�������� */
sbit LCD12864_PSB = P1 ^ 3; /* ģʽѡ�����ţ�ST7920��������1Ϊ8λ���нӿڣ�0Ϊ���нӿ� */

#define LCDPORT P0 /* �������� */

void LCD12864_Init ( void );
void LCD12864_WriteInfomation ( unsigned char ucData, bit bComOrData );
void LCD12864_CheckBusy ( void );
void LCD12864_DisplayOneLine ( unsigned char ucPos, unsigned char *ucStr );
void Delay ( unsigned int uiCount );
#endif