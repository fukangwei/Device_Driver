#ifndef __HX711_H__
#define __HX711_H__

#include <reg52.h>

sbit HX711_DOUT = P2 ^ 0;
sbit HX711_SCK = P2 ^ 1;

void Delay__hx711_us ( void );
unsigned long HX711_Read ( void );

#endif