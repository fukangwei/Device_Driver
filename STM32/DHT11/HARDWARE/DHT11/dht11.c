#include "dht11.h"
#include "delay.h"

void DHT11_Rst ( void ) { /* 复位DHT11 */
    DHT11_IO_OUT(); /* SET OUTPUT */
    DHT11_DQ_OUT = 0; /* 拉低DQ */
    delay_ms ( 20 ); /* 拉低至少18ms */
    DHT11_DQ_OUT = 1; /* DQ = 1 */
    delay_us ( 30 ); /* 主机拉高20至40us */
}

u8 DHT11_Check ( void ) { /* 等待DHT11的回应，返回0表示DHT11存在，返回1表示不存在 */
    u8 retry = 0;
    DHT11_IO_IN(); /* SET INPUT */

    while ( DHT11_DQ_IN && retry < 100 ) { /* DHT11会拉低40至80us */
        retry++;
        delay_us ( 1 );
    };

    if ( retry >= 100 ) {
        return 1;
    } else {
        retry = 0;
    }

    while ( !DHT11_DQ_IN && retry < 100 ) { /* DHT11拉低后会再次拉高40至80us */
        retry++;
        delay_us ( 1 );
    };

    if ( retry >= 100 ) {
        return 1;
    }

    return 0;
}

u8 DHT11_Read_Bit ( void ) { /* 从DHT11读取一个位 */
    u8 retry = 0;

    while ( DHT11_DQ_IN && retry < 100 ) { /* 等待变为低电平 */
        retry++;
        delay_us ( 1 );
    }

    retry = 0;

    while ( !DHT11_DQ_IN && retry < 100 ) { /* 等待变高电平 */
        retry++;
        delay_us ( 1 );
    }

    delay_us ( 40 ); /* 等待40us */

    if ( DHT11_DQ_IN ) {
        return 1;
    } else {
        return 0;
    }
}

u8 DHT11_Read_Byte ( void ) { /* 从DHT11读取一个字节 */
    u8 i, dat;
    dat = 0;

    for ( i = 0; i < 8; i++ ) {
        dat <<= 1;
        dat |= DHT11_Read_Bit();
    }

    return dat;
}

u8 DHT11_Read_Data ( u8 *temp, u8 *humi ) { /* 从DHT11读取一次数据，参数temp温度值(0至50)，humi湿度值(20%至90%) */
    u8 buf[5];
    u8 i;
    DHT11_Rst();

    if ( DHT11_Check() == 0 ) {
        for ( i = 0; i < 5; i++ ) { /* 读取40位数据 */
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

u8 DHT11_Init ( void ) { /* 初始化DHT11的IO口DQ，同时检测DHT11的存在，返回1表示不存在，0表示存在 */
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