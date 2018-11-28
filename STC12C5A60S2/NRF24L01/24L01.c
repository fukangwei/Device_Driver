#include "24L01.h"

static uchar TX_ADDRESS[TX_ADR_WIDTH] = {0xE7, 0xE7, 0xE7, 0xE7, 0xE7}; /* 本地地址 */
static uchar RX_ADDRESS[RX_ADR_WIDTH] = {0xE7, 0xE7, 0xE7, 0xE7, 0xE7}; /* 接收地址 */

/* GPIO模拟SPI的功能，将输出字节(MOSI)从MSB循环输出，同时将输入字节(MISO)从LSB循环移入。上升沿读入，下降沿输出 */
static uchar NRF24SPI_Send_Byte ( uchar dat ) {
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

/*---------------------------------------------------------------------------------------------------------------------
 * 发射缓冲区访问函数，主要用来把数组里的数放到发射FIFO缓冲区中。基本思路就是通过WRITE_REG命令把数据存到发射FIFO(WR_TX_PLOAD)中去
 * 参数reg为寄存器地址，pBuf为待写入数据地址，uchars为写入数据的个数
 --------------------------------------------------------------------------------------------------------------------*/
static uchar SPI_Write_Buf ( uchar reg, uchar *pBuf, uchar Len ) {
    uint status, i;
    CSN = 0; /* Set CSN low, init SPI tranaction(置CSN为低，使能芯片，配置芯片各个参数) */
    status = NRF24SPI_Send_Byte ( reg ); /* Select register to write to and read status byte */

    for ( i = 0; i < Len; i++ ) {
        NRF24SPI_Send_Byte ( *pBuf ); /* then write all byte in buffer(*pBuf) */
        pBuf ++;
    }

    CSN = 1; /* Set CSN high again */
    return ( status ); /* return nRF24L01 status byte */
}

/*-------------------------------------------------------------------------------------------------------------------------------
* 接收缓冲区访问函数：主要用来在接收时读取FIFO缓冲区中的值。基本思路就是通过READ_REG命令把数据从接收FIFO(RD_RX_PLOAD)中读出并存到数组里面去
* 参数reg为寄存器地址，pBuf为待读出数据地址，uchars为读出数据的个数
-------------------------------------------------------------------------------------------------------------------------------*/
static uchar SPI_Read_Buf ( uchar reg, uchar *pBuf, uchar Len ) {
    uint status, i;
    CSN = 0; /* Set CSN low, init SPI tranaction */
    status = NRF24SPI_Send_Byte ( reg ); /* Select register to write to and read status uchar */

    for ( i = 0; i < Len; i++ ) {
        pBuf[i] = NRF24SPI_Send_Byte ( 0 ); /* Perform SPI_RW to read byte from nRF24L01 */
    }

    CSN = 1; /* Set CSN high again */
    return ( status ); /* return nRF24L01 status */
}

uchar NRF24L01_Check ( void ) { /* 检测NRF24L01是否正常工作 */
    uchar bu[5] = {0XA5, 0XA5, 0XA5, 0XA5, 0XA5};
    uchar bu1[5];
    uchar i;
    SPI_Write_Buf ( WRITE_REG + TX_ADDR, bu, 5 ); /* 写入5个字节的地址 */
    SPI_Read_Buf ( TX_ADDR, bu1, 5 ); /* 读出写入的地址 */

    for ( i = 0; i < 5; i++ )
        if ( bu1[i] != 0XA5 ) {
            break;
        }

    if ( i != 5 ) {
        return 1; /* NRF24L01不正常 */
    }

    return 0; /* NRF24L01正常 */
}

void init_NRF24L01 ( void ) { /* NRF24L01初始化 */
    uchar buf[5] = {0};
    CE = 0;
    CSN = 0;
    SPI_Read_Buf ( TX_ADDR, buf, TX_ADR_WIDTH );
}

/* 读取寄存器值：基本思路就是通过READ_REG命令(也就是0x00+寄存器地址)，把寄存器中的值读出来。对于函数来说，就是把reg寄存器的值读到reg_val中去 */
static uchar SPI_RD_Reg ( uchar reg ) {
    uchar reg_val;
    CSN = 0; /* CSN low, initialize SPI communication */
    NRF24SPI_Send_Byte ( reg ); /* Select register to read from */
    reg_val = NRF24SPI_Send_Byte ( 0 ); /* then read register value */
    CSN = 1; /* CSN high, terminate SPI communication */
    return ( reg_val ); /* return register value */
}

/* ---------------------------------------------------------------------------------------
 * 寄存器访问函数：用来设置24L01的寄存器的值。基本思路就是通过WRITE_REG命令(也就是0x20 + 寄存器地址)
   把要设定的值写到相应的寄存器地址里面去，并读取返回值。对于函数来说，也就是把value值写到reg寄存器中
 * 注意，访问NRF24L01之前首先要enable芯片(CSN = 0)，访问完了以后再disable芯片(CSN = 1)
----------------------------------------------------------------------------------------*/
static uchar SPI_WR_Reg ( uchar reg, uchar value ) {
    uchar status;
    CSN = 0; /* CSN low, init SPI transaction */
    status = NRF24SPI_Send_Byte ( reg ); /* select register */
    NRF24SPI_Send_Byte ( value ); /* and write value to it */
    CSN = 1; /* CSN high again */
    return ( status ); /* return nRF24L01 status */
}

void RX_Mode ( void ) { /* 数据接收模式配置 */
    CE = 0;
    SPI_Write_Buf ( WRITE_REG + RX_ADDR_P0, RX_ADDRESS, RX_ADR_WIDTH ); /* 写接收端地址 */
    SPI_WR_Reg ( WRITE_REG + EN_AA, 0 ); /* 频道0自动ACK应答允许 */
    SPI_WR_Reg ( WRITE_REG + EN_RXADDR, 0x01 ); /* 允许接收地址只有频道0，如果需要多频道可以参考数据手册 */
    SPI_WR_Reg ( WRITE_REG + RF_CH, 40 ); /* 设置信道工作为2.4GHz，收发必须一致 */
    SPI_WR_Reg ( WRITE_REG + RX_PW_P0, RX_PLOAD_WIDTH ); /* 设置接收数据长度，本次设置为32字节 */
    SPI_WR_Reg ( WRITE_REG + RF_SETUP, 0x0F ); /* 设置发射速率为2MHz，发射功率为最大值0dB */
    SPI_WR_Reg ( WRITE_REG + CONFIG, 0x0F );
    CE = 1;
}

uchar nRF24L01_RxPacket ( unsigned char *rx_buf ) { /* 数据读取后放入rx_buf接收缓冲区中 */
    uchar flag = 0;
    uchar status;
    status = SPI_RD_Reg ( NRFRegSTATUS ); /* 读取状态寄存其来判断数据接收状况 */

    if ( status & 0x40 ) { /* 判断是否接收到数据 */
        CE = 0; /* SPI使能 */
        SPI_Read_Buf ( RD_RX_PLOAD, rx_buf, TX_PLOAD_WIDTH ); /* read receive payload from RX_FIFO buffer */
        flag = 1; /* 读取数据完成标志 */
    }

    SPI_WR_Reg ( WRITE_REG + NRFRegSTATUS, status ); /* 接收到数据后，RX_DR、TX_DS、MAX_RT都置高为1，通过写1来清除中断标志 */
    return flag;
}

void TX_Mode ( void ) { /* 数据发送配置 */
    CE = 0;
    SPI_Write_Buf ( WRITE_REG + TX_ADDR, TX_ADDRESS, TX_ADR_WIDTH ); /* 写本地地址 */
    SPI_Write_Buf ( WRITE_REG + RX_ADDR_P0, RX_ADDRESS, RX_ADR_WIDTH ); /* 写接收端地址 */
    SPI_WR_Reg ( WRITE_REG + EN_AA, 0 ); /* 频道0自动ACK应答允许 */
    SPI_WR_Reg ( WRITE_REG + EN_RXADDR, 0 ); /* 允许接收地址只有频道0，如果需要多频道可以参考数据手册 */
    SPI_WR_Reg ( WRITE_REG + SETUP_RETR, 0 ); /* 设置自动重发时间和次数 */
    SPI_WR_Reg ( WRITE_REG + RF_CH, 40 ); /* 设置信道工作为2.4GHz，收发必须一致 */
    SPI_WR_Reg ( WRITE_REG + RX_PW_P0, RX_PLOAD_WIDTH ); /* 设置接收数据长度，本次设置为32字节 */
    SPI_WR_Reg ( WRITE_REG + RF_SETUP, 0x0F ); /* 设置发射速率为2MHz，发射功率为最大值0dB */
    SPI_WR_Reg ( WRITE_REG + CONFIG, 0x0E );
    CE = 1;
}

void nRF24L01_TxPacket ( unsigned char *tx_buf ) { /* 发送tx_buf中的数据 */
    CE = 0;
    SPI_Write_Buf ( WRITE_REG + RX_ADDR_P0, TX_ADDRESS, TX_ADR_WIDTH ); /* 装载接收端地址 */
    SPI_Write_Buf ( WR_TX_PLOAD, tx_buf, TX_PLOAD_WIDTH ); /* 装载数据 */
    CE = 1; /* 置高CE，数据发送 */
}