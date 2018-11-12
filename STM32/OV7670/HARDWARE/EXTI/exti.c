#include "exti.h"
#include "led.h"
#include "delay.h"
#include "usart.h"
#include "ov7670.h"

void EXTI0_IRQHandler ( void ) { /* 外部中断0服务程序 */
    EXTI->PR = 1 << 0; /* 清除LINE0上的中断标志位 */
}

void EXTI9_5_IRQHandler ( void ) { /* 外部中断9~5服务程序 */
    EXTI->PR = 1 << 5; /* 清除LINE5上的中断标志位 */
}

void EXTI_Init ( void ) { /* 外部中断初始化程序，初始化PA0、PC5、PA15为中断输入 */
    RCC->APB2ENR |= 1 << 2; /* 使能PORTA时钟 */
    GPIOA->CRL &= 0XFFFFFFF0; /* PA0设置成输入 */
    GPIOA->CRL |= 0X00000008;
    GPIOA->CRH &= 0X0F0FFFFF; /* PA13、15设置成输入 */
    GPIOA->CRH |= 0X80800000;
    GPIOA->ODR |= 1 << 13; /* PA13上拉，PA0默认下拉 */
    GPIOA->ODR |= 1 << 15; /* PA15上拉 */
    Ex_NVIC_Config ( GPIO_A, 0, RTIR ); /* 上升沿触发 */
    Ex_NVIC_Config ( GPIO_C, 5, FTIR ); /* 下降沿触发 */
    Ex_NVIC_Config ( GPIO_A, 15, FTIR ); /* 下降沿触发 */
    MY_NVIC_Init ( 2, 2, EXTI0_IRQn, 2 ); /* 抢占2，子优先级2，组2 */
    MY_NVIC_Init ( 2, 1, EXTI9_5_IRQn, 2 ); /* 抢占2，子优先级1，组2 */
    MY_NVIC_Init ( 2, 0, EXTI15_10_IRQn, 2 ); /* 抢占2，子优先级0，组2 */
}

u8 ov_sta;

void EXTI15_10_IRQHandler ( void ) { /* 中断服务函数 */
    if ( EXTI->PR & ( 1 << 15 ) ) { /* 是15线的中断 */
        if ( ov_sta < 2 ) {
            if ( ov_sta == 0 ) {
                OV7670_WRST = 0; /* 复位写指针 */
                OV7670_WRST = 1;
                OV7670_WREN = 1; /* 允许写入FIFO */
            } else {
                OV7670_WREN = 0; /* 禁止写入FIFO */
            }

            ov_sta++;
        }
    }

    EXTI->PR = 1 << 15; /* 清除LINE15上的中断标志位 */
}

void EXTI15_Init ( void ) { /* 外部中断初始化程序，初始化PA15为中断输入 */
    RCC->APB2ENR |= 1 << 2; /* 使能PORTA时钟 */
    JTAG_Set ( SWD_ENABLE ); /* 关闭JTAG */
    GPIOA->CRH &= 0X0FFFFFFF; /* PA15设置成输入 */
    GPIOA->CRH |= 0X80000000;
    GPIOA->ODR |= 1 << 15; /* PA15上拉 */
    Ex_NVIC_Config ( GPIO_A, 15, FTIR ); /* 下降沿触发 */
    MY_NVIC_Init ( 2, 1, EXTI15_10_IRQn, 2 ); /* 抢占2，子优先级1，组2 */
}