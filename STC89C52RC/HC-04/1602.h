#define uchar unsigned char
#define uint unsigned int

sbit dula  = P2 ^ 6;
sbit wela  = P2 ^ 7;
sbit rs    = P1 ^ 0;
sbit rw    = P1 ^ 1;
sbit lcden = P2 ^ 5;

void delay ( uint x ) {
    uint a, b;

    for ( a = x; a > 0; a-- ) {
        for ( b = 10; b > 0; b-- ) {
            ;
        }
    }
}

void write_com ( uchar com ) { /* Lcd1602写命令函数 */
    P0 = com;
    rs = 0;
    lcden = 0;
    delay ( 10 );
    lcden = 1;
    delay ( 10 );
    lcden = 0;
}

void write_date ( uchar date ) { /* Lcd1602写数据函数 */
    P0 = date;
    rs = 1;
    lcden = 0;
    delay ( 10 );
    lcden = 1;
    delay ( 10 );
    lcden = 0;
}

void initLCD ( void ) { /* Lcd1602初始化函数 */
    rw = 0;
    dula = 0;
    wela = 0;
    write_com ( 0x38 );
    delay ( 20 );
    write_com ( 0x0f );
    delay ( 20 );
    write_com ( 0x06 );
    delay ( 20 );
    write_com ( 0x01 );
    delay ( 20 );
}