#include "ds1302.h"

uint8 set_ds1302_time[7] = {0, 0, 0, 0, 0, 0, 0}; /* 设置DS1302的初始时间，格式为“秒分时日月周年” */
uint8 read_ds1302_time[7] = {0, 0, 0, 0, 0, 0, 0}; /* 读取DS1302中的运行时间，格式为“秒分时日月周年” */

/* 向DS1302写一个字节 */
void write_ds1302_byte ( uint8 dat )
{
    uint8 i;

    for ( i = 0; i < 8; i++ )
    {
        SDA = dat & 0x01;
        SCK = 1;
        dat >>= 1;
        SCK = 0;
    }
}

/* 从DS1302读一个字节 */
uint8 read_ds1302_byte ( void )
{
    uint8 i, dat = 0;

    for ( i = 0; i < 8; i++ )
    {
        dat >>= 1;

        if ( SDA )
        {
            dat |= 0x80;
        }

        SCK = 1;
        SCK = 0;
    }

    return dat;
}

/* 对DS1302设备进行重启 */
void reset_ds1302 ( void )
{
    RST = 0;
    SCK = 0;
    RST = 1;
}

/* DS1302清除写保护 */
void clear_ds1302_WP ( void )
{
    reset_ds1302();
    RST = 1;
    write_ds1302_byte ( 0x8E );
    write_ds1302_byte ( 0 );
    SDA = 0;
    RST = 0;
}

/* 对DS1302设置写保护 */
void set_ds1302_WP ( void )
{
    reset_ds1302();
    RST = 1;
    write_ds1302_byte ( 0x8E );
    write_ds1302_byte ( 0x80 );
    SDA = 0;
    RST = 0;
}

/* 将数据写入DS1302 */
void write_ds1302 ( uint8 addr, uint8 dat )
{
    reset_ds1302();
    RST = 1;
    write_ds1302_byte ( addr );
    write_ds1302_byte ( dat );
    SDA = 0;
    RST = 0;
}

/* 从地址中读出DS1302数据 */
uint8 read_ds1302 ( uint8 addr )
{
    uint8 temp = 0;
    reset_ds1302();
    RST = 1;
    write_ds1302_byte ( addr );
    temp = read_ds1302_byte();
    SDA = 0;
    RST = 0;
    return ( temp );
}

/* 对DS1302设定时钟数据 */
void set_time ( uint8 *timedata )
{
    uint8 i, tmp;

    for ( i = 0; i < 7; i++ ) //转化为BCD格式
    {
        tmp = timedata[i] / 10;
        timedata[i] = timedata[i] % 10;
        timedata[i] = timedata[i] + tmp * 16;
    }

    clear_ds1302_WP();
    tmp = DS1302_W_ADDR; //传写地址

    for ( i = 0; i < 7; i++ ) //分7次写入，分别是“秒分时日月周年”
    {
        write_ds1302 ( tmp, timedata[i] );
        tmp += 2;
    }

    set_ds1302_WP();
}

/* 读时钟数据（BCD格式） */
void read_time ( uint8 *timedata )
{
    uint8 i, tmp;
    tmp = DS1302_R_ADDR;

    for ( i = 0; i < 7; i++ ) //分7次读取，格式是“秒分时日月周年”
    {
        timedata[i] = read_ds1302 ( tmp );
        tmp += 2;
    }
}
