#include "rf4432.h"
#include "spi.h"
#include "sys.h"
#include "usart.h"
#include "delay.h"

#define RR 0x00
#define WR 0x80

unsigned char ITSTATUS;
extern char RF4432_TxBuf[RF4432_TxBuf_Len];

unsigned char RF4432_ReadReg ( unsigned char  addr ) {
    unsigned char value;
    RF4432_SEL = 0;
    SPIx_ReadWriteByte ( addr | RR );
    value = SPIx_ReadWriteByte ( addr | RR );
    RF4432_SEL = 1;
    return value;
}

void RF4432_WriteReg ( unsigned char  addr, unsigned char value ) {
    RF4432_SEL = 0;
    SPIx_ReadWriteByte ( addr | WR );
    SPIx_ReadWriteByte ( value );
    RF4432_SEL = 1;
}

void RF4432_WriteBurestReg ( char  addr, char  *p, char  count ) {
    unsigned char i;
    RF4432_SEL = 0;
    SPIx_ReadWriteByte ( addr | WR );

    for ( i = 0; i < count; i++ ) {
        SPIx_ReadWriteByte ( p[i] );
    }

    RF4432_SEL = 1;
}

void RF4432_ReadBurestReg ( unsigned  char  addr, unsigned   char  *p, unsigned  char  count ) {
    unsigned char i;
    RF4432_SEL = 0;
    SPIx_ReadWriteByte ( addr | RR );

    for ( i = 0; i < count; i++ ) {
        p[i] = SPIx_ReadWriteByte ( addr | RR );
    }

    RF4432_SEL = 1;
}

void RF4432_IOInit ( void ) {
    GPIO_InitTypeDef  GPIO_InitStructure;
    RCC_APB2PeriphClockCmd ( RCC_APB2Periph_GPIOC, ENABLE );
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_2;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init ( GPIOC, &GPIO_InitStructure );
    GPIO_SetBits ( GPIOC, GPIO_Pin_0 | GPIO_Pin_2 );
    GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_1;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init ( GPIOC, &GPIO_InitStructure );
}

void RF4432_Init ( void ) {
    RF4432_IOInit();
    SPIx_Init();
    RF4432_IRQ = 1;
    RF4432_SDN = 0;
    delay_ms ( 70 );
    RF4432_ReadReg ( INTERRUPT_STATUS_1 );
    RF4432_ReadReg ( INTERRUPT_STATUS_2 );
    RF4432_WriteReg ( OPERATING_FUNCTION_CONTROL_1, 0x80 );

    while ( RF4432_IRQ == 1 );

    RF4432_WriteReg ( INTERRUPT_ENABLE_1, 0x00 );
    RF4432_WriteReg ( INTERRUPT_ENABLE_2, 0x00 );
    RF4432_WriteReg ( 0x75, 0x53 );
    RF4432_WriteReg ( 0x76, 0x64 );
    RF4432_WriteReg ( 0x77, 0x00 );
    RF4432_WriteReg ( 0x2a, 0x14 );
    RF4432_WriteReg ( 0x6e, 0x09 );
    RF4432_WriteReg ( 0x6f, 0xd5 );
    RF4432_WriteReg ( 0x70, 0x2c );
    RF4432_WriteReg ( 0x72, 0x38 );
    RF4432_WriteReg ( 0x1C, 0x1b );
    RF4432_WriteReg ( 0x20, 0x83 );
    RF4432_WriteReg ( 0x21, 0xc0 );
    RF4432_WriteReg ( 0x22, 0x13 );
    RF4432_WriteReg ( 0x23, 0xa9 );
    RF4432_WriteReg ( 0x24, 0x00 );
    RF4432_WriteReg ( 0x25, 0x03 );
    RF4432_WriteReg ( 0x1D, 0x40 );
    RF4432_WriteReg ( 0x1E, 0x0A );
    RF4432_WriteReg ( 0x2A, 0x14 );
    RF4432_WriteReg ( 0x34, 0X0A );
    RF4432_WriteReg ( 0x35, 0x2A );
    RF4432_WriteReg ( 0x33, 0x02 );
    RF4432_WriteReg ( 0x36, 0x2d );
    RF4432_WriteReg ( 0x37, 0xd4 );
    RF4432_WriteReg ( 0x30, 0x8D );
    RF4432_WriteReg ( 0x32, 0x00 );
    RF4432_WriteReg ( 0x71, 0x63 );
    RF4432_WriteReg ( 0x0b, 0x12 );
    RF4432_WriteReg ( 0x0c, 0x15 );
    RF4432_WriteReg ( 0x09, 0xD7 );
    RF4432_WriteReg ( 0x69, 0x60 );
    RF4432_WriteReg ( 0x6d, 0x07 );
    RF4432_WriteReg ( 0x07, 0x05 );
    RF4432_WriteReg ( 0x05, 0x03 );
    RF4432_WriteReg ( 0x06, 0x00 );
    RF4432_ReadReg ( 0x03 );
    RF4432_ReadReg ( 0x04 );
    RF4432_WriteReg ( 0x07, 0x05 );
    RF4432_WriteReg ( 0x0d, 0xf4 );
}

