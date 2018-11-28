#ifndef __LED_H
#define __LED_H
#include "sys.h"

#define LATCH   PAout(0)
#define SRCLK   PAout(1)
#define SER     PAout(2)
#define LATCH_B PAout(3)
#define SRCLK_B PAout(4)
#define SER_B   PAout(5)

void LED_Init ( void );
void SendByte ( unsigned char dat );
void Send2Byte ( unsigned char dat1, unsigned char dat2 );
void Out595 ( void );
void SendSeg ( unsigned char dat );
#endif