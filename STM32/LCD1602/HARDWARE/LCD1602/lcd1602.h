#ifndef __LCD1602_H
#define __LCD1602_H
#include "sys.h"

/* 1602液晶指令/数据选择引脚 */
#define LCD_RS_Set() GPIO_SetBits(GPIOC, GPIO_Pin_9)
#define LCD_RS_Clr() GPIO_ResetBits(GPIOC, GPIO_Pin_9)
/* 1602液晶读写引脚 */
#define LCD_RW_Set() GPIO_SetBits(GPIOC, GPIO_Pin_8)
#define LCD_RW_Clr() GPIO_ResetBits(GPIOC, GPIO_Pin_8)
/* 1602液晶使能引脚 */
#define LCD_EN_Set() GPIO_SetBits(GPIOC, GPIO_Pin_7)
#define LCD_EN_Clr() GPIO_ResetBits(GPIOC, GPIO_Pin_7)
#define DATAOUT(x) GPIO_Write(GPIOA, x) /* 1602液晶数据端口 */

void GPIO_Configuration ( void );
void LCD1602_Wait_Ready ( void );
void LCD1602_Write_Cmd ( u8 cmd );
void LCD1602_Write_Dat ( u8 dat );
void LCD1602_ClearScreen ( void );
void LCD1602_Set_Cursor ( u8 x, u8 y );
void LCD1602_Show_Str ( u8 x, u8 y, u8 *str );
void LCD1602_Init ( void );
#endif