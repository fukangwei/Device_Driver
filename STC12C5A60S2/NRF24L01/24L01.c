#include "24L01.h"

static uchar TX_ADDRESS[TX_ADR_WIDTH] = {0xE7, 0xE7, 0xE7, 0xE7, 0xE7}; /* ���ص�ַ */
static uchar RX_ADDRESS[RX_ADR_WIDTH] = {0xE7, 0xE7, 0xE7, 0xE7, 0xE7}; /* ���յ�ַ */

/* GPIOģ��SPI�Ĺ��ܣ�������ֽ�(MOSI)��MSBѭ�������ͬʱ�������ֽ�(MISO)��LSBѭ�����롣�����ض��룬�½������ */
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
 * ���仺�������ʺ�������Ҫ����������������ŵ�����FIFO�������С�����˼·����ͨ��WRITE_REG��������ݴ浽����FIFO(WR_TX_PLOAD)��ȥ
 * ����regΪ�Ĵ�����ַ��pBufΪ��д�����ݵ�ַ��ucharsΪд�����ݵĸ���
 --------------------------------------------------------------------------------------------------------------------*/
static uchar SPI_Write_Buf ( uchar reg, uchar *pBuf, uchar Len ) {
    uint status, i;
    CSN = 0; /* Set CSN low, init SPI tranaction(��CSNΪ�ͣ�ʹ��оƬ������оƬ��������) */
    status = NRF24SPI_Send_Byte ( reg ); /* Select register to write to and read status byte */

    for ( i = 0; i < Len; i++ ) {
        NRF24SPI_Send_Byte ( *pBuf ); /* then write all byte in buffer(*pBuf) */
        pBuf ++;
    }

    CSN = 1; /* Set CSN high again */
    return ( status ); /* return nRF24L01 status byte */
}

/*-------------------------------------------------------------------------------------------------------------------------------
* ���ջ��������ʺ�������Ҫ�����ڽ���ʱ��ȡFIFO�������е�ֵ������˼·����ͨ��READ_REG��������ݴӽ���FIFO(RD_RX_PLOAD)�ж������浽��������ȥ
* ����regΪ�Ĵ�����ַ��pBufΪ���������ݵ�ַ��ucharsΪ�������ݵĸ���
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

uchar NRF24L01_Check ( void ) { /* ���NRF24L01�Ƿ��������� */
    uchar bu[5] = {0XA5, 0XA5, 0XA5, 0XA5, 0XA5};
    uchar bu1[5];
    uchar i;
    SPI_Write_Buf ( WRITE_REG + TX_ADDR, bu, 5 ); /* д��5���ֽڵĵ�ַ */
    SPI_Read_Buf ( TX_ADDR, bu1, 5 ); /* ����д��ĵ�ַ */

    for ( i = 0; i < 5; i++ )
        if ( bu1[i] != 0XA5 ) {
            break;
        }

    if ( i != 5 ) {
        return 1; /* NRF24L01������ */
    }

    return 0; /* NRF24L01���� */
}

void init_NRF24L01 ( void ) { /* NRF24L01��ʼ�� */
    uchar buf[5] = {0};
    CE = 0;
    CSN = 0;
    SPI_Read_Buf ( TX_ADDR, buf, TX_ADR_WIDTH );
}

/* ��ȡ�Ĵ���ֵ������˼·����ͨ��READ_REG����(Ҳ����0x00+�Ĵ�����ַ)���ѼĴ����е�ֵ�����������ں�����˵�����ǰ�reg�Ĵ�����ֵ����reg_val��ȥ */
static uchar SPI_RD_Reg ( uchar reg ) {
    uchar reg_val;
    CSN = 0; /* CSN low, initialize SPI communication */
    NRF24SPI_Send_Byte ( reg ); /* Select register to read from */
    reg_val = NRF24SPI_Send_Byte ( 0 ); /* then read register value */
    CSN = 1; /* CSN high, terminate SPI communication */
    return ( reg_val ); /* return register value */
}

/* ---------------------------------------------------------------------------------------
 * �Ĵ������ʺ�������������24L01�ļĴ�����ֵ������˼·����ͨ��WRITE_REG����(Ҳ����0x20 + �Ĵ�����ַ)
   ��Ҫ�趨��ֵд����Ӧ�ļĴ�����ַ����ȥ������ȡ����ֵ�����ں�����˵��Ҳ���ǰ�valueֵд��reg�Ĵ�����
 * ע�⣬����NRF24L01֮ǰ����ҪenableоƬ(CSN = 0)�����������Ժ���disableоƬ(CSN = 1)
----------------------------------------------------------------------------------------*/
static uchar SPI_WR_Reg ( uchar reg, uchar value ) {
    uchar status;
    CSN = 0; /* CSN low, init SPI transaction */
    status = NRF24SPI_Send_Byte ( reg ); /* select register */
    NRF24SPI_Send_Byte ( value ); /* and write value to it */
    CSN = 1; /* CSN high again */
    return ( status ); /* return nRF24L01 status */
}

