#include "rf4432.h"
#include "spi.h"

#define RR 0x00
#define WR 0x80

unsigned char ITSTATUS;

unsigned char  RF4432_ReadReg ( unsigned char  addr ) { /* RF4432�Ĵ�����ȡ���� */
    unsigned char value;
    RF4432_SEL = 0;
    SPI_Write ( addr | RR );
    value = SPI_Write ( addr | RR ) ;
    RF4432_SEL = 1;
    return value;
}

void  RF4432_WriteReg ( unsigned char  addr, unsigned char value ) { /* RF4432�Ĵ���д�뺯�� */
    RF4432_SEL = 0;
    SPI_Write ( addr | WR );
    SPI_Write ( value );
    RF4432_SEL = 1;
}

void  RF4432_WriteBurestReg ( char  addr, char  *p, char  count ) { /* RF4432�Ĵ�������д�뺯�� */
    unsigned char i;
    RF4432_SEL = 0;
    SPI_Write ( addr | WR );

    for ( i = 0; i < count; i++ ) {
        SPI_Write ( p[i] );
    }

    RF4432_SEL = 1;
}

void RF4432_ReadBurestReg ( char  addr,  char  *p, char  count ) { /* RF4432�Ĵ���������ȡ���� */
    unsigned char i;
    RF4432_SEL = 0;
    SPI_Write ( addr | RR );

    for ( i = 0; i < count; i++ ) {
        p[i] = SPI_Write ( addr | RR );
    }

    RF4432_SEL = 1;
}

void RF4432_Init ( void ) { /* RF4432��ƵоƬ��ʼ������ */
    RF4432_IRQ = 1;
    RF4432_SDN = 0; /* ����RF4432���� */
    delay_ms ( 30 );
    delay_ms ( 30 );
    RF4432_ReadReg ( INTERRUPT_STATUS_1 ); /* ���ж� */
    RF4432_ReadReg ( INTERRUPT_STATUS_2 );
    RF4432_WriteReg ( OPERATING_FUNCTION_CONTROL_1, 0x80 ); /* �����λ */

    while ( RF4432_IRQ == 1 ); /* �ȴ���λ��ɣ�����λ��ɺ����жϷ��� */

    RF4432_WriteReg ( INTERRUPT_ENABLE_1, 0x00 ); /* ���ж� */
    RF4432_WriteReg ( INTERRUPT_ENABLE_2, 0x00 ); /* ��ֹPOR�ж� */
    /* Ƶ������434 */
    RF4432_WriteReg ( 0x75, 0x53 );
    RF4432_WriteReg ( 0x76, 0x64 );
    RF4432_WriteReg ( 0x77, 0x00 );
    /* ��1.2K bps���������� */
    RF4432_WriteReg ( 0x2a, 0x14 );
    RF4432_WriteReg ( 0x6e, 0x09 );
    RF4432_WriteReg ( 0x6f, 0xd5 );
    RF4432_WriteReg ( 0x70, 0x2c );
    RF4432_WriteReg ( 0x72, 0x38 ); /* Ƶ��ƫ��(1.2kbps) */
    /* ��������ø���Silabs��Excel (9.6 kbps, deviation: 45 kHz, channel filter BW: 102.2 kHz) */
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
    /* ǰ����(ͬ����) */
    RF4432_WriteReg ( 0x34, 0X0A ); /* ����5�ֽڵ�Preamble */
    RF4432_WriteReg ( 0x35, 0x2A ); /* ��Ҫ���Preamble */
    RF4432_WriteReg ( 0x33, 0x02 );
    RF4432_WriteReg ( 0x36, 0x2d ); /* ͬ����Ϊ0x2dd4 */
    RF4432_WriteReg ( 0x37, 0xd4 );
    RF4432_WriteReg ( 0x30, 0x8D ); /* ʹ�ܡ�PH + FIFO��ģʽ����λ��ǰ�棬ʹ��CRCУ�� */
    RF4432_WriteReg ( 0x32, 0x00 ); /* ��ֹ֡ͷ */
    RF4432_WriteReg ( 0x71, 0x63 );
    RF4432_WriteReg ( 0x0b, 0x12 );
    RF4432_WriteReg ( 0x0c, 0x15 );
    RF4432_WriteReg ( 0x09, 0xD7 ); /* ���ص��� */
    RF4432_WriteReg ( 0x69, 0x60 ); /* AGC���� */
    RF4432_WriteReg ( 0x6d, 0x17 ); /* ���书�� */
    RF4432_WriteReg ( 0x07, 0x05 ); /* �ֶ��򿪽��� */
    RF4432_WriteReg ( 0x05, 0x03 ); /* �򿪽����ж� */
    RF4432_WriteReg ( 0x06, 0x00 );
    /* ���ж� */
    RF4432_ReadReg ( 0x03 ); /* read the Interrupt Status1 register */
    RF4432_ReadReg ( 0x04 ); /* read the Interrupt Status2 register */
    RF4432_WriteReg ( 0x07, 0x05 ); /* �򿪽��� */
    RF4432_WriteReg ( 0x0d, 0xf4 ); /* GPIO2�������� */
}

