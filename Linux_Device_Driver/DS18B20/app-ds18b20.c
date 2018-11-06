#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <linux/ioctl.h>

void ds18b20_delay ( int i );

int main() {
    int fd, i;
    unsigned char result[2]; /* 从ds18b20读出的结果，result[0]存放低八位 */
    unsigned char integer_value = 0;
    float decimal_value = 0; /* 温度数值，decimal_value为小数部分的值 */
    float temperature = 0;
    fd = open ( "/dev/ds18b20", 0 );

    if ( fd < 0 ) {
        perror ( "open device failed\n" );
        exit ( 1 );
    }

    while ( 1 ) {
        i++;
        read ( fd, &result, sizeof ( result ) );
        integer_value = ( ( result[0] & 0xf0 ) >> 4 ) | ( ( result[1] & 0x07 ) << 4 );
        decimal_value = 0.5 * ( ( result[0] & 0x0f ) >> 3 ) + 0.25 * ( ( result[0] & 0x07 ) >> 2 ); /* 精确到0.25度 */
        temperature = ( float ) integer_value + decimal_value;
        printf ( "Current Temperature:%6.2f\n", temperature );
        ds18b20_delay ( 500 );
    }
}

void ds18b20_delay ( int i ) {
    int j, k;

    for ( j = 0; j < i; j++ )
        for ( k = 0; k < 50000; k++ );
}