#include "sys.h"
#include "mmc_sd.h"
#include "delay.h"
#include "usart.h"

#define MOSI_H GPIO_SetBits(GPIOC, GPIO_Pin_5)
#define MOSI_L GPIO_ResetBits(GPIOC, GPIO_Pin_5)
#define SCLK_H GPIO_SetBits(GPIOC, GPIO_Pin_4)
#define SCLK_L GPIO_ResetBits(GPIOC, GPIO_Pin_4)
#define MISO   GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_3)

u8 SD_SPI_ReadWriteByte ( u8 dt ) {
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

u8 SD_Type = 0; /* SD�������� */

void SD_SPI_Init ( void ) { /* SPIӲ�����ʼ�� */
    GPIO_InitTypeDef GPIO_InitStructure;
    RCC_APB2PeriphClockCmd ( RCC_APB2Periph_GPIOC, ENABLE );
    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_13 | GPIO_Pin_5 | GPIO_Pin_4;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP; /* ������� */
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init ( GPIOC, &GPIO_InitStructure );
    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_3;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IPU;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init ( GPIOC, &GPIO_InitStructure );
    delay_ms ( 2 );
    SD_CS = 1;
}

void SD_DisSelect ( void ) { /* ȡ��ѡ���ͷ�SPI���� */
    SD_CS = 1;
    SD_SPI_ReadWriteByte ( 0xff );
}

u8 SD_Select ( void ) { /* ѡ��sd�����ȴ���׼����ϡ�����0��ʾ׼���ɹ���1��ʾʧ�� */
    SD_CS = 0;

    if ( SD_WaitReady() == 0 ) {
        return 0;
    }

    SD_DisSelect();
    return 1;
}

u8 SD_WaitReady ( void ) { /* �ȴ���׼���ã�����0��ʾ׼����ϣ��������Ǵ��� */
    u32 t = 0;

    do {
        if ( SD_SPI_ReadWriteByte ( 0XFF ) == 0XFF ) {
            return 0;
        }

        t++;
    } while ( t < 0XFFFFFF );

    return 1;
}

u8 SD_GetResponse ( u8 Response ) { /* �ȴ�SD����Ӧ������Response��ʾҪ�õ��Ļ�Ӧֵ������0��ʾ�ɹ��õ��˸û�Ӧֵ����������ʧ�� */
    u16 Count = 0xFFF;

    while ( ( SD_SPI_ReadWriteByte ( 0XFF ) != Response ) && Count ) {
        Count--;
    }

    if ( Count == 0 ) {
        return MSD_RESPONSE_FAILURE;
    } else {
        return MSD_RESPONSE_NO_ERROR;
    }
}

u8 SD_RecvData ( u8 *buf, u16 len ) { /* ��sd����ȡһ�����ݰ������ݡ�����buf�����ݻ�������len��Ҫ��ȡ�����ݳ��ȡ�����0��ʾ�ɹ�����������ʧ�� */
    if ( SD_GetResponse ( 0xFE ) ) {
        return 1; /* �ȴ�SD������������ʼ����0xFE */
    }

    while ( len-- ) { /* ��ʼ�������� */
        *buf = SD_SPI_ReadWriteByte ( 0xFF );
        buf++;
    }

    /* ������2��αCRC(dummy CRC) */
    SD_SPI_ReadWriteByte ( 0xFF );
    SD_SPI_ReadWriteByte ( 0xFF );
    return 0; /* ��ȡ�ɹ� */
}

u8 SD_SendBlock ( u8 *buf, u8 cmd ) { /* ��sd��д��һ�����ݰ�������(512�ֽ�)������buf�����ݻ�������cmd��ָ�����0��ʾ�ɹ�����������ʧ�� */
    u16 t;

    if ( SD_WaitReady() ) {
        return 1;
    }

    SD_SPI_ReadWriteByte ( cmd );

    if ( cmd != 0XFD ) { /* ���ǽ���ָ�� */
        for ( t = 0; t < 512; t++ ) {
            SD_SPI_ReadWriteByte ( buf[t] );
        }

        SD_SPI_ReadWriteByte ( 0xFF ); /* ����crc */
        SD_SPI_ReadWriteByte ( 0xFF );
        t = SD_SPI_ReadWriteByte ( 0xFF ); /* ������Ӧ */

        if ( ( t & 0x1F ) != 0x05 ) {
            return 2; /* ��Ӧ���� */
        }
    }

    return 0; /* д��ɹ� */
}

u8 SD_SendCmd ( u8 cmd, u32 arg, u8 crc ) { /* ��SD������һ���������cmd�����arg�����������crc��У��ֵ���ú�������SD������Ӧ */
    u8 r1;
    u8 Retry = 0;
    SD_DisSelect(); /* ȡ���ϴ�Ƭѡ */

    if ( SD_Select() ) {
        return 0XFF; /* ƬѡʧЧ */
    }

    SD_SPI_ReadWriteByte ( cmd | 0x40 ); /* �ֱ�д������ */
    SD_SPI_ReadWriteByte ( arg >> 24 );
    SD_SPI_ReadWriteByte ( arg >> 16 );
    SD_SPI_ReadWriteByte ( arg >> 8 );
    SD_SPI_ReadWriteByte ( arg );
    SD_SPI_ReadWriteByte ( crc );

    if ( cmd == CMD12 ) {
        SD_SPI_ReadWriteByte ( 0xff ); /* Skip a stuff byte when stop reading */
    }

    Retry = 0X1F;

    do {
        r1 = SD_SPI_ReadWriteByte ( 0xFF );
    } while ( ( r1 & 0X80 ) && Retry-- );

    return r1; /* ����״ֵ̬ */
}

u8 SD_GetCID ( u8 *cid_data ) { /* ��ȡSD����CID��Ϣ��������������Ϣ������cid_data�Ǵ��CID���ڴ�(����16Byte)������0��ʾNO_ERR��1��ʾ���� */
    u8 r1;
    r1 = SD_SendCmd ( CMD10, 0, 0x01 ); /* ��CMD10�����CID */

    if ( r1 == 0x00 ) {
        r1 = SD_RecvData ( cid_data, 16 ); /* ����16���ֽڵ����� */
    }

    SD_DisSelect(); /* ȡ��Ƭѡ */

    if ( r1 ) {
        return 1;
    } else {
        return 0;
    }
}

u8 SD_GetCSD ( u8 *csd_data ) { /* ��ȡSD����CSD��Ϣ�������������ٶ���Ϣ������cid_data�Ǵ��CSD���ڴ�(����16Byte)������0��ʾNO_ERR��1��ʾ���� */
    u8 r1;
    r1 = SD_SendCmd ( CMD9, 0, 0x01 ); /* ��CMD9�����CSD */

    if ( r1 == 0 ) {
        r1 = SD_RecvData ( csd_data, 16 ); /* ����16���ֽڵ����� */
    }

    SD_DisSelect(); /* ȡ��Ƭѡ */

    if ( r1 ) {
        return 1;
    } else {
        return 0;
    }
}

u32 SD_GetSectorCount ( void ) { /* ��ȡSD������������(������)������SD��������(������/512�ֽ�)��0��ʾ����ÿ�������ֽ�����Ϊ512�������ʼ������ͨ�� */
    u8 csd[16];
    u32 Capacity;
    u8 n;
    u16 csize;

    if ( SD_GetCSD ( csd ) != 0 ) {
        return 0;
    }

    /* ���ΪSDHC������������ķ�ʽ���� */
    if ( ( csd[0] & 0xC0 ) == 0x40 ) { /* V2.00�Ŀ� */
        csize = csd[9] + ( ( u16 ) csd[8] << 8 ) + 1;
        Capacity = ( u32 ) csize << 10; /* �õ������� */
    } else { /* V1.XX�Ŀ� */
        n = ( csd[5] & 15 ) + ( ( csd[10] & 128 ) >> 7 ) + ( ( csd[9] & 3 ) << 1 ) + 2;
        csize = ( csd[8] >> 6 ) + ( ( u16 ) csd[7] << 2 ) + ( ( u16 ) ( csd[6] & 3 ) << 10 ) + 1;
        Capacity = ( u32 ) csize << ( n - 9 ); /* �õ������� */
    }

    return Capacity;
}

u8 SD_Idle_Sta ( void ) {
    u16 i;
    u8 retry;

    for ( i = 0; i < 0xf00; i++ ); /* ����ʱ���ȴ�SD���ϵ���� */

    for ( i = 0; i < 10; i++ ) {
        SD_SPI_ReadWriteByte ( 0xFF );
    }

    /* SD����λ��idle��ѭ����������CMD0��ֱ��SD������0x01������IDLE״̬����ʱ��ֱ���˳� */
    retry = 0;

    do {
        i = SD_SendCmd ( CMD0, 0, 0x95 ); /* ����CMD0����SD������IDLE״̬ */
        retry++;
    } while ( ( i != 0x01 ) && ( retry < 200 ) );

    if ( retry == 200 ) {
        return 1; /* ʧ�� */
    }

    return 0; /* �ɹ� */
}

u8 SD_Initialize ( void ) { /* ��ʼ��SD�� */
    u8 r1; /* ���SD���ķ���ֵ */
    u16 retry; /* �������г�ʱ���� */
    u8 buf[4];
    u16 i;
    SD_SPI_Init(); /* ��ʼ��IO */

    for ( i = 0; i < 10; i++ ) {
        SD_SPI_ReadWriteByte ( 0XFF );
    }

    retry = 20;

    do {
        r1 = SD_SendCmd ( CMD0, 0, 0x95 ); /* ����IDLE״̬ */
    } while ( ( r1 != 0X01 ) && retry-- );

    SD_Type = 0; /* Ĭ�����޿� */

    if ( r1 == 0X01 ) {
        if ( SD_SendCmd ( CMD8, 0x1AA, 0x87 ) == 1 ) { /* SD V2.0 */
            for ( i = 0; i < 4; i++ ) {
                buf[i] = SD_SPI_ReadWriteByte ( 0XFF ); /* Get trailing return value of R7 resp */
            }

            if ( buf[2] == 0X01 && buf[3] == 0XAA ) { /* ���Ƿ�֧��2.7��3.6V */
                retry = 0XFFFE;

                do {
                    SD_SendCmd ( CMD55, 0, 0X01 ); /* ����CMD55 */
                    r1 = SD_SendCmd ( CMD41, 0x40000000, 0X01 ); /* ����CMD41 */
                } while ( r1 && retry-- );

                if ( retry && SD_SendCmd ( CMD58, 0, 0X01 ) == 0 ) { /* ����SD2.0���汾��ʼ */
                    for ( i = 0; i < 4; i++ ) {
                        buf[i] = SD_SPI_ReadWriteByte ( 0XFF ); /* �õ�OCRֵ */
                    }

                    if ( buf[0] & 0x40 ) {
                        SD_Type = SD_TYPE_V2HC; /* ���CCS */
                    } else {
                        SD_Type = SD_TYPE_V2;
                    }
                }
            }
        } else {
            SD_SendCmd ( CMD55, 0, 0X01 );  /* ����CMD55 */
            r1 = SD_SendCmd ( CMD41, 0, 0X01 ); /* ����CMD41 */

            if ( r1 <= 1 ) {
                SD_Type = SD_TYPE_V1;
                retry = 0XFFFE;

                do { /* �ȴ��˳�IDLEģʽ */
                    SD_SendCmd ( CMD55, 0, 0X01 ); /* ����CMD55 */
                    r1 = SD_SendCmd ( CMD41, 0, 0X01 ); /* ����CMD41 */
                } while ( r1 && retry-- );
            } else {
                SD_Type = SD_TYPE_MMC;
                retry = 0XFFFE;

                do { /* �ȴ��˳�IDLEģʽ */
                    r1 = SD_SendCmd ( CMD1, 0, 0X01 ); /* ����CMD1 */
                } while ( r1 && retry-- );
            }

            if ( retry == 0 || SD_SendCmd ( CMD16, 512, 0X01 ) != 0 ) {
                SD_Type = SD_TYPE_ERR; /* ����Ŀ� */
            }
        }
    }

    SD_DisSelect(); /* ȡ��Ƭѡ */

    if ( SD_Type ) {
        return 0;
    } else if ( r1 ) {
        return r1;
    }

    return 0xaa;
}

u8 SD_ReadDisk ( u8 *buf, u32 sector, u8 cnt ) { /* ��SD��������buf�����ݻ�������sector��������cnt��������������0��ʾok����������ʧ�� */
    u8 r1;

    if ( SD_Type != SD_TYPE_V2HC ) {
        sector <<= 9; /* ת��Ϊ�ֽڵ�ַ */
    }

    if ( cnt == 1 ) {
        r1 = SD_SendCmd ( CMD17, sector, 0X01 ); /* ������ */

        if ( r1 == 0 ) { /* ָ��ͳɹ� */
            r1 = SD_RecvData ( buf, 512 ); /* ����512���ֽ� */
        }
    } else {
        r1 = SD_SendCmd ( CMD18, sector, 0X01 ); /* ���������� */

        do {
            r1 = SD_RecvData ( buf, 512 ); /* ����512���ֽ� */
            buf += 512;
        } while ( --cnt && r1 == 0 );

        SD_SendCmd ( CMD12, 0, 0X01 ); /* ����ֹͣ���� */
    }

    SD_DisSelect();
    return r1;
}

u8 SD_WriteDisk ( u8 *buf, u32 sector, u8 cnt ) { /* дSD��������buf�����ݻ�������sector����ʼ������cnt��������������0��ʾok����������ʧ�� */
    u8 r1;

    if ( SD_Type != SD_TYPE_V2HC ) {
        sector *= 512; /* ת��Ϊ�ֽڵ�ַ */
    }

    if ( cnt == 1 ) {
        r1 = SD_SendCmd ( CMD24, sector, 0X01 ); /* ������ */

        if ( r1 == 0 ) { /* ָ��ͳɹ� */
            r1 = SD_SendBlock ( buf, 0xFE ); /* д512���ֽ� */
        }
    } else {
        if ( SD_Type != SD_TYPE_MMC ) {
            SD_SendCmd ( CMD55, 0, 0X01 );
            SD_SendCmd ( CMD23, cnt, 0X01 ); /* ����ָ�� */
        }

        r1 = SD_SendCmd ( CMD25, sector, 0X01 ); /* ���������� */

        if ( r1 == 0 ) {
            do {
                r1 = SD_SendBlock ( buf, 0xFC ); /* ����512���ֽ� */
                buf += 512;
            } while ( --cnt && r1 == 0 );

            r1 = SD_SendBlock ( 0, 0xFD ); /* ����512���ֽ� */
        }
    }

    SD_DisSelect(); /* ȡ��Ƭѡ */
    return r1;
}