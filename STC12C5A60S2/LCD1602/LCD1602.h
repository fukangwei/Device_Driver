#ifndef __LCD1602_H_
#define __LCD1602_H_

#include <reg52.h>

typedef unsigned char uint8;

sbit RS   = P1 ^ 0;
sbit RW   = P1 ^ 1;
sbit EN   = P2 ^ 5;
sbit STA7 = P0 ^ 7;

void Init_LCD1602 ( void );
void w_string ( uint8 addr_start, uint8 *p );

#endif