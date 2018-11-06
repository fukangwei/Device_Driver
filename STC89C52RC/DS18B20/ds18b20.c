#include "ds18b20.h"

void delay ( uint count ) { /* delay */
	uint i;

	while ( count ) {
		i = 200;

		while ( i > 0 ) {
			i--;
		}

		count--;
	}
}

void dsreset ( void ) { /* send reset and initialization command */
	uint i;
	DS = 0;
	i = 103;

	while ( i > 0 ) {
		i--;
	}

	DS = 1;
	i = 4;

	while ( i > 0 ) {
		i--;
	}
}

bit tmpreadbit ( void ) { /* read a bit */
	uint i;
	bit dat;
	DS = 0;
	i++;
	DS = 1;
	i++;
	i++;
	dat = DS;
	i = 8;

	while ( i > 0 ) {
		i--;
	}

	return ( dat );
}

uchar tmpread ( void ) { /* read a byte date */
	uchar i, j, dat;
	dat = 0;

	for ( i = 1; i <= 8; i++ ) {
		j = tmpreadbit();
		dat = ( j << 7 ) | ( dat >> 1 ); /* �������������λ����ǰ�棬�����պ�һ���ֽ���dat�� */
	}

	return ( dat );
}

void tmpwritebyte ( uchar dat ) { /* write a byte to ds18b20 */
	uint i;
	uchar j;
	bit testb;

	for ( j = 1; j <= 8; j++ ) {
		testb = dat & 0x01;
		dat = dat >> 1;

		if ( testb ) { /* write 1 */
			DS = 0;
			i++;
			i++;
			DS = 1;
			i = 8;

			while ( i > 0 ) {
				i--;
			}
		}
		else {
			DS = 0; /* write 0 */
			i = 8;

			while ( i > 0 ) {
				i--;
			}

			DS = 1;
			i++;
			i++;
		}
	}
}

void tmpchange ( void ) { /* DS18B20 begin change */
	dsreset();
	delay ( 1 );
	tmpwritebyte ( 0xcc ); /* address all drivers on bus */
	tmpwritebyte ( 0x44 ); /* initiates a single temperature conversion */
}

uint tmp ( void ) { /* get the temperature */
	uint temp; /* variable of temperature */
	float tt;
	uchar a, b;
	dsreset();
	delay ( 1 );
	tmpwritebyte ( 0xcc );
	tmpwritebyte ( 0xbe );
	a = tmpread();
	b = tmpread();
	temp = b;
	temp <<= 8; /* two byte, compose a int variable */
	temp = temp | a;
	tt = temp * 0.0625;
	temp = tt * 10 + 0.5;
	return temp;
}