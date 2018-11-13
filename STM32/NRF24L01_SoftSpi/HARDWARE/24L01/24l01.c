#include "24l01.h"
#include "delay.h"

const u8 TX_ADDRESS[TX_ADR_WIDTH] = {0xE7, 0xE7, 0xE7, 0xE7, 0xE7}; /* ���ص�ַ */
const u8 RX_ADDRESS[RX_ADR_WIDTH] = {0xE7, 0xE7, 0xE7, 0xE7, 0xE7}; /* ���յ�ַ */

#define MOSI_H GPIO_SetBits(GPIOA,   GPIO_Pin_0)
#define MOSI_L GPIO_ResetBits(GPIOA, GPIO_Pin_0)
#define SCLK_H GPIO_SetBits(GPIOA,   GPIO_Pin_1)
#define SCLK_L GPIO_ResetBits(GPIOA, GPIO_Pin_1)
#define MISO   GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_2)

#define NRF24L01_IRQ     (GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_3))
#define Set_NRF24L01_CE  {GPIO_SetBits(GPIOA, GPIO_Pin_4);}
#define Clr_NRF24L01_CE  {GPIO_ResetBits(GPIOA, GPIO_Pin_4);}
#define Set_NRF24L01_CSN {GPIO_SetBits(GPIOA, GPIO_Pin_5);}
#define Clr_NRF24L01_CSN {GPIO_ResetBits(GPIOA, GPIO_Pin_5);}

u8 SPI_ReadWriteByte ( u8 dt ) {
    u8 i;
    u8 temp = 0;

    for ( i = 8; i > 0; i-- ) {
        if ( dt & 0x80 ) {
            MOSI_H;
        } else {
            MOSI_L;
        }

        dt <<= 1;
        SCLK_H;
        temp <<= 1;

        if ( MISO ) {
            temp++;
        }

        SCLK_L;
    }

    return temp;
}

void NRF24L01_Init ( void ) {
    GPIO_InitTypeDef GPIO_InitStructure;
    RCC_APB2PeriphClockCmd ( RCC_APB2Periph_GPIOA, ENABLE );
    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_4 | GPIO_Pin_5;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init ( GPIOA, &GPIO_InitStructure );
    GPIO_SetBits ( GPIOA, GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_4 | GPIO_Pin_5 );
    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_2 | GPIO_Pin_3;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IPU;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init ( GPIOA, &GPIO_InitStructure );
    Clr_NRF24L01_CE;
    Set_NRF24L01_CSN;
}

u8 NRF24L01_Check ( void ) {
    u8 buf[5] = {0XA5, 0XA5, 0XA5, 0XA5, 0XA5};
    u8 i;
    NRF24L01_Write_Buf ( NRF24L01_WRITE_REG + TX_ADDR, buf, 5 ); /* д��5���ֽڵĵ�ַ */
    NRF24L01_Read_Buf ( TX_ADDR, buf, 5 ); /* ����д��ĵ�ַ */

    for ( i = 0; i < 5; i++ )
        if ( buf[i] != 0XA5 ) {
            break;
        }

    if ( i != 5 ) {
        return 1; /* ���24L01���� */
    }

    return 0; /* ��⵽24L01 */
}

u8 NRF24L01_Write_Reg ( u8 reg, u8 value ) {
    u8 status;
    Clr_NRF24L01_CSN; /* ʹ��SPI���� */
    status = SPI_ReadWriteByte ( reg ); /* ���ͼĴ����� */
    SPI_ReadWriteByte ( value ); /* д��Ĵ�����ֵ */
    Set_NRF24L01_CSN; /* ��ֹSPI���� */
    return ( status ); /* ����״ֵ̬ */
}

u8 NRF24L01_Read_Reg ( u8 reg ) {
    u8 reg_val;
    Clr_NRF24L01_CSN; /* ʹ��SPI���� */
    SPI_ReadWriteByte ( reg ); /* ���ͼĴ����� */
    reg_val = SPI_ReadWriteByte ( 0XFF ); /* ��ȡ�Ĵ������� */
    Set_NRF24L01_CSN; /* ��ֹSPI���� */
    return ( reg_val ); /* ����״ֵ̬ */
}

u8 NRF24L01_Read_Buf ( u8 reg, u8 *pBuf, u8 len ) {
    u8 status, u8_ctr;
    Clr_NRF24L01_CSN; /* ʹ��SPI���� */
    status = SPI_ReadWriteByte ( reg ); /* ���ͼĴ���ֵ(λ��)������ȡ״ֵ̬ */

    for ( u8_ctr = 0; u8_ctr < len; u8_ctr++ ) {
        pBuf[u8_ctr] = SPI_ReadWriteByte ( 0XFF ); /* �������� */
    }

    Set_NRF24L01_CSN; /* �ر�SPI���� */
    return status; /* ���ض�����״ֵ̬ */
}

u8 NRF24L01_Write_Buf ( u8 reg, u8 *pBuf, u8 len ) {
    u8 status, u8_ctr;
    Clr_NRF24L01_CSN; /* ʹ��SPI���� */
    status = SPI_ReadWriteByte ( reg ); /* ���ͼĴ���ֵ(λ��)������ȡ״ֵ̬ */

    for ( u8_ctr = 0; u8_ctr < len; u8_ctr++ ) {
        SPI_ReadWriteByte ( *pBuf++ ); /* д������ */
    }

    Set_NRF24L01_CSN; /* �ر�SPI���� */
    return status; /* ���ض�����״ֵ̬ */
}

