#include "24cxx.h"
#include "delay.h"

/* 24CXX驱动函数(适合24C01之24C16，24C32至256未经过测试，有待验证) */
void AT24CXX_Init ( void ) { /* 初始化IIC接口 */
    IIC_Init();
}

u8 AT24CXX_ReadOneByte ( u16 ReadAddr ) { /* 在AT24CXX指定地址读出一个数据，参数ReadAddr是地址 */
    u8 temp = 0;
    IIC_Start();

    if ( EE_TYPE > AT24C16 ) {
        IIC_Send_Byte ( 0XA0 ); /* 发送写命令 */
        IIC_Wait_Ack();
        IIC_Send_Byte ( ReadAddr >> 8 ); /* 发送高地址 */
    } else {
        IIC_Send_Byte ( 0XA0 + ( ( ReadAddr / 256 ) << 1 ) ); /* 发送器件地址0XA0，写数据 */
    }

    IIC_Wait_Ack();
    IIC_Send_Byte ( ReadAddr % 256 ); /* 发送低地址 */
    IIC_Wait_Ack();
    IIC_Start();
    IIC_Send_Byte ( 0XA1 ); /* 进入接收模式 */
    IIC_Wait_Ack();
    temp = IIC_Read_Byte ( 0 );
    IIC_Stop(); /* 产生一个停止条件 */
    return temp;
}

/* 在AT24CXX指定地址写入一个数据，参数WriteAddr是地址，DataToWrite是数据 */
void AT24CXX_WriteOneByte ( u16 WriteAddr, u8 DataToWrite ) {
    IIC_Start();

    if ( EE_TYPE > AT24C16 ) {
        IIC_Send_Byte ( 0XA0 ); /* 发送写命令 */
        IIC_Wait_Ack();
        IIC_Send_Byte ( WriteAddr >> 8 ); /* 发送高地址 */
    } else {
        IIC_Send_Byte ( 0XA0 + ( ( WriteAddr / 256 ) << 1 ) ); /* 发送器件地址0XA0，写数据 */
    }

    IIC_Wait_Ack();
    IIC_Send_Byte ( WriteAddr % 256 ); /* 发送低地址 */
    IIC_Wait_Ack();
    IIC_Send_Byte ( DataToWrite ); /* 发送字节 */
    IIC_Wait_Ack();
    IIC_Stop(); /* 产生一个停止条件 */
    delay_ms ( 10 );
}

/* 在AT24CXX指定地址写入长度为Len的数据，用于写入16bit或者32bit的数据。参数WriteAddr是地址，DataToWrite是数据数组首地址，Len是要写入数据的长度(2或4) */
void AT24CXX_WriteLenByte ( u16 WriteAddr, u32 DataToWrite, u8 Len ) {
    u8 t;

    for ( t = 0; t < Len; t++ ) {
        AT24CXX_WriteOneByte ( WriteAddr + t, ( DataToWrite >> ( 8 * t ) ) & 0xff );
    }
}

/* 在AT24CXX指定地址读出长度为Len的数据，用于读出16bit或者32bit的数据。参数ReadAddr是地址，Len是读出数据的长度(2或4) */
u32 AT24CXX_ReadLenByte ( u16 ReadAddr, u8 Len ) {
    u8 t;
    u32 temp = 0;

    for ( t = 0; t < Len; t++ ) {
        temp <<= 8;
        temp += AT24CXX_ReadOneByte ( ReadAddr + Len - t - 1 );
    }

    return temp;
}

/* 检查AT24CXX是否正常，这里用了24XX的最后一个地址(255)来存储标志字(如果用其他24C系列，这个地址要修改)。返回1表示检测失败，返回0表示检测成功 */
u8 AT24CXX_Check ( void ) {
    u8 temp;
    temp = AT24CXX_ReadOneByte ( EE_TYPE );

    if ( temp == 0X55 ) {
        return 0;
    } else { /* 排除第一次初始化的情况 */
        AT24CXX_WriteOneByte ( EE_TYPE, 0X55 );
        temp = AT24CXX_ReadOneByte ( EE_TYPE );

        if ( temp == 0X55 ) {
            return 0;
        }
    }

    return 1;
}

/* 在AT24CXX指定地址开始读出指定个数的数据。参数ReadAddr是地址，对24c02为0至255；pBuffer是数据数组首地址，NumToRead是要读出数据的个数 */
void AT24CXX_Read ( u16 ReadAddr, u8 *pBuffer, u16 NumToRead ) {
    while ( NumToRead ) {
        *pBuffer++ = AT24CXX_ReadOneByte ( ReadAddr++ );
        NumToRead--;
    }
}

/* 在AT24CXX指定地址开始写入指定个数的数据。参数WriteAddr是地址，对24c02为0至255；pBuffer是数据数组首地址，NumToWrite是要写入数据的个数 */
void AT24CXX_Write ( u16 WriteAddr, u8 *pBuffer, u16 NumToWrite ) {
    while ( NumToWrite-- ) {
        AT24CXX_WriteOneByte ( WriteAddr, *pBuffer );
        WriteAddr++;
        pBuffer++;
    }
}