#include "exti.h"
#include "led.h"
#include "delay.h"
#include "usart.h"
#include "ov7670.h"

void EXTI0_IRQHandler ( void ) { /* �ⲿ�ж�0������� */
    EXTI->PR = 1 << 0; /* ���LINE0�ϵ��жϱ�־λ */
}

void EXTI9_5_IRQHandler ( void ) { /* �ⲿ�ж�9~5������� */
    EXTI->PR = 1 << 5; /* ���LINE5�ϵ��жϱ�־λ */
}

void EXTI_Init ( void ) { /* �ⲿ�жϳ�ʼ�����򣬳�ʼ��PA0��PC5��PA15Ϊ�ж����� */
    RCC->APB2ENR |= 1 << 2; /* ʹ��PORTAʱ�� */
    GPIOA->CRL &= 0XFFFFFFF0; /* PA0���ó����� */
    GPIOA->CRL |= 0X00000008;
    GPIOA->CRH &= 0X0F0FFFFF; /* PA13��15���ó����� */
    GPIOA->CRH |= 0X80800000;
    GPIOA->ODR |= 1 << 13; /* PA13������PA0Ĭ������ */
    GPIOA->ODR |= 1 << 15; /* PA15���� */
    Ex_NVIC_Config ( GPIO_A, 0, RTIR ); /* �����ش��� */
    Ex_NVIC_Config ( GPIO_C, 5, FTIR ); /* �½��ش��� */
    Ex_NVIC_Config ( GPIO_A, 15, FTIR ); /* �½��ش��� */
    MY_NVIC_Init ( 2, 2, EXTI0_IRQn, 2 ); /* ��ռ2�������ȼ�2����2 */
    MY_NVIC_Init ( 2, 1, EXTI9_5_IRQn, 2 ); /* ��ռ2�������ȼ�1����2 */
    MY_NVIC_Init ( 2, 0, EXTI15_10_IRQn, 2 ); /* ��ռ2�������ȼ�0����2 */
}

u8 ov_sta;

void EXTI15_10_IRQHandler ( void ) { /* �жϷ����� */
    if ( EXTI->PR & ( 1 << 15 ) ) { /* ��15�ߵ��ж� */
        if ( ov_sta < 2 ) {
            if ( ov_sta == 0 ) {
                OV7670_WRST = 0; /* ��λдָ�� */
                OV7670_WRST = 1;
                OV7670_WREN = 1; /* ����д��FIFO */
            } else {
                OV7670_WREN = 0; /* ��ֹд��FIFO */
            }

            ov_sta++;
        }
    }

    EXTI->PR = 1 << 15; /* ���LINE15�ϵ��жϱ�־λ */
}

void EXTI15_Init ( void ) { /* �ⲿ�жϳ�ʼ�����򣬳�ʼ��PA15Ϊ�ж����� */
    RCC->APB2ENR |= 1 << 2; /* ʹ��PORTAʱ�� */
    JTAG_Set ( SWD_ENABLE ); /* �ر�JTAG */
    GPIOA->CRH &= 0X0FFFFFFF; /* PA15���ó����� */
    GPIOA->CRH |= 0X80000000;
    GPIOA->ODR |= 1 << 15; /* PA15���� */
    Ex_NVIC_Config ( GPIO_A, 15, FTIR ); /* �½��ش��� */
    MY_NVIC_Init ( 2, 1, EXTI15_10_IRQn, 2 ); /* ��ռ2�������ȼ�1����2 */
}