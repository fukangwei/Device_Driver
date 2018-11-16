#ifndef __SPI_H
#define __SPI_H
#include "sys.h"

#define WIZ_SCS  GPIO_Pin_4
#define WIZ_SCLK GPIO_Pin_5
#define WIZ_MISO GPIO_Pin_6
#define WIZ_MOSI GPIO_Pin_7

void WIZ_CS ( uint8_t val );
void SPIx_Init ( void );
void SPIx_SetSpeed ( u8 SpeedSet );
u8 SPIx_ReadWriteByte ( u8 TxData );
#endif