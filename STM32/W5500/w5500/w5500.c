#include <stdio.h>
#include <string.h>
#include "config.h"
#include "spi.h"
#include "w5500.h"

#ifdef __DEF_IINCHIP_PPP__
    #include "md5.h"
#endif

static uint8 I_STATUS[MAX_SOCK_NUM];
static uint16 SSIZE[MAX_SOCK_NUM]; /* Max Tx buffer size by each channel */
static uint16 RSIZE[MAX_SOCK_NUM]; /* Max Rx buffer size by each channel */

uint8 getISR ( uint8 s ) {
    return I_STATUS[s];
}

void putISR ( uint8 s, uint8 val ) {
    I_STATUS[s] = val;
}

uint16 getIINCHIP_RxMAX ( uint8 s ) {
    return RSIZE[s];
}

uint16 getIINCHIP_TxMAX ( uint8 s ) {
    return SSIZE[s];
}

void IINCHIP_CSoff ( void ) {
    WIZ_CS ( LOW );
}

void IINCHIP_CSon ( void ) {
    WIZ_CS ( HIGH );
}

u8  IINCHIP_SpiSendData ( uint8 dat ) {
    return ( SPIx_ReadWriteByte ( dat ) );
}

void IINCHIP_WRITE ( uint32 addrbsb,  uint8 data ) {
    IINCHIP_ISR_DISABLE(); /* Interrupt Service Routine Disable */
    IINCHIP_CSoff(); /* CS = 0, SPI start */
    IINCHIP_SpiSendData ( ( addrbsb & 0x00FF0000 ) >> 16 ); /* Address byte 1 */
    IINCHIP_SpiSendData ( ( addrbsb & 0x0000FF00 ) >> 8 ); /* Address byte 2 */
    IINCHIP_SpiSendData ( ( addrbsb & 0x000000F8 ) + 4 ); /* Data write command and Write data length 1 */
    IINCHIP_SpiSendData ( data ); /* Data write (write 1byte data) */
    IINCHIP_CSon(); /* CS = 1, SPI end */
    IINCHIP_ISR_ENABLE(); /* Interrupt Service Routine Enable */
}

uint8 IINCHIP_READ ( uint32 addrbsb ) {
    uint8 data = 0;
    IINCHIP_ISR_DISABLE(); /* Interrupt Service Routine Disable */
    IINCHIP_CSoff(); /* CS = 0, SPI start */
    IINCHIP_SpiSendData ( ( addrbsb & 0x00FF0000 ) >> 16 ); /* Address byte 1 */
    IINCHIP_SpiSendData ( ( addrbsb & 0x0000FF00 ) >> 8 ); /* Address byte 2 */
    IINCHIP_SpiSendData ( ( addrbsb & 0x000000F8 ) ); /* Data read command and Read data length 1 */
    data = IINCHIP_SpiSendData ( 0x00 ); /* Data read (read 1byte data) */
    IINCHIP_CSon(); /* CS = 1, SPI end */
    IINCHIP_ISR_ENABLE(); /* Interrupt Service Routine Enable */
    return data;
}

uint16 wiz_write_buf ( uint32 addrbsb, uint8 *buf, uint16 len ) {
    uint16 idx = 0;

    if ( len == 0 ) {
        printf ( "Unexpected2 length 0\r\n" );
    }

    IINCHIP_ISR_DISABLE();
    IINCHIP_CSoff(); /* CS = 0, SPI start */
    IINCHIP_SpiSendData ( ( addrbsb & 0x00FF0000 ) >> 16 ); /* Address byte 1 */
    IINCHIP_SpiSendData ( ( addrbsb & 0x0000FF00 ) >> 8 ); /* Address byte 2 */
    IINCHIP_SpiSendData ( ( addrbsb & 0x000000F8 ) + 4 ); /* Data write command and Write data length 1 */

    for ( idx = 0; idx < len; idx++ ) { /* Write data in loop */
        IINCHIP_SpiSendData ( buf[idx] );
    }

    IINCHIP_CSon(); /* CS = 1, SPI end */
    IINCHIP_ISR_ENABLE(); /* Interrupt Service Routine Enable */
    return len;
}

uint16 wiz_read_buf ( uint32 addrbsb, uint8 *buf, uint16 len ) {
    uint16 idx = 0;

    if ( len == 0 ) {
        printf ( "Unexpected2 length 0\r\n" );
    }

    IINCHIP_ISR_DISABLE();
    IINCHIP_CSoff(); /* CS = 0, SPI start */
    IINCHIP_SpiSendData ( ( addrbsb & 0x00FF0000 ) >> 16 ); /* Address byte 1 */
    IINCHIP_SpiSendData ( ( addrbsb & 0x0000FF00 ) >> 8 ); /* Address byte 2 */
    IINCHIP_SpiSendData ( ( addrbsb & 0x000000F8 ) ); /* Data write command and Write data length 1 */

    for ( idx = 0; idx < len; idx++ ) { /* Write data in loop */
        buf[idx] = IINCHIP_SpiSendData ( 0x00 );
    }

    IINCHIP_CSon(); /* CS = 1, SPI end */
    IINCHIP_ISR_ENABLE(); /* Interrupt Service Routine Enable */
    return len;
}

