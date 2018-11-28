#ifndef _SPI_H
#define _SPI_H
#include <STC12C5A.H>

void SPI_Init ( void );
unsigned char SPI_Read ( void );
unsigned char SPI_Write ( unsigned char txdata );
void delay_ms ( int num );
void delay_10us ( char n );
#endif