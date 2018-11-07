#include "STC12C5A.h"
#include "12864.h"

unsigned char code ucStr1[] = "慧净电子--欢迎您";
unsigned char code ucStr2[] = " HJ-C52 1T单片机";
unsigned char code ucStr3[] = " WWW.HJMCU.COM  ";
unsigned char code ucStr4[] = " 2013 新年快乐  ";

void main ( void ) {
    LCD12864_Init();
    LCD12864_DisplayOneLine ( 0x80, ucStr1 );
    LCD12864_DisplayOneLine ( 0x90, ucStr2 );
    LCD12864_DisplayOneLine ( 0x88, ucStr3 );
    LCD12864_DisplayOneLine ( 0x98, ucStr4 );

    while ( 1 );
}