/* This function is for resetting of the iinchip. Initializes the iinchip to work in whether DIRECT or INDIRECT mode */
void iinchip_init ( void ) {
    setMR ( MR_RST );
#ifdef __DEF_IINCHIP_DBG__
    printf ( "MR value is %02x \r\n", IINCHIP_READ_COMMON ( MR ) );
#endif
}

void sysinit ( uint8 *tx_size, uint8 *rx_size  ) {
    int16 i;
    int16 ssum, rsum;
#ifdef __DEF_IINCHIP_DBG__
    printf ( "sysinit()\r\n" );
#endif
    ssum = 0;
    rsum = 0;

    for ( i = 0 ; i < MAX_SOCK_NUM; i++ ) { /* Set the size, masking and base address of Tx & Rx memory by each channel */
        IINCHIP_WRITE ( ( Sn_TXMEM_SIZE ( i ) ), tx_size[i] );
        IINCHIP_WRITE ( ( Sn_RXMEM_SIZE ( i ) ), rx_size[i] );
#ifdef __DEF_IINCHIP_DBG__
        printf ( "tx_size[%d]: %d, Sn_TXMEM_SIZE = %d\r\n", i, tx_size[i], IINCHIP_READ ( Sn_TXMEM_SIZE ( i ) ) );
        printf ( "rx_size[%d]: %d, Sn_RXMEM_SIZE = %d\r\n", i, rx_size[i], IINCHIP_READ ( Sn_RXMEM_SIZE ( i ) ) );
#endif
        SSIZE[i] = ( int16 ) ( 0 );
        RSIZE[i] = ( int16 ) ( 0 );

        if ( ssum <= 16384 ) {
            switch ( tx_size[i] ) {
                case 1:
                    SSIZE[i] = ( int16 ) ( 1024 );
                    break;

                case 2:
                    SSIZE[i] = ( int16 ) ( 2048 );
                    break;

                case 4:
                    SSIZE[i] = ( int16 ) ( 4096 );
                    break;

                case 8:
                    SSIZE[i] = ( int16 ) ( 8192 );
                    break;

                case 16:
                    SSIZE[i] = ( int16 ) ( 16384 );
                    break;

                default :
                    RSIZE[i] = ( int16 ) ( 2048 );
                    break;
            }
        }

        if ( rsum <= 16384 ) {
            switch ( rx_size[i] ) {
                case 1:
                    RSIZE[i] = ( int16 ) ( 1024 );
                    break;

                case 2:
                    RSIZE[i] = ( int16 ) ( 2048 );
                    break;

                case 4:
                    RSIZE[i] = ( int16 ) ( 4096 );
                    break;

                case 8:
                    RSIZE[i] = ( int16 ) ( 8192 );
                    break;

                case 16:
                    RSIZE[i] = ( int16 ) ( 16384 );
                    break;

                default :
                    RSIZE[i] = ( int16 ) ( 2048 );
                    break;
            }
        }

        ssum += SSIZE[i];
        rsum += RSIZE[i];
    }
}

void setGAR ( uint8 *addr ) { /* brief  This function sets up gateway IP address */
    wiz_write_buf ( GAR0, addr, 4 );
}

void getGWIP ( uint8 *addr ) {
    wiz_read_buf ( GAR0, addr, 4 );
}

void setSUBR ( uint8 *addr ) { /* It sets up SubnetMask address */
    wiz_write_buf ( SUBR0, addr, 4 );
}

void setSHAR ( uint8 *addr ) { /* This function sets up MAC address */
    wiz_write_buf ( SHAR0, addr, 6 );
}

void setSIPR ( uint8 *addr ) { /* This function sets up Source IP address */
    wiz_write_buf ( SIPR0, addr, 4 );
}

void getGAR ( uint8 *addr ) { /* This function sets up Source IP address */
    wiz_read_buf ( GAR0, addr, 4 );
}

void getSUBR ( uint8 *addr ) {
    wiz_read_buf ( SUBR0, addr, 4 );
}

void getSHAR ( uint8 *addr ) {
    wiz_read_buf ( SHAR0, addr, 6 );
}

void getSIPR ( uint8 *addr ) {
    wiz_read_buf ( SIPR0, addr, 4 );
}

void setMR ( uint8 val ) {
    IINCHIP_WRITE ( MR, val );
}

uint8 getIR ( void ) { /* This function gets Interrupt register in common register */
    return IINCHIP_READ ( IR );
}

void setRTR ( uint16 timeout ) { /* This function sets up Retransmission time.*/
    IINCHIP_WRITE ( RTR0, ( uint8 ) ( ( timeout & 0xff00 ) >> 8 ) );
    IINCHIP_WRITE ( RTR1, ( uint8 ) ( timeout & 0x00ff ) );
}

