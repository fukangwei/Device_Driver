#ifndef __UART_H__
#define __UART_H__

void UART_Init();
void send ( unsigned char d );
void send_str ( unsigned char *dat );
void send_str_len ( unsigned char *dat, unsigned int len );
#endif