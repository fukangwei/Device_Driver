#ifndef __24L01_H
#define __24L01_H
#include "sys.h"

/* NRF24L01�Ĵ����������� */
#define NRF24L01_READ_REG    0x00 /* �����üĴ�������5λΪ�Ĵ�����ַ */
#define NRF24L01_WRITE_REG   0x20 /* д���üĴ�������5λΪ�Ĵ�����ַ */
#define NRF24L01_RD_RX_PLOAD 0x61 /* ��RX��Ч���ݣ�1��32�ֽ� */
#define NRF24L01_WR_TX_PLOAD 0xA0 /* дTX��Ч���ݣ�1��32�ֽ� */
#define NRF24L01_FLUSH_TX    0xE1 /* �����TX FIFO���Ĵ���������ģʽ���� */
#define NRF24L01_FLUSH_RX    0xE2 /* �����RX FIFO���Ĵ���������ģʽ���� */
#define NRF24L01_REUSE_TX_PL 0xE3 /* ����ʹ����һ�����ݣ���CEΪ��ʱ�����ݰ������Ϸ��� */
#define NRF24L01_NOP         0xFF /* �ղ���������������״̬�Ĵ��� */

/* SPI(NRF24L01)�Ĵ�����ַ */
#define CONFIG      0x00 /* ���üĴ�����ַ */
#define EN_AA       0x01 /* ʹ���Զ�Ӧ���� */
#define EN_RXADDR   0x02 /* ���յ�ַ���� */
#define SETUP_AW    0x03 /* ���õ�ַ���(��������ͨ��) */
#define SETUP_RETR  0x04 /* �����Զ��ط� */
#define RF_CH       0x05 /* RFͨ�� */
#define RF_SETUP    0x06 /* RF�Ĵ��� */
#define STATUS      0x07 /* ״̬�Ĵ��� */
#define MAX_TX      0x10 /* �ﵽ����ʹ����ж� */
#define TX_OK       0x20 /* TX��������ж� */
#define RX_OK       0x40 /* ���յ������ж� */
#define OBSERVE_TX  0x08 /* ���ͼ��Ĵ��� */
#define CD          0x09 /* �ز����Ĵ��� */
#define RX_ADDR_P0  0x0A /* ����ͨ��0���յ�ַ */
#define RX_ADDR_P1  0x0B /* ����ͨ��1���յ�ַ */
#define RX_ADDR_P2  0x0C /* ����ͨ��2���յ�ַ */
#define RX_ADDR_P3  0x0D /* ����ͨ��3���յ�ַ */
#define RX_ADDR_P4  0x0E /* ����ͨ��4���յ�ַ */
#define RX_ADDR_P5  0x0F /* ����ͨ��5���յ�ַ */
#define TX_ADDR     0x10 /* ���͵�ַ(���ֽ���ǰ)����ShockBurstTMģʽ�£�RX_ADDR_P0��˵�ַ��� */
#define RX_PW_P0    0x11 /* ��������ͨ��0��Ч���ݿ�� */
#define RX_PW_P1    0x12 /* ��������ͨ��1��Ч���ݿ�� */
#define RX_PW_P2    0x13 /* ��������ͨ��2��Ч���ݿ�� */
#define RX_PW_P3    0x14 /* ��������ͨ��3��Ч���ݿ�� */
#define RX_PW_P4    0x15 /* ��������ͨ��4��Ч���ݿ�� */
#define RX_PW_P5    0x16 /* ��������ͨ��5��Ч���ݿ�� */
#define FIFO_STATUS 0x17 /* FIFO״̬�Ĵ��� */

/* 24L01���ͽ������ݿ�ȶ��� */
#define TX_ADR_WIDTH    5  /* 5�ֽڵĵ�ַ��� */
#define RX_ADR_WIDTH    5  /* 5�ֽڵĵ�ַ��� */
#define TX_PLOAD_WIDTH  32 /* 20�ֽڵ��û����ݿ�� */
#define RX_PLOAD_WIDTH  32 /* 20�ֽڵ��û����ݿ�� */

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