#include "usart.h"

#if SYSTEM_SUPPORT_UCOS
    #include "includes.h"
#endif

#if 1
#pragma import(__use_no_semihosting)
struct __FILE {
    int handle;
};

FILE __stdout;

_sys_exit ( int x ) {
    x = x;
}

int fputc ( int ch, FILE *f ) {
    while ( ( USART1->SR & 0X40 ) == 0 );

    USART1->DR = ( u8 ) ch;
    return ch;
}
#endif

#if EN_USART1_RX

u8 USART_RX_BUF[USART_REC_LEN];
u16 USART_RX_STA = 0;

void USART1_IRQHandler ( void ) {
    u8 res;
#ifdef OS_CRITICAL_METHOD
    OSIntEnter();
#endif

    if ( USART1->SR & ( 1 << 5 ) ) {
        res = USART1->DR;

        if ( ( USART_RX_STA & 0x8000 ) == 0 ) {
            if ( USART_RX_STA & 0x4000 ) {
                if ( res != 0x0a ) {
                    USART_RX_STA = 0;
                } else {
                    USART_RX_STA |= 0x8000;
                }
            } else {
                if ( res == 0x0d ) {
                    USART_RX_STA |= 0x4000;
                } else {
                    USART_RX_BUF[USART_RX_STA & 0X3FFF] = res;
                    USART_RX_STA++;

                    if ( USART_RX_STA > ( USART_REC_LEN - 1 ) ) {
                        USART_RX_STA = 0;
                    }
                }
            }
        }
    }

#ifdef OS_CRITICAL_METHOD
    OSIntExit();
#endif
}
#endif

void uart_init ( u32 pclk2, u32 bound ) {
    float temp;
    u16 mantissa;
    u16 fraction;
    temp = ( float ) ( pclk2 * 1000000 ) / ( bound * 16 );
    mantissa = temp;
    fraction = ( temp - mantissa ) * 16;
    mantissa <<= 4;
    mantissa += fraction;
    RCC->APB2ENR |= 1 << 2;
    RCC->APB2ENR |= 1 << 14;
    GPIOA->CRH &= 0XFFFFF00F;
    GPIOA->CRH |= 0X000008B0;
    RCC->APB2RSTR |= 1 << 14;
    RCC->APB2RSTR &= ~ ( 1 << 14 );
    USART1->BRR = mantissa;
    USART1->CR1 |= 0X200C;
#if EN_USART1_RX
    USART1->CR1 |= 1 << 8;
    USART1->CR1 |= 1 << 5;
    MY_NVIC_Init ( 3, 3, USART1_IRQn, 2 );
#endif
}