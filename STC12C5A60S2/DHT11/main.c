#include "STC12C5A.H"
#include "delay.h"
#include "type.h"

sbit dht11 = P0 ^ 1; /* 定义DHT11接口 */

U8 U8FLAG;
U8 U8temp;
U8 U8T_data_H, U8T_data_L, U8RH_data_H, U8RH_data_L, U8checkdata;
U8 U8T_data_H_temp, U8T_data_L_temp, U8RH_data_H_temp, U8RH_data_L_temp, U8checkdata_temp;
U8 U8comdata;
U8 str[20];

void UART_init ( void ) {
    PCON &= 0x7F;
    SCON = 0x50;
    AUXR &= 0xFB;
    BRT = 0xFD;
    AUXR |= 0x01;
    AUXR |= 0x10;
}

void UART_send_byte ( uchar dat ) {
    ES = 0;
    TI = 0;
    SBUF = dat;

    while ( TI == 0 );

    TI = 0;
    ES = 1;
}

void UART_send_string ( uchar *buf ) {
    while ( *buf != '\0' ) {
        UART_send_byte ( *buf++ );
    }
}

void COM ( void ) {
    U8 i;

    for ( i = 0; i < 8; i++ ) {
        U8FLAG = 2;

        /* 每一bit数据都以50us低电平开始，高电平的长短决定了数据位是0还是1 */
        while ( ( dht11 == 0 ) && U8FLAG++ );

        /* 高电平持续26us-28us，表示数字0信号 */
        Delay_10us();
        Delay_10us();
        Delay_10us();
        U8temp = 0;

        /* 高电平持续70us，表示数字1信号 */
        if ( dht11 ) {
            U8temp = 1;
        }

        U8FLAG = 2;

        while ( dht11 && U8FLAG++ );

        if ( U8FLAG == 1 ) { /* 超时则跳出for循环 */
            break;
        }

        /* 将数据填充进U8comdata中 */
        U8comdata <<= 1;
        U8comdata |= U8temp;
    }
}

/*--------------------------------
 * 功能：温湿度读取子程序
 * 温度高8位 == U8T_data_H
 * 温度低8位 == U8T_data_L
 * 湿度高8位 == U8RH_data_H
 * 湿度低8位 == U8RH_data_L
 * 校验位8位 == U8checkdata
 *-------------------------------*/
void RH ( void ) {
    /* 主机把总线拉低大于18ms，保证DHT11能检测到起始信号 */
    dht11 = 0;
    Delay18ms();
    /* 主机发送开始信号结束后，延时等待20us-40us，读取DHT11的响应信号 */
    dht11 = 1;
    Delay_10us();
    Delay_10us();
    dht11 = 1; /* 主机发送开始信号结束后，可以切换到输入模式，或者输出高电平，判断从机响应信号 */

    if ( dht11 == 0 ) { /* 总线为低电平，说明DHT11发送响应信号 */
        U8FLAG = 2;

        /* DHT11接收到主机的开始信号后，等待主机开始信号结束，然后发送80us的低电平响应信号。判断从机是否发出80us的低电平响应信号 */
        while ( ( dht11 == 0 ) && U8FLAG++ );

        U8FLAG = 2;

        /* DHT11发送响应信号后，再把总线拉高80us，准备发送数据。判断从机是否发出80us的高电平，如果发出则进入数据接收状态 */
        while ( dht11 && U8FLAG++ );

        /* 数据接收状态 */
        COM();
        U8RH_data_H_temp = U8comdata; /* 8bit湿度整数数据 */
        COM();
        U8RH_data_L_temp = U8comdata; /* 8bit湿度小数数据 */
        COM();
        U8T_data_H_temp = U8comdata;  /* 8bit温度整数数据 */
        COM();
        U8T_data_L_temp = U8comdata;  /* 8bit温度小数数据 */
        COM();
        U8checkdata_temp = U8comdata; /* 数据校验和 */
        dht11 = 1;
        /* 数据校验：数据传送正确的校验和数据等于“8bit湿度整数数据 + 8bit湿度小数数据 + 8bit温度整数数据 + 8bit温度小数数据”所得结果的末8位 */
        U8temp = ( U8T_data_H_temp + U8T_data_L_temp + U8RH_data_H_temp + U8RH_data_L_temp );

        if ( U8temp == U8checkdata_temp ) { /* 若校验正确，则将数据赋给相应的变量 */
            U8RH_data_H = U8RH_data_H_temp;
            U8RH_data_L = U8RH_data_L_temp;
            U8T_data_H  = U8T_data_H_temp;
            U8T_data_L  = U8T_data_L_temp;
            U8checkdata = U8checkdata_temp;
        }
    }
}

int main ( void ) {
    UART_init();

    while ( 1 ) {
        RH();
        str[0] = 'H';
        str[1] = ':';
        str[2] = U8RH_data_H / 10 + '0';
        str[3] = U8RH_data_H % 10 + '0';
        str[4] = 'T';
        str[5] = ':';
        str[6] = U8T_data_H / 10 + '0';
        str[7] = U8T_data_H % 10 + '0';
        str[8] = '\r';
        str[9] = '\n';
        str[10] = '\0';
        UART_send_string ( str ) ; /* 发送到串口 */
        /* 采样周期间隔不得低于1秒钟 */
        Delay500ms();
        Delay500ms();
    }
}