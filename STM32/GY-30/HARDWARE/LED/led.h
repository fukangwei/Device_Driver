#ifndef __LED_H
#define __LED_H
#include "stm32f10x.h"

#define LED0 PBout(10)
#define LED1 PBout(11)

void LED_Init ( void );

#endif