#include "SysTick.h"

uint32_t RunningTime = 0;

void SysTick_Init ( void ) {
    /* SystemFrequency / 1000��1ms�ж�һ��
       SystemFrequency / 100000��10us�ж�һ��
       SystemFrequency / 1000000��1us�ж�һ�� */
    if ( SysTick_Config ( SystemCoreClock / 1000 ) ) { /* ST3.5.0��汾 */
        /* Capture error */
        while ( 1 );
    }

    SysTick->CTRL &= ~ SysTick_CTRL_ENABLE_Msk; /* �رյδ�ʱ�� */
}

void TimeStart ( void ) {
    SysTick->CTRL |=  SysTick_CTRL_ENABLE_Msk; /* ʹ�ܵδ�ʱ�� */
}

void TimingDelay_Decrement ( void ) {
    RunningTime++;
}