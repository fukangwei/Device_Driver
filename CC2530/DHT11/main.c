#include <ioCC2530.h>
#include <string.h>
#include "UART.h"
#include "DHT11.h"

void main ( void ) {
	Delay_ms ( 1000 ); /* 让设备稳定 */
	InitUart(); /* 串口初始化 */

	while ( 1 ) {
		DHT11(); /* 获取温湿度 */
		P0DIR |= 0x40; /* IO口需要重新配置 */
		/*----- 温湿度的ASCII码转换 -----*/
		temp[0] = wendu_shi + 0x30;
		temp[1] = wendu_ge + 0x30;
		humidity[0] = shidu_shi + 0x30;
		humidity[1] = shidu_ge + 0x30;
		/*------- 信息通过串口打印 --------*/
		Uart_Send_String ( temp1, 5 );
		Uart_Send_String ( temp, 2 );
		Uart_Send_String ( "\n", 1 );
		Uart_Send_String ( humidity1, 9 );
		Uart_Send_String ( humidity, 2 );
		Uart_Send_String ( "\n", 1 );
		Delay_ms ( 2000 ); /* 延时，使周期性2S读取1次 */
	}
}