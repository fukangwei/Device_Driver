#ifndef DS18B20_H
#define DS18B20_H

#include <reg52.h>

#define uchar unsigned char
#define uint unsigned int

sbit DS = P2 ^ 2; /* define the interface */

void delay ( uint count );
void dsreset ( void );
bit tmpreadbit ( void );
uchar tmpread ( void ) ;
void tmpwritebyte ( uchar dat );
void tmpchange ( void );
uint tmp ( void );

#endif