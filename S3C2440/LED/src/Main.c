#define GLOBAL_CLK 1
#include <stdlib.h>
#include <string.h>
#include "def.h"
#include "option.h"
#include "2440addr.h"
#include "2440lib.h"
#include "2440slib.h"
#include "profile.h"
#include "memtest.h"

void Led_port_init ( void );
void Disp_run ( void );

#define LED1_ON ~(1<<5)
#define LED2_ON ~(1<<6)
#define LED3_ON ~(1<<7)
#define LED4_ON ~(1<<8)

#define LED1_OFF (1<<5)
#define LED2_OFF (1<<6)
#define LED3_OFF (1<<7)
#define LED4_OFF (1<<8)

void delay ( int times ) {
    int i;

    for ( ; times > 0; times-- )
        for ( i = 0; i < 400; i++ );
}

void Main ( void ) {
    Led_port_init();
    Disp_run();
}

void Led_port_init ( void ) { /* 初始化连接LED灯的引脚 */
    rGPBCON &= ~ ( ( 3 << 10 ) | ( 3 << 12 ) | ( 3 << 14 ) | ( 3 << 16 ) );
    rGPBCON |= ( 1 << 10 ) | ( 1 << 12 ) | ( 1 << 14 ) | ( 1 << 16 ); /* 全部设置为输出 */
}

void Disp_run ( void ) {
    rGPBDAT = rGPBDAT | ( LED1_OFF ) | ( LED2_OFF ) | ( LED3_OFF ) | ( LED4_OFF );

    while ( 1 ) {
        rGPBDAT = ( ~rGPBDAT ) & 0xfffe;
        delay ( 100000 );
    }
}