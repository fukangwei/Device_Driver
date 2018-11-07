#include <intrins.h>
#include <STC12C5A.H>
#include <string.h>
#include "mfrc522.h"

#define MAXRLEN 18

/*---------------------------------------------------------------------------------------------
 * 功能：寻卡，若成功则返回MI_OK
 * 参数：req_code是寻卡方式，0x52是寻感应区内所有符合14443A标准的卡，0x26是寻未进入休眠状态的卡
 *       pTagType是卡片类型代码。0x4400是Mifare_UltraLight，0x0400是Mifare_One(S50)，
 *       0x0200是Mifare_One(S70)，0x0800是Mifare_Pro(X)，0x4403是Mifare_DESFire
 *-------------------------------------------------------------------------------------------*/
char PcdRequest ( unsigned char req_code, unsigned char *pTagType ) {
    char status;
    unsigned int  unLen;
    unsigned char ucComMF522Buf[MAXRLEN];
    ClearBitMask ( Status2Reg, 0x08 );
    WriteRawRC ( BitFramingReg, 0x07 );
    SetBitMask ( TxControlReg, 0x03 );
    ucComMF522Buf[0] = req_code;
    status = PcdComMF522 ( PCD_TRANSCEIVE, ucComMF522Buf, 1, ucComMF522Buf, &unLen );

    if ( ( status == MI_OK ) && ( unLen == 0x10 ) ) {
        *pTagType = ucComMF522Buf[0];
        * ( pTagType + 1 ) = ucComMF522Buf[1];
    } else {
        status = MI_ERR;
    }

    return status;
}

char PcdAnticoll ( unsigned char *pSnr ) { /* 防冲突功能。参数pSnr是卡片序列号(4字节) */
    char status;
    unsigned char i, snr_check = 0;
    unsigned int  unLen;
    unsigned char ucComMF522Buf[MAXRLEN];
    ClearBitMask ( Status2Reg, 0x08 );
    WriteRawRC ( BitFramingReg, 0x00 );
    ClearBitMask ( CollReg, 0x80 );
    ucComMF522Buf[0] = PICC_ANTICOLL1;
    ucComMF522Buf[1] = 0x20;
    status = PcdComMF522 ( PCD_TRANSCEIVE, ucComMF522Buf, 2, ucComMF522Buf, &unLen );

    if ( status == MI_OK ) {
        for ( i = 0; i < 4; i++ ) {
            * ( pSnr + i )  = ucComMF522Buf[i];
            snr_check ^= ucComMF522Buf[i];
        }

        if ( snr_check != ucComMF522Buf[i] ) {
            status = MI_ERR;
        }
    }

    SetBitMask ( CollReg, 0x80 );
    return status;
}

char PcdSelect ( unsigned char *pSnr ) { /* 选定卡片。参数pSnr是卡片序列号(4字节) */
    char status;
    unsigned char i;
    unsigned int  unLen;
    unsigned char ucComMF522Buf[MAXRLEN];
    ucComMF522Buf[0] = PICC_ANTICOLL1;
    ucComMF522Buf[1] = 0x70;
    ucComMF522Buf[6] = 0;

    for ( i = 0; i < 4; i++ ) {
        ucComMF522Buf[i + 2] = * ( pSnr + i );
        ucComMF522Buf[6]  ^= * ( pSnr + i );
    }

    CalulateCRC ( ucComMF522Buf, 7, &ucComMF522Buf[7] );
    ClearBitMask ( Status2Reg, 0x08 );
    status = PcdComMF522 ( PCD_TRANSCEIVE, ucComMF522Buf, 9, ucComMF522Buf, &unLen );

    if ( ( status == MI_OK ) && ( unLen == 0x18 ) ) {
        status = MI_OK;
    } else {
        status = MI_ERR;
    }

    return status;
}

