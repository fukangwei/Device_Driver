#include "flash.h"
#include "spi.h"
#include "delay.h"

/* 4KbytesΪһ��Sector��16������Ϊ1��Block��W25X16����Ϊ2M�ֽڣ�����32��Block��512��Sector */
void SPI_Flash_Init ( void ) {
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_4; 
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;  
    GPIO_Init ( GPIOA, &GPIO_InitStructure );
    GPIO_SetBits ( GPIOA, GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_4 );
    SPIx_Init(); 
}

u8 SPI_Flash_ReadSR ( void ) { /* ��ȡSPI_FLASH��״̬�Ĵ��� */
    u8 byte = 0;
    SPI_FLASH_CS = 0; /* ʹ������ */
    SPIx_ReadWriteByte ( W25X_ReadStatusReg ); /* ���Ͷ�ȡ״̬�Ĵ������� */
    byte = SPIx_ReadWriteByte ( 0Xff ); /* ��ȡһ���ֽ� */
    SPI_FLASH_CS = 1; /* ȡ��Ƭѡ */
    return byte;
}

void SPI_FLASH_Write_SR ( u8 sr ) { /* дSPI_FLASH״̬�Ĵ��� */
    SPI_FLASH_CS = 0; /* ʹ������ */
    SPIx_ReadWriteByte ( W25X_WriteStatusReg ); /* ����дȡ״̬�Ĵ������� */
    SPIx_ReadWriteByte ( sr ); /* д��һ���ֽ� */
    SPI_FLASH_CS = 1; /* ȡ��Ƭѡ */
}

void SPI_FLASH_Write_Enable ( void ) { /* SPI_FLASHдʹ�� */
    SPI_FLASH_CS = 0; /* ʹ������ */
    SPIx_ReadWriteByte ( W25X_WriteEnable ); /* ����дʹ�� */
    SPI_FLASH_CS = 1; /* ȡ��Ƭѡ */
}

void SPI_FLASH_Write_Disable ( void ) { /* SPI_FLASHд��ֹ */
    SPI_FLASH_CS = 0; /* ʹ������ */
    SPIx_ReadWriteByte ( W25X_WriteDisable ); /* ����д��ָֹ�� */
    SPI_FLASH_CS = 1; /* ȡ��Ƭѡ */
}

u16 SPI_Flash_ReadID ( void ) { /* ��ȡоƬID��W25X16��ID��0XEF14 */
    u16 Temp = 0;
    SPI_FLASH_CS = 0;
    SPIx_ReadWriteByte ( 0x90 ); /* ���Ͷ�ȡID���� */
    SPIx_ReadWriteByte ( 0x00 );
    SPIx_ReadWriteByte ( 0x00 );
    SPIx_ReadWriteByte ( 0x00 );
    Temp |= SPIx_ReadWriteByte ( 0xFF ) << 8;
    Temp |= SPIx_ReadWriteByte ( 0xFF );
    SPI_FLASH_CS = 1;
    return Temp;
}

void SPI_Flash_Read ( u8 *pBuffer, u32 ReadAddr, u16 NumByteToRead ) { /* ��ָ����ַ��ʼ��ȡָ�����ȵ����� */
    u16 i;
    SPI_FLASH_CS = 0; /* ʹ������ */
    SPIx_ReadWriteByte ( W25X_ReadData ); /* ���Ͷ�ȡ���� */
    SPIx_ReadWriteByte ( ( u8 ) ( ( ReadAddr ) >> 16 ) ); /* ����24bit��ַ */
    SPIx_ReadWriteByte ( ( u8 ) ( ( ReadAddr ) >> 8 ) );
    SPIx_ReadWriteByte ( ( u8 ) ReadAddr );

    for ( i = 0; i < NumByteToRead; i++ ) {
        pBuffer[i] = SPIx_ReadWriteByte ( 0XFF ); /* ѭ������ */
    }

    SPI_FLASH_CS = 1; /* ȡ��Ƭѡ */
}

