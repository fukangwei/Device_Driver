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

u8 SD_Type = 0; /* SD卡的类型 */

void SD_SPI_Init ( void ) { /* SPI硬件层初始化 */
    GPIO_InitTypeDef GPIO_InitStructure;
    RCC_APB2PeriphClockCmd ( RCC_APB2Periph_GPIOC, ENABLE );
    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_13 | GPIO_Pin_5 | GPIO_Pin_4;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP; /* 推挽输出 */
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init ( GPIOC, &GPIO_InitStructure );
    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_3;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IPU;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init ( GPIOC, &GPIO_InitStructure );
    delay_ms ( 2 );
    SD_CS = 1;
}

void SD_DisSelect ( void ) { /* 取消选择，释放SPI总线 */
    SD_CS = 1;
    SD_SPI_ReadWriteByte ( 0xff );
}

u8 SD_Select ( void ) { /* 选择sd卡，等待卡准备完毕。返回0表示准备成功，1表示失败 */
    SD_CS = 0;

    if ( SD_WaitReady() == 0 ) {
        return 0;
    }

    SD_DisSelect();
    return 1;
}

u8 SD_WaitReady ( void ) { /* 等待卡准备好，返回0表示准备完毕，其他则是错误 */
    u32 t = 0;

    do {
        if ( SD_SPI_ReadWriteByte ( 0XFF ) == 0XFF ) {
            return 0;
        }

        t++;
    } while ( t < 0XFFFFFF );

    return 1;
}