/*--------------------------------------------------------------------------------------------------------------------
 * 功能：验证卡片密码，若成功返回MI_OK
 * 参数：auth_mode是密码验证模式，0x60是验证A密钥，0x61是验证B密钥；addr是块地址；pKey是密码；pSnr是卡片序列号(4字节)
---------------------------------------------------------------------------------------------------------------------*/
char PcdAuthState ( unsigned char auth_mode, unsigned char addr, unsigned char *pKey, unsigned char *pSnr ) {
    char status;
    unsigned int  unLen;
    unsigned char i, ucComMF522Buf[MAXRLEN];
    ucComMF522Buf[0] = auth_mode;
    ucComMF522Buf[1] = addr;

    for ( i = 0; i < 6; i++ ) {
        ucComMF522Buf[i + 2] = * ( pKey + i );
    }

    for ( i = 0; i < 6; i++ ) {
        ucComMF522Buf[i + 8] = * ( pSnr + i );
    }

    status = PcdComMF522 ( PCD_AUTHENT, ucComMF522Buf, 12, ucComMF522Buf, &unLen );

    if ( ( status != MI_OK ) || ( ! ( ReadRawRC ( Status2Reg ) & 0x08 ) ) ) {
        status = MI_ERR;
    }

    return status;
}

char PcdRead ( unsigned char addr, unsigned char *pData ) { /* 读取M1卡一块数据。参数addr是块地址，pData是读出的数据(16字节) */
    char status;
    unsigned int  unLen;
    unsigned char i, ucComMF522Buf[MAXRLEN];
    ucComMF522Buf[0] = PICC_READ;
    ucComMF522Buf[1] = addr;
    CalulateCRC ( ucComMF522Buf, 2, &ucComMF522Buf[2] );
    status = PcdComMF522 ( PCD_TRANSCEIVE, ucComMF522Buf, 4, ucComMF522Buf, &unLen );

    if ( ( status == MI_OK ) && ( unLen == 0x90 ) ) {
        for ( i = 0; i < 16; i++ ) {
            * ( pData + i ) = ucComMF522Buf[i];
        }
    } else {
        status = MI_ERR;
    }

    return status;
}

char PcdWrite ( unsigned char addr, unsigned char *pData ) { /* 写数据到M1卡。参数addr是块地址，pData是写入的数据(16字节) */
    char status;
    unsigned int  unLen;
    unsigned char i, ucComMF522Buf[MAXRLEN];
    ucComMF522Buf[0] = PICC_WRITE;
    ucComMF522Buf[1] = addr;
    CalulateCRC ( ucComMF522Buf, 2, &ucComMF522Buf[2] );
    status = PcdComMF522 ( PCD_TRANSCEIVE, ucComMF522Buf, 4, ucComMF522Buf, &unLen );

    if ( ( status != MI_OK ) || ( unLen != 4 ) || ( ( ucComMF522Buf[0] & 0x0F ) != 0x0A ) ) {
        status = MI_ERR;
    }

    if ( status == MI_OK ) {
        for ( i = 0; i < 16; i++ ) {
            ucComMF522Buf[i] = * ( pData + i );
        }

        CalulateCRC ( ucComMF522Buf, 16, &ucComMF522Buf[16] );
        status = PcdComMF522 ( PCD_TRANSCEIVE, ucComMF522Buf, 18, ucComMF522Buf, &unLen );

        if ( ( status != MI_OK ) || ( unLen != 4 ) || ( ( ucComMF522Buf[0] & 0x0F ) != 0x0A ) ) {
            status = MI_ERR;
        }
    }

    return status;
}

char PcdHalt ( void ) { /* 命令卡片进入休眠状态。若成功返回MI_OK */
    char status;
    unsigned int  unLen;
    unsigned char ucComMF522Buf[MAXRLEN];
    ucComMF522Buf[0] = PICC_HALT;
    ucComMF522Buf[1] = 0;
    CalulateCRC ( ucComMF522Buf, 2, &ucComMF522Buf[2] );
    status = PcdComMF522 ( PCD_TRANSCEIVE, ucComMF522Buf, 4, ucComMF522Buf, &unLen );
    return MI_OK;
}

