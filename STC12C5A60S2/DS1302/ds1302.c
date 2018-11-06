#include "ds1302.h"

uint8 set_ds1302_time[7] = {0, 0, 0, 0, 0, 0, 0}; /* ����DS1302�ĳ�ʼʱ�䣬��ʽΪ�����ʱ�������ꡱ */
uint8 read_ds1302_time[7] = {0, 0, 0, 0, 0, 0, 0}; /* ��ȡDS1302�е�����ʱ�䣬��ʽΪ�����ʱ�������ꡱ */

/* ��DS1302дһ���ֽ� */
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

/* ��DS1302��һ���ֽ� */
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

/* ��DS1302�豸�������� */
void reset_ds1302 ( void )
{
    RST = 0;
    SCK = 0;
    RST = 1;
}

/* DS1302���д���� */
void clear_ds1302_WP ( void )
{
    reset_ds1302();
    RST = 1;
    write_ds1302_byte ( 0x8E );
    write_ds1302_byte ( 0 );
    SDA = 0;
    RST = 0;
}

/* ��DS1302����д���� */
void set_ds1302_WP ( void )
{
    reset_ds1302();
    RST = 1;
    write_ds1302_byte ( 0x8E );
    write_ds1302_byte ( 0x80 );
    SDA = 0;
    RST = 0;
}

/* ������д��DS1302 */
void write_ds1302 ( uint8 addr, uint8 dat )
{
    reset_ds1302();
    RST = 1;
    write_ds1302_byte ( addr );
    write_ds1302_byte ( dat );
    SDA = 0;
    RST = 0;
}

/* �ӵ�ַ�ж���DS1302���� */
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

/* ��DS1302�趨ʱ������ */
void set_time ( uint8 *timedata )
{
    uint8 i, tmp;

    for ( i = 0; i < 7; i++ ) //ת��ΪBCD��ʽ
    {
        tmp = timedata[i] / 10;
        timedata[i] = timedata[i] % 10;
        timedata[i] = timedata[i] + tmp * 16;
    }

    clear_ds1302_WP();
    tmp = DS1302_W_ADDR; //��д��ַ

    for ( i = 0; i < 7; i++ ) //��7��д�룬�ֱ��ǡ����ʱ�������ꡱ
    {
        write_ds1302 ( tmp, timedata[i] );
        tmp += 2;
    }

    set_ds1302_WP();
}

/* ��ʱ�����ݣ�BCD��ʽ�� */
void read_time ( uint8 *timedata )
{
    uint8 i, tmp;
    tmp = DS1302_R_ADDR;

    for ( i = 0; i < 7; i++ ) //��7�ζ�ȡ����ʽ�ǡ����ʱ�������ꡱ
    {
        timedata[i] = read_ds1302 ( tmp );
        tmp += 2;
    }
}
