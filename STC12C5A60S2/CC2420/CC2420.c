#include "CC2420.h"

uint8 pdata CC2420_PSDU[1 + CC2420_PSDU_Len]; /* pdata��һ�ֱ������ͣ��ɴ�ȡ256�ֽ� */
uint8 CC2420_Source_PANID[2] = {0xcc, 0xcc};
uint8 CC2420_Source_ShortAddr[2] = {0xcc, 0xcc};
uint8 CC2420_Source_IEEEAddr[8] = {0x12, 0x34, 0x56, 0x7c, 0xcc, 0xcc, 0xcc, 0xcc}; /* ������ַ */
uint8 CC2420_Destination_PANID[2] = {0xcc, 0xcc};
uint8 CC2420_Destination_ShortAddr[2] = {0xcc, 0xcc};
uint8 CC2420_Destination_IEEEAddr[8] = {0x12, 0x34, 0x56, 0x7c, 0xcc, 0xcc, 0xcc, 0xcc}; /* Ŀ���ַ */

extern void delay_ms ( uint16 n );

void SPI_Init ( void ) {
    SCLK_OFF();
    CSN_ON();
}

uint8 SPI_Read ( void ) { /* SPI���ֽڶ�ȡ���� */
    uint8 i, rxdata = 0x00;

    for ( i = 0; i < 8; i++ ) {
        rxdata = rxdata << 1;
        SCLK_ON();

        if ( MISO_IN ) {
            rxdata |= 0x01;
        } else {
            rxdata &= ~0x01;
        }

        SCLK_OFF();
    }

    return rxdata;
}

void SPI_Write ( uint8 txdata ) { /* SPI���ֽ�д�뺯�� */
    uint8 i;

    for ( i = 0; i < 8; i++ ) {
        if ( txdata & 0x80 ) {
            MOSI_ON();
        } else {
            MOSI_OFF();
        }

        SCLK_ON();
        txdata = txdata << 1;
        SCLK_OFF();
    }
}

uint8 SPI_WriteRead ( uint8 txdata ) { /* SPI���ֽ�д���ȡ���� */
    uint8 i, rxdata;
    rxdata = 0x00;

    for ( i = 0; i < 8; i++ ) {
        rxdata = rxdata << 1;

        if ( txdata & 0x80 ) {
            MOSI_ON();
        } else {
            MOSI_OFF();
        }

        SCLK_ON();

        if ( MISO_IN ) {
            rxdata |= 0x01;
        } else {
            rxdata &= ~0x01;
        }

        SCLK_OFF();
        txdata = txdata << 1;
    }

    return rxdata;
}

uint16 SPI_Word_Read ( void ) { /* SPI˫�ֽڶ�ȡ���� */
    uint8 i;
    uint16 rxdata;
    rxdata = 0x0000;

    for ( i = 0; i < 16; i++ ) {
        rxdata = rxdata << 1;
        SCLK_ON();

        if ( MISO_IN ) {
            rxdata |= 0x01;
        } else {
            rxdata &= ~0x01;
        }

        SCLK_OFF();
    }

    return rxdata;
}

void SPI_Word_Write ( uint16 txdata ) { /* SPI˫�ֽ�д�뺯�� */
    uint8 i;

    for ( i = 0; i < 16; i++ ) {
        if ( txdata & 0x8000 ) {
            MOSI_ON();
        } else {
            MOSI_OFF();
        }

        SCLK_ON();
        txdata = txdata << 1;
        SCLK_OFF();
    }
}

uint16 CC2420_ReadReg ( uint8  addr ) { /* CC2420�Ĵ�����ȡ���� */
    uint16 value;
    CSN_OFF();
    SPI_Write ( addr | REG_READ );
    value = SPI_Word_Read();
    CSN_ON();
    return value;
}

void CC2420_WriteReg ( uint8  addr, uint16 value ) { /* CC2420�Ĵ���д�뺯�� */
    CSN_OFF();
    SPI_Write ( addr | REG_WRITE );
    SPI_Word_Write ( value );
    CSN_ON();
}

void CC2420_ReadRXFIFO ( void ) { /* ��ȡFIFO�Ĵ��� */
    uint8 i;
    CSN_OFF();
    SPI_Write ( REG_RXFIFO | REG_READ );
    CC2420_PSDU[0] = SPI_Read();

    for ( i = 0; i < CC2420_PSDU[0]; i++ ) {
        CC2420_PSDU[1 + i] = SPI_Read();
    }

    CSN_ON();
    CC2420_Command ( CMD_SFLUSHRX );
}

void CC2420_WriteTXFIFO ( void ) { /* д��FIFO�Ĵ��� */
    uint8 i;
    CC2420_Command ( CMD_SFLUSHTX ); /* ˢ�·��ͼĴ��� */
    CSN_OFF();
    SPI_Write ( REG_TXFIFO | REG_WRITE ); 
    SPI_Write ( CC2420_PSDU[0] );

    for ( i = 0; i < CC2420_PSDU[0]; i++ ) {
        SPI_Write ( CC2420_PSDU[1 + i] );
    }

    CSN_ON();
}

void CC2420_Command ( uint8 cmd ) { /* CC2420����� */
    CSN_OFF();
    SPI_Write ( cmd );
    CSN_ON();
}

uint8 CC2420_RAM_Read ( uint8 addr, uint8 block ) { /* CC2420��RAM��ȡ���� */
    uint8 value;
    CSN_OFF();
    SPI_Write ( addr | RAM );
    SPI_Write ( ( block << 6 ) | RAM_READ );
    value = SPI_Read();
    CSN_ON();
    return value;
}