void CalulateCRC ( unsigned char *pIndata, unsigned char len, unsigned char *pOutData ) { /* 用MF522计算CRC16函数 */
    unsigned char i, n;
    ClearBitMask ( DivIrqReg, 0x04 );
    WriteRawRC ( CommandReg, PCD_IDLE );
    SetBitMask ( FIFOLevelReg, 0x80 );

    for ( i = 0; i < len; i++ ) {
        WriteRawRC ( FIFODataReg, * ( pIndata + i ) );
    }

    WriteRawRC ( CommandReg, PCD_CALCCRC );
    i = 0xFF;

    do {
        n = ReadRawRC ( DivIrqReg );
        i--;
    } while ( ( i != 0 ) && ! ( n & 0x04 ) );

    pOutData[0] = ReadRawRC ( CRCResultRegL );
    pOutData[1] = ReadRawRC ( CRCResultRegM );
}

char PcdReset ( void ) { /* 复位RC522。若成功返回MI_OK */
    MF522_RST = 1;
    _nop_();
    MF522_RST = 0;
    _nop_();
    MF522_RST = 1;
    _nop_();
    WriteRawRC ( CommandReg, PCD_RESETPHASE );
    _nop_();
    WriteRawRC ( ModeReg, 0x3D ); /* 和Mifare卡通讯，CRC初始值为0x6363 */
    WriteRawRC ( TReloadRegL, 30 );
    WriteRawRC ( TReloadRegH, 0 );
    WriteRawRC ( TModeReg, 0x8D );
    WriteRawRC ( TPrescalerReg, 0x3E );
    WriteRawRC ( TxAutoReg, 0x40 );
    return MI_OK;
}

char M500PcdConfigISOType ( unsigned char type ) { /* 设置RC632的工作方式 */
    if ( type == 'A' ) { /* ISO14443_A */
        ClearBitMask ( Status2Reg, 0x08 );
        WriteRawRC ( ModeReg, 0x3D ); /* 3F */
        WriteRawRC ( RxSelReg, 0x86 ); /* 84 */
        WriteRawRC ( RFCfgReg, 0x7F ); /* 4F */
        WriteRawRC ( TReloadRegL, 30 );
        WriteRawRC ( TReloadRegH, 0 );
        WriteRawRC ( TModeReg, 0x8D );
        WriteRawRC ( TPrescalerReg, 0x3E );
        delay_10ms ( 1 );
        PcdAntennaOn();
    } else {
        return -1;
    }

    return MI_OK;
}

unsigned char ReadRawRC ( unsigned char Address ) { /* 读取RC632寄存器。参数Address是寄存器地址 */
    unsigned char i, ucAddr;
    unsigned char ucResult = 0;
    MF522_SCK = 0;
    MF522_NSS = 0;
    ucAddr = ( ( Address << 1 ) & 0x7E ) | 0x80;

    for ( i = 8; i > 0; i-- ) {
        MF522_SI = ( ( ucAddr & 0x80 ) == 0x80 );
        MF522_SCK = 1;
        ucAddr <<= 1;
        MF522_SCK = 0;
    }

    for ( i = 8; i > 0; i-- ) {
        MF522_SCK = 1;
        ucResult <<= 1;
        ucResult |= ( bit ) MF522_SO;
        MF522_SCK = 0;
    }

    MF522_NSS = 1;
    MF522_SCK = 1;
    return ucResult;
}

void WriteRawRC ( unsigned char Address, unsigned char value ) { /* 写RC632寄存器。参数Address是寄存器地址，value是写入的值 */
    unsigned char i, ucAddr;
    MF522_SCK = 0;
    MF522_NSS = 0;
    ucAddr = ( ( Address << 1 ) & 0x7E );

    for ( i = 8; i > 0; i-- ) {
        MF522_SI = ( ( ucAddr & 0x80 ) == 0x80 );
        MF522_SCK = 1;
        ucAddr <<= 1;
        MF522_SCK = 0;
    }

    for ( i = 8; i > 0; i-- ) {
        MF522_SI = ( ( value & 0x80 ) == 0x80 );
        MF522_SCK = 1;
        value <<= 1;
        MF522_SCK = 0;
    }

    MF522_NSS = 1;
    MF522_SCK = 1;
}

void SetBitMask ( unsigned char reg, unsigned char mask ) { /* 置RC522寄存器位。参数reg是寄存器地址，mask是置位值 */
    char tmp = 0x0;
    tmp = ReadRawRC ( reg );
    WriteRawRC ( reg, tmp | mask ); /* set bit mask */
}

