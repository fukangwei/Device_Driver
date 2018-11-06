#include <reg52.h>
#include "at24c02.h"

void UART_init ( void ) {
	SCON = 0x50;
	TMOD = 0x20;
	TH1  = 0xFD;
	TL1  = 0xFD;
	TR1  = 1;
}

void UART_send_byte ( uint8 dat ) {
	SBUF = dat;
	while ( !TI );
	TI = 0;
}

int main() {
	uint8 addr = 0x00, databyte = 0xe4;
	uint8 c = 0;
	uint16 i;
	UART_init();
	InitI2C();

	while ( 1 ) {
		write_eeprom ( addr, databyte );

		for ( i = 0; i < 1000; i++ ) {
			Delay();
		}

		c = read_eeprom ( addr );
		UART_send_byte ( c );
		addr++;
		databyte++;

		if ( addr == 0xff ) {
			addr = 0;
		}

		if ( databyte == 0xff ) {
			databyte = 0;
		}

		for ( i = 0; i < 1000; i++ ) {
			Delay();
		}
	}
}