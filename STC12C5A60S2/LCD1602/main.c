#include <reg52.h>
#include "LCD1602.h"
#include <intrins.h>

unsigned char code word1[] = {"QQ:398115088"};
unsigned char code word2[] = {"WWW.HJMCU.COM"};

int main ( void ) {
    Init_LCD1602();
    w_string ( 0x80, word1 );
    w_string ( 0xC0, word2 );

    while ( 1 );
}