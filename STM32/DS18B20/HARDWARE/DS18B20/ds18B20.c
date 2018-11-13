#include "ds18b20.h"
#include "delay.h"

static void DS18B20_Rst ( void ) { /* ��λDS18B20 */
    DS18B20_IO_OUT();
    DS18B20_DQ_OUT = 0; /* �������ߵ�ƽ */
    delay_us ( 480 ); /* �������ͣ���ڵ͵�ƽ����480us�������ϵ�����������������λ */
    DS18B20_DQ_OUT = 1; /* ʹ���߻ָ�����״̬ */
    delay_us ( 15 );
}

static u8 DS18B20_Check ( void ) { /* �ȴ�DS18B20�Ļ�Ӧ������1��ʾδ��⵽DS18B20������0��ʾ��⵽ */
    u8 i = 0;
    DS18B20_IO_IN();

    while ( DS18B20_DQ_IN && i++ < 60 ) {
        delay_us ( 1 );
    }

    if ( i >= 60 ) {
        return 1;
    }

    i = 0;

    while ( !DS18B20_DQ_IN && i++ < 240 ) {
        delay_us ( 1 );
    };

    if ( i >= 240 ) {
        return 1;
    }

    return 0;
}

static u8 DS18B20_Read_Bit ( void ) { /* ��DS18B20��ȡһ��λ */
    u8 data;
    DS18B20_IO_OUT();
    DS18B20_DQ_OUT = 0;
    delay_us ( 2 );
    DS18B20_DQ_OUT = 1;
    DS18B20_IO_IN();
    delay_us ( 12 );

    if ( DS18B20_DQ_IN ) {
        data = 1;
    } else {
        data = 0;
    }

    delay_us ( 50 );
    return data;
}

static u8 DS18B20_Read_Byte ( void ) { /* ��DS18B20��ȡһ���ֽ� */
    u8 i, j, dat;
    dat = 0;

    for ( i = 1; i <= 8; i++ ) {
        j = DS18B20_Read_Bit();
        dat = ( j << 7 ) | ( dat >> 1 );
    }

    return dat;
}

static void DS18B20_Write_Byte ( u8 dat ) { /* дһ���ֽڵ�DS18B20 */
    u8 j;
    u8 testb;
    DS18B20_IO_OUT();

    for ( j = 1; j <= 8; j++ ) {
        testb = dat & 0x01;
        dat = dat >> 1;

        if ( testb ) { /* ���д���λΪ1 */
            DS18B20_DQ_OUT = 0;
            delay_us ( 2 );
            DS18B20_DQ_OUT = 1;
            delay_us ( 60 );
        } else { /* ���д���λΪ0 */
            DS18B20_DQ_OUT = 0;
            delay_us ( 60 );
            DS18B20_DQ_OUT = 1;
            delay_us ( 2 );
        }
    }
}

static void DS18B20_Start ( void ) { /* ��ʼ�¶�ת�� */
    DS18B20_Rst();
    delay_us ( 480 ); /* ��λ֮������ʱһ��ʱ�� */
    DS18B20_Write_Byte ( 0xcc ); /* ����ROMָ�� */
    DS18B20_Write_Byte ( 0x44 ); /* �¶�ת��ָ�� */
}

u8 DS18B20_Init ( void ) { /* ��ʼ��DS18B20��IO�ڣ����DS18B20�Ƿ���ڡ�����1��ʾ�����ڣ�0��ʾ���� */
    GPIO_InitTypeDef GPIO_InitStructure;
    RCC_APB2PeriphClockCmd ( RCC_APB2Periph_GPIOA, ENABLE );
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init ( GPIOA, &GPIO_InitStructure );
    GPIO_SetBits ( GPIOA, GPIO_Pin_0 );
    DS18B20_Rst();
    return DS18B20_Check();
}

/* ��DS18B20�õ��¶�ֵ(��-550����1250)���¶�Ϊ��ʱ��ȡ��������16����ȡ�����1����ת����10���� */
short DS18B20_Get_Temp ( void ) {
    u8 temp;
    u8 TL, TH;
    short tem;
    DS18B20_Start();
    DS18B20_Rst();
    DS18B20_Check();
    DS18B20_Write_Byte ( 0xcc );
    DS18B20_Write_Byte ( 0xbe );
    TL = DS18B20_Read_Byte(); /* �¶ȵĵ�8λ */
    TH = DS18B20_Read_Byte(); /* �¶ȵĸ�8λ */

    if ( TH > 7 ) {
        TH = ~TH;
        TL = ~TL;
        TL += 1;
        temp = 0; /* �¶�Ϊ�� */
    } else {
        temp = 1; /* �¶�Ϊ�� */
    }

    tem = TH; /* ��ø߰�λ */
    tem <<= 8;
    tem += TL; /* ��õͰ�λ */
    tem = ( float ) tem * 0.625;

    if ( temp ) {
        return tem;
    } else {
        return ( -tem );
    }
}