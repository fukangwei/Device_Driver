#include "led.h"
#include "delay.h"
#include "sys.h"
#include "usart.h"

#define ADC1_DR_Address ((u32)0x4001244C)
u16 ADC_ConvertedValue = 0;

void DMA_Config ( void ) {
    DMA_InitTypeDef DMA_InitStructure; /* 定义DMA初始化结构体 */
    RCC_AHBPeriphClockCmd ( RCC_AHBPeriph_DMA1, ENABLE );
    DMA_DeInit ( DMA1_Channel1 ); /* 复位DMA通道1 */
    DMA_InitStructure.DMA_PeripheralBaseAddr = ADC1_DR_Address; /* 定义DMA通道外设基地址 = ADC1_DR_Address */
    DMA_InitStructure.DMA_MemoryBaseAddr = ( u32 ) &ADC_ConvertedValue; /* 定义DMA通道存储器地址 */
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC; /* 指定外设为源地址 */
    DMA_InitStructure.DMA_BufferSize = 1; /* 定义DMA缓冲区大小1 */
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable; /* 当前外设寄存器地址不变 */
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Disable; /* 当前存储器地址不变 */
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord; /* 定义外设数据宽度16位 */
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord; /* 定义存储器数据宽度16位 */
    DMA_InitStructure.DMA_Mode = DMA_Mode_Circular; /* DMA通道操作模式位环形缓冲模式 */
    DMA_InitStructure.DMA_Priority = DMA_Priority_High; /* DMA通道优先级高 */
    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable; /* 禁止DMA通道存储器到存储器传输 */
    DMA_Init ( DMA1_Channel1, &DMA_InitStructure ); /* 初始化DMA通道1 */
    DMA_Cmd ( DMA1_Channel1, ENABLE ); /* 使能DMA通道1 */
}

void Adc_Init ( void ) {
    ADC_InitTypeDef ADC_InitStructure;
    GPIO_InitTypeDef GPIO_InitStructure;
    RCC_APB2PeriphClockCmd ( RCC_APB2Periph_GPIOA | RCC_APB2Periph_ADC1, ENABLE ); /* 使能ADC1通道时钟 */
    RCC_ADCCLKConfig ( RCC_PCLK2_Div6 ); /* 72M/6 = 12M，ADC最大时间不能超过14M */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN; /* 模拟输入引脚 */
    GPIO_Init ( GPIOA, &GPIO_InitStructure );
    ADC_DeInit ( ADC1 ); /* 将外设ADC1的全部寄存器重设为缺省值 */
    ADC_InitStructure.ADC_Mode = ADC_Mode_Independent; /* ADC1和ADC2工作在独立模式 */
    ADC_InitStructure.ADC_ScanConvMode = ENABLE; /* 使能扫描 */
    ADC_InitStructure.ADC_ContinuousConvMode = ENABLE; /* ADC转换工作在连续模式 */
    ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None; /* 有软件控制转换 */
    ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right; /* 转换数据右对齐 */
    ADC_InitStructure.ADC_NbrOfChannel = 1; /* 转换通道为通道1 */
    ADC_Init ( ADC1, &ADC_InitStructure ); /* 初始化ADC */
    ADC_RegularChannelConfig ( ADC1, ADC_Channel_1, 1, ADC_SampleTime_239Cycles5 ); /* ADC1选择信道1，音序器等级1，采样时间239.5个周期 */
    ADC_DMACmd ( ADC1, ENABLE ); /* 使能ADC1模块DMA */
    ADC_Cmd ( ADC1, ENABLE ); /* 使能ADC1 */
    ADC_ResetCalibration ( ADC1 ); /* 重置ADC1校准寄存器 */

    while ( ADC_GetResetCalibrationStatus ( ADC1 ) ); /* 等待ADC1校准重置完成 */

    ADC_StartCalibration ( ADC1 ); /* 开始ADC1校准 */

    while ( ADC_GetCalibrationStatus ( ADC1 ) ); /* 等待ADC1校准完成 */

    ADC_SoftwareStartConvCmd ( ADC1, ENABLE ); /* 使能ADC1软件开始转换 */
}

void GP2y_LED_Init ( void ) {
    GPIO_InitTypeDef GPIO_InitStructure;
    RCC_APB2PeriphClockCmd ( RCC_APB2Periph_GPIOA, ENABLE );
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init ( GPIOA, &GPIO_InitStructure );
}

#define GP2Y_High GPIO_SetBits(GPIOA, GPIO_Pin_0);
#define GP2Y_Low  GPIO_ResetBits(GPIOA, GPIO_Pin_0);

void GetGP2Y ( void ) {
    uint16_t ADCVal;
    GP2Y_Low;
    delay_us ( 280 );
    ADCVal = ADC_ConvertedValue;
    delay_us ( 40 );
    GP2Y_High;
    /* 延时9680微秒 */
    delay_us ( 1500 );
    delay_us ( 1500 );
    delay_us ( 1500 );
    delay_us ( 1500 );
    delay_us ( 1500 );
    delay_us ( 1500 );
    delay_us ( 680 );
    printf ( "%f\r\n", ( ADCVal * 3.3 * 1000 / 4096 * 5 - 3 ) / 34 );
}

int main ( void ) {
    SystemInit();
    delay_init ( 72 );
    NVIC_Configuration();
    uart_init ( 9600 );
    DMA_Config();
    Adc_Init();
    GP2y_LED_Init();

    while ( 1 ) {
        GetGP2Y();
        delay_ms ( 500 );
    }
}