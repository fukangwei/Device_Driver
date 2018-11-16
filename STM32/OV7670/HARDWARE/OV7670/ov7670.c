#include "sys.h"
#include "ov7670.h"
#include "ov7670cfg.h"
#include "timer.h"
#include "delay.h"
#include "usart.h"
#include "sccb.h"
#include "exti.h"

u8 OV7670_Init ( void ) { /* 初始化OV7670。返回0为成功，返回其他值为错误代码 */
    u8 temp;
    u16 i = 0;
    /* 设置IO */
    RCC->APB2ENR |= 1 << 2; /* 先使能外设PORTA时钟 */
    RCC->APB2ENR |= 1 << 3; /* 先使能外设PORTB时钟 */
    GPIOA->CRL &= 0XFFF0FF00;
    GPIOA->CRL |= 0X00030033; /* PA0、1和4为输出 */
    GPIOA->ODR |= 1 << 4;
    GPIOA->ODR |= 3 << 0;
    GPIOA->CRH &= 0X00F00FFF;
    GPIOA->CRH |= 0X83033000; /* PA15为输入、PA11、12和14为输出 */
    GPIOA->ODR |= 3 << 14;
    GPIOA->ODR |= 3 << 11;
    JTAG_Set ( SWD_ENABLE );
    SCCB_Init(); /* 初始化SCCB的IO口 */

    if ( SCCB_WR_Reg ( 0x12, 0x80 ) ) {
        return 1; /* 复位SCCB */
    }

    delay_ms ( 50 );
    temp = SCCB_RD_Reg ( 0x0b ); /* 读取产品型号 */

    if ( temp != 0x73 ) {
        return 2;
    }

    temp = SCCB_RD_Reg ( 0x0a );

    if ( temp != 0x76 ) {
        return 2;
    }

    for ( i = 0; i < sizeof ( ov7670_init_reg_tbl ) / sizeof ( ov7670_init_reg_tbl[0] ); i++ ) { /* 初始化序列 */
        SCCB_WR_Reg ( ov7670_init_reg_tbl[i][0], ov7670_init_reg_tbl[i][1] );
        delay_ms ( 2 );
    }

    return 0x00; /* ok */
}

/* OV7670功能设置(白平衡设置)，mode为0是自动，1是太阳sunny，2是阴天cloudy，3是办公室office，4是家里home */
void OV7670_Light_Mode ( u8 mode ) {
    u8 reg13val = 0XE7; /* 默认就是设置为自动白平衡 */
    u8 reg01val = 0;
    u8 reg02val = 0;

    switch ( mode ) {
        case 1: /* sunny */
            reg13val = 0XE5;
            reg01val = 0X5A;
            reg02val = 0X5C;
            break;

        case 2: /* cloudy */
            reg13val = 0XE5;
            reg01val = 0X58;
            reg02val = 0X60;
            break;

        case 3: /* office */
            reg13val = 0XE5;
            reg01val = 0X84;
            reg02val = 0X4c;
            break;

        case 4: /* home */
            reg13val = 0XE5;
            reg01val = 0X96;
            reg02val = 0X40;
            break;
    }

    SCCB_WR_Reg ( 0X13, reg13val ); /* COM8设置 */
    SCCB_WR_Reg ( 0X01, reg01val ); /* AWB蓝色通道增益 */
    SCCB_WR_Reg ( 0X02, reg02val ); /* AWB红色通道增益 */
}

void OV7670_Color_Saturation ( u8 sat ) { /* 色度设置，sat为0是“-2”，1是“-1”，2是0，3是1，4是2 */
    u8 reg4f5054val = 0X80; /* 默认“sat = 2”，即不调节色度的设置 */
    u8 reg52val = 0X22;
    u8 reg53val = 0X5E;

    switch ( sat ) {
        case 0: /* -2 */
            reg4f5054val = 0X40;
            reg52val = 0X11;
            reg53val = 0X2F;
            break;

        case 1: /* -1 */
            reg4f5054val = 0X66;
            reg52val = 0X1B;
            reg53val = 0X4B;
            break;

        case 3: /* 1 */
            reg4f5054val = 0X99;
            reg52val = 0X28;
            reg53val = 0X71;
            break;

        case 4: /* 2 */
            reg4f5054val = 0XC0;
            reg52val = 0X33;
            reg53val = 0X8D;
            break;
    }

    SCCB_WR_Reg ( 0X4F, reg4f5054val ); /* 色彩矩阵系数1 */
    SCCB_WR_Reg ( 0X50, reg4f5054val ); /* 色彩矩阵系数2 */
    SCCB_WR_Reg ( 0X51, 0X00 ); /* 色彩矩阵系数3 */
    SCCB_WR_Reg ( 0X52, reg52val ); /* 色彩矩阵系数4 */
    SCCB_WR_Reg ( 0X53, reg53val ); /* 色彩矩阵系数5 */
    SCCB_WR_Reg ( 0X54, reg4f5054val ); /* 色彩矩阵系数6 */
    SCCB_WR_Reg ( 0X58, 0X9E ); /* MTXS */
}

