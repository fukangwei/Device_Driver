#include "SI4432_spi.h"

#define RR 0x00
#define WR 0x80

unsigned char ITSTATUS;
char RF4432_RxBuf[RF4432_RxBuf_Len];
char RF4432_TxBuf[RF4432_TxBuf_Len];

static void delay_ms ( int num ) {
    int x, y;

    for ( y = 0; y < num; y++ ) {
        for ( x = 0; x < 500; x ) {
            x++;
        }
    }
}

uchar SPI_Write ( uchar dat ) {
    uchar i;

    for ( i = 0; i < 8; i++ ) { /* output 8-bit */
        MOSI = ( dat & 0x80 );
        dat <<= 1; /* shift next bit into MSB */
        SCK = 1; /* Set SCK high */
        dat |= MISO;
        SCK = 0; /* then set SCK low again */
    }

    return ( dat ); /* return read uchar */
}

unsigned char  RF4432_ReadReg ( unsigned char  addr ) { /* RF4432寄存器读取函数 */
    unsigned char value;
    RF4432_SEL = 0;
    SPI_Write ( addr | RR );
    value = SPI_Write ( addr | RR );
    RF4432_SEL = 1;
    return value;
}

void  RF4432_WriteReg ( unsigned char  addr, unsigned char value ) { /* RF4432寄存器写入函数 */
    RF4432_SEL = 0;
    SPI_Write ( addr | WR );
    SPI_Write ( value );
    RF4432_SEL = 1;
}

void  RF4432_WriteBurestReg ( char  addr, char  *p, char  count ) { /* RF4432寄存器连续写入函数 */
    unsigned char i;
    RF4432_SEL = 0;
    SPI_Write ( addr | WR );

    for ( i = 0; i < count; i++ ) {
        SPI_Write ( p[i] );
    }

    RF4432_SEL = 1;
}

void RF4432_ReadBurestReg ( char  addr,  char  *p, char  count ) { /* RF4432寄存器连续读取函数 */
    unsigned char i;
    RF4432_SEL = 0;
    SPI_Write ( addr | RR );

    for ( i = 0; i < count; i++ ) {
        p[i] = SPI_Write ( addr | RR );
    }

    RF4432_SEL = 1;
}

void RF4432_Init ( void ) { /* RF4432射频芯片初始化函数 */
    RF4432_IRQ = 1;
    RF4432_SDN = 0; /* 允许RF4432工作 */
    delay_ms ( 30 );
    delay_ms ( 30 );
    RF4432_ReadReg ( INTERRUPT_STATUS_1 ); /* 清中断 */
    RF4432_ReadReg ( INTERRUPT_STATUS_2 );
    RF4432_WriteReg ( OPERATING_FUNCTION_CONTROL_1, 0x80 ); /* 软件复位 */

    while ( RF4432_IRQ == 1 ); /* 等待软复位完成，当软复位完成后有中断发生 */

    RF4432_WriteReg ( INTERRUPT_ENABLE_1, 0x00 ); /* 开中断 */
    RF4432_WriteReg ( INTERRUPT_ENABLE_2, 0x00 ); /* 禁止POR中断 */
    /* 频率设置434 */
    RF4432_WriteReg ( 0x75, 0x53 );
    RF4432_WriteReg ( 0x76, 0x64 );
    RF4432_WriteReg ( 0x77, 0x00 );
    /* “1.2K bps”发射速率 */
    RF4432_WriteReg ( 0x2a, 0x14 );
    RF4432_WriteReg ( 0x6e, 0x09 );
    RF4432_WriteReg ( 0x6f, 0xd5 );
    RF4432_WriteReg ( 0x70, 0x2c );
    RF4432_WriteReg ( 0x72, 0x38 ); /* 频率偏差(1.2kbps) */
    /* 下面的设置根据Silabs的Excel (9.6 kbps, deviation: 45 kHz, channel filter BW: 102.2 kHz) */
    RF4432_WriteReg ( 0x1C, 0x1b ); /* write 0x1E to the IF Filter Bandwidth register */
    RF4432_WriteReg ( 0x20, 0x83 ); /* write 0xD0 to the Clock Recovery Oversampling Ratio register */
    RF4432_WriteReg ( 0x21, 0xc0 ); /* write 0x00 to the Clock Recovery Offset 2 register */
    RF4432_WriteReg ( 0x22, 0x13 ); /* write 0x9D to the Clock Recovery Offset 1 register */
    RF4432_WriteReg ( 0x23, 0xa9 ); /* write 0x49 to the Clock Recovery Offset 0 register */
    RF4432_WriteReg ( 0x24, 0x00 ); /* write 0x00 to the Clock Recovery Timing Loop Gain 1 register */
    RF4432_WriteReg ( 0x25, 0x03 ); /* write 0x24 to the Clock Recovery Timing Loop Gain 0 register */
    RF4432_WriteReg ( 0x1D, 0x40 ); /* write 0x40 to the AFC Loop Gearshift Override register */
    RF4432_WriteReg ( 0x1E, 0x0A ); /* write 0x0A to the AFC Timing Control register */
    RF4432_WriteReg ( 0x2A, 0x14 ); /* write 0x20 to the AFC Limiter register */
    /* 前导码(同步字) */
    RF4432_WriteReg ( 0x34, 0X0A ); /* 发射5字节的Preamble */
    RF4432_WriteReg ( 0x35, 0x2A ); /* 需要检测Preamble */
    RF4432_WriteReg ( 0x33, 0x02 );
    RF4432_WriteReg ( 0x36, 0x2d ); /* 同步字为0x2dd4 */
    RF4432_WriteReg ( 0x37, 0xd4 );
    RF4432_WriteReg ( 0x30, 0x8D ); /* 使能“PH + FIFO”模式，高位在前面，使能CRC校验 */
    RF4432_WriteReg ( 0x32, 0x00 ); /* 禁止帧头 */
    RF4432_WriteReg ( 0x71, 0x63 );
    RF4432_WriteReg ( 0x0b, 0x12 );
    RF4432_WriteReg ( 0x0c, 0x15 );
    RF4432_WriteReg ( 0x09, 0xD7 ); /* 负载电容 */
    RF4432_WriteReg ( 0x69, 0x60 ); /* AGC过载 */
    RF4432_WriteReg ( 0x6d, 0x17 ); /* 发射功率 */
    RF4432_WriteReg ( 0x07, 0x05 ); /* 手动打开接收 */
    /* 打开接收中断 */
    RF4432_WriteReg ( 0x05, 0x03 );
    RF4432_WriteReg ( 0x06, 0x00 );
    /* 清中断 */
    RF4432_ReadReg ( 0x03 ); /* read the Interrupt Status1 register */
    RF4432_ReadReg ( 0x04 ); /* read the Interrupt Status2 register */
    RF4432_WriteReg ( 0x07, 0x05 ); /* 打开接收 */
    RF4432_WriteReg ( 0x0d, 0xf4 ); /* GPIO2接收数据 */
}

