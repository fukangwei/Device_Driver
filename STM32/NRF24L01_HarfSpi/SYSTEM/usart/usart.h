#ifndef __USART_H
#define __USART_H
#include "stdio.h"

extern u8 USART_RX_BUF[64]; 
extern u8 USART_RX_STA;

void uart_init ( u32 bound );

#endif