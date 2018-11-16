#include "myiic.h"
#include "delay.h"

void IIC_Init ( void ) { /* ��ʼ��IIC */
    GPIO_InitTypeDef GPIO_InitStructure;
    RCC_APB2PeriphClockCmd ( RCC_APB2Periph_GPIOB, ENABLE );
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP ; /* ������� */
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init ( GPIOB, &GPIO_InitStructure );
    IIC_SCL = 1;
    IIC_SDA = 1;
}

void IIC_Start ( void ) { /* ����IIC��ʼ�ź� */
    SDA_OUT(); /* sda����� */
    IIC_SDA = 1;
    IIC_SCL = 1;
    delay_us ( 4 );
    IIC_SDA = 0; /* START: when CLK is high, DATA change form high to low */
    delay_us ( 4 );
    IIC_SCL = 0; /* ǯסI2C���ߣ�׼�����ͻ�������� */
}

void IIC_Stop ( void ) { /* ����IICֹͣ�ź� */
    SDA_OUT(); /* sda����� */
    IIC_SCL = 0;
    IIC_SDA = 0; /* STOP: when CLK is high DATA change form low to high */
    delay_us ( 4 );
    IIC_SCL = 1;
    IIC_SDA = 1; /* ����I2C���߽����ź� */
    delay_us ( 4 );
}

u8 IIC_Wait_Ack ( void ) { /* �ȴ�Ӧ���źŵ���������ֵ1��ʾ����Ӧ��ʧ�ܣ�0��ʾ�ɹ� */
    u8 ucErrTime = 0;
    SDA_IN(); /* SDA����Ϊ���� */
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

    IIC_SCL = 0; /* ʱ�����0 */
    return 0;
}

void IIC_Ack ( void ) { /* ����ACKӦ�� */
    IIC_SCL = 0;
    SDA_OUT();
    IIC_SDA = 0;
    delay_us ( 2 );
    IIC_SCL = 1;
    delay_us ( 2 );
    IIC_SCL = 0;
}

void IIC_NAck ( void ) { /* ������ACKӦ�� */
    IIC_SCL = 0;
    SDA_OUT();
    IIC_SDA = 1;
    delay_us ( 2 );
    IIC_SCL = 1;
    delay_us ( 2 );
    IIC_SCL = 0;
}

void IIC_Send_Byte ( u8 txd ) { /* IIC����һ���ֽڣ����شӻ�����Ӧ��1��ʾ��Ӧ��0��ʾ��Ӧ�� */
    u8 t;
    SDA_OUT();
    IIC_SCL = 0; /* ����ʱ�ӿ�ʼ���ݴ��� */

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

u8 IIC_Read_Byte ( unsigned char ack ) { /* ��1���ֽڣ���ackΪ1ʱ������ACK��ackΪ0ʱ������nACK */
    unsigned char i, receive = 0;
    SDA_IN(); /* SDA����Ϊ���� */

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
        IIC_NAck(); /* ����nACK */
    } else {
        IIC_Ack(); /* ����ACK */
    }

    return receive;
}