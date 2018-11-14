#ifndef __LED_H
#define __LED_H
#include "stm32f10x.h"

#define LED0 PCout(4)
#define LED1 PDout(2)

void LED_Init ( void );

#endif