void RF4432_SetRxMode ( void ) { /* RF4432���ý���ģʽ���� */
    RF4432_WriteReg ( OPERATING_FUNCTION_CONTROL_2, RF4432_ReadReg ( OPERATING_FUNCTION_CONTROL_2 ) | 0x02 ); /* ����FIFO��0 */
    RF4432_WriteReg ( OPERATING_FUNCTION_CONTROL_2, RF4432_ReadReg ( OPERATING_FUNCTION_CONTROL_2 ) & 0xFD );
    RF4432_WriteReg ( GPIO1_CONFIGURATION, 0xca );
    RF4432_WriteReg ( GPIO2_CONFIGURATION, 0xca );
    RF4432_WriteReg ( IO_PORT_CONFIGURATION, 0x04 );
    RF4432_WriteReg ( HEADER_CONTROL_1, 0x8c );
    RF4432_WriteReg ( HEADER_CONTROL_2, 0x02 ); /* û�з���֡ͷ��֡ͷ���������ݰ����ȣ�0x02�ǲ����� */
    RF4432_ReadReg ( INTERRUPT_STATUS_1 ); /* ���ж� */
    RF4432_ReadReg ( INTERRUPT_STATUS_2 );
    RF4432_WriteReg ( INTERRUPT_ENABLE_1, 0x02 ); /* ʹ�ܽ��հ��ж� */
    RF4432_WriteReg ( INTERRUPT_ENABLE_2, 0x00 );
    RF4432_WriteReg ( OPERATING_FUNCTION_CONTROL_1, 0x05 ); /* �˹�����ģʽ���򿪾��� */
    RF4432_WriteReg ( TRANSMIT_PACKET_LENGTH, RF4432_TxBuf_Len ); /* ���÷�������� */
    RF4432_WriteReg ( RECEIVED_PACKET_LENGTH, RF4432_RxBuf_Len ); /* ���ý��հ����� */
}

bit RF4432_RxPacket ( void ) { /* RF4432���ݰ����պ��� */
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

void RF4432_TxPacket ( void ) { /* RF4432���ݰ����ͺ��� */
    RF4432_WriteReg ( OPERATING_FUNCTION_CONTROL_2, RF4432_ReadReg ( OPERATING_FUNCTION_CONTROL_2 ) | 0x01 ); /* ����FIFO��0 */
    RF4432_WriteReg ( OPERATING_FUNCTION_CONTROL_2, RF4432_ReadReg ( OPERATING_FUNCTION_CONTROL_2 ) & 0xFE );
    RF4432_WriteBurestReg ( FIFO_ACCESS, RF4432_TxBuf, RF4432_TxBuf_Len ); /* ���� */
    RF4432_ReadReg ( INTERRUPT_STATUS_1 ); /* ���ж� */
    RF4432_ReadReg ( INTERRUPT_STATUS_2 );
    RF4432_WriteReg ( INTERRUPT_ENABLE_1, 0x04 ); /* ʹ�ܰ������ж� */
    RF4432_WriteReg ( OPERATING_FUNCTION_CONTROL_1, 0x09 ); /* �˹�����ģʽ���򿪾��� */

    while ( RF4432_IRQ == 1 ); /* �ȴ�������ɣ������ж� */

    if ( !RF4432_IRQ ) {
        ITSTATUS = RF4432_ReadReg ( INTERRUPT_STATUS_1 );

        while ( ( ITSTATUS & 0x04 ) != 0x04 ); /* �Ѿ�����һ����Ч�������2λΪ1�����Ϊ0��ȴ� */
    }
}