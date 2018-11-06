#ifndef __NRF24L01_H__
#define __NRF24L01_H__

#include "STC12C5A.H"

#define uchar unsigned char
#define uint  unsigned int

sbit MISO = P2 ^ 6; /* Master input slave output */
sbit MOSI = P2 ^ 5; /* Master output slave input */
sbit SCK  = P2 ^ 4; /* SPIʱ�� */
sbit CE   = P2 ^ 3; /* оƬ��ģʽ�����ߣ���CSNΪ�͵�����£�CEЭͬNRF24L01��CONFIG�Ĵ�����ͬ����NRF24L01��״̬ */
sbit CSN  = P2 ^ 2; /* оƬʹ�ܣ�Ϊ�͵�ƽʱоƬ���� */
sbit IRQ  = P2 ^ 1; /* �ж��źţ�����ͨ�Ź�����MCU��Ҫ��ͨ��IRQ��NRF24L01����ͨ�� */

#define TX_ADR_WIDTH   5  /* 5 uints TX address width */
#define RX_ADR_WIDTH   5  /* 5 uints RX address width */
#define TX_PLOAD_WIDTH 32 /* 32 uints TX payload */
#define RX_PLOAD_WIDTH 32 /* 32 uints TX payload */

/* NRF24L01�Ĵ���ָ�� */
#define READ_REG    0x00 /* ���Ĵ���ָ�� */
#define WRITE_REG   0x20 /* д�Ĵ���ָ�� */
#define RD_RX_PLOAD 0x61 /* ��ȡ��������ָ�� */
#define WR_TX_PLOAD 0xA0 /* д��������ָ�� */
#define FLUSH_TX    0xE1 /* ��ϴ����FIFOָ�� */
#define FLUSH_RX    0xE2 /* ��ϴ����FIFOָ�� */
#define REUSE_TX_PL 0xE3 /* �����ظ�װ������ָ�� */
#define NOP         0xFF /* �ղ��� */

/* SPI(nRF24L01)�Ĵ�����ַ */
#define CONFIG       0x00 /* �����շ�״̬��CRCУ��ģʽ�Լ��շ�״̬��Ӧ��ʽ*/
#define EN_AA        0x01 /* �Զ�Ӧ�������� */
#define EN_RXADDR    0x02 /* �����ŵ����� */
#define SETUP_AW     0x03 /* �շ���ַ������� */
#define SETUP_RETR   0x04 /* �Զ��ط��������� */
#define RF_CH        0x05 /* ����Ƶ������ */
#define RF_SETUP     0x06 /* �������ʡ����Ĺ������� */
#define NRFRegSTATUS 0x07 /* ״̬�Ĵ��� */
#define OBSERVE_TX   0x08 /* ���ͼ�⹦�� */
#define CD           0x09 /* ��ַ��� */
#define RX_ADDR_P0   0x0A /* Ƶ��0�������ݵ�ַ */
#define RX_ADDR_P1   0x0B /* Ƶ��1�������ݵ�ַ */
#define RX_ADDR_P2   0x0C /* Ƶ��2�������ݵ�ַ */
#define RX_ADDR_P3   0x0D /* Ƶ��3�������ݵ�ַ */
#define RX_ADDR_P4   0x0E /* Ƶ��4�������ݵ�ַ */
#define RX_ADDR_P5   0x0F /* Ƶ��5�������ݵ�ַ */
#define TX_ADDR      0x10 /* ���͵�ַ�Ĵ��� */
#define RX_PW_P0     0x11 /* ����Ƶ��0�������ݳ��� */
#define RX_PW_P1     0x12 /* ����Ƶ��1�������ݳ��� */
#define RX_PW_P2     0x13 /* ����Ƶ��2�������ݳ��� */
#define RX_PW_P3     0x14 /* ����Ƶ��3�������ݳ��� */
#define RX_PW_P4     0x15 /* ����Ƶ��4�������ݳ��� */
#define RX_PW_P5     0x16 /* ����Ƶ��5�������ݳ��� */
#define FIFO_STATUS  0x17 /* FIFOջ��ջ��״̬�Ĵ������� */

void init_NRF24L01 ( void );
void TX_Mode ( void );
void RX_Mode ( void );
uchar NRF24L01_Check ( void );
uchar nRF24L01_RxPacket ( unsigned char *rx_buf );
void nRF24L01_TxPacket ( unsigned char *tx_buf );

#endif /*__NRF24L01_H__ */