#include "sys.h"
#include "usart.h"

#if 1 /* �������´��룬֧��printf������������Ҫѡ��use MicroLIB�� */
#pragma import(__use_no_semihosting)

struct __FILE { /* ��׼����Ҫ��֧�ֺ��� */
    int handle;
};

FILE __stdout;

_sys_exit ( int x ) { /* ���塰_sys_exit�������Ա���ʹ�ð�����ģʽ */
    x = x;
}

int fputc ( int ch, FILE *f ) { /* �ض���fputc���� */
    while ( ( USART3->SR & 0X40 ) == 0 ); /* ѭ�����ͣ�ֱ��������� */

    USART3->DR = ( u8 ) ch;
    return ch;
}
#endif

u8 USART_RX_BUF[64]; /* ���ջ��壬���64���ֽ� */
u8 USART_RX_STA = 0; /* ����״̬��ǣ�bit7�ǽ�����ɱ�־��bit6�ǽ��յ�0x0d��bit5��0�ǽ��յ�����Ч�ֽ���Ŀ */

void uart_init ( u32 bound ) {
    GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;
    RCC_APB2PeriphClockCmd ( RCC_APB2Periph_GPIOB | RCC_APB2Periph_AFIO, ENABLE );
    RCC_APB1PeriphClockCmd ( RCC_APB1Periph_USART3, ENABLE );
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init ( GPIOB, &GPIO_InitStructure );
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init ( GPIOB, &GPIO_InitStructure );
    NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init ( &NVIC_InitStructure );
    USART_InitStructure.USART_BaudRate = bound;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART_Init ( USART3, &USART_InitStructure );
    USART_ITConfig ( USART3, USART_IT_RXNE, ENABLE );
    USART_Cmd ( USART3, ENABLE );
}

void USART3_IRQHandler ( void ) { /* ����3�жϷ������ */
    u8 Res;

    if ( USART_GetITStatus ( USART3, USART_IT_RXNE ) != RESET ) { /* �����ж�(���յ������ݱ����ǡ�0x0d 0x0a����β) */
        Res = USART_ReceiveData ( USART3 ); /* ��ȡ���յ������� */

        if ( ( USART_RX_STA & 0x80 ) == 0 ) { /* ����δ��� */
            if ( USART_RX_STA & 0x40 ) { /* ���յ���0x0d */
                if ( Res != 0x0a ) {
                    USART_RX_STA = 0; /* ���մ������¿�ʼ */
                } else {
                    USART_RX_STA |= 0x80; /* ��������� */
                }
            } else { /* ��û�յ�0X0D */
                if ( Res == 0x0d ) {
                    USART_RX_STA |= 0x40;
                } else {
                    USART_RX_BUF[USART_RX_STA & 0X3F] = Res;
                    USART_RX_STA++;

                    if ( USART_RX_STA > 63 ) {
                        USART_RX_STA = 0; /* �������ݴ������¿�ʼ���� */
                    }
                }
            }
        }
    }
}