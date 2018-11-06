#ifndef __NRF24L01_H__
#define __NRF24L01_H__

#include "STC12C5A.H"

#define uchar unsigned char
#define uint  unsigned int

sbit MISO = P2 ^ 6; /* Master input slave output */
sbit MOSI = P2 ^ 5; /* Master output slave input */
sbit SCK  = P2 ^ 4; /* SPI时钟 */
sbit CE   = P2 ^ 3; /* 芯片的模式控制线，在CSN为低的情况下，CE协同NRF24L01的CONFIG寄存器共同决定NRF24L01的状态 */
sbit CSN  = P2 ^ 2; /* 芯片使能，为低电平时芯片工作 */
sbit IRQ  = P2 ^ 1; /* 中断信号，无线通信过程中MCU主要是通过IRQ与NRF24L01进行通信 */

#define TX_ADR_WIDTH   5  /* 5 uints TX address width */
#define RX_ADR_WIDTH   5  /* 5 uints RX address width */
#define TX_PLOAD_WIDTH 32 /* 32 uints TX payload */
#define RX_PLOAD_WIDTH 32 /* 32 uints TX payload */

/* NRF24L01寄存器指令 */
#define READ_REG    0x00 /* 读寄存器指令 */
#define WRITE_REG   0x20 /* 写寄存器指令 */
#define RD_RX_PLOAD 0x61 /* 读取接收数据指令 */
#define WR_TX_PLOAD 0xA0 /* 写待发数据指令 */
#define FLUSH_TX    0xE1 /* 冲洗发送FIFO指令 */
#define FLUSH_RX    0xE2 /* 冲洗接收FIFO指令 */
#define REUSE_TX_PL 0xE3 /* 定义重复装载数据指令 */
#define NOP         0xFF /* 空操作 */

/* SPI(nRF24L01)寄存器地址 */
#define CONFIG       0x00 /* 配置收发状态、CRC校验模式以及收发状态响应方式*/
#define EN_AA        0x01 /* 自动应答功能设置 */
#define EN_RXADDR    0x02 /* 可用信道设置 */
#define SETUP_AW     0x03 /* 收发地址宽度设置 */
#define SETUP_RETR   0x04 /* 自动重发功能设置 */
#define RF_CH        0x05 /* 工作频率设置 */
#define RF_SETUP     0x06 /* 发射速率、功耗功能设置 */
#define NRFRegSTATUS 0x07 /* 状态寄存器 */
#define OBSERVE_TX   0x08 /* 发送监测功能 */
#define CD           0x09 /* 地址检测 */
#define RX_ADDR_P0   0x0A /* 频道0接收数据地址 */
#define RX_ADDR_P1   0x0B /* 频道1接收数据地址 */
#define RX_ADDR_P2   0x0C /* 频道2接收数据地址 */
#define RX_ADDR_P3   0x0D /* 频道3接收数据地址 */
#define RX_ADDR_P4   0x0E /* 频道4接收数据地址 */
#define RX_ADDR_P5   0x0F /* 频道5接收数据地址 */
#define TX_ADDR      0x10 /* 发送地址寄存器 */
#define RX_PW_P0     0x11 /* 接收频道0接收数据长度 */
#define RX_PW_P1     0x12 /* 接收频道1接收数据长度 */
#define RX_PW_P2     0x13 /* 接收频道2接收数据长度 */
#define RX_PW_P3     0x14 /* 接收频道3接收数据长度 */
#define RX_PW_P4     0x15 /* 接收频道4接收数据长度 */
#define RX_PW_P5     0x16 /* 接收频道5接收数据长度 */
#define FIFO_STATUS  0x17 /* FIFO栈入栈出状态寄存器设置 */

void init_NRF24L01 ( void );
void TX_Mode ( void );
void RX_Mode ( void );
uchar NRF24L01_Check ( void );
uchar nRF24L01_RxPacket ( unsigned char *rx_buf );
void nRF24L01_TxPacket ( unsigned char *tx_buf );

#endif /*__NRF24L01_H__ */