#include "SysTick.h"

uint32_t RunningTime = 0;

void SysTick_Init ( void ) {
    /* SystemFrequency / 1000：1ms中断一次
       SystemFrequency / 100000：10us中断一次
       SystemFrequency / 1000000：1us中断一次 */
    if ( SysTick_Config ( SystemCoreClock / 1000 ) ) { /* ST3.5.0库版本 */
        /* Capture error */
        while ( 1 );
    }

    SysTick->CTRL &= ~ SysTick_CTRL_ENABLE_Msk; /* 关闭滴答定时器 */
}

void TimeStart ( void ) {
    SysTick->CTRL |=  SysTick_CTRL_ENABLE_Msk; /* 使能滴答定时器 */
}

void TimingDelay_Decrement ( void ) {
    RunningTime++;
}