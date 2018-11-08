#include "stm32f10x.h"
#include "led.h"

void LED_Init ( void ) {
    GPIO_InitTypeDef  GPIO_InitStructure;
    RCC_APB2PeriphClockCmd ( RCC_APB2Periph_GPIOA, ENABLE ); /* ʹ��PA�˿�ʱ�� */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8; /* ��LED0 -> PA.8���˿����� */
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; /* ������� */
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init ( GPIOA, &GPIO_InitStructure );
    GPIO_SetBits ( GPIOA, GPIO_Pin_8 ); /* PA.8����� */
    RCC_APB2PeriphClockCmd ( RCC_APB2Periph_GPIOD, ENABLE );
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init ( GPIOD, &GPIO_InitStructure );
    GPIO_SetBits ( GPIOD, GPIO_Pin_2 );
}