void RF4432_SetRxMode ( void ) {
    RF4432_WriteReg ( OPERATING_FUNCTION_CONTROL_2, RF4432_ReadReg ( OPERATING_FUNCTION_CONTROL_2 ) | 0x02 );
    RF4432_WriteReg ( OPERATING_FUNCTION_CONTROL_2, RF4432_ReadReg ( OPERATING_FUNCTION_CONTROL_2 ) & 0xFD );
    RF4432_WriteReg ( GPIO1_CONFIGURATION, 0xca );
    RF4432_WriteReg ( GPIO2_CONFIGURATION, 0xca );
    RF4432_WriteReg ( IO_PORT_CONFIGURATION, 0x04 );
    RF4432_WriteReg ( HEADER_CONTROL_1, 0x8c );
    RF4432_WriteReg ( HEADER_CONTROL_2, 0x02 );
    RF4432_ReadReg ( INTERRUPT_STATUS_1 );
    RF4432_ReadReg ( INTERRUPT_STATUS_2 );
    RF4432_WriteReg ( INTERRUPT_ENABLE_1, 0x02 );
    RF4432_WriteReg ( INTERRUPT_ENABLE_2, 0x00 );
    RF4432_WriteReg ( OPERATING_FUNCTION_CONTROL_1, 0x05 );
    RF4432_WriteReg ( TRANSMIT_PACKET_LENGTH, RF4432_TxBuf_Len );
    RF4432_WriteReg ( RECEIVED_PACKET_LENGTH, RF4432_RxBuf_Len );
}

unsigned char RF4432_RxPacket ( void ) {
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

void RF4432_TxPacket ( void ) {
    RF4432_WriteReg ( OPERATING_FUNCTION_CONTROL_2, RF4432_ReadReg ( OPERATING_FUNCTION_CONTROL_2 ) | 0x01 );
    RF4432_WriteReg ( OPERATING_FUNCTION_CONTROL_2, RF4432_ReadReg ( OPERATING_FUNCTION_CONTROL_2 ) & 0xFE );
    RF4432_WriteBurestReg ( FIFO_ACCESS, RF4432_TxBuf, RF4432_TxBuf_Len );
    RF4432_ReadReg ( INTERRUPT_STATUS_1 );
    RF4432_ReadReg ( INTERRUPT_STATUS_2 );
    RF4432_WriteReg ( INTERRUPT_ENABLE_1, 0x04 );
    RF4432_WriteReg ( OPERATING_FUNCTION_CONTROL_1, 0x09 );

    while ( RF4432_IRQ == 1 );

    if ( !RF4432_IRQ ) {
        ITSTATUS = RF4432_ReadReg ( INTERRUPT_STATUS_1 );

        while ( ( ITSTATUS & 0x04 ) != 0x04 );
    }
}