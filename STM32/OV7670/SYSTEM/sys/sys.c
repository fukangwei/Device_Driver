#include "sys.h"

void MY_NVIC_SetVectorTable ( u32 NVIC_VectTab, u32 Offset ) { /* 设置向量表偏移地址，参数NVIC_VectTab是基址，Offset是偏移量 */
    SCB->VTOR = NVIC_VectTab | ( Offset & ( u32 ) 0x1FFFFF80 ); /* 设置NVIC的向量表偏移寄存器 */
}

void MY_NVIC_PriorityGroupConfig ( u8 NVIC_Group ) { /* 设置NVIC分组，参数NVIC_Group是NVIC分组(0至4) */
    u32 temp, temp1;
    temp1 = ( ~NVIC_Group ) & 0x07; /* 取后三位 */
    temp1 <<= 8;
    temp = SCB->AIRCR; /* 读取先前的设置 */
    temp &= 0X0000F8FF; /* 清空先前分组 */
    temp |= 0X05FA0000;
    temp |= temp1;
    SCB->AIRCR = temp; /* 设置分组 */
}

/* 设置NVIC。参数NVIC_PreemptionPriority是抢占优先级，NVIC_SubPriority是响应优先级，NVIC_Channel是中断编号，
   NVIC_Group是中断分组(0至4)。注意优先级不能超过设定的组的范围，否则会出现意想不到的错误。组划分如下所示：
   组0：0位抢占优先级，4位响应优先级；组1：1位抢占优先级，3位响应优先级；组2：2位抢占优先级，2位响应优先级；
   组3：3位抢占优先级，1位响应优先级；组4：4位抢占优先级，0位响应优先级 */
void MY_NVIC_Init ( u8 NVIC_PreemptionPriority, u8 NVIC_SubPriority, u8 NVIC_Channel, u8 NVIC_Group ) {
    u32 temp;
    MY_NVIC_PriorityGroupConfig ( NVIC_Group ); /* 设置分组 */
    temp = NVIC_PreemptionPriority << ( 4 - NVIC_Group );
    temp |= NVIC_SubPriority & ( 0x0f >> NVIC_Group );
    temp &= 0xf; /* 取低四位 */
    NVIC->ISER[NVIC_Channel / 32] |= ( 1 << NVIC_Channel % 32 ); /* 使能中断位 */
    NVIC->IP[NVIC_Channel] |= temp << 4; /* 设置响应优先级和抢断优先级 */
}

/* 外部中断配置函数，只针对GPIOA至G，不包括PVD、RTC和USB唤醒。参数GPIOx是0至6，代表GPIOA至G；BITx是需要使能的位；
   TRIM是触发模式，1是下升沿，2是上降沿，3是任意电平触发。该函数一次只能配置1个IO口，对于多个IO口，需多次调用 */
void Ex_NVIC_Config ( u8 GPIOx, u8 BITx, u8 TRIM ) {
    u8 EXTADDR;
    u8 EXTOFFSET;
    EXTADDR = BITx / 4; /* 得到中断寄存器组的编号 */
    EXTOFFSET = ( BITx % 4 ) * 4;
    RCC->APB2ENR |= 0x01; /* 使能IO复用时钟 */
    AFIO->EXTICR[EXTADDR] &= ~ ( 0x000F << EXTOFFSET ); /* 清除原来设置 */
    AFIO->EXTICR[EXTADDR] |= GPIOx << EXTOFFSET; /* EXTI.BITx映射到GPIOx.BITx */
    EXTI->IMR |= 1 << BITx; /* 开启“line BITx”上的中断 */

    /* EXTI->EMR |= 1 << BITx; //不屏蔽“line BITx”上的事件(如果不屏蔽这句，是在软件仿真时无法进入中断) */
    if ( TRIM & 0x01 ) {
        EXTI->FTSR |= 1 << BITx; /* “line BITx”上事件下降沿触发 */
    }

    if ( TRIM & 0x02 ) {
        EXTI->RTSR |= 1 << BITx; /* “line BITx”上事件上升降沿触发 */
    }
}