void OV7670_Brightness ( u8 bright ) { /* 亮度设置，bright为0是“-2”，1是“-1”，2是0，3是1，4是2 */
    u8 reg55val = 0X00; /* 默认“bright=2” */

    switch ( bright ) {
        case 0: /* -2 */
            reg55val = 0XB0;
            break;

        case 1: /* -1 */
            reg55val = 0X98;
            break;

        case 3: /* 1 */
            reg55val = 0X18;
            break;

        case 4: /* 2 */
            reg55val = 0X30;
            break;
    }

    SCCB_WR_Reg ( 0X55, reg55val ); /* 亮度调节 */
}

void OV7670_Contrast ( u8 contrast ) { /* 对比度设置，contrast为0是“-2”，1是“-1”，2是0，3是1，4是2 */
    u8 reg56val = 0X40; /* 默认“contrast = 2” */

    switch ( contrast ) {
        case 0: /* -2 */
            reg56val = 0X30;
            break;

        case 1: /* -1 */
            reg56val = 0X38;
            break;

        case 3: /* 1 */
            reg56val = 0X50;
            break;

        case 4: /* 2 */
            reg56val = 0X60;
            break;
    }

    SCCB_WR_Reg ( 0X56, reg56val ); /* 对比度调节 */
}

/* 特效设置，eft为0是普通模式，1是负片，2是黑白，3是偏红色，4是偏绿色，5是偏蓝色，6是复古 */
void OV7670_Special_Effects ( u8 eft ) {
    u8 reg3aval = 0X04; /* 默认为普通模式 */
    u8 reg67val = 0XC0;
    u8 reg68val = 0X80;

    switch ( eft ) {
        case 1: /* 负片 */
            reg3aval = 0X24;
            reg67val = 0X80;
            reg68val = 0X80;
            break;

        case 2: /* 黑白 */
            reg3aval = 0X14;
            reg67val = 0X80;
            reg68val = 0X80;
            break;

        case 3: /* 偏红色 */
            reg3aval = 0X14;
            reg67val = 0Xc0;
            reg68val = 0X80;
            break;

        case 4: /* 偏绿色 */
            reg3aval = 0X14;
            reg67val = 0X40;
            reg68val = 0X40;
            break;

        case 5: /* 偏蓝色 */
            reg3aval = 0X14;
            reg67val = 0X80;
            reg68val = 0XC0;
            break;

        case 6: /* 复古 */
            reg3aval = 0X14;
            reg67val = 0XA0;
            reg68val = 0X40;
            break;
    }

    SCCB_WR_Reg ( 0X3A, reg3aval ); /* TSLB设置 */
    SCCB_WR_Reg ( 0X68, reg67val ); /* MANU，手动U值 */
    SCCB_WR_Reg ( 0X67, reg68val ); /* MANV，手动V值 */
}

void OV7670_Window_Set ( u16 sx, u16 sy, u16 width, u16 height ) { /* 设置图像输出窗口，对QVGA设置 */
    u16 endx;
    u16 endy;
    u8 temp;
    endx = sx + width * 2; /* V*2 */
    endy = sy + height * 2;

    if ( endy > 784 ) {
        endy -= 784;
    }

    temp = SCCB_RD_Reg ( 0X03 ); /* 读取Vref之前的值 */
    temp &= 0XF0;
    temp |= ( ( endx & 0X03 ) << 2 ) | ( sx & 0X03 );
    SCCB_WR_Reg ( 0X03, temp ); /* 设置Vref的start和end的最低2位 */
    SCCB_WR_Reg ( 0X19, sx >> 2 ); /* 设置Vref的start高8位 */
    SCCB_WR_Reg ( 0X1A, endx >> 2 ); /* 设置Vref的end的高8位 */
    temp = SCCB_RD_Reg ( 0X32 ); /* 读取Href之前的值 */
    temp &= 0XC0;
    temp |= ( ( endy & 0X07 ) << 3 ) | ( sy & 0X07 );
    SCCB_WR_Reg ( 0X17, sy >> 3 ); /* 设置Href的start高8位 */
    SCCB_WR_Reg ( 0X18, endy >> 3 ); /* 设置Href的end的高8位 */
}