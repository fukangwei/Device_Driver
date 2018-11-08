#ifndef __DHT11_H
#define __DHT11_H
#include "sys.h"

/* IO·½ÏòÉèÖÃ */
#define DHT11_IO_IN()  {GPIOA->CRL &= 0XFFFFFFF0; GPIOA->CRL |= 8 << 0;}
#define DHT11_IO_OUT() {GPIOA->CRL &= 0XFFFFFFF0; GPIOA->CRL |= 3 << 0;}

#define DHT11_DQ_OUT PAout(0)
#define DHT11_DQ_IN  PAin(0)

u8 DHT11_Init ( void );
u8 DHT11_Read_Data ( u8 *temp, u8 *humi );
u8 DHT11_Read_Byte ( void );
u8 DHT11_Read_Bit ( void );
u8 DHT11_Check ( void );
void DHT11_Rst ( void );

#endif