u8 SD_GetResponse ( u8 Response ) { /* 等待SD卡回应，参数Response表示要得到的回应值。返回0表示成功得到了该回应值，其他则是失败 */
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

u8 SD_RecvData ( u8 *buf, u16 len ) { /* 从sd卡读取一个数据包的内容。参数buf是数据缓存区，len是要读取的数据长度。返回0表示成功，其他则是失败 */
    if ( SD_GetResponse ( 0xFE ) ) {
        return 1; /* 等待SD卡发回数据起始令牌0xFE */
    }

    while ( len-- ) { /* 开始接收数据 */
        *buf = SD_SPI_ReadWriteByte ( 0xFF );
        buf++;
    }

    /* 下面是2个伪CRC(dummy CRC) */
    SD_SPI_ReadWriteByte ( 0xFF );
    SD_SPI_ReadWriteByte ( 0xFF );
    return 0; /* 读取成功 */
}

u8 SD_SendBlock ( u8 *buf, u8 cmd ) { /* 向sd卡写入一个数据包的内容(512字节)。参数buf是数据缓存区，cmd是指令。返回0表示成功，其他则是失败 */
    u16 t;

    if ( SD_WaitReady() ) {
        return 1;
    }

    SD_SPI_ReadWriteByte ( cmd );

    if ( cmd != 0XFD ) { /* 不是结束指令 */
        for ( t = 0; t < 512; t++ ) {
            SD_SPI_ReadWriteByte ( buf[t] );
        }

        SD_SPI_ReadWriteByte ( 0xFF ); /* 忽略crc */
        SD_SPI_ReadWriteByte ( 0xFF );
        t = SD_SPI_ReadWriteByte ( 0xFF ); /* 接收响应 */

        if ( ( t & 0x1F ) != 0x05 ) {
            return 2; /* 响应错误 */
        }
    }

    return 0; /* 写入成功 */
}

u8 SD_SendCmd ( u8 cmd, u32 arg, u8 crc ) { /* 向SD卡发送一个命令。参数cmd是命令，arg是命令参数，crc是校验值。该函数返回SD卡的响应 */
    u8 r1;
    u8 Retry = 0;
    SD_DisSelect(); /* 取消上次片选 */

    if ( SD_Select() ) {
        return 0XFF; /* 片选失效 */
    }

    SD_SPI_ReadWriteByte ( cmd | 0x40 ); /* 分别写入命令 */
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

    return r1; /* 返回状态值 */
}

u8 SD_GetCID ( u8 *cid_data ) { /* 获取SD卡的CID信息，包括制造商信息。参数cid_data是存放CID的内存(至少16Byte)。返回0表示NO_ERR，1表示错误 */
    u8 r1;
    r1 = SD_SendCmd ( CMD10, 0, 0x01 ); /* 发CMD10命令，读CID */

    if ( r1 == 0x00 ) {
        r1 = SD_RecvData ( cid_data, 16 ); /* 接收16个字节的数据 */
    }

    SD_DisSelect(); /* 取消片选 */

    if ( r1 ) {
        return 1;
    } else {
        return 0;
    }
}

u8 SD_GetCSD ( u8 *csd_data ) { /* 获取SD卡的CSD信息，包括容量和速度信息。参数cid_data是存放CSD的内存(至少16Byte)。返回0表示NO_ERR，1表示错误 */
    u8 r1;
    r1 = SD_SendCmd ( CMD9, 0, 0x01 ); /* 发CMD9命令，读CSD */

    if ( r1 == 0 ) {
        r1 = SD_RecvData ( csd_data, 16 ); /* 接收16个字节的数据 */
    }

    SD_DisSelect(); /* 取消片选 */

    if ( r1 ) {
        return 1;
    } else {
        return 0;
    }
}

u32 SD_GetSectorCount ( void ) { /* 获取SD卡的总扇区数(扇区数)。返回SD卡的容量(扇区数/512字节)，0表示出错。每扇区的字节数必为512，否则初始化不能通过 */
    u8 csd[16];
    u32 Capacity;
    u8 n;
    u16 csize;

    if ( SD_GetCSD ( csd ) != 0 ) {
        return 0;
    }

    /* 如果为SDHC卡，按照下面的方式计算 */
    if ( ( csd[0] & 0xC0 ) == 0x40 ) { /* V2.00的卡 */
        csize = csd[9] + ( ( u16 ) csd[8] << 8 ) + 1;
        Capacity = ( u32 ) csize << 10; /* 得到扇区数 */
    } else { /* V1.XX的卡 */
        n = ( csd[5] & 15 ) + ( ( csd[10] & 128 ) >> 7 ) + ( ( csd[9] & 3 ) << 1 ) + 2;
        csize = ( csd[8] >> 6 ) + ( ( u16 ) csd[7] << 2 ) + ( ( u16 ) ( csd[6] & 3 ) << 10 ) + 1;
        Capacity = ( u32 ) csize << ( n - 9 ); /* 得到扇区数 */
    }

    return Capacity;
}

u8 SD_Idle_Sta ( void ) {
    u16 i;
    u8 retry;

    for ( i = 0; i < 0xf00; i++ ); /* 纯延时，等待SD卡上电完成 */

    for ( i = 0; i < 10; i++ ) {
        SD_SPI_ReadWriteByte ( 0xFF );
    }

    /* SD卡复位到idle，循环连续发送CMD0，直到SD卡返回0x01，进入IDLE状态；超时则直接退出 */
    retry = 0;

    do {
        i = SD_SendCmd ( CMD0, 0, 0x95 ); /* 发送CMD0，让SD卡进入IDLE状态 */
        retry++;
    } while ( ( i != 0x01 ) && ( retry < 200 ) );

    if ( retry == 200 ) {
        return 1; /* 失败 */
    }

    return 0; /* 成功 */
}

u8 SD_Initialize ( void ) { /* 初始化SD卡 */
    u8 r1; /* 存放SD卡的返回值 */
    u16 retry; /* 用来进行超时计数 */
    u8 buf[4];
    u16 i;
    SD_SPI_Init(); /* 初始化IO */

    for ( i = 0; i < 10; i++ ) {
        SD_SPI_ReadWriteByte ( 0XFF );
    }

    retry = 20;

    do {
        r1 = SD_SendCmd ( CMD0, 0, 0x95 ); /* 进入IDLE状态 */
    } while ( ( r1 != 0X01 ) && retry-- );

    SD_Type = 0; /* 默认是无卡 */

    if ( r1 == 0X01 ) {
        if ( SD_SendCmd ( CMD8, 0x1AA, 0x87 ) == 1 ) { /* SD V2.0 */
            for ( i = 0; i < 4; i++ ) {
                buf[i] = SD_SPI_ReadWriteByte ( 0XFF ); /* Get trailing return value of R7 resp */
            }

            if ( buf[2] == 0X01 && buf[3] == 0XAA ) { /* 卡是否支持2.7至3.6V */
                retry = 0XFFFE;

                do {
                    SD_SendCmd ( CMD55, 0, 0X01 ); /* 发送CMD55 */
                    r1 = SD_SendCmd ( CMD41, 0x40000000, 0X01 ); /* 发送CMD41 */
                } while ( r1 && retry-- );

                if ( retry && SD_SendCmd ( CMD58, 0, 0X01 ) == 0 ) { /* 鉴别SD2.0卡版本开始 */
                    for ( i = 0; i < 4; i++ ) {
                        buf[i] = SD_SPI_ReadWriteByte ( 0XFF ); /* 得到OCR值 */
                    }

                    if ( buf[0] & 0x40 ) {
                        SD_Type = SD_TYPE_V2HC; /* 检查CCS */
                    } else {
                        SD_Type = SD_TYPE_V2;
                    }
                }
            }
        } else {
            SD_SendCmd ( CMD55, 0, 0X01 );  /* 发送CMD55 */
            r1 = SD_SendCmd ( CMD41, 0, 0X01 ); /* 发送CMD41 */

            if ( r1 <= 1 ) {
                SD_Type = SD_TYPE_V1;
                retry = 0XFFFE;

                do { /* 等待退出IDLE模式 */
                    SD_SendCmd ( CMD55, 0, 0X01 ); /* 发送CMD55 */
                    r1 = SD_SendCmd ( CMD41, 0, 0X01 ); /* 发送CMD41 */
                } while ( r1 && retry-- );
            } else {
                SD_Type = SD_TYPE_MMC;
                retry = 0XFFFE;

                do { /* 等待退出IDLE模式 */
                    r1 = SD_SendCmd ( CMD1, 0, 0X01 ); /* 发送CMD1 */
                } while ( r1 && retry-- );
            }

            if ( retry == 0 || SD_SendCmd ( CMD16, 512, 0X01 ) != 0 ) {
                SD_Type = SD_TYPE_ERR; /* 错误的卡 */
            }
        }
    }

    SD_DisSelect(); /* 取消片选 */

    if ( SD_Type ) {
        return 0;
    } else if ( r1 ) {
        return r1;
    }

    return 0xaa;
}

u8 SD_ReadDisk ( u8 *buf, u32 sector, u8 cnt ) { /* 读SD卡。参数buf是数据缓存区，sector是扇区，cnt是扇区数。返回0表示ok，其他则是失败 */
    u8 r1;

    if ( SD_Type != SD_TYPE_V2HC ) {
        sector <<= 9; /* 转换为字节地址 */
    }

    if ( cnt == 1 ) {
        r1 = SD_SendCmd ( CMD17, sector, 0X01 ); /* 读命令 */

        if ( r1 == 0 ) { /* 指令发送成功 */
            r1 = SD_RecvData ( buf, 512 ); /* 接收512个字节 */
        }
    } else {
        r1 = SD_SendCmd ( CMD18, sector, 0X01 ); /* 连续读命令 */

        do {
            r1 = SD_RecvData ( buf, 512 ); /* 接收512个字节 */
            buf += 512;
        } while ( --cnt && r1 == 0 );

        SD_SendCmd ( CMD12, 0, 0X01 ); /* 发送停止命令 */
    }

    SD_DisSelect();
    return r1;
}

u8 SD_WriteDisk ( u8 *buf, u32 sector, u8 cnt ) { /* 写SD卡。参数buf是数据缓存区，sector是起始扇区，cnt是扇区数。返回0表示ok，其他则是失败 */
    u8 r1;

    if ( SD_Type != SD_TYPE_V2HC ) {
        sector *= 512; /* 转换为字节地址 */
    }

    if ( cnt == 1 ) {
        r1 = SD_SendCmd ( CMD24, sector, 0X01 ); /* 读命令 */

        if ( r1 == 0 ) { /* 指令发送成功 */
            r1 = SD_SendBlock ( buf, 0xFE ); /* 写512个字节 */
        }
    } else {
        if ( SD_Type != SD_TYPE_MMC ) {
            SD_SendCmd ( CMD55, 0, 0X01 );
            SD_SendCmd ( CMD23, cnt, 0X01 ); /* 发送指令 */
        }

        r1 = SD_SendCmd ( CMD25, sector, 0X01 ); /* 连续读命令 */

        if ( r1 == 0 ) {
            do {
                r1 = SD_SendBlock ( buf, 0xFC ); /* 接收512个字节 */
                buf += 512;
            } while ( --cnt && r1 == 0 );

            r1 = SD_SendBlock ( 0, 0xFD ); /* 接收512个字节 */
        }
    }

    SD_DisSelect(); /* 取消片选 */
    return r1;
}