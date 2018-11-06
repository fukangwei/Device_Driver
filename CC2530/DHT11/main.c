#include <ioCC2530.h>
#include <string.h>
#include "UART.h"
#include "DHT11.h"

void main ( void ) {
	Delay_ms ( 1000 ); /* ���豸�ȶ� */
	InitUart(); /* ���ڳ�ʼ�� */

	while ( 1 ) {
		DHT11(); /* ��ȡ��ʪ�� */
		P0DIR |= 0x40; /* IO����Ҫ�������� */
		/*----- ��ʪ�ȵ�ASCII��ת�� -----*/
		temp[0] = wendu_shi + 0x30;
		temp[1] = wendu_ge + 0x30;
		humidity[0] = shidu_shi + 0x30;
		humidity[1] = shidu_ge + 0x30;
		/*------- ��Ϣͨ�����ڴ�ӡ --------*/
		Uart_Send_String ( temp1, 5 );
		Uart_Send_String ( temp, 2 );
		Uart_Send_String ( "\n", 1 );
		Uart_Send_String ( humidity1, 9 );
		Uart_Send_String ( humidity, 2 );
		Uart_Send_String ( "\n", 1 );
		Delay_ms ( 2000 ); /* ��ʱ��ʹ������2S��ȡ1�� */
	}
}