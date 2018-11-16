#define GLOBAL_CLK 1
#include <stdlib.h>
#include <string.h>
#include "def.h"
#include "option.h"
#include "2440addr.h"
#include "2440lib.h"
#include "2440slib.h"
#include "mmu.h"
#include "profile.h"
#include "memtest.h"

static void cal_cpu_bus_clk ( void );
void Set_Clk ( void );
void beep_init ( void );
void beep_run ( void );

void delay ( int times ) {
    int i, j;

    for ( i = 0; i < times; i++ )
        for ( j = 0; j < 400; j++ );
}

void Main ( void ) {
    int i;
    PCLK = 50000000;
    Uart_Init ( 0, 9600 ); /* ���ò����ʡ�����λ��ֹͣλ��У��λ */
    Uart_Select ( 0 ); /* ѡ��ͨ��0��������0 */

    for ( i = 0; i < 10; i++ ) {
        Uart_Printf ( "Hello World!\n" );
    }
}