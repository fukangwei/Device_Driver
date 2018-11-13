#include "ds18b20.h"
#include "delay.h"

static void DS18B20_Rst ( void ) { /* 复位DS18B20 */
    DS18B20_IO_OUT();
    DS18B20_DQ_OUT = 0; /* 拉低总线电平 */
    delay_us ( 480 ); /* 如果总线停留在低电平超过480us，总线上的所有器件都将被复位 */
    DS18B20_DQ_OUT = 1; /* 使总线恢复空闲状态 */
    delay_us ( 15 );
}

static u8 DS18B20_Check ( void ) { /* 等待DS18B20的回应。返回1表示未检测到DS18B20，返回0表示检测到 */
    u8 i = 0;
    DS18B20_IO_IN();

    while ( DS18B20_DQ_IN && i++ < 60 ) {
        delay_us ( 1 );
    }

    if ( i >= 60 ) {
        return 1;
    }

    i = 0;

    while ( !DS18B20_DQ_IN && i++ < 240 ) {
        delay_us ( 1 );
    };

    if ( i >= 240 ) {
        return 1;
    }

    return 0;
}

static u8 DS18B20_Read_Bit ( void ) { /* 从DS18B20读取一个位 */
    u8 data;
    DS18B20_IO_OUT();
    DS18B20_DQ_OUT = 0;
    delay_us ( 2 );
    DS18B20_DQ_OUT = 1;
    DS18B20_IO_IN();
    delay_us ( 12 );

    if ( DS18B20_DQ_IN ) {
        data = 1;
    } else {
        data = 0;
    }

    delay_us ( 50 );
    return data;
}

static u8 DS18B20_Read_Byte ( void ) { /* 从DS18B20读取一个字节 */
    u8 i, j, dat;
    dat = 0;

    for ( i = 1; i <= 8; i++ ) {
        j = DS18B20_Read_Bit();
        dat = ( j << 7 ) | ( dat >> 1 );
    }

    return dat;
}

static void DS18B20_Write_Byte ( u8 dat ) { /* 写一个字节到DS18B20 */
    u8 j;
    u8 testb;
    DS18B20_IO_OUT();

    for ( j = 1; j <= 8; j++ ) {
        testb = dat & 0x01;
        dat = dat >> 1;

        if ( testb ) { /* 如果写入的位为1 */
            DS18B20_DQ_OUT = 0;
            delay_us ( 2 );
            DS18B20_DQ_OUT = 1;
            delay_us ( 60 );
        } else { /* 如果写入的位为0 */
            DS18B20_DQ_OUT = 0;
            delay_us ( 60 );
            DS18B20_DQ_OUT = 1;
            delay_us ( 2 );
        }
    }
}

static void DS18B20_Start ( void ) { /* 开始温度转换 */
    DS18B20_Rst();
    delay_us ( 480 ); /* 复位之后再延时一段时间 */
    DS18B20_Write_Byte ( 0xcc ); /* 忽略ROM指令 */
    DS18B20_Write_Byte ( 0x44 ); /* 温度转换指令 */
}

u8 DS18B20_Init ( void ) { /* 初始化DS18B20的IO口，检测DS18B20是否存在。返回1表示不存在，0表示存在 */
    GPIO_InitTypeDef GPIO_InitStructure;
    RCC_APB2PeriphClockCmd ( RCC_APB2Periph_GPIOA, ENABLE );
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init ( GPIOA, &GPIO_InitStructure );
    GPIO_SetBits ( GPIOA, GPIO_Pin_0 );
    DS18B20_Rst();
    return DS18B20_Check();
}

/* 从DS18B20得到温度值(“-550”至1250)。温度为负时读取方法：将16进制取反后加1，再转换成10进制 */
short DS18B20_Get_Temp ( void ) {
    u8 temp;
    u8 TL, TH;
    short tem;
    DS18B20_Start();
    DS18B20_Rst();
    DS18B20_Check();
    DS18B20_Write_Byte ( 0xcc );
    DS18B20_Write_Byte ( 0xbe );
    TL = DS18B20_Read_Byte(); /* 温度的低8位 */
    TH = DS18B20_Read_Byte(); /* 温度的高8位 */

    if ( TH > 7 ) {
        TH = ~TH;
        TL = ~TL;
        TL += 1;
        temp = 0; /* 温度为负 */
    } else {
        temp = 1; /* 温度为正 */
    }

    tem = TH; /* 获得高八位 */
    tem <<= 8;
    tem += TL; /* 获得低八位 */
    tem = ( float ) tem * 0.625;

    if ( temp ) {
        return tem;
    } else {
        return ( -tem );
    }
}