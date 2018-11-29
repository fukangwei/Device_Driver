#include "mmc_sd.h"
#include "diskio.h"
#include  <stdio.h>
#include "malloc.h"

#define SD_CARD  0
#define EX_FLASH 1

#define FLASH_SECTOR_SIZE  512
#define FLASH_SECTOR_COUNT 4096
#define FLASH_BLOCK_SIZE   8

DSTATUS disk_initialize ( BYTE drv ) {
    u8 res;

    switch ( drv ) {
        case SD_CARD:
            res = SD_Initialize();
            break;

        default:
            res = 1;
    }

    if ( res ) {
        return  STA_NOINIT;
    } else {
        return 0;
    }
}

DSTATUS disk_status ( BYTE drv ) {
    return 0;
}

DRESULT disk_read ( BYTE drv, BYTE *buff, DWORD sector, BYTE count ) {
    u8 res = 0;

    if ( !count ) {
        return RES_PARERR;
    }

    switch ( drv ) {
        case SD_CARD:
            res = SD_ReadDisk ( buff, sector, count );
            break;

        default:
            res = 1;
    }

    if ( res == 0x00 ) {
        return RES_OK;
    } else {
        return RES_ERROR;
    }
}

#if _READONLY == 0
DRESULT disk_write ( BYTE drv, const BYTE *buff, DWORD sector, BYTE count ) {
    u8 res = 0;

    if ( !count ) {
        return RES_PARERR;
    }

    switch ( drv ) {
        case SD_CARD:
            res = SD_WriteDisk ( ( u8 * ) buff, sector, count );
            break;

        default:
            res = 1;
    }

    if ( res == 0x00 ) {
        return RES_OK;
    } else {
        return RES_ERROR;
    }
}
#endif

DRESULT disk_ioctl ( BYTE drv, BYTE ctrl, void *buff ) {
    DRESULT res;

    if ( drv == SD_CARD ) {
        switch ( ctrl ) {
            case CTRL_SYNC:
                SD_CS = 0;

                if ( SD_WaitReady() == 0 ) {
                    res = RES_OK;
                } else {
                    res = RES_ERROR;
                }

                SD_CS = 1;
                break;

            case GET_SECTOR_SIZE:
                * ( WORD * ) buff = 512;
                res = RES_OK;
                break;

            case GET_BLOCK_SIZE:
                * ( WORD * ) buff = 8;
                res = RES_OK;
                break;

            case GET_SECTOR_COUNT:
                * ( DWORD * ) buff = SD_GetSectorCount();
                res = RES_OK;
                break;

            default:
                res = RES_PARERR;
                break;
        }
    } else if ( drv == EX_FLASH ) {
        switch ( ctrl ) {
            case CTRL_SYNC:
                res = RES_OK;
                break;

            case GET_SECTOR_SIZE:
                * ( WORD * ) buff = FLASH_SECTOR_SIZE;
                res = RES_OK;
                break;

            case GET_BLOCK_SIZE:
                * ( WORD * ) buff = FLASH_BLOCK_SIZE;
                res = RES_OK;
                break;

            case GET_SECTOR_COUNT:
                * ( DWORD * ) buff = FLASH_SECTOR_COUNT;
                res = RES_OK;
                break;

            default:
                res = RES_PARERR;
                break;
        }
    } else {
        res = RES_ERROR;
    }

    return res;
}

DWORD get_fattime ( void ) {
    return 0;
}

void *ff_memalloc ( UINT size ) {
    return ( void * ) mymalloc ( size );
}

void ff_memfree ( void *mf ) {
    myfree ( mf );
}