#ifndef LCD12864_H
#define LCD12864_H
#include "STC12C5A.h"

sbit LCD12864_RS  = P1 ^ 0; /* RS控制引脚 */
sbit LCD12864_RW  = P1 ^ 1; /* RW控制引脚 */
sbit LCD12864_EN  = P2 ^ 5; /* EN控制引脚 */
sbit LCD12864_PSB = P1 ^ 3; /* 模式选择引脚，ST7920控制器，1为8位并行接口，0为串行接口 */

#define LCDPORT P0 /* 数据引脚 */

void LCD12864_Init ( void );
void LCD12864_WriteInfomation ( unsigned char ucData, bit bComOrData );
void LCD12864_CheckBusy ( void );
void LCD12864_DisplayOneLine ( unsigned char ucPos, unsigned char *ucStr );
void Delay ( unsigned int uiCount );
#endif