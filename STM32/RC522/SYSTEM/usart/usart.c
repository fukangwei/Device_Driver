#include "sys.h"
#include "usart.h"

#if 1 /* 加入以下代码，支持printf函数，而不需要选择“use MicroLIB” */
#pragma import(__use_no_semihosting)

struct __FILE { /* 标准库需要的支持函数 */
    int handle;
};

FILE __stdout;

_sys_exit ( int x ) { /* 定义“_sys_exit”函数以避免使用半主机模式 */
    x = x;
}

int fputc ( int ch, FILE *f ) { /* 重定义fputc函数 */
    while ( ( USART3->SR & 0X40 ) == 0 ); /* 循环发送，直到发送完毕 */

    USART3->DR = ( u8 ) ch;
    return ch;
}
#endif

u8 USART_RX_BUF[64]; /* 接收缓冲，最大64个字节 */
u8 USART_RX_STA = 0; /* 接收状态标记，bit7是接收完成标志，bit6是接收到0x0d，bit5至0是接收到的有效字节数目 */

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

void USART3_IRQHandler ( void ) { /* 串口3中断服务程序 */
    u8 Res;

    if ( USART_GetITStatus ( USART3, USART_IT_RXNE ) != RESET ) { /* 接收中断(接收到的数据必须是“0x0d 0x0a”结尾) */
        Res = USART_ReceiveData ( USART3 ); /* 读取接收到的数据 */

        if ( ( USART_RX_STA & 0x80 ) == 0 ) { /* 接收未完成 */
            if ( USART_RX_STA & 0x40 ) { /* 接收到了0x0d */
                if ( Res != 0x0a ) {
                    USART_RX_STA = 0; /* 接收错误，重新开始 */
                } else {
                    USART_RX_STA |= 0x80; /* 接收完成了 */
                }
            } else { /* 还没收到0X0D */
                if ( Res == 0x0d ) {
                    USART_RX_STA |= 0x40;
                } else {
                    USART_RX_BUF[USART_RX_STA & 0X3F] = Res;
                    USART_RX_STA++;

                    if ( USART_RX_STA > 63 ) {
                        USART_RX_STA = 0; /* 接收数据错误，重新开始接收 */
                    }
                }
            }
        }
    }
}