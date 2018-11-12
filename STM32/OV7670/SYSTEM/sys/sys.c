#include "sys.h"

void MY_NVIC_SetVectorTable ( u32 NVIC_VectTab, u32 Offset ) { /* ����������ƫ�Ƶ�ַ������NVIC_VectTab�ǻ�ַ��Offset��ƫ���� */
    SCB->VTOR = NVIC_VectTab | ( Offset & ( u32 ) 0x1FFFFF80 ); /* ����NVIC��������ƫ�ƼĴ��� */
}

void MY_NVIC_PriorityGroupConfig ( u8 NVIC_Group ) { /* ����NVIC���飬����NVIC_Group��NVIC����(0��4) */
    u32 temp, temp1;
    temp1 = ( ~NVIC_Group ) & 0x07; /* ȡ����λ */
    temp1 <<= 8;
    temp = SCB->AIRCR; /* ��ȡ��ǰ������ */
    temp &= 0X0000F8FF; /* �����ǰ���� */
    temp |= 0X05FA0000;
    temp |= temp1;
    SCB->AIRCR = temp; /* ���÷��� */
}

/* ����NVIC������NVIC_PreemptionPriority����ռ���ȼ���NVIC_SubPriority����Ӧ���ȼ���NVIC_Channel���жϱ�ţ�
   NVIC_Group���жϷ���(0��4)��ע�����ȼ����ܳ����趨����ķ�Χ�������������벻���Ĵ����黮��������ʾ��
   ��0��0λ��ռ���ȼ���4λ��Ӧ���ȼ�����1��1λ��ռ���ȼ���3λ��Ӧ���ȼ�����2��2λ��ռ���ȼ���2λ��Ӧ���ȼ���
   ��3��3λ��ռ���ȼ���1λ��Ӧ���ȼ�����4��4λ��ռ���ȼ���0λ��Ӧ���ȼ� */
void MY_NVIC_Init ( u8 NVIC_PreemptionPriority, u8 NVIC_SubPriority, u8 NVIC_Channel, u8 NVIC_Group ) {
    u32 temp;
    MY_NVIC_PriorityGroupConfig ( NVIC_Group ); /* ���÷��� */
    temp = NVIC_PreemptionPriority << ( 4 - NVIC_Group );
    temp |= NVIC_SubPriority & ( 0x0f >> NVIC_Group );
    temp &= 0xf; /* ȡ����λ */
    NVIC->ISER[NVIC_Channel / 32] |= ( 1 << NVIC_Channel % 32 ); /* ʹ���ж�λ */
    NVIC->IP[NVIC_Channel] |= temp << 4; /* ������Ӧ���ȼ����������ȼ� */
}

/* �ⲿ�ж����ú�����ֻ���GPIOA��G��������PVD��RTC��USB���ѡ�����GPIOx��0��6������GPIOA��G��BITx����Ҫʹ�ܵ�λ��
   TRIM�Ǵ���ģʽ��1�������أ�2���Ͻ��أ�3�������ƽ�������ú���һ��ֻ������1��IO�ڣ����ڶ��IO�ڣ����ε��� */
void Ex_NVIC_Config ( u8 GPIOx, u8 BITx, u8 TRIM ) {
    u8 EXTADDR;
    u8 EXTOFFSET;
    EXTADDR = BITx / 4; /* �õ��жϼĴ�����ı�� */
    EXTOFFSET = ( BITx % 4 ) * 4;
    RCC->APB2ENR |= 0x01; /* ʹ��IO����ʱ�� */
    AFIO->EXTICR[EXTADDR] &= ~ ( 0x000F << EXTOFFSET ); /* ���ԭ������ */
    AFIO->EXTICR[EXTADDR] |= GPIOx << EXTOFFSET; /* EXTI.BITxӳ�䵽GPIOx.BITx */
    EXTI->IMR |= 1 << BITx; /* ������line BITx���ϵ��ж� */

    /* EXTI->EMR |= 1 << BITx; //�����Ρ�line BITx���ϵ��¼�(�����������䣬�����������ʱ�޷������ж�) */
    if ( TRIM & 0x01 ) {
        EXTI->FTSR |= 1 << BITx; /* ��line BITx�����¼��½��ش��� */
    }

    if ( TRIM & 0x02 ) {
        EXTI->RTSR |= 1 << BITx; /* ��line BITx�����¼��������ش��� */
    }
}