void RF4432_SetRxMode ( void ) { /* RF4432设置接收模式函数 */
    RF4432_WriteReg ( OPERATING_FUNCTION_CONTROL_2, RF4432_ReadReg ( OPERATING_FUNCTION_CONTROL_2 ) | 0x02 ); /* 接收FIFO清0 */
    RF4432_WriteReg ( OPERATING_FUNCTION_CONTROL_2, RF4432_ReadReg ( OPERATING_FUNCTION_CONTROL_2 ) & 0xFD );
    RF4432_WriteReg ( GPIO1_CONFIGURATION, 0xca ); /* GPIO设置 */
    RF4432_WriteReg ( GPIO2_CONFIGURATION, 0xca );
    RF4432_WriteReg ( IO_PORT_CONFIGURATION, 0x04 );
    RF4432_WriteReg ( HEADER_CONTROL_1, 0x8c );
    RF4432_WriteReg ( HEADER_CONTROL_2, 0x02 ); /* 没有发射帧头，帧头不包括数据包长度，02是不包括 */
    RF4432_ReadReg ( INTERRUPT_STATUS_1 ); /* 清中断 */
    RF4432_ReadReg ( INTERRUPT_STATUS_2 );
    RF4432_WriteReg ( INTERRUPT_ENABLE_1, 0x02 ); /* 使能接收包中断 */
    RF4432_WriteReg ( INTERRUPT_ENABLE_2, 0x00 );
    RF4432_WriteReg ( OPERATING_FUNCTION_CONTROL_1, 0x05 ); /* 人工接收模式，打开晶振 */
    RF4432_WriteReg ( TRANSMIT_PACKET_LENGTH, RF4432_TxBuf_Len ); /* 设置发射包长度 */
    RF4432_WriteReg ( RECEIVED_PACKET_LENGTH, RF4432_RxBuf_Len ); /* 设置接收包长度 */
}

bit RF4432_RxPacket ( void ) { /* RF4432数据包接收函数 */
    if ( !RF4432_IRQ ) {
        ITSTATUS = RF4432_ReadReg ( INTERRUPT_STATUS_1 ) ;

        if ( ( ITSTATUS & 0x02 ) == 0x02 ) {
            return 1;
        } else {
            return 0;
        }
    } else {
        return 0;
    }
}

void RF4432_TxPacket ( void ) { /* RF4432数据包发送函数 */
    RF4432_WriteReg ( OPERATING_FUNCTION_CONTROL_2, RF4432_ReadReg ( OPERATING_FUNCTION_CONTROL_2 ) | 0x01 ); /* 发射FIFO清0 */
    RF4432_WriteReg ( OPERATING_FUNCTION_CONTROL_2, RF4432_ReadReg ( OPERATING_FUNCTION_CONTROL_2 ) & 0xFE );
    RF4432_WriteBurestReg ( FIFO_ACCESS, RF4432_TxBuf, RF4432_TxBuf_Len ); /* 发射 */
    RF4432_ReadReg ( INTERRUPT_STATUS_1 ); /* 清中断 */
    RF4432_ReadReg ( INTERRUPT_STATUS_2 );
    RF4432_WriteReg ( INTERRUPT_ENABLE_1, 0x04 ); /* 使能包发送中断 */
    RF4432_WriteReg ( OPERATING_FUNCTION_CONTROL_1, 0x09 ); /* 人工发射模式，打开晶振 */

    while ( RF4432_IRQ == 1 ); /* 等待发送完成，产生中断 */

    if ( !RF4432_IRQ ) {
        ITSTATUS = RF4432_ReadReg ( INTERRUPT_STATUS_1 );

        while ( ( ITSTATUS & 0x04 ) != 0x04 ); /* 已经发射一个有效包则第2位为1，如果为0则等待 */
    }
}