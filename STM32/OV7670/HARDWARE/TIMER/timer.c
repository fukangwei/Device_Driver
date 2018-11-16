#include "timer.h"
#include "led.h"
#include "usart.h"

u8 ov_frame = 0;

void TIM3_IRQHandler ( void ) { /* 定时器3中断服务程序 */
    if ( TIM3->SR & 0X0001 ) { /* 溢出中断 */
        printf ( "frame:%dfps\r\n", ov_frame );
        ov_frame = 0;
    }

    TIM3->SR &= ~ ( 1 << 0 ); /* 清除中断标志位 */
}

void TIM3_Int_Init ( u16 arr, u16 psc ) { /* 通用定时器中断初始化。参数arr是自动重装值，psc是时钟预分频数 */
    RCC->APB1ENR |= 1 << 1; /* TIM3时钟使能 */
    TIM3->ARR = arr; /* 设定计数器自动重装值 */
    TIM3->PSC = psc; /* 预分频器设置 */
    TIM3->DIER |= 1 << 0; /* 允许更新中断 */
    TIM3->CR1 |= 0x01; /* 使能定时器3 */
    MY_NVIC_Init ( 1, 3, TIM3_IRQn, 2 ); /* 抢占1，子优先级3，组2 */
}