void ClearBitMask ( unsigned char reg, unsigned char mask ) { /* 清RC522寄存器位。参数reg是寄存器地址，mask是清位值 */
    char tmp = 0x0;
    tmp = ReadRawRC ( reg );
    WriteRawRC ( reg, tmp & ~mask ); /* clear bit mask */
}

/*------------------------------------------------------------------------------------------------
 * 功能：通过RC522和ISO14443卡通讯
 * 参数：Command是RC522命令字，pInData是通过RC522发送到卡片的数据，InLenByte是发送数据的字节长度，
 *       pOutData是接收到的卡片返回数据，pOutLenBit[OUT]是返回数据的位长度
 * ---------------------------------------------------------------------------------------------*/
char PcdComMF522 ( unsigned char Command, unsigned char *pInData, unsigned char InLenByte, unsigned char *pOutData, unsigned int  *pOutLenBit ) {
    char status = MI_ERR;
    unsigned char irqEn   = 0x00;
    unsigned char waitFor = 0x00;
    unsigned char lastBits;
    unsigned char n;
    unsigned int i;

    switch ( Command ) {
        case PCD_AUTHENT:
            irqEn   = 0x12;
            waitFor = 0x10;
            break;

        case PCD_TRANSCEIVE:
            irqEn   = 0x77;
            waitFor = 0x30;
            break;

        default:
            break;
    }

    WriteRawRC ( ComIEnReg, irqEn | 0x80 );
    ClearBitMask ( ComIrqReg, 0x80 );
    WriteRawRC ( CommandReg, PCD_IDLE );
    SetBitMask ( FIFOLevelReg, 0x80 ); /* fifo清零 */

    for ( i = 0; i < InLenByte; i++ ) {
        WriteRawRC ( FIFODataReg, pInData[i] );
    }

    WriteRawRC ( CommandReg, Command );

    if ( Command == PCD_TRANSCEIVE ) {
        SetBitMask ( BitFramingReg, 0x80 );
    }

    i = 2000; /* 根据时钟频率调整，操作M1卡最大等待时间为25ms */

    do {
        n = ReadRawRC ( ComIrqReg );
        i--;
    } while ( ( i != 0 ) && ! ( n & 0x01 ) && ! ( n & waitFor ) );

    ClearBitMask ( BitFramingReg, 0x80 );

    if ( i != 0 ) {
        if ( ! ( ReadRawRC ( ErrorReg ) & 0x1B ) ) {
            status = MI_OK;

            if ( n & irqEn & 0x01 ) {
                status = MI_NOTAGERR;
            }

            if ( Command == PCD_TRANSCEIVE ) {
                n = ReadRawRC ( FIFOLevelReg );
                lastBits = ReadRawRC ( ControlReg ) & 0x07;

                if ( lastBits ) {
                    *pOutLenBit = ( n - 1 ) * 8 + lastBits;
                } else {
                    *pOutLenBit = n * 8;
                }

                if ( n == 0 ) {
                    n = 1;
                }

                if ( n > MAXRLEN ) {
                    n = MAXRLEN;
                }

                for ( i = 0; i < n; i++ ) {
                    pOutData[i] = ReadRawRC ( FIFODataReg );
                }
            }
        } else {
            status = MI_ERR;
        }
    }

    SetBitMask ( ControlReg, 0x80 ); /* stop timer now */
    WriteRawRC ( CommandReg, PCD_IDLE );
    return status;
}

void PcdAntennaOn() { /* 开启天线 */
    unsigned char i;
    i = ReadRawRC ( TxControlReg );

    if ( ! ( i & 0x03 ) ) {
        SetBitMask ( TxControlReg, 0x03 );
    }
}

void PcdAntennaOff() { /* 关闭天线 */
    ClearBitMask ( TxControlReg, 0x03 );
}

/*------------------------------------------------------------------------------------------------------
 * 功能：扣款和充值，若成功返回MI_OK
 * 参数：dd_mode是命令字，0xC0为扣款，0xC1为充值；addr是钱包地址，pValue是数值，4字节增(减)值，低位在前
 *-----------------------------------------------------------------------------------------------------*/
