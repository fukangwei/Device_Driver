#ifndef __24L01_H
#define __24L01_H
#include "sys.h"

/* NRF24L01寄存器操作命令 */
#define NRF24L01_READ_REG    0x00 /* 读配置寄存器，低5位为寄存器地址 */
#define NRF24L01_WRITE_REG   0x20 /* 写配置寄存器，低5位为寄存器地址 */
#define NRF24L01_RD_RX_PLOAD 0x61 /* 读RX有效数据，1至32字节 */
#define NRF24L01_WR_TX_PLOAD 0xA0 /* 写TX有效数据，1至32字节 */
#define NRF24L01_FLUSH_TX    0xE1 /* 清除“TX FIFO”寄存器，发射模式下用 */
#define NRF24L01_FLUSH_RX    0xE2 /* 清除“RX FIFO”寄存器，接收模式下用 */
#define NRF24L01_REUSE_TX_PL 0xE3 /* 重新使用上一包数据，当CE为高时，数据包被不断发送 */
#define NRF24L01_NOP         0xFF /* 空操作，可以用来读状态寄存器 */

/* SPI(NRF24L01)寄存器地址 */
#define CONFIG      0x00 /* 配置寄存器地址 */
#define EN_AA       0x01 /* 使能自动应答功能 */
#define EN_RXADDR   0x02 /* 接收地址允许 */
#define SETUP_AW    0x03 /* 设置地址宽度(所有数据通道) */
#define SETUP_RETR  0x04 /* 建立自动重发 */
#define RF_CH       0x05 /* RF通道 */
#define RF_SETUP    0x06 /* RF寄存器 */
#define STATUS      0x07 /* 状态寄存器 */
#define MAX_TX      0x10 /* 达到最大发送次数中断 */
#define TX_OK       0x20 /* TX发送完成中断 */
#define RX_OK       0x40 /* 接收到数据中断 */
#define OBSERVE_TX  0x08 /* 发送检测寄存器 */
#define CD          0x09 /* 载波检测寄存器 */
#define RX_ADDR_P0  0x0A /* 数据通道0接收地址 */
#define RX_ADDR_P1  0x0B /* 数据通道1接收地址 */
#define RX_ADDR_P2  0x0C /* 数据通道2接收地址 */
#define RX_ADDR_P3  0x0D /* 数据通道3接收地址 */
#define RX_ADDR_P4  0x0E /* 数据通道4接收地址 */
#define RX_ADDR_P5  0x0F /* 数据通道5接收地址 */
#define TX_ADDR     0x10 /* 发送地址(低字节在前)，在ShockBurstTM模式下，RX_ADDR_P0与此地址相等 */
#define RX_PW_P0    0x11 /* 接收数据通道0有效数据宽度 */
#define RX_PW_P1    0x12 /* 接收数据通道1有效数据宽度 */
#define RX_PW_P2    0x13 /* 接收数据通道2有效数据宽度 */
#define RX_PW_P3    0x14 /* 接收数据通道3有效数据宽度 */
#define RX_PW_P4    0x15 /* 接收数据通道4有效数据宽度 */
#define RX_PW_P5    0x16 /* 接收数据通道5有效数据宽度 */
#define FIFO_STATUS 0x17 /* FIFO状态寄存器 */

/* 24L01发送接收数据宽度定义 */
#define TX_ADR_WIDTH    5  /* 5字节的地址宽度 */
#define RX_ADR_WIDTH    5  /* 5字节的地址宽度 */
#define TX_PLOAD_WIDTH  32 /* 20字节的用户数据宽度 */
#define RX_PLOAD_WIDTH  32 /* 20字节的用户数据宽度 */

void NRF24L01_SPI_Init ( void );
void NRF24L01_Init ( void );
void RX_Mode ( void );
void TX_Mode ( void );
u8 NRF24L01_Write_Buf ( u8 reg, u8 *pBuf, u8 u8s );
u8 NRF24L01_Read_Buf ( u8 reg, u8 *pBuf, u8 u8s );
u8 NRF24L01_Read_Reg ( u8 reg );
u8 NRF24L01_Write_Reg ( u8 reg, u8 value );
u8 NRF24L01_Check ( void );
u8 NRF24L01_TxPacket ( u8 *txbuf );
u8 NRF24L01_RxPacket ( u8 *rxbuf );

#endif