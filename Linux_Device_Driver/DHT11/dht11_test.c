#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

int main ( void ) {
    int fd;
    int retval;
    char buf[5];
    fd = open ( "/dev/dht11" , O_RDONLY );

    if ( fd == -1 ) {
        perror ( "open dht11 error\n" );
        exit ( -1 ) ;
    }

    printf ( "open /dev/dht11 successfully\n" );
    sleep ( 2 );

    while ( 1 ) {
        sleep ( 1 );
        retval = read ( fd, buf, 5 );

        if ( retval == -1 ) {
            perror ( "read dht11 error" );
            printf ( "read dht11 error" );
            exit ( -1 );
        }

        printf ( "Humidity:%d.%d %%RH Temperature:%d.%d C\n", buf[0], buf[1], buf[2], buf[3] );
    }

    close ( fd );
}