/* SPI��һҳ(0��65535)��д������256���ֽڵ����ݡ�����pBuffer�����ݴ洢����WriteAddr�ǿ�ʼд��ĵ�ַ��
   NumByteToWrite��Ҫд����ֽ���(���256)��������Ӧ�ó�����ҳ��ʣ���ֽ��� */
void SPI_Flash_Write_Page ( u8 *pBuffer, u32 WriteAddr, u16 NumByteToWrite ) {
    u16 i;
    SPI_FLASH_Write_Enable(); 
    SPI_FLASH_CS = 0; /* ʹ������ */
    SPIx_ReadWriteByte ( W25X_PageProgram ); /* ����дҳ���� */
    SPIx_ReadWriteByte ( ( u8 ) ( ( WriteAddr ) >> 16 ) ); /* ����24bit��ַ */
    SPIx_ReadWriteByte ( ( u8 ) ( ( WriteAddr ) >> 8 ) );
    SPIx_ReadWriteByte ( ( u8 ) WriteAddr );

    for ( i = 0; i < NumByteToWrite; i++ ) {
        SPIx_ReadWriteByte ( pBuffer[i] ); /* ѭ��д�� */
    }

    SPI_FLASH_CS = 1; /* ȡ��Ƭѡ */
    SPI_Flash_Wait_Busy(); /* �ȴ�д����� */
}

/* �޼���дSPI_FLASH������ȷ����д�ĵ�ַ��Χ�ڵ�����ȫ��Ϊ0XFF�������ڷ�0XFF��д������ݽ�ʧ�ܣ������Զ���ҳ����
   ����pBuffer�����ݴ洢����WriteAddr�ǿ�ʼд��ĵ�ַ��NumByteToWrite��Ҫд����ֽ���(���65535) */
void SPI_Flash_Write_NoCheck ( u8 *pBuffer, u32 WriteAddr, u16 NumByteToWrite ) {
    u16 pageremain;
    pageremain = 256 - WriteAddr % 256; /* ��ҳʣ����ֽ��� */

    if ( NumByteToWrite <= pageremain ) {
        pageremain = NumByteToWrite; /* ������256���ֽ� */
    }

    while ( 1 ) {
        SPI_Flash_Write_Page ( pBuffer, WriteAddr, pageremain );

        if ( NumByteToWrite == pageremain ) {
            break; /* д������� */
        } else { /* NumByteToWrite > pageremain */
            pBuffer += pageremain;
            WriteAddr += pageremain;
            NumByteToWrite -= pageremain; /* ��ȥ�Ѿ�д���˵��ֽ��� */

            if ( NumByteToWrite > 256 ) {
                pageremain = 256; /* һ�ο���д��256���ֽ� */
            } else {
                pageremain = NumByteToWrite; /* ����256���ֽ��� */
            }
        }
    };
}

