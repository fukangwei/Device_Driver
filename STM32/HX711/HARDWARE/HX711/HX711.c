#include "HX711.h"
#include "delay.h"

#define GapValue 430

u32 Weight_Maopi;
s32 Weight_Shiwu;

void HX711_InIt ( void ) {
    GPIO_InitTypeDef GPIO_InitStructure;
    RCC_APB2PeriphClockCmd ( RCC_APB2Periph_GPIOB, ENABLE );
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init ( GPIOB, &GPIO_InitStructure );
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init ( GPIOB, &GPIO_InitStructure );
}

unsigned long HX711_Read() {
    unsigned int val = 0;
    unsigned char i = 0;
    DOUT = 1;
    SCK = 0;

    while ( DIN );

    delay_us ( 1 );

    for ( i = 0; i < 24; i++ ) {
        SCK = 1;
        val = val << 1;
        delay_us ( 1 );
        SCK = 0;

        if ( DIN ) {
            val++;
        }

        delay_us ( 1 );
    }

    SCK = 1;
    val = val ^ 0x800000;
    delay_us ( 1 );
    SCK = 0;
    delay_us ( 1 );
    return val;
}

u32 Get_Maopi() {
    Weight_Maopi = HX711_Read();
    return Weight_Maopi;
}

void Get_Weight() {
    Weight_Shiwu = HX711_Read();
    Weight_Shiwu = Weight_Shiwu - Weight_Maopi;

    if ( Weight_Shiwu > 0 ) {
        Weight_Shiwu = ( unsigned short ) ( Weight_Shiwu * 1.0 / GapValue );
    } else {
        Weight_Shiwu = 0;
    }
}