#ifndef __SPI_H
#define __SPI_H

#include "sys.h"

void SPIx_Init ( void );
void SPIx_SetSpeed ( u8 SpeedSet );
u8 SPIx_ReadWriteByte ( u8 TxData ); 

#endif