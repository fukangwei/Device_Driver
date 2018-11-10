#include "led.h"
#include "delay.h"
#include "sys.h"
#include "usart.h"

#define ADC1_DR_Address ((u32)0x4001244C)
u16 ADC_ConvertedValue = 0;

void DMA_Config ( void ) {
    DMA_InitTypeDef DMA_InitStructure; /* ����DMA��ʼ���ṹ�� */
    RCC_AHBPeriphClockCmd ( RCC_AHBPeriph_DMA1, ENABLE );
    DMA_DeInit ( DMA1_Channel1 ); /* ��λDMAͨ��1 */
    DMA_InitStructure.DMA_PeripheralBaseAddr = ADC1_DR_Address; /* ����DMAͨ���������ַ = ADC1_DR_Address */
    DMA_InitStructure.DMA_MemoryBaseAddr = ( u32 ) &ADC_ConvertedValue; /* ����DMAͨ���洢����ַ */
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC; /* ָ������ΪԴ��ַ */
    DMA_InitStructure.DMA_BufferSize = 1; /* ����DMA��������С1 */
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable; /* ��ǰ����Ĵ�����ַ���� */
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Disable; /* ��ǰ�洢����ַ���� */
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord; /* �����������ݿ��16λ */
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord; /* ����洢�����ݿ��16λ */
    DMA_InitStructure.DMA_Mode = DMA_Mode_Circular; /* DMAͨ������ģʽλ���λ���ģʽ */
    DMA_InitStructure.DMA_Priority = DMA_Priority_High; /* DMAͨ�����ȼ��� */
    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable; /* ��ֹDMAͨ���洢�����洢������ */
    DMA_Init ( DMA1_Channel1, &DMA_InitStructure ); /* ��ʼ��DMAͨ��1 */
    DMA_Cmd ( DMA1_Channel1, ENABLE ); /* ʹ��DMAͨ��1 */
}

void Adc_Init ( void ) {
    ADC_InitTypeDef ADC_InitStructure;
    GPIO_InitTypeDef GPIO_InitStructure;
    RCC_APB2PeriphClockCmd ( RCC_APB2Periph_GPIOA | RCC_APB2Periph_ADC1, ENABLE ); /* ʹ��ADC1ͨ��ʱ�� */
    RCC_ADCCLKConfig ( RCC_PCLK2_Div6 ); /* 72M/6 = 12M��ADC���ʱ�䲻�ܳ���14M */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN; /* ģ���������� */
    GPIO_Init ( GPIOA, &GPIO_InitStructure );
    ADC_DeInit ( ADC1 ); /* ������ADC1��ȫ���Ĵ�������Ϊȱʡֵ */
    ADC_InitStructure.ADC_Mode = ADC_Mode_Independent; /* ADC1��ADC2�����ڶ���ģʽ */
    ADC_InitStructure.ADC_ScanConvMode = ENABLE; /* ʹ��ɨ�� */
    ADC_InitStructure.ADC_ContinuousConvMode = ENABLE; /* ADCת������������ģʽ */
    ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None; /* ���������ת�� */
    ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right; /* ת�������Ҷ��� */
    ADC_InitStructure.ADC_NbrOfChannel = 1; /* ת��ͨ��Ϊͨ��1 */
    ADC_Init ( ADC1, &ADC_InitStructure ); /* ��ʼ��ADC */
    ADC_RegularChannelConfig ( ADC1, ADC_Channel_1, 1, ADC_SampleTime_239Cycles5 ); /* ADC1ѡ���ŵ�1���������ȼ�1������ʱ��239.5������ */
    ADC_DMACmd ( ADC1, ENABLE ); /* ʹ��ADC1ģ��DMA */
    ADC_Cmd ( ADC1, ENABLE ); /* ʹ��ADC1 */
    ADC_ResetCalibration ( ADC1 ); /* ����ADC1У׼�Ĵ��� */

    while ( ADC_GetResetCalibrationStatus ( ADC1 ) ); /* �ȴ�ADC1У׼������� */

    ADC_StartCalibration ( ADC1 ); /* ��ʼADC1У׼ */

    while ( ADC_GetCalibrationStatus ( ADC1 ) ); /* �ȴ�ADC1У׼��� */

    ADC_SoftwareStartConvCmd ( ADC1, ENABLE ); /* ʹ��ADC1�����ʼת�� */
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
    /* ��ʱ9680΢�� */
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