void MYRCC_DeInit ( void ) { /* 把所有时钟寄存器复位 */
    RCC->APB1RSTR = 0x00000000; /* 复位结束 */
    RCC->APB2RSTR = 0x00000000;
    RCC->AHBENR = 0x00000014; /* 睡眠模式时闪存和SRAM时钟使能，其他关闭 */
    RCC->APB2ENR = 0x00000000; /* 外设时钟关闭 */
    RCC->APB1ENR = 0x00000000;
    RCC->CR |= 0x00000001; /* 使能内部高速时钟HSION */
    RCC->CFGR &= 0xF8FF0000; /* 复位SW[1:0]、HPRE[3:0]、PPRE1[2:0]、PPRE2[2:0]、ADCPRE[1:0]和MCO[2:0] */
    RCC->CR &= 0xFEF6FFFF; /* 复位HSEON、CSSON和PLLON */
    RCC->CR &= 0xFFFBFFFF; /* 复位HSEBYP */
    RCC->CFGR &= 0xFF80FFFF; /* 复位PLLSRC、PLLXTPRE、PLLMUL[3:0]和USBPRE */
    RCC->CIR = 0x00000000; /* 关闭所有中断 */
#ifdef VECT_TAB_RAM /* 配置向量表 */
    MY_NVIC_SetVectorTable ( 0x20000000, 0x0 );
#else
    MY_NVIC_SetVectorTable ( 0x08000000, 0x0 );
#endif
}

__asm void WFI_SET ( void ) { /* THUMB指令不支持汇编内联，采用如下方法实现执行汇编指令WFI */
    WFI;
}

__asm void INTX_DISABLE ( void ) { /* 关闭所有中断 */
    CPSID I;
}

__asm void INTX_ENABLE ( void ) { /* 开启所有中断 */
    CPSIE I;
}

__asm void MSR_MSP ( u32 addr ) { /* 设置栈顶地址，参数addr是栈顶地址 */
    MSR MSP, r0
    BX r14
}

void Sys_Standby ( void ) { /* 进入待机模式 */
    SCB->SCR |= 1 << 2; /* 使能SLEEPDEEP位(SYS->CTRL) */
    RCC->APB1ENR |= 1 << 28; /* 使能电源时钟 */
    PWR->CSR |= 1 << 8; /* 设置WKUP用于唤醒 */
    PWR->CR |= 1 << 2; /* 清除Wake-up标志 */
    PWR->CR |= 1 << 1; /* PDDS置位 */
    WFI_SET(); /* 执行WFI指令 */
}

void Sys_Soft_Reset ( void ) { /* 系统软复位 */
    SCB->AIRCR = 0X05FA0000 | ( u32 ) 0x04;
}

/* JTAG模式设置，用于设置JTAG的模式。参数mode是jtag和swd模式设置，00全使能，01使能SWD，10全关闭 */
void JTAG_Set ( u8 mode ) {
    u32 temp;
    temp = mode;
    temp <<= 25;
    RCC->APB2ENR |= 1 << 0; /* 开启辅助时钟 */
    AFIO->MAPR &= 0XF8FFFFFF; /* 清除MAPR的[26:24] */
    AFIO->MAPR |= temp; /* 设置jtag模式 */
}

void Stm32_Clock_Init ( u8 PLL ) { /* 系统时钟初始化函数。参数pll是选择的倍频数，从2开始，最大值为16 */
    unsigned char temp = 0;
    MYRCC_DeInit(); /* 复位并配置向量表 */
    RCC->CR |= 0x00010000; /* 外部高速时钟使能HSEON */

    while ( ! ( RCC->CR >> 17 ) ); /* 等待外部时钟就绪 */

    RCC->CFGR = 0X00000400; /* APB1 = DIV2; APB2 = DIV1; AHB = DIV1; */
    PLL -= 2; /* 抵消2个单位 */
    RCC->CFGR |= PLL << 18; /* 设置PLL值(2至16) */
    RCC->CFGR |= 1 << 16;
    FLASH->ACR |= 0x32;
    RCC->CR |= 0x01000000;

    while ( ! ( RCC->CR >> 25 ) ); /* 等待PLL锁定 */

    RCC->CFGR |= 0x00000002; /* PLL作为系统时钟 */

    while ( temp != 0x02 ) { /* 等待PLL作为系统时钟设置成功 */
        temp = RCC->CFGR >> 2;
        temp &= 0x03;
    }
}