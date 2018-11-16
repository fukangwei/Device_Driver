#include "24l01.h"
#include "lcd.h"
#include "delay.h"
#include "spi.h"

const u8 TX_ADDRESS[TX_ADR_WIDTH] = {0xE7, 0xE7, 0xE7, 0xE7, 0xE7}; /* ���ص�ַ */
const u8 RX_ADDRESS[RX_ADR_WIDTH] = {0xE7, 0xE7, 0xE7, 0xE7, 0xE7}; /* ���յ�ַ */

void NRF24L01_Init ( void ) { /* ��ʼ��24L01��IO�� */
    GPIO_InitTypeDef GPIO_InitStructure;
    RCC_APB2PeriphClockCmd ( RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOC, ENABLE );
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_4;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init ( GPIOA, &GPIO_InitStructure );
    GPIO_SetBits ( GPIOA, GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_4 );
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init ( GPIOC, &GPIO_InitStructure );
    GPIO_SetBits ( GPIOC, GPIO_Pin_4 );
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init ( GPIOC, &GPIO_InitStructure );
    SPIx_Init(); /* ��ʼ��SPI */
    Clr_NRF24L01_CE; /* ʹ��24L01 */
    Set_NRF24L01_CSN; /* SPIƬѡȡ�� */
}

u8 NRF24L01_Check ( void ) { /* ���24L01�Ƿ���ڣ�����0Ϊ���ڣ�1Ϊ������ */
    u8 buf[5] = {0XA5, 0XA5, 0XA5, 0XA5, 0XA5};
    u8 i;
    SPIx_SetSpeed ( SPI_BaudRatePrescaler_8 ); /* spi�ٶ�Ϊ9Mhz(24L01�����SPIʱ��Ϊ10Mhz) */
    NRF24L01_Write_Buf ( NRF24L01_WRITE_REG + TX_ADDR, buf, 5 );
    NRF24L01_Read_Buf ( TX_ADDR, buf, 5 );

    for ( i = 0; i < 5; i++ )
        if ( buf[i] != 0XA5 ) {
            break;
        }

    if ( i != 5 ) {
        return 1;
    }

    return 0;
}

u8 NRF24L01_Write_Reg ( u8 reg, u8 value ) { /* SPIд�Ĵ���������reg��ָ���Ĵ�����ַ��value��д���ֵ */
    u8 status;
    Clr_NRF24L01_CSN; /* ʹ��SPI���� */
    status = SPIx_ReadWriteByte ( reg ); /* ���ͼĴ����� */
    SPIx_ReadWriteByte ( value ); /* д��Ĵ�����ֵ */
    Set_NRF24L01_CSN; /* ��ֹSPI���� */
    return ( status ); /* ����״ֵ̬ */
}

u8 NRF24L01_Read_Reg ( u8 reg ) { /* ��ȡSPI�Ĵ���ֵ������reg��Ҫ���ļĴ��� */
    u8 reg_val;
    Clr_NRF24L01_CSN; /* ʹ��SPI���� */
    SPIx_ReadWriteByte ( reg ); /* ���ͼĴ����� */
    reg_val = SPIx_ReadWriteByte ( 0XFF ); /* ��ȡ�Ĵ������� */
    Set_NRF24L01_CSN; /* ��ֹSPI���� */
    return ( reg_val ); /* ����״ֵ̬ */
}

u8 NRF24L01_Read_Buf ( u8 reg, u8 *pBuf, u8 len ) { /* ��ָ��λ�ö���ָ�����ȵ����ݡ�����reg�ǼĴ�����pBuf������ָ�룬len�����ݳ��� */
    u8 status, u8_ctr;
    Clr_NRF24L01_CSN; /* ʹ��SPI���� */
    status = SPIx_ReadWriteByte ( reg ); /* ���ͼĴ���ֵλ�ã�����ȡ״ֵ̬ */

    for ( u8_ctr = 0; u8_ctr < len; u8_ctr++ ) {
        pBuf[u8_ctr] = SPIx_ReadWriteByte ( 0XFF ); /* �������� */
    }

    Set_NRF24L01_CSN; /* �ر�SPI���� */
    return status; /* ���ض�����״ֵ̬ */
}

u8 NRF24L01_Write_Buf ( u8 reg, u8 *pBuf, u8 len ) { /* ��ָ��λ��дָ�����ȵ����ݡ�����reg�ǼĴ�����pBuf������ָ�룬len�����ݳ��� */
    u8 status, u8_ctr;
    Clr_NRF24L01_CSN; /* ʹ��SPI���� */
    status = SPIx_ReadWriteByte ( reg ); /* ���ͼĴ���ֵλ�ã�����ȡ״ֵ̬ */

    for ( u8_ctr = 0; u8_ctr < len; u8_ctr++ ) {
        SPIx_ReadWriteByte ( *pBuf++ ); /* д������ */
    }

    Set_NRF24L01_CSN; /* �ر�SPI���� */
    return status; /* ���ض�����״ֵ̬ */
}

u8 NRF24L01_TxPacket ( u8 *txbuf ) { /* ����NRF24L01����һ�����ݣ�����txbuf�Ǵ����������׵�ַ */
    u8 sta;
    SPIx_SetSpeed ( SPI_BaudRatePrescaler_8 ); /* spi�ٶ�Ϊ9Mhz(24L01�����SPIʱ��Ϊ10Mhz) */
    Clr_NRF24L01_CE;
    NRF24L01_Write_Buf ( NRF24L01_WR_TX_PLOAD, txbuf, TX_PLOAD_WIDTH ); /* д���ݵ���TX BUF��(32�ֽ�) */
    Set_NRF24L01_CE; /* �������� */

    while ( NRF24L01_IRQ != 0 ); /* �ȴ�������� */

    sta = NRF24L01_Read_Reg ( STATUS ); /* ��ȡ״̬�Ĵ�����ֵ */
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

u8 NRF24L01_RxPacket ( u8 *rxbuf ) { /* ����NRF24L01����һ�����ݣ�����txbuf�ǽ������ݻ������׵�ַ */
    u8 sta;
    SPIx_SetSpeed ( SPI_BaudRatePrescaler_8 ); /* spi�ٶ�Ϊ9Mhz(24L01�����SPIʱ��Ϊ10Mhz) */
    sta = NRF24L01_Read_Reg ( STATUS ); /* ��ȡ״̬�Ĵ�����ֵ */
    NRF24L01_Write_Reg ( NRF24L01_WRITE_REG + STATUS, sta ); /* ���TX_DS��MAX_RT�жϱ�־ */

    if ( sta & RX_OK ) { /* ���յ����� */
        NRF24L01_Read_Buf ( NRF24L01_RD_RX_PLOAD, rxbuf, RX_PLOAD_WIDTH ); /* ��ȡ���� */
        NRF24L01_Write_Reg ( NRF24L01_FLUSH_RX, 0xff ); /* �����RX FIFO���Ĵ��� */
        return 0;
    }

    return 1; /* û�յ��κ����� */
}

/* ��ʼ��NRF24L01��RXģʽ������RX��ַ��дRX���ݿ�ȣ�ѡ��RFƵ���������ʺ͡�LNA HCURR������CE��ߺ󣬼�����RXģʽ�������Խ��������� */
void RX_Mode ( void ) {
    Clr_NRF24L01_CE;
    NRF24L01_Write_Buf ( NRF24L01_WRITE_REG + RX_ADDR_P0, ( u8 * ) RX_ADDRESS, RX_ADR_WIDTH ); /* дRX�ڵ��ַ */
    NRF24L01_Write_Reg ( NRF24L01_WRITE_REG + EN_AA, 0x00 ); /* ʹ��ͨ��0���Զ�Ӧ�� */
    NRF24L01_Write_Reg ( NRF24L01_WRITE_REG + EN_RXADDR, 0x01 ); /* ʹ��ͨ��0�Ľ��յ�ַ */
    NRF24L01_Write_Reg ( NRF24L01_WRITE_REG + RF_CH, 40 ); /* ����RFͨ��Ƶ�� */
    NRF24L01_Write_Reg ( NRF24L01_WRITE_REG + RX_PW_P0, RX_PLOAD_WIDTH ); /* ѡ��ͨ��0����Ч���ݿ�� */
    NRF24L01_Write_Reg ( NRF24L01_WRITE_REG + RF_SETUP, 0x0f ); /* ����TX���������0db���棬2Mbps�����������濪�� */
    NRF24L01_Write_Reg ( NRF24L01_WRITE_REG + CONFIG, 0x0f ); /* ���û�������ģʽ�Ĳ�����PWR_UP��EN_CRC��16BIT_CRC������ģʽ */
    Set_NRF24L01_CE; /* CEΪ��ʱ���������ģʽ */
}

/* ��ʼ��NRF24L01��TXģʽ������TX��ַ��дTX���ݿ�ȣ�����RX�Զ�Ӧ��ĵ�ַ�����TX�������ݣ�ѡ��RFƵ���������ʺ͡�LNA HCURR����PWR_UP��CRCʹ�ܡ�
   ��CE��ߺ󣬼�����TXģʽ�������Է��������ˡ�CEΪ�ߴ���10us������������ */
void TX_Mode ( void ) {
    Clr_NRF24L01_CE;
    NRF24L01_Write_Buf ( NRF24L01_WRITE_REG + TX_ADDR, ( u8 * ) TX_ADDRESS, TX_ADR_WIDTH ); /* дTX�ڵ��ַ */
    NRF24L01_Write_Buf ( NRF24L01_WRITE_REG + RX_ADDR_P0, ( u8 * ) RX_ADDRESS, RX_ADR_WIDTH ); /* ����TX�ڵ��ַ����ҪΪ��ʹ��ACK */
    NRF24L01_Write_Reg ( NRF24L01_WRITE_REG + EN_AA, 0x00 ); /* ʹ��ͨ��0���Զ�Ӧ�� */
    NRF24L01_Write_Reg ( NRF24L01_WRITE_REG + EN_RXADDR, 0x00 ); /* ʹ��ͨ��0�Ľ��յ�ַ */
    NRF24L01_Write_Reg ( NRF24L01_WRITE_REG + SETUP_RETR, 0x00 ); /* �����Զ��ط����ʱ�䡰500us + 86us��������Զ��ط�����10�� */
    NRF24L01_Write_Reg ( NRF24L01_WRITE_REG + RF_CH, 40 ); /* ����RFͨ��Ϊ40 */
    NRF24L01_Write_Reg ( NRF24L01_WRITE_REG + RF_SETUP, 0x0f ); /* ����TX���������0db����,��Mbps�����������濪�� */
    NRF24L01_Write_Reg ( NRF24L01_WRITE_REG + CONFIG, 0x0e ); /* ���û�������ģʽ�Ĳ�����PWR_UP��EN_CRC��16BIT_CRC������ģʽ�����������ж� */
    Set_NRF24L01_CE; /* CEΪ��ʱ��10us���������� */
}