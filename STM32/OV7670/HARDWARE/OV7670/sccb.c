#include "sys.h"
#include "sccb.h"
#include "delay.h"

void SCCB_Init ( void ) { /* ��ʼ��SCCB�ӿڣ���CHECK OK�� */
    RCC->APB2ENR |= 1 << 4; /* ��ʹ������PORTCʱ�� */
    /* PC4��5��OUT */
    GPIOC->CRL &= 0XFF00FFFF;
    GPIOC->CRL |= 0X00330000;
    GPIOC->ODR |= 3 << 4;
    SCCB_SDA_OUT();
}

/* SCCB��ʼ�źţ���ʱ��Ϊ�ߵ�ʱ�������ߴӸߵ���ΪSCCB��ʼ�źš��ڼ���״̬�£�SDA��SCL��Ϊ�͵�ƽ */
void SCCB_Start ( void ) {
    SCCB_SDA = 1; /* �����߸ߵ�ƽ */
    SCCB_SCL = 1; /* ��ʱ���߸ߵ�ʱ���������ɸ����� */
    delay_us ( 50 );
    SCCB_SDA = 0;
    delay_us ( 50 );
    SCCB_SCL = 0; /* �����߻ָ��͵�ƽ��������������Ҫ */
}

void SCCB_Stop ( void ) { /* SCCBֹͣ�źţ���ʱ��Ϊ�ߵ�ʱ�������ߴӵ͵���ΪSCCBֹͣ�ź� */
    SCCB_SDA = 0;
    delay_us ( 50 );
    SCCB_SCL = 1;
    delay_us ( 50 );
    SCCB_SDA = 1;
    delay_us ( 50 );
}

void SCCB_No_Ack ( void ) { /* ����NA�ź� */
    delay_us ( 50 );
    SCCB_SDA = 1;
    SCCB_SCL = 1;
    delay_us ( 50 );
    SCCB_SCL = 0;
    delay_us ( 50 );
    SCCB_SDA = 0;
    delay_us ( 50 );
}

u8 SCCB_WR_Byte ( u8 dat ) { /* SCCBд��һ���ֽڣ�����0Ϊ�ɹ���1Ϊʧ�� */
    u8 j, res;

    for ( j = 0; j < 8; j++ ) { /* ѭ��8�η������� */
        if ( dat & 0x80 ) {
            SCCB_SDA = 1;
        } else {
            SCCB_SDA = 0;
        }

        dat <<= 1;
        delay_us ( 50 );
        SCCB_SCL = 1;
        delay_us ( 50 );
        SCCB_SCL = 0;
    }

    SCCB_SDA_IN(); /* ����SDAΪ���� */
    delay_us ( 50 );
    SCCB_SCL = 1; /* ���յھ�λ�����ж��Ƿ��ͳɹ� */
    delay_us ( 50 );

    if ( SCCB_READ_SDA ) {
        res = 1; /* ����ʧ�ܷ���1 */
    } else {
        res = 0; /* ���ͳɹ�����0 */
    }

    SCCB_SCL = 0;
    SCCB_SDA_OUT(); /* ����SDAΪ��� */
    return res;
}

u8 SCCB_RD_Byte ( void ) {
    u8 temp = 0, j;
    SCCB_SDA_IN(); /* ����SDAΪ���� */

    for ( j = 8; j > 0; j-- ) { /* ѭ��8�ν������� */
        delay_us ( 50 );
        SCCB_SCL = 1;
        temp = temp << 1;

        if ( SCCB_READ_SDA ) {
            temp++;
        }

        delay_us ( 50 );
        SCCB_SCL = 0;
    }

    SCCB_SDA_OUT(); /* ����SDAΪ��� */
    return temp;
}

u8 SCCB_WR_Reg ( u8 reg, u8 data ) { /* д�Ĵ���������0Ϊ�ɹ���1Ϊʧ�� */
    u8 res = 0;
    SCCB_Start(); /* ����SCCB���� */

    if ( SCCB_WR_Byte ( SCCB_ID ) ) {
        res = 1; /* д����ID */
    }

    delay_us ( 100 );

    if ( SCCB_WR_Byte ( reg ) ) {
        res = 1; /* д�Ĵ�����ַ */
    }

    delay_us ( 100 );

    if ( SCCB_WR_Byte ( data ) ) {
        res = 1; /* д���� */
    }

    SCCB_Stop();
    return  res;
}

u8 SCCB_RD_Reg ( u8 reg ) { /* ���Ĵ��������ض����ļĴ���ֵ */
    u8 val = 0;
    SCCB_Start(); /* ����SCCB���� */
    SCCB_WR_Byte ( SCCB_ID ); /* д����ID */
    delay_us ( 100 );
    SCCB_WR_Byte ( reg ); /* д�Ĵ�����ַ */
    delay_us ( 100 );
    SCCB_Stop();
    delay_us ( 100 );
    SCCB_Start();
    SCCB_WR_Byte ( SCCB_ID | 0X01 ); /* ���Ͷ����� */
    delay_us ( 100 );
    val = SCCB_RD_Byte(); /* ��ȡ���� */
    SCCB_No_Ack();
    SCCB_Stop();
    return val;
}