void CC2420_RAM_Write ( uint8  addr, uint8 block, uint8 value ) { /* CC2420��RAMд�뺯�� */
    CSN_OFF();
    SPI_Write ( addr | RAM );
    SPI_Write ( ( block << 6 ) | RAM_WRITE );
    SPI_Write ( value );
    CSN_ON();
}

void CC2420_SetRxMode ( void ) { /* CC2420���ý��պ��� */
    CC2420_Command ( CMD_SRFOFF );
    CC2420_Command ( CMD_SRXON );
}

uint8 CC2420_RxPacket ( void ) { /* CC2420�жϽ��պ��� */
    SFD_IN = 1;
    FIFO_IN = 1;

    if ( ( !SFD_IN ) && ( FIFO_IN ) ) {
        return TRUE;
    }

    return FALSE;
}

void CC2420_TxPacket ( void ) { /* CC2420���ͺ��� */
    CC2420_Command ( CMD_SRFOFF );
    CC2420_Command ( CMD_STXON );

   
    while ( !SFD_IN );

    while ( SFD_IN );
}

void CC2420_Init ( void ) { /* CC2420��ƵоƬ��ʼ������ */
    RESET_OFF();
    delay_ms ( 10 );
    RESET_ON();
    delay_ms ( 10 );
    CC2420_Command ( CMD_SXOSCON );
    delay_ms ( 10 );
    CC2420_PSDU[ 1 ] = ( PAN_ID_COMPRESSION << 6 ) | ( ACKNOWLEDGMENT_REQUEST << 5 ) | ( FRAME_PENDING << 4 ) |
                       ( SECURITY_ENABLE << 3 ) | ( FRAME_TYPE_DATA << 0 );
    CC2420_PSDU[ 2 ] = ( SOURCE_ADDRESSING_MODE << 6 ) | ( FRAME_VERSION << 4 ) | ( DEST_ADDRESSING_MODE << 2 );
    CC2420_PSDU[ 3 ] = SEQUENCE_NUMBER;
    CC2420_PSDU[ 4 ] =  CC2420_Destination_PANID[0];
    CC2420_PSDU[ 5 ] =  CC2420_Destination_PANID[1];
    CC2420_PSDU[ 6 ] =  CC2420_Destination_IEEEAddr[0];
    CC2420_PSDU[ 7 ] =  CC2420_Destination_IEEEAddr[1];
    CC2420_PSDU[ 8 ] =  CC2420_Destination_IEEEAddr[2];
    CC2420_PSDU[ 9 ] =  CC2420_Destination_IEEEAddr[3];
    CC2420_PSDU[10] =  CC2420_Destination_IEEEAddr[4];
    CC2420_PSDU[11] =  CC2420_Destination_IEEEAddr[5];
    CC2420_PSDU[12] =  CC2420_Destination_IEEEAddr[6];
    CC2420_PSDU[13] =  CC2420_Destination_IEEEAddr[7];
    CC2420_PSDU[14] =  CC2420_Source_PANID[0];
    CC2420_PSDU[15] =  CC2420_Source_PANID[1];
    CC2420_RAM_Write ( RAM_PANID, 2,  CC2420_Source_PANID[0] );
    CC2420_RAM_Write ( RAM_PANID + 1, 2, CC2420_Source_PANID[1] );
    CC2420_PSDU[16] =  CC2420_Source_IEEEAddr[0];
    CC2420_PSDU[17] =  CC2420_Source_IEEEAddr[1];
    CC2420_PSDU[18] =  CC2420_Source_IEEEAddr[2];
    CC2420_PSDU[19] =  CC2420_Source_IEEEAddr[3];
    CC2420_PSDU[20] = CC2420_Source_IEEEAddr[4];
    CC2420_PSDU[21] =  CC2420_Source_IEEEAddr[5];
    CC2420_PSDU[22] =  CC2420_Source_IEEEAddr[6];
    CC2420_PSDU[23] =  CC2420_Source_IEEEAddr[7];
    CC2420_RAM_Write ( RAM_IEEEADR, 2, CC2420_Source_IEEEAddr[0] );
    CC2420_RAM_Write ( RAM_IEEEADR + 1, 2, CC2420_Source_IEEEAddr[1] );
    CC2420_RAM_Write ( RAM_IEEEADR + 2, 2, CC2420_Source_IEEEAddr[2] );
    CC2420_RAM_Write ( RAM_IEEEADR + 3, 2, CC2420_Source_IEEEAddr[3] );
    CC2420_RAM_Write ( RAM_IEEEADR + 4, 2, CC2420_Source_IEEEAddr[4] );
    CC2420_RAM_Write ( RAM_IEEEADR + 5, 2, CC2420_Source_IEEEAddr[5] );
    CC2420_RAM_Write ( RAM_IEEEADR + 6, 2, CC2420_Source_IEEEAddr[6] );
    CC2420_RAM_Write ( RAM_IEEEADR + 7, 2, CC2420_Source_IEEEAddr[7] );
    CC2420_WriteReg ( REG_MDMCTRL0, CCA_HYST | CCA_MODE | PREAMBLE_LENGTH | AUTOCRC | ADR_DECODE );
    CC2420_WriteReg ( REG_SYNCWORD, SYNCWORD );
    CC2420_WriteReg ( REG_SECCTRL0, 0 );
    CSN_OFF();
    SPI_Write ( REG_RXFIFO | REG_READ );
    SPI_Read();
    CSN_ON();
    CC2420_Command ( CMD_SFLUSHRX );
    CC2420_Command ( CMD_SFLUSHTX );
    delay_ms ( 10 );
}