void setRCR ( uint8 retry ) { /* This function set the number of Retransmission */
    IINCHIP_WRITE ( WIZ_RCR, retry );
}

void clearIR ( uint8 mask ) { /* Set the interrupt mask Enable/Disable appropriate Interrupt('1': interrupt enable) */
    IINCHIP_WRITE ( IR, ~mask | getIR() ); /* must be setted 0x10 */
}

void setSn_MSS ( SOCKET s, uint16 Sn_MSSR ) { /* Set the maximum segment size of TCP in Active Mode) */
    IINCHIP_WRITE ( Sn_MSSR0 ( s ), ( uint8 ) ( ( Sn_MSSR & 0xff00 ) >> 8 ) );
    IINCHIP_WRITE ( Sn_MSSR1 ( s ), ( uint8 ) ( Sn_MSSR & 0x00ff ) );
}

void setSn_TTL ( SOCKET s, uint8 ttl ) {
    IINCHIP_WRITE ( Sn_TTL ( s ) , ttl );
}

uint8 getSn_IR ( SOCKET s ) { /* get socket interrupt status */
    return IINCHIP_READ ( Sn_IR ( s ) );
}

uint8 getSn_SR ( SOCKET s ) { /* get socket status */
    return IINCHIP_READ ( Sn_SR ( s ) );
}

uint16 getSn_TX_FSR ( SOCKET s ) { /* get socket TX free buf size */
    uint16 val = 0, val1 = 0;

    do {
        val1 = IINCHIP_READ ( Sn_TX_FSR0 ( s ) );
        val1 = ( val1 << 8 ) + IINCHIP_READ ( Sn_TX_FSR1 ( s ) );

        if ( val1 != 0 ) {
            val = IINCHIP_READ ( Sn_TX_FSR0 ( s ) );
            val = ( val << 8 ) + IINCHIP_READ ( Sn_TX_FSR1 ( s ) );
        }
    } while ( val != val1 );

    return val;
}

uint16 getSn_RX_RSR ( SOCKET s ) { /* get socket RX recv buf size */
    uint16 val = 0, val1 = 0;

    do {
        val1 = IINCHIP_READ ( Sn_RX_RSR0 ( s ) );
        val1 = ( val1 << 8 ) + IINCHIP_READ ( Sn_RX_RSR1 ( s ) );

        if ( val1 != 0 ) {
            val = IINCHIP_READ ( Sn_RX_RSR0 ( s ) );
            val = ( val << 8 ) + IINCHIP_READ ( Sn_RX_RSR1 ( s ) );
        }
    } while ( val != val1 );

    return val;
}

void send_data_processing ( SOCKET s, uint8 *data, uint16 len ) { /* Called by send and sendto */
    uint16 ptr = 0;
    uint32 addrbsb = 0;

    if ( len == 0 ) {
        printf ( "CH: %d Unexpected1 length 0\r\n", s );
        return;
    }

    ptr = IINCHIP_READ ( Sn_TX_WR0 ( s ) );
    ptr = ( ( ptr & 0x00ff ) << 8 ) + IINCHIP_READ ( Sn_TX_WR1 ( s ) );
    addrbsb = ( uint32 ) ( ptr << 8 ) + ( s << 5 ) + 0x10;
    wiz_write_buf ( addrbsb, data, len );
    ptr += len;
    IINCHIP_WRITE ( Sn_TX_WR0 ( s ) , ( uint8 ) ( ( ptr & 0xff00 ) >> 8 ) );
    IINCHIP_WRITE ( Sn_TX_WR1 ( s ), ( uint8 ) ( ptr & 0x00ff ) );
}

void recv_data_processing ( SOCKET s, uint8 *data, uint16 len ) { /* This function is being called by recv */
    uint16 ptr = 0;
    uint32 addrbsb = 0;

    if ( len == 0 ) {
        printf ( "CH: %d Unexpected2 length 0\r\n", s );
        return;
    }

    ptr = IINCHIP_READ ( Sn_RX_RD0 ( s ) );
    ptr = ( ( ptr & 0x00ff ) << 8 ) + IINCHIP_READ ( Sn_RX_RD1 ( s ) );
    addrbsb = ( uint32 ) ( ptr << 8 ) + ( s << 5 ) + 0x18;
    wiz_read_buf ( addrbsb, data, len );
    ptr += len;
    IINCHIP_WRITE ( Sn_RX_RD0 ( s ), ( uint8 ) ( ( ptr & 0xff00 ) >> 8 ) );
    IINCHIP_WRITE ( Sn_RX_RD1 ( s ), ( uint8 ) ( ptr & 0x00ff ) );
}

void setSn_IR ( uint8 s, uint8 val ) {
    IINCHIP_WRITE ( Sn_IR ( s ), val );
}