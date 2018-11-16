#include "led.h"
#include "spi.h"
#include "delay.h"
#include "sys.h"
#include "usart.h"
#include "device.h"

void Reset_W5500 ( void );
void w5500_init ( void );

int main ( void ) {
    SystemInit();
    delay_init ( 72 );
    NVIC_Configuration();
    uart_init ( 9600 );
    LED_Init();
    w5500_init();
    printf ( "W5500 initialized!\r\n" );
    set_default();
    set_network();
    printf ( "Network is ready.\r\n" );

    while ( 1 ) {
        printf ( "hello\n" );
        delay_ms ( 500 );
    }
}

#define WIZ_RESET GPIO_Pin_3 /* RST的引脚为PA3 */

void Reset_W5500 ( void ) { /* 硬重启W5500 */
    GPIO_ResetBits ( GPIOA, WIZ_RESET );
    delay_us ( 2 );
    GPIO_SetBits ( GPIOA, WIZ_RESET );
    delay_ms ( 1600 );
}

void w5500_init ( void ) { /* w5500引脚初始化函数 */
    GPIO_InitTypeDef GPIO_InitStructure;
    RCC_APB2PeriphClockCmd ( RCC_APB2Periph_GPIOA, ENABLE );
    GPIO_InitStructure.GPIO_Pin   = WIZ_RESET; /* 初始化w5500的RST引脚 */
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;  /* RST模式为输出 */
    GPIO_Init ( GPIOA, &GPIO_InitStructure );
    GPIO_SetBits ( GPIOA, WIZ_RESET );
    Reset_W5500();
    SPIx_Init(); /* 初始化SPI口，这里使用SPI1 */
}