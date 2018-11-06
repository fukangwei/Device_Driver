#ifndef AT24C02_H
#define AT24C02_H

#include <reg52.h>

typedef unsigned char uint8;
typedef unsigned int uint16;
typedef unsigned long uint32;

sbit SCL = P2 ^ 1;
sbit SDA = P2 ^ 0;

void Delay ( void );
void InitI2C ( void );
void I2CStart ( void );
void I2CStop ( void );
void I2CSend ( uint8 byte );
uint8 I2CRead ( void );
uint8 read_eeprom ( uint8 addr ) ;
void write_eeprom ( uint8 addr, uint8 databyte );

#endif