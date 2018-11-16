#ifndef DS1302_H
#define DS1302_H

#include <reg52.h>

typedef unsigned char uint8;
typedef unsigned int  uint16;

sbit SCK = P2 ^ 1; /* ʱ���ź��� */
sbit SDA = P2 ^ 0; /* �����ź��� */
sbit RST = P2 ^ 4; /* DS1302��λ(Ƭѡ) */

#define DS1302_W_ADDR 0x80 /* DS1302д��ַ */
#define DS1302_R_ADDR 0x81 /* DS1302����ַ */

void write_ds1302_byte ( uint8 dat );
uint8 read_ds1302_byte ( void );
void reset_ds1302 ( void );
void clear_ds1302_WP ( void );
void set_ds1302_WP ( void );
void write_ds1302 ( uint8 addr, uint8 dat );
uint8 read_ds1302 ( uint8 addr );
void set_time ( uint8 *timedata );
void read_time ( uint8 *timedata );

#endif