#include "led.h"
#include "delay.h"
#include "sys.h"
#include "usart.h"
#include "mmc_sd.h"
#include "ff.h"

unsigned char buffer[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};

FATFS fs;
FIL fsrc;
UINT bw;

int main ( void ) {
    u32 sector_size;
    SystemInit();
    delay_init ( 72 );
    NVIC_Configuration();
    uart_init ( 9600 );

    while ( SD_Initialize() != 0 ) {
        delay_ms ( 500 );
    }

    sector_size = SD_GetSectorCount();
    printf ( " SD Sector Size:%d\n", sector_size );
    f_mount ( 0, &fs );
    f_open ( &fsrc, "20150815081236.html", FA_OPEN_ALWAYS  | FA_WRITE );
    f_write ( &fsrc, buffer, sizeof ( buffer ), &bw );
    f_close ( &fsrc );
    f_mount ( 0, NULL );
    printf ( "write OK!\n" );
}