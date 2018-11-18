#include "led.h"
#include "delay.h"
#include "sys.h"
#include "usart.h"
#include "spi.h"
#include "string.h"
#include "flash.h"

const u8 TEXT_Buffer_SPI[] = {"MiniSTM32 SPI TEST"};

#define SIZE sizeof(TEXT_Buffer_SPI)

int main ( void ) {
    u8 datatemp[SIZE];
    SystemInit();
    delay_init ( 72 );
    NVIC_Configuration();
    uart_init ( 9600 );
    LED_Init();
    SPI_Flash_Init();

    while ( SPI_Flash_ReadID() != FLASH_ID ) {
        u16 i = 0;
        i = SPI_Flash_ReadID();
        printf ( "ID:%d", i );
        delay_ms ( 500 );
        LED0 = !LED0;
    }

    while ( 1 ) {
        memset ( datatemp, 0, sizeof ( datatemp ) );
        SPI_Flash_Write ( ( u8 * ) TEXT_Buffer_SPI, 1000, SIZE );
        printf ( "Flash Write Finish!\r\n" );
        delay_ms ( 500 );
        SPI_Flash_Read ( datatemp, 1000, SIZE );
        printf ( "Flash Read %s\r\n", datatemp );
        LED0 = !LED0;
        delay_ms ( 500 );
    }
}