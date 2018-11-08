#include "delay.h"
#include "sys.h"
#include "usart.h"
#include "dht11.h"

int main ( void ) {
    u8 temperature = 0;
    u8 humidity = 0;
    SystemInit();
    delay_init ( 72 );
    NVIC_Configuration();
    uart_init ( 9600 );
    DHT11_Init();

    while ( 1 ) {
        DHT11_Read_Data ( &temperature, &humidity ); /* ¶ÁÈ¡ÎÂÊª¶ÈÖµ */
        printf ( "TEMP is %d\r\nhumi is %d\r\n", temperature, humidity );
        delay_ms ( 500 );
    }
}