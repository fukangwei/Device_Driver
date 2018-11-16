#include "timer.h"
#include "led.h"
#include "usart.h"

u8 ov_frame = 0;

void TIM3_IRQHandler ( void ) { /* ��ʱ��3�жϷ������ */
    if ( TIM3->SR & 0X0001 ) { /* ����ж� */
        printf ( "frame:%dfps\r\n", ov_frame );
        ov_frame = 0;
    }

    TIM3->SR &= ~ ( 1 << 0 ); /* ����жϱ�־λ */
}

void TIM3_Int_Init ( u16 arr, u16 psc ) { /* ͨ�ö�ʱ���жϳ�ʼ��������arr���Զ���װֵ��psc��ʱ��Ԥ��Ƶ�� */
    RCC->APB1ENR |= 1 << 1; /* TIM3ʱ��ʹ�� */
    TIM3->ARR = arr; /* �趨�������Զ���װֵ */
    TIM3->PSC = psc; /* Ԥ��Ƶ������ */
    TIM3->DIER |= 1 << 0; /* ��������ж� */
    TIM3->CR1 |= 0x01; /* ʹ�ܶ�ʱ��3 */
    MY_NVIC_Init ( 1, 3, TIM3_IRQn, 2 ); /* ��ռ1�������ȼ�3����2 */
}