char PcdValue ( unsigned char dd_mode, unsigned char addr, unsigned char *pValue ) {
    char status;
    unsigned int  unLen;
    unsigned char ucComMF522Buf[MAXRLEN];
    ucComMF522Buf[0] = dd_mode;
    ucComMF522Buf[1] = addr;
    CalulateCRC ( ucComMF522Buf, 2, &ucComMF522Buf[2] );
    status = PcdComMF522 ( PCD_TRANSCEIVE, ucComMF522Buf, 4, ucComMF522Buf, &unLen );

    if ( ( status != MI_OK ) || ( unLen != 4 ) || ( ( ucComMF522Buf[0] & 0x0F ) != 0x0A ) ) {
        status = MI_ERR;
    }

    if ( status == MI_OK ) {
        memcpy ( ucComMF522Buf, pValue, 4 );
        CalulateCRC ( ucComMF522Buf, 4, &ucComMF522Buf[4] );
        unLen = 0;
        status = PcdComMF522 ( PCD_TRANSCEIVE, ucComMF522Buf, 6, ucComMF522Buf, &unLen );

        if ( status != MI_ERR ) {
            status = MI_OK;
        }
    }

    if ( status == MI_OK ) {
        ucComMF522Buf[0] = PICC_TRANSFER;
        ucComMF522Buf[1] = addr;
        CalulateCRC ( ucComMF522Buf, 2, &ucComMF522Buf[2] );
        status = PcdComMF522 ( PCD_TRANSCEIVE, ucComMF522Buf, 4, ucComMF522Buf, &unLen );

        if ( ( status != MI_OK ) || ( unLen != 4 ) || ( ( ucComMF522Buf[0] & 0x0F ) != 0x0A ) ) {
            status = MI_ERR;
        }
    }

    return status;
}

char PcdBakValue ( unsigned char sourceaddr, unsigned char goaladdr ) { /* 备份钱包。参数sourceaddr是源地址，goaladdr是目标地址 */
    char status;
    unsigned int  unLen;
    unsigned char ucComMF522Buf[MAXRLEN];
    ucComMF522Buf[0] = PICC_RESTORE;
    ucComMF522Buf[1] = sourceaddr;
    CalulateCRC ( ucComMF522Buf, 2, &ucComMF522Buf[2] );
    status = PcdComMF522 ( PCD_TRANSCEIVE, ucComMF522Buf, 4, ucComMF522Buf, &unLen );

    if ( ( status != MI_OK ) || ( unLen != 4 ) || ( ( ucComMF522Buf[0] & 0x0F ) != 0x0A ) ) {
        status = MI_ERR;
    }

    if ( status == MI_OK ) {
        ucComMF522Buf[0] = 0;
        ucComMF522Buf[1] = 0;
        ucComMF522Buf[2] = 0;
        ucComMF522Buf[3] = 0;
        CalulateCRC ( ucComMF522Buf, 4, &ucComMF522Buf[4] );
        status = PcdComMF522 ( PCD_TRANSCEIVE, ucComMF522Buf, 6, ucComMF522Buf, &unLen );

        if ( status != MI_ERR ) {
            status = MI_OK;
        }
    }

    if ( status != MI_OK ) {
        return MI_ERR;
    }

    ucComMF522Buf[0] = PICC_TRANSFER;
    ucComMF522Buf[1] = goaladdr;
    CalulateCRC ( ucComMF522Buf, 2, &ucComMF522Buf[2] );
    status = PcdComMF522 ( PCD_TRANSCEIVE, ucComMF522Buf, 4, ucComMF522Buf, &unLen );

    if ( ( status != MI_OK ) || ( unLen != 4 ) || ( ( ucComMF522Buf[0] & 0x0F ) != 0x0A ) ) {
        status = MI_ERR;
    }

    return status;
}

void Delay10ms() { /* @11.0592MHz */
    unsigned char i, j;
    _nop_();
    _nop_();
    i = 108;
    j = 144;

    do {
        while ( --j );
    } while ( --i );
}

void delay_10ms ( unsigned int _10ms ) {
    while ( _10ms-- ) {
        Delay10ms();
    }
}