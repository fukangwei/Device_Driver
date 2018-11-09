#ifndef __LED_H
#define __LED_H
#include "stm32f10x.h"

#define LED PCout(4)
void LED_Init ( void );

#endif