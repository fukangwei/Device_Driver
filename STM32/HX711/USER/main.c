#include "delay.h"
#include "sys.h"
#include "usart.h"
#include "HX711.h"

extern s32 Weight_Shiwu;

int main ( void ) {
    SystemInit();
    delay_init ( 72 );
    NVIC_Configuration();
    uart_init ( 9600 );
    HX711_InIt();
    Get_Maopi();

    while ( 1 ) {
        Get_Weight();
        printf ( "%dg\r\n", Weight_Shiwu );
        delay_ms ( 500 );
    }
}