#include <intrins.h>

void Delay_10us() {
    unsigned char i;
    _nop_();
    _nop_();
    _nop_();
    i = 24;

    while ( --i );
}

void Delay18ms() {
    unsigned char i, j, k;
    _nop_();
    _nop_();
    i = 1;
    j = 194;
    k = 159;

    do {
        do {
            while ( --k );
        } while ( --j );
    } while ( --i );
}

void Delay500ms() {
    unsigned char i, j, k;
    i = 22;
    j = 3;
    k = 227;

    do {
        do {
            while ( --k );
        } while ( --j );
    } while ( --i );
}