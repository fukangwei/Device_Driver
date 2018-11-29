#ifndef __DS18B20_H
#define __DS18B20_H
#include "sys.h"

#define DS18B20_IO_IN()  {GPIOA->CRL &= 0XFFFFFFF0; GPIOA->CRL |= 8 << 0;} /* 将PA0配置成上拉/下拉输入模式 */
#define DS18B20_IO_OUT() {GPIOA->CRL &= 0XFFFFFFF0; GPIOA->CRL |= 3 << 0;} /* 将PA0配置成通用推挽输出模式 */
#define DS18B20_DQ_OUT PAout(0) /* PA0为数据输出端口 */
#define DS18B20_DQ_IN  PAin(0)  /* PA0为数据输入端口 */

u8 DS18B20_Init ( void );
short DS18B20_Get_Temp ( void );
#endif