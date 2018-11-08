#include "dht11.h"
#include "delay.h"

void DHT11_Rst ( void ) { /* ��λDHT11 */
    DHT11_IO_OUT(); /* SET OUTPUT */
    DHT11_DQ_OUT = 0; /* ����DQ */
    delay_ms ( 20 ); /* ��������18ms */
    DHT11_DQ_OUT = 1; /* DQ = 1 */
    delay_us ( 30 ); /* ��������20��40us */
}

u8 DHT11_Check ( void ) { /* �ȴ�DHT11�Ļ�Ӧ������0��ʾDHT11���ڣ�����1��ʾ������ */
    u8 retry = 0;
    DHT11_IO_IN(); /* SET INPUT */

    while ( DHT11_DQ_IN && retry < 100 ) { /* DHT11������40��80us */
        retry++;
        delay_us ( 1 );
    };

    if ( retry >= 100 ) {
        return 1;
    } else {
        retry = 0;
    }

    while ( !DHT11_DQ_IN && retry < 100 ) { /* DHT11���ͺ���ٴ�����40��80us */
        retry++;
        delay_us ( 1 );
    };

    if ( retry >= 100 ) {
        return 1;
    }

    return 0;
}

u8 DHT11_Read_Bit ( void ) { /* ��DHT11��ȡһ��λ */
    u8 retry = 0;

    while ( DHT11_DQ_IN && retry < 100 ) { /* �ȴ���Ϊ�͵�ƽ */
        retry++;
        delay_us ( 1 );
    }

    retry = 0;

    while ( !DHT11_DQ_IN && retry < 100 ) { /* �ȴ���ߵ�ƽ */
        retry++;
        delay_us ( 1 );
    }

    delay_us ( 40 ); /* �ȴ�40us */

    if ( DHT11_DQ_IN ) {
        return 1;
    } else {
        return 0;
    }
}

u8 DHT11_Read_Byte ( void ) { /* ��DHT11��ȡһ���ֽ� */
    u8 i, dat;
    dat = 0;

    for ( i = 0; i < 8; i++ ) {
        dat <<= 1;
        dat |= DHT11_Read_Bit();
    }

    return dat;
}

u8 DHT11_Read_Data ( u8 *temp, u8 *humi ) { /* ��DHT11��ȡһ�����ݣ�����temp�¶�ֵ(0��50)��humiʪ��ֵ(20%��90%) */
    u8 buf[5];
    u8 i;
    DHT11_Rst();

    if ( DHT11_Check() == 0 ) {
        for ( i = 0; i < 5; i++ ) { /* ��ȡ40λ���� */
            buf[i] = DHT11_Read_Byte();
        }

        if ( ( buf[0] + buf[1] + buf[2] + buf[3] ) == buf[4] ) {
            *humi = buf[0];
            *temp = buf[2];
        }
    } else {
        return 1;
    }

    return 0;
}

u8 DHT11_Init ( void ) { /* ��ʼ��DHT11��IO��DQ��ͬʱ���DHT11�Ĵ��ڣ�����1��ʾ�����ڣ�0��ʾ���� */
    GPIO_InitTypeDef  GPIO_InitStructure;
    RCC_APB2PeriphClockCmd ( RCC_APB2Periph_GPIOA, ENABLE );
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init ( GPIOA, &GPIO_InitStructure );
    GPIO_SetBits ( GPIOA, GPIO_Pin_0 );
    DHT11_Rst();
    return DHT11_Check();
}