u8 SPI_FLASH_BUF[4096];
void SPI_Flash_Write ( u8 *pBuffer, u32 WriteAddr, u16 NumByteToWrite ) { /* ��ָ����ַ��ʼд��ָ�����ȵ����� */
    u32 secpos;
    u16 secoff;
    u16 secremain;
    u16 i;
    secpos = WriteAddr / 4096; /* ������ַ(0��511 for w25x16) */
    secoff = WriteAddr % 4096; /* �������ڵ�ƫ�� */
    secremain = 4096 - secoff; /* ����ʣ��ռ��С */

    if ( NumByteToWrite <= secremain ) {
        secremain = NumByteToWrite; /* ������4096���ֽ� */
    }

    while ( 1 ) {
        SPI_Flash_Read ( SPI_FLASH_BUF, secpos * 4096, 4096 ); /* ������������������ */

        for ( i = 0; i < secremain; i++ ) { /* У������ */
            if ( SPI_FLASH_BUF[secoff + i] != 0XFF ) {
                break; /* ��Ҫ���� */
            }
        }

        if ( i < secremain ) { /* ��Ҫ���� */
            SPI_Flash_Erase_Sector ( secpos ); /* ����������� */

            for ( i = 0; i < secremain; i++ ) { /* ���� */
                SPI_FLASH_BUF[i + secoff] = pBuffer[i];
            }

            SPI_Flash_Write_NoCheck ( SPI_FLASH_BUF, secpos * 4096, 4096 ); /* д���������� */
        } else {
            SPI_Flash_Write_NoCheck ( pBuffer, WriteAddr, secremain ); /* д�Ѿ������˵ģ�ֱ��д������ʣ������ */
        }

        if ( NumByteToWrite == secremain ) {
            break; /* д������� */
        } else { /* д��δ���� */
            secpos++; /* ������ַ��1 */
            secoff = 0; /* ƫ��λ��Ϊ0 */
            pBuffer += secremain; /* ָ��ƫ�� */
            WriteAddr += secremain; /* д��ַƫ�� */
            NumByteToWrite -= secremain; /* �ֽ����ݼ� */

            if ( NumByteToWrite > 4096 ) {
                secremain = 4096; /* ��һ����������д���� */
            } else {
                secremain = NumByteToWrite; /* ��һ����������д���� */
            }
        }
    };
}

void SPI_Flash_Erase_Chip ( void ) { /* ��������оƬ����Ƭ����ʱ�䣺W25X16Ϊ25s��W25X32Ϊ40s��W25X64Ϊ40s */
    SPI_FLASH_Write_Enable();
    SPI_Flash_Wait_Busy();
    SPI_FLASH_CS = 0; /* ʹ������ */
    SPIx_ReadWriteByte ( W25X_ChipErase ); /* ����Ƭ�������� */
    SPI_FLASH_CS = 1; /* ȡ��Ƭѡ */
    SPI_Flash_Wait_Busy(); /* �ȴ�оƬ�������� */
}

/* ����һ������������Dst_Addr��������ַ������һ������������ʱ����150ms */
void SPI_Flash_Erase_Sector ( u32 Dst_Addr ) {
    Dst_Addr *= 4096;
    SPI_FLASH_Write_Enable();
    SPI_Flash_Wait_Busy();
    SPI_FLASH_CS = 0; /* ʹ������ */
    SPIx_ReadWriteByte ( W25X_SectorErase ); /* ������������ָ�� */
    SPIx_ReadWriteByte ( ( u8 ) ( ( Dst_Addr ) >> 16 ) ); /* ����24bit��ַ */
    SPIx_ReadWriteByte ( ( u8 ) ( ( Dst_Addr ) >> 8 ) );
    SPIx_ReadWriteByte ( ( u8 ) Dst_Addr );
    SPI_FLASH_CS = 1; /* ȡ��Ƭѡ */
    SPI_Flash_Wait_Busy(); /* �ȴ�������� */
}

void SPI_Flash_Wait_Busy ( void ) { /* �ȴ����� */
    while ( ( SPI_Flash_ReadSR() & 0x01 ) == 0x01 ); /* �ȴ�BUSYλ��� */
}

void SPI_Flash_PowerDown ( void ) { /* �������ģʽ */
    SPI_FLASH_CS = 0; /* ʹ������ */
    SPIx_ReadWriteByte ( W25X_PowerDown ); /* ���͵������� */
    SPI_FLASH_CS = 1; /* ȡ��Ƭѡ */
    delay_us ( 3 ); /* �ȴ�TPD */
}

void SPI_Flash_WAKEUP ( void ) { /* ���� */
    SPI_FLASH_CS = 0; /* ʹ������ */
    SPIx_ReadWriteByte ( W25X_ReleasePowerDown );
    SPI_FLASH_CS = 1; /* ȡ��Ƭѡ */
    delay_us ( 3 ); /* �ȴ�TRES1 */
}