u8 NRF24L01_TxPacket ( u8 *txbuf ) {
    u8 sta;
    delay_ms ( 50 );
    Clr_NRF24L01_CE;
    delay_ms ( 50 );
    NRF24L01_Write_Buf ( NRF24L01_WR_TX_PLOAD, txbuf, TX_PLOAD_WIDTH ); /*д���ݵ���TX BUF��(32���ֽ�) */
    Set_NRF24L01_CE; /* �������� */

    while ( NRF24L01_IRQ != 0 ); /* �ȴ�������� */

    sta = NRF24L01_Read_Reg ( STATUS ); /* ��ȡ״̬�Ĵ�����ֵ */
    delay_ms ( 50 );
    NRF24L01_Write_Reg ( NRF24L01_WRITE_REG + STATUS, sta ); /* ���TX_DS��MAX_RT�жϱ�־ */

    if ( sta & MAX_TX ) { /* �ﵽ����ط����� */
        NRF24L01_Write_Reg ( NRF24L01_FLUSH_TX, 0xff ); /* �����TX FIFO���Ĵ��� */
        return MAX_TX;
    }

    if ( sta & TX_OK ) { /* ������� */
        return TX_OK;
    }

    return 0xff; /* ����ԭ����ʧ�� */
}

u8 NRF24L01_RxPacket ( u8 *rxbuf ) { /* ����NRF24L01����һ������ */
    u8 sta;
    sta = NRF24L01_Read_Reg ( STATUS ); /* ��ȡ״̬�Ĵ�����ֵ */
    NRF24L01_Write_Reg ( NRF24L01_WRITE_REG + STATUS, sta ); /* ���TX_DS��MAX_RT�жϱ�־ */

    if ( sta & RX_OK ) { /* ���յ����� */
        NRF24L01_Read_Buf ( NRF24L01_RD_RX_PLOAD, rxbuf, RX_PLOAD_WIDTH ); /* ��ȡ���� */
        NRF24L01_Write_Reg ( NRF24L01_FLUSH_RX, 0xff ); /* �����RX FIFO���Ĵ��� */
        return 0;
    }

    return 1; /* û�յ��κ����� */
}

void RX_Mode ( void ) { /* ��ʼ��NRF24L01��RXģʽ */
    Clr_NRF24L01_CE;
    NRF24L01_Write_Buf ( NRF24L01_WRITE_REG + RX_ADDR_P0, ( u8 * ) RX_ADDRESS, RX_ADR_WIDTH ); /* дRX�ڵ��ַ */
    NRF24L01_Write_Reg ( NRF24L01_WRITE_REG + EN_AA, 0x00 ); /* ʹ��ͨ��0���Զ�Ӧ�� */
    NRF24L01_Write_Reg ( NRF24L01_WRITE_REG + EN_RXADDR, 0x01 ); /* ʹ��ͨ��0�Ľ��յ�ַ */
    NRF24L01_Write_Reg ( NRF24L01_WRITE_REG + RF_CH, 40 ); /* ����RFͨ��Ƶ�� */
    NRF24L01_Write_Reg ( NRF24L01_WRITE_REG + RX_PW_P0, RX_PLOAD_WIDTH ); /* ѡ��ͨ��0����Ч���ݿ�� */
    NRF24L01_Write_Reg ( NRF24L01_WRITE_REG + RF_SETUP, 0x0f ); /* ����TX���������0db���棬2Mbps�����������濪�� */
    NRF24L01_Write_Reg ( NRF24L01_WRITE_REG + CONFIG, 0x0f ); /* ���û�������ģʽ�Ĳ�����PWR_UP��EN_CRC��16BIT_CRC�ͽ���ģʽ */
    Set_NRF24L01_CE; /* CEΪ��ʱ���������ģʽ */
}

void TX_Mode ( void ) { /* ��ʼ��NRF24L01��TXģʽ */
    Clr_NRF24L01_CE;
    NRF24L01_Write_Buf ( NRF24L01_WRITE_REG + TX_ADDR, ( u8 * ) TX_ADDRESS, TX_ADR_WIDTH ); /* дTX�ڵ��ַ */
    NRF24L01_Write_Buf ( NRF24L01_WRITE_REG + RX_ADDR_P0, ( u8 * ) RX_ADDRESS, RX_ADR_WIDTH ); /* ����TX�ڵ��ַ����ҪΪ��ʹ��ACK */
    NRF24L01_Write_Reg ( NRF24L01_WRITE_REG + EN_AA, 0x00 ); /* ʹ��ͨ��0���Զ�Ӧ�� */
    NRF24L01_Write_Reg ( NRF24L01_WRITE_REG + EN_RXADDR, 0x00 ); /* ʹ��ͨ��0�Ľ��յ�ַ */
    NRF24L01_Write_Reg ( NRF24L01_WRITE_REG + SETUP_RETR, 0x00 ); /* �����Զ��ط����ʱ�� */
    NRF24L01_Write_Reg ( NRF24L01_WRITE_REG + RF_CH, 40 ); /* ����RFͨ��Ϊ40 */
    NRF24L01_Write_Reg ( NRF24L01_WRITE_REG + RF_SETUP, 0x0f ); /* ����TX���������0db���棬2Mbps�����������濪�� */
    NRF24L01_Write_Reg ( NRF24L01_WRITE_REG + CONFIG, 0x0e ); /* ���û�������ģʽ�Ĳ�����PWR_UP��EN_CRC��16BIT_CRC������ģʽ�Ϳ��������ж� */
    Set_NRF24L01_CE; /* CEΪ��ʱ��10us���������� */
}