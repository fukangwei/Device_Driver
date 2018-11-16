#include "myiic.h"
#include "delay.h"

void IIC_Init ( void ) { /* 初始化IIC */
    GPIO_InitTypeDef GPIO_InitStructure;
    RCC_APB2PeriphClockCmd ( RCC_APB2Periph_GPIOB, ENABLE );
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP ; /* 推挽输出 */
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init ( GPIOB, &GPIO_InitStructure );
    IIC_SCL = 1;
    IIC_SDA = 1;
}

void IIC_Start ( void ) { /* 产生IIC起始信号 */
    SDA_OUT(); /* sda线输出 */
    IIC_SDA = 1;
    IIC_SCL = 1;
    delay_us ( 4 );
    IIC_SDA = 0; /* START: when CLK is high, DATA change form high to low */
    delay_us ( 4 );
    IIC_SCL = 0; /* 钳住I2C总线，准备发送或接收数据 */
}

void IIC_Stop ( void ) { /* 产生IIC停止信号 */
    SDA_OUT(); /* sda线输出 */
    IIC_SCL = 0;
    IIC_SDA = 0; /* STOP: when CLK is high DATA change form low to high */
    delay_us ( 4 );
    IIC_SCL = 1;
    IIC_SDA = 1; /* 发送I2C总线结束信号 */
    delay_us ( 4 );
}

u8 IIC_Wait_Ack ( void ) { /* 等待应答信号到来，返回值1表示接收应答失败，0表示成功 */
    u8 ucErrTime = 0;
    SDA_IN(); /* SDA设置为输入 */
    IIC_SDA = 1;
    delay_us ( 1 );
    IIC_SCL = 1;
    delay_us ( 1 );

    while ( READ_SDA ) {
        ucErrTime++;

        if ( ucErrTime > 250 ) {
            IIC_Stop();
            return 1;
        }
    }

    IIC_SCL = 0; /* 时钟输出0 */
    return 0;
}

void IIC_Ack ( void ) { /* 产生ACK应答 */
    IIC_SCL = 0;
    SDA_OUT();
    IIC_SDA = 0;
    delay_us ( 2 );
    IIC_SCL = 1;
    delay_us ( 2 );
    IIC_SCL = 0;
}

void IIC_NAck ( void ) { /* 不产生ACK应答 */
    IIC_SCL = 0;
    SDA_OUT();
    IIC_SDA = 1;
    delay_us ( 2 );
    IIC_SCL = 1;
    delay_us ( 2 );
    IIC_SCL = 0;
}

void IIC_Send_Byte ( u8 txd ) { /* IIC发送一个字节，返回从机有无应答，1表示有应答，0表示无应答 */
    u8 t;
    SDA_OUT();
    IIC_SCL = 0; /* 拉低时钟开始数据传输 */

    for ( t = 0; t < 8; t++ ) {
        IIC_SDA = ( txd & 0x80 ) >> 7;
        txd <<= 1;
        delay_us ( 2 );
        IIC_SCL = 1;
        delay_us ( 2 );
        IIC_SCL = 0;
        delay_us ( 2 );
    }
}

u8 IIC_Read_Byte ( unsigned char ack ) { /* 读1个字节，当ack为1时，发送ACK；ack为0时，发送nACK */
    unsigned char i, receive = 0;
    SDA_IN(); /* SDA设置为输入 */

    for ( i = 0; i < 8; i++ ) {
        IIC_SCL = 0;
        delay_us ( 2 );
        IIC_SCL = 1;
        receive <<= 1;

        if ( READ_SDA ) {
            receive++;
        }

        delay_us ( 1 );
    }

    if ( !ack ) {
        IIC_NAck(); /* 发送nACK */
    } else {
        IIC_Ack(); /* 发送ACK */
    }

    return receive;
}