void MYRCC_DeInit ( void ) { /* ������ʱ�ӼĴ�����λ */
    RCC->APB1RSTR = 0x00000000; /* ��λ���� */
    RCC->APB2RSTR = 0x00000000;
    RCC->AHBENR = 0x00000014; /* ˯��ģʽʱ�����SRAMʱ��ʹ�ܣ������ر� */
    RCC->APB2ENR = 0x00000000; /* ����ʱ�ӹر� */
    RCC->APB1ENR = 0x00000000;
    RCC->CR |= 0x00000001; /* ʹ���ڲ�����ʱ��HSION */
    RCC->CFGR &= 0xF8FF0000; /* ��λSW[1:0]��HPRE[3:0]��PPRE1[2:0]��PPRE2[2:0]��ADCPRE[1:0]��MCO[2:0] */
    RCC->CR &= 0xFEF6FFFF; /* ��λHSEON��CSSON��PLLON */
    RCC->CR &= 0xFFFBFFFF; /* ��λHSEBYP */
    RCC->CFGR &= 0xFF80FFFF; /* ��λPLLSRC��PLLXTPRE��PLLMUL[3:0]��USBPRE */
    RCC->CIR = 0x00000000; /* �ر������ж� */
#ifdef VECT_TAB_RAM /* ���������� */
    MY_NVIC_SetVectorTable ( 0x20000000, 0x0 );
#else
    MY_NVIC_SetVectorTable ( 0x08000000, 0x0 );
#endif
}

__asm void WFI_SET ( void ) { /* THUMBָ�֧�ֻ���������������·���ʵ��ִ�л��ָ��WFI */
    WFI;
}

__asm void INTX_DISABLE ( void ) { /* �ر������ж� */
    CPSID I;
}

__asm void INTX_ENABLE ( void ) { /* ���������ж� */
    CPSIE I;
}

__asm void MSR_MSP ( u32 addr ) { /* ����ջ����ַ������addr��ջ����ַ */
    MSR MSP, r0
    BX r14
}

void Sys_Standby ( void ) { /* �������ģʽ */
    SCB->SCR |= 1 << 2; /* ʹ��SLEEPDEEPλ(SYS->CTRL) */
    RCC->APB1ENR |= 1 << 28; /* ʹ�ܵ�Դʱ�� */
    PWR->CSR |= 1 << 8; /* ����WKUP���ڻ��� */
    PWR->CR |= 1 << 2; /* ���Wake-up��־ */
    PWR->CR |= 1 << 1; /* PDDS��λ */
    WFI_SET(); /* ִ��WFIָ�� */
}

void Sys_Soft_Reset ( void ) { /* ϵͳ��λ */
    SCB->AIRCR = 0X05FA0000 | ( u32 ) 0x04;
}

/* JTAGģʽ���ã���������JTAG��ģʽ������mode��jtag��swdģʽ���ã�00ȫʹ�ܣ�01ʹ��SWD��10ȫ�ر� */
void JTAG_Set ( u8 mode ) {
    u32 temp;
    temp = mode;
    temp <<= 25;
    RCC->APB2ENR |= 1 << 0; /* ��������ʱ�� */
    AFIO->MAPR &= 0XF8FFFFFF; /* ���MAPR��[26:24] */
    AFIO->MAPR |= temp; /* ����jtagģʽ */
}

void Stm32_Clock_Init ( u8 PLL ) { /* ϵͳʱ�ӳ�ʼ������������pll��ѡ��ı�Ƶ������2��ʼ�����ֵΪ16 */
    unsigned char temp = 0;
    MYRCC_DeInit(); /* ��λ������������ */
    RCC->CR |= 0x00010000; /* �ⲿ����ʱ��ʹ��HSEON */

    while ( ! ( RCC->CR >> 17 ) ); /* �ȴ��ⲿʱ�Ӿ��� */

    RCC->CFGR = 0X00000400; /* APB1 = DIV2; APB2 = DIV1; AHB = DIV1; */
    PLL -= 2; /* ����2����λ */
    RCC->CFGR |= PLL << 18; /* ����PLLֵ(2��16) */
    RCC->CFGR |= 1 << 16;
    FLASH->ACR |= 0x32;
    RCC->CR |= 0x01000000;

    while ( ! ( RCC->CR >> 25 ) ); /* �ȴ�PLL���� */

    RCC->CFGR |= 0x00000002; /* PLL��Ϊϵͳʱ�� */

    while ( temp != 0x02 ) { /* �ȴ�PLL��Ϊϵͳʱ�����óɹ� */
        temp = RCC->CFGR >> 2;
        temp &= 0x03;
    }
}