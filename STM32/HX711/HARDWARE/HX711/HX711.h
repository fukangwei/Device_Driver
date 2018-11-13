#ifndef _HX711_H
#define _HX711_H
#include "sys.h"

#define DOUT PBout(12)
#define SCK  PBout(13)
#define DIN  PBin(12)

void HX711_InIt ( void );
void Get_Weight ( void );
u32 Get_Maopi ( void );

#endif
