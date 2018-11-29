#ifndef __DS18B20_H
#define __DS18B20_H
#include "sys.h"

#define DS18B20_IO_IN()  {GPIOA->CRL &= 0XFFFFFFF0; GPIOA->CRL |= 8 << 0;} /* ��PA0���ó�����/��������ģʽ */
#define DS18B20_IO_OUT() {GPIOA->CRL &= 0XFFFFFFF0; GPIOA->CRL |= 3 << 0;} /* ��PA0���ó�ͨ���������ģʽ */
#define DS18B20_DQ_OUT PAout(0) /* PA0Ϊ��������˿� */
#define DS18B20_DQ_IN  PAin(0)  /* PA0Ϊ��������˿� */

u8 DS18B20_Init ( void );
short DS18B20_Get_Temp ( void );
#endif