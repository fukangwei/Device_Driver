#include "at24c02.h"
#include <INTRINS.H>

void Delay ( void ) {
	_nop_();
	_nop_();
	_nop_();
	_nop_();
}

void InitI2C ( void ) {
	SDA = 1;
	SCL = 1;
}

void I2CStart ( void ) {
	SDA = 1;
	Delay();     
	SCL = 1;
	Delay();
	SDA = 0;
	Delay();
	SCL = 0;
}

void I2CStop ( void ) {
	SCL = 0;
	Delay();
	SDA = 0;
	Delay();
	SCL = 1;
	Delay();
	SDA = 1;
	Delay();
}

void I2CSend ( uint8 byte ) {
	uint8 mask;
	uint8 i;
	uint8 j;
	mask = 0x80;

	for ( i = 0; i < 8; i++ ) {
		SCL = 0;
		Delay();

		if ( ( mask & byte ) == 0 ) {
			SDA = 0;
		}
		else {
			SDA = 1;
		}

		mask >>= 1;
		Delay();
		SCL = 1;
		Delay();
	}

	SCL = 0;
	SDA = 1;
	Delay();
	SCL = 1;
	j = SDA;
	Delay();
	SCL = 0;
}

uint8 I2CRead ( void ) {
	uint8 byte;
	uint8 i;
	byte = 0;

	for ( i = 0; i < 8; i++ ) {
		SCL = 0;
		SDA = 1;
		Delay();
		SCL = 1;
		Delay();
		byte <<= 1;

		if ( SDA == 1 ) {
			byte |= 0x01;
		}

		Delay();
	}

	SCL = 0;
	SDA = 1;
	Delay();
	SCL = 1;
	Delay();
	SCL = 0;
	return byte;
}

uint8 read_eeprom ( uint8 addr ) {
	uint8 databyte;
	I2CStart();
	I2CSend ( 0xa0 );
	I2CSend ( addr );
	I2CStart();
	I2CSend ( 0xa1 );
	databyte = I2CRead();
	I2CStop();
	return databyte;
}

void write_eeprom ( uint8 addr, uint8 databyte ) {
	I2CStart();
	I2CSend ( 0xa0 );
	I2CSend ( addr );
	I2CSend ( databyte );
	I2CStop();
}