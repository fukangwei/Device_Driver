#ifndef __TOUCH_H__
#define __TOUCH_H__
#include "sys.h"

#define Key_Down 0x01
#define Key_Up   0x00

typedef struct {
    u16 X0; /* ԭʼ���� */
    u16 Y0;
    u16 X; /* �ݴ����� */
    u16 Y;
    u8  Key_Sta; /* ���ص�״̬ */
    /* ������У׼���� */
    float xfac;
    float yfac;
    short xoff;
    short yoff;
    u8 touchtype; /* ��touchtype = 0��ʱ�ʺ�����ΪX���꣬����ΪY����Ĵ���������touchtype = 1����֮ */
} Pen_Holder;

extern Pen_Holder Pen_Point;

/* �봥����оƬ�������� */
#define PEN  PCin(1)
#define DOUT PCin(2)
#define TDIN PCout(3)
#define TCLK PCout(0)
#define TCS  PCout(13)

extern u8 CMD_RDX; /* �ò�ַ�ʽ��X���� */
extern u8 CMD_RDY; /* �ò�ַ�ʽ��Y���� */

#define TEMP_RD 0XF0
#define ADJ_SAVE_ENABLE

void Touch_Init ( void );
u8 Read_ADS ( u16 *x, u16 *y );
u8 Read_ADS2 ( u16 *x, u16 *y );
u16 ADS_Read_XY ( u8 xy );
u16 ADS_Read_AD ( u8 CMD );
void ADS_Write_Byte ( u8 num );
void Drow_Touch_Point ( u8 x, u16 y );
void Draw_Big_Point ( u8 x, u16 y );
void Touch_Adjust ( void );
void Save_Adjdata ( void );
u8 Get_Adjdata ( void );
void Pen_Int_Set ( u8 en );
void Convert_Pos ( void );
#endif