#include <reg52.h>
#include "stdio.h"
#include "string.h"
#include "intrins.h"
#include "ds18b20.h"

void InitUART ( void ) {
	TMOD = 0x20;
	SCON = 0x40;
	TH1 = 0xFD;
	TL1 = TH1;
	PCON = 0x00;
	TR1 = 1;
}

void SendOneByte ( unsigned char c ) {
	SBUF = c;

	while ( !TI );

	TI = 0;
}

void UART_send_string ( uchar *buf ) {
	while ( *buf != '\0' ) {
		SendOneByte ( *buf++ );
	}
}

void delay500ms ( void ) {
	unsigned char a, b, c;

	for ( c = 98; c > 0; c-- )
		for ( b = 127; b > 0; b-- )
			for ( a = 17; a > 0; a-- );

	_nop_();  /* if Keil, require use intrins.h */
}

int main ( void ) {
	uchar send_buff[50] = {0};
	uint read_temp = 0;
	InitUART();

	while ( 1 ) {
		tmpchange();
		read_temp = tmp();
		memset ( send_buff, 0, sizeof ( send_buff ) );
		sprintf ( send_buff, "Temp is %.1f\r\n", read_temp * 1.0 / 10 );
		UART_send_string ( send_buff );
		delay500ms();
	}

	return 0;
}