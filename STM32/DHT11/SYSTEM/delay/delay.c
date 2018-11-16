#include "delay.h"
#include "sys.h"

static u8  fac_us = 0;
static u16 fac_ms = 0;

#ifdef OS_CRITICAL_METHOD /* 如果OS_CRITICAL_METHOD定义了，说明使用ucosII */

void SysTick_Handler ( void ) { /* systick中断服务函数，ucos需要该函数 */
    OSIntEnter(); /* 进入中断 */
    OSTimeTick(); /* 调用ucos的时钟服务程序 */
    OSIntExit(); /* 触发任务切换软中断 */
}

#endif

void delay_init ( u8 SYSCLK ) {
#ifdef OS_CRITICAL_METHOD
    u32 reload;
#endif
    SysTick->CTRL &= ~ ( 1 << 2 );
    fac_us = SYSCLK / 8;
#ifdef OS_TICKS_PER_SEC
    reload = SYSCLK / 8; /* 每秒钟的计数次数，单位为K */
    reload *= 1000000 / OS_TICKS_PER_SEC; /* 根据OS_TICKS_PER_SEC设定溢出时间 */
    /* reload为24位寄存器，最大值为16777216，在72M下约合1.86s左右 */
    fac_ms = 1000 / OS_TICKS_PER_SEC; /* 代表ucos可以延时的最少单位 */
    SysTick->CTRL |= 1 << 1; /* 开启SYSTICK中断 */
    SysTick->LOAD = reload; /* 每1/OS_TICKS_PER_SEC秒中断一次 */
    SysTick->CTRL |= 1 << 0; /* 开启SYSTICK */
#else
    fac_ms = ( u16 ) fac_us * 1000; /* 非ucos下，代表每个ms需要的systick时钟数 */
#endif
}

#ifdef OS_TICKS_PER_SEC

void delay_us ( u32 nus ) {
    u32 ticks;
    u32 told, tnow, tcnt = 0;
    u32 reload = SysTick->LOAD;
    ticks = nus * fac_us;
    tcnt = 0;
    told = SysTick->VAL;

    while ( 1 ) {
        tnow = SysTick->VAL;

        if ( tnow != told ) {
            if ( tnow < told ) {
                tcnt += told - tnow;
            } else {
                tcnt += reload - tnow + told;
            }

            told = tnow;

            if ( tcnt >= ticks ) {
                break;
            }
        }
    };
}

void delay_ms ( u16 nms ) {
    if ( OSRunning == TRUE ) { /* 如果os已经在跑了 */
        if ( nms >= fac_ms ) { /* 延时的时间大于ucos的最少时间周期 */
            OSTimeDly ( nms / fac_ms ); /* ucos延时 */
        }

        nms %= fac_ms; /* ucos已经无法提供这么小的延时了，采用普通方式延时 */
    }

    delay_us ( ( u32 ) ( nms * 1000 ) ); /* 普通方式延时，此时ucos无法启动调度 */
}
#else

void delay_us ( u32 nus ) {
    u32 temp;
    SysTick->LOAD = nus * fac_us;
    SysTick->VAL = 0x00;
    SysTick->CTRL = 0x01;

    do {
        temp = SysTick->CTRL;
    } while ( temp & 0x01 && ! ( temp & ( 1 << 16 ) ) );

    SysTick->CTRL = 0x00;
    SysTick->VAL = 0X00;
}

void delay_ms ( u16 nms ) {
    u32 temp;
    SysTick->LOAD = ( u32 ) nms * fac_ms;
    SysTick->VAL = 0x00;
    SysTick->CTRL = 0x01;

    do {
        temp = SysTick->CTRL;
    } while ( temp & 0x01 && ! ( temp & ( 1 << 16 ) ) );

    SysTick->CTRL = 0x00;
    SysTick->VAL = 0X00;
}

#endif

void JTAG_Set ( u8 mode ) {
    u32 temp;
    temp = mode;
    temp <<= 25;
    RCC->APB2ENR |= 1 << 0; /* 开启辅助时钟 */
    AFIO->MAPR &= 0XF8FFFFFF; /* 清除MAPR的[26:24] */
    AFIO->MAPR |= temp; /* 设置jtag模式 */
}