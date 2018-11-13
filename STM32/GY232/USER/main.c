#include "led.h"
#include "string.h"
#include "key.h"
#include "delay.h"
#include "sys.h"
#include "usart.h"
#include "GY273.h"
#include "math.h"
#include "mmc_sd.h"
#include "ff.h"

extern uint BUF[];
extern uint ge, shi, bai, qian, wan;

signed short x, y, z;
double angle;

unsigned char FileBuf[20] = {0};
unsigned char WriteX[10] = {0};
unsigned char WriteY[10] = {0};

void Write2Buf ( char *to, const char *from1, const char *from2 ) {
    memset ( to, 0, 20 );
    to[0] = 'X';
    to[1] = ':';
    to[7] = ' ';
    to[8] = 'Y';
    to[9] = ':';
    to[15] = '\r';
    to[16] = '\n';
    strncpy ( to + 2, from1, 5 );
    strncpy ( to + 10, from2, 5 );
    return ;
}

void Num2String ( signed short num, unsigned char *string ) {
    unsigned char temp = 0;
    unsigned char i = 0;

    if ( num >= 0 ) {
        string[0] = '+';
    } else {
        string[0] = '-';
        num = -num;
    }

    for ( i = 4; i != 0; i-- ) {
        temp = num % 10 + '0';
        num /= 10;
        string[i] = temp;
    }

    return ;
}

FATFS fs;
FIL fsrc;
UINT bw;

int main ( void ) {
    SystemInit();
    delay_init ( 72 );
    NVIC_Configuration();
    uart_init ( 9600 );
    LED_Init();
    KEY_Init();

    while ( SD_Initialize() != 0 ) {
        printf ( "SD Card Failed!" );
        delay_ms ( 500 );
    }

    Init_HMC5883();
    f_mount ( 0, &fs );
    f_open ( &fsrc, "1.txt", FA_OPEN_ALWAYS | FA_WRITE );

    while ( 1 ) {
        Multi_read_HMC5883();
        x = ( BUF[0] << 8 ) | BUF[1];
        y = ( BUF[4] << 8 ) | BUF[5];
        z = ( BUF[2] << 8 ) | BUF[3];
        angle = atan2 ( ( double ) ( y + 100 ), ( double ) ( x - 100 ) ) * ( 180 / 3.14159265 ) + 180;
        conversion ( ( uint ) angle );
        printf ( "%d \r\n", bai * 100 + shi * 10 + ge );
        printf ( "x: %d, y: %d\r\n", x, y );
        Num2String ( x, WriteX );
        Num2String ( y, WriteY );
        Write2Buf ( FileBuf, WriteX, WriteY );
        printf ( "test x: %s\r\n", WriteX );
        printf ( "test y: %s\r\n", WriteY );
        printf ( "%s", FileBuf );
        printf ( "\r\n" );
        f_write ( &fsrc, &FileBuf, sizeof ( FileBuf ), &bw );

        if ( KEY_Scan() == 1 ) {
            f_close ( &fsrc );
            f_mount ( 0, NULL );
            printf ( "KEY 1!\r\n" );
        }

        delay_ms ( 200 );
    }
}