void RX_Mode ( void ) { /* ���ݽ���ģʽ���� */
    CE = 0;
    SPI_Write_Buf ( WRITE_REG + RX_ADDR_P0, RX_ADDRESS, RX_ADR_WIDTH ); /* д���ն˵�ַ */
    SPI_WR_Reg ( WRITE_REG + EN_AA, 0 ); /* Ƶ��0�Զ�ACKӦ������ */
    SPI_WR_Reg ( WRITE_REG + EN_RXADDR, 0x01 ); /* ������յ�ַֻ��Ƶ��0�������Ҫ��Ƶ�����Բο������ֲ� */
    SPI_WR_Reg ( WRITE_REG + RF_CH, 40 ); /* �����ŵ�����Ϊ2.4GHz���շ�����һ�� */
    SPI_WR_Reg ( WRITE_REG + RX_PW_P0, RX_PLOAD_WIDTH ); /* ���ý������ݳ��ȣ���������Ϊ32�ֽ� */
    SPI_WR_Reg ( WRITE_REG + RF_SETUP, 0x0F ); /* ���÷�������Ϊ2MHz�����书��Ϊ���ֵ0dB */
    SPI_WR_Reg ( WRITE_REG + CONFIG, 0x0F );
    CE = 1;
}

uchar nRF24L01_RxPacket ( unsigned char *rx_buf ) { /* ���ݶ�ȡ�����rx_buf���ջ������� */
    uchar flag = 0;
    uchar status;
    status = SPI_RD_Reg ( NRFRegSTATUS ); /* ��ȡ״̬�Ĵ������ж����ݽ���״�� */

    if ( status & 0x40 ) { /* �ж��Ƿ���յ����� */
        CE = 0; /* SPIʹ�� */
        SPI_Read_Buf ( RD_RX_PLOAD, rx_buf, TX_PLOAD_WIDTH ); /* read receive payload from RX_FIFO buffer */
        flag = 1; /* ��ȡ������ɱ�־ */
    }

    SPI_WR_Reg ( WRITE_REG + NRFRegSTATUS, status ); /* ���յ����ݺ�RX_DR��TX_DS��MAX_RT���ø�Ϊ1��ͨ��д1������жϱ�־ */
    return flag;
}

void TX_Mode ( void ) { /* ���ݷ������� */
    CE = 0;
    SPI_Write_Buf ( WRITE_REG + TX_ADDR, TX_ADDRESS, TX_ADR_WIDTH ); /* д���ص�ַ */
    SPI_Write_Buf ( WRITE_REG + RX_ADDR_P0, RX_ADDRESS, RX_ADR_WIDTH ); /* д���ն˵�ַ */
    SPI_WR_Reg ( WRITE_REG + EN_AA, 0 ); /* Ƶ��0�Զ�ACKӦ������ */
    SPI_WR_Reg ( WRITE_REG + EN_RXADDR, 0 ); /* ������յ�ַֻ��Ƶ��0�������Ҫ��Ƶ�����Բο������ֲ� */
    SPI_WR_Reg ( WRITE_REG + SETUP_RETR, 0 ); /* �����Զ��ط�ʱ��ʹ��� */
    SPI_WR_Reg ( WRITE_REG + RF_CH, 40 ); /* �����ŵ�����Ϊ2.4GHz���շ�����һ�� */
    SPI_WR_Reg ( WRITE_REG + RX_PW_P0, RX_PLOAD_WIDTH ); /* ���ý������ݳ��ȣ���������Ϊ32�ֽ� */
    SPI_WR_Reg ( WRITE_REG + RF_SETUP, 0x0F ); /* ���÷�������Ϊ2MHz�����书��Ϊ���ֵ0dB */
    SPI_WR_Reg ( WRITE_REG + CONFIG, 0x0E );
    CE = 1;
}

void nRF24L01_TxPacket ( unsigned char *tx_buf ) { /* ����tx_buf�е����� */
    CE = 0;
    SPI_Write_Buf ( WRITE_REG + RX_ADDR_P0, TX_ADDRESS, TX_ADR_WIDTH ); /* װ�ؽ��ն˵�ַ */
    SPI_Write_Buf ( WR_TX_PLOAD, tx_buf, TX_PLOAD_WIDTH ); /* װ������ */
    CE = 1; /* �ø�CE�����ݷ��� */
}