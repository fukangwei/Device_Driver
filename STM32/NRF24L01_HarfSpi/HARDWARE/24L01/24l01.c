#include "24l01.h"
#include "lcd.h"
#include "delay.h"
#include "spi.h"

const u8 TX_ADDRESS[TX_ADR_WIDTH] = {0xE7, 0xE7, 0xE7, 0xE7, 0xE7}; /* 本地地址 */
const u8 RX_ADDRESS[RX_ADR_WIDTH] = {0xE7, 0xE7, 0xE7, 0xE7, 0xE7}; /* 接收地址 */

void NRF24L01_Init ( void ) { /* 初始化24L01的IO口 */
    GPIO_InitTypeDef GPIO_InitStructure;
    RCC_APB2PeriphClockCmd ( RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOC, ENABLE );
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_4;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init ( GPIOA, &GPIO_InitStructure );
    GPIO_SetBits ( GPIOA, GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_4 );
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init ( GPIOC, &GPIO_InitStructure );
    GPIO_SetBits ( GPIOC, GPIO_Pin_4 );
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init ( GPIOC, &GPIO_InitStructure );
    SPIx_Init(); /* 初始化SPI */
    Clr_NRF24L01_CE; /* 使能24L01 */
    Set_NRF24L01_CSN; /* SPI片选取消 */
}

u8 NRF24L01_Check ( void ) { /* 检测24L01是否存在，返回0为存在，1为不存在 */
    u8 buf[5] = {0XA5, 0XA5, 0XA5, 0XA5, 0XA5};
    u8 i;
    SPIx_SetSpeed ( SPI_BaudRatePrescaler_8 ); /* spi速度为9Mhz(24L01的最大SPI时钟为10Mhz) */
    NRF24L01_Write_Buf ( NRF24L01_WRITE_REG + TX_ADDR, buf, 5 );
    NRF24L01_Read_Buf ( TX_ADDR, buf, 5 );

    for ( i = 0; i < 5; i++ )
        if ( buf[i] != 0XA5 ) {
            break;
        }

    if ( i != 5 ) {
        return 1;
    }

    return 0;
}

u8 NRF24L01_Write_Reg ( u8 reg, u8 value ) { /* SPI写寄存器。参数reg是指定寄存器地址，value是写入的值 */
    u8 status;
    Clr_NRF24L01_CSN; /* 使能SPI传输 */
    status = SPIx_ReadWriteByte ( reg ); /* 发送寄存器号 */
    SPIx_ReadWriteByte ( value ); /* 写入寄存器的值 */
    Set_NRF24L01_CSN; /* 禁止SPI传输 */
    return ( status ); /* 返回状态值 */
}

u8 NRF24L01_Read_Reg ( u8 reg ) { /* 读取SPI寄存器值。参数reg是要读的寄存器 */
    u8 reg_val;
    Clr_NRF24L01_CSN; /* 使能SPI传输 */
    SPIx_ReadWriteByte ( reg ); /* 发送寄存器号 */
    reg_val = SPIx_ReadWriteByte ( 0XFF ); /* 读取寄存器内容 */
    Set_NRF24L01_CSN; /* 禁止SPI传输 */
    return ( reg_val ); /* 返回状态值 */
}

u8 NRF24L01_Read_Buf ( u8 reg, u8 *pBuf, u8 len ) { /* 在指定位置读出指定长度的数据。参数reg是寄存器，pBuf是数据指针，len是数据长度 */
    u8 status, u8_ctr;
    Clr_NRF24L01_CSN; /* 使能SPI传输 */
    status = SPIx_ReadWriteByte ( reg ); /* 发送寄存器值位置，并读取状态值 */

    for ( u8_ctr = 0; u8_ctr < len; u8_ctr++ ) {
        pBuf[u8_ctr] = SPIx_ReadWriteByte ( 0XFF ); /* 读出数据 */
    }

    Set_NRF24L01_CSN; /* 关闭SPI传输 */
    return status; /* 返回读到的状态值 */
}

u8 NRF24L01_Write_Buf ( u8 reg, u8 *pBuf, u8 len ) { /* 在指定位置写指定长度的数据。参数reg是寄存器，pBuf是数据指针，len是数据长度 */
    u8 status, u8_ctr;
    Clr_NRF24L01_CSN; /* 使能SPI传输 */
    status = SPIx_ReadWriteByte ( reg ); /* 发送寄存器值位置，并读取状态值 */

    for ( u8_ctr = 0; u8_ctr < len; u8_ctr++ ) {
        SPIx_ReadWriteByte ( *pBuf++ ); /* 写入数据 */
    }

    Set_NRF24L01_CSN; /* 关闭SPI传输 */
    return status; /* 返回读到的状态值 */
}

u8 NRF24L01_TxPacket ( u8 *txbuf ) { /* 启动NRF24L01发送一次数据，参数txbuf是待发送数据首地址 */
    u8 sta;
    SPIx_SetSpeed ( SPI_BaudRatePrescaler_8 ); /* spi速度为9Mhz(24L01的最大SPI时钟为10Mhz) */
    Clr_NRF24L01_CE;
    NRF24L01_Write_Buf ( NRF24L01_WR_TX_PLOAD, txbuf, TX_PLOAD_WIDTH ); /* 写数据到“TX BUF”(32字节) */
    Set_NRF24L01_CE; /* 启动发送 */

    while ( NRF24L01_IRQ != 0 ); /* 等待发送完成 */

    sta = NRF24L01_Read_Reg ( STATUS ); /* 读取状态寄存器的值 */
    NRF24L01_Write_Reg ( NRF24L01_WRITE_REG + STATUS, sta ); /* 清除TX_DS或MAX_RT中断标志 */

    if ( sta & MAX_TX ) { /* 达到最大重发次数 */
        NRF24L01_Write_Reg ( NRF24L01_FLUSH_TX, 0xff ); /* 清除“TX FIFO”寄存器 */
        return MAX_TX;
    }

    if ( sta & TX_OK ) { /* 发送完成 */
        return TX_OK;
    }

    return 0xff; /* 其他原因发送失败 */
}

u8 NRF24L01_RxPacket ( u8 *rxbuf ) { /* 启动NRF24L01接收一次数据，参数txbuf是接收数据缓存区首地址 */
    u8 sta;
    SPIx_SetSpeed ( SPI_BaudRatePrescaler_8 ); /* spi速度为9Mhz(24L01的最大SPI时钟为10Mhz) */
    sta = NRF24L01_Read_Reg ( STATUS ); /* 读取状态寄存器的值 */
    NRF24L01_Write_Reg ( NRF24L01_WRITE_REG + STATUS, sta ); /* 清除TX_DS或MAX_RT中断标志 */

    if ( sta & RX_OK ) { /* 接收到数据 */
        NRF24L01_Read_Buf ( NRF24L01_RD_RX_PLOAD, rxbuf, RX_PLOAD_WIDTH ); /* 读取数据 */
        NRF24L01_Write_Reg ( NRF24L01_FLUSH_RX, 0xff ); /* 清除“RX FIFO”寄存器 */
        return 0;
    }

    return 1; /* 没收到任何数据 */
}

/* 初始化NRF24L01到RX模式。设置RX地址，写RX数据宽度，选择RF频道、波特率和“LNA HCURR”。当CE变高后，即进入RX模式，并可以接收数据了 */
void RX_Mode ( void ) {
    Clr_NRF24L01_CE;
    NRF24L01_Write_Buf ( NRF24L01_WRITE_REG + RX_ADDR_P0, ( u8 * ) RX_ADDRESS, RX_ADR_WIDTH ); /* 写RX节点地址 */
    NRF24L01_Write_Reg ( NRF24L01_WRITE_REG + EN_AA, 0x00 ); /* 使能通道0的自动应答 */
    NRF24L01_Write_Reg ( NRF24L01_WRITE_REG + EN_RXADDR, 0x01 ); /* 使能通道0的接收地址 */
    NRF24L01_Write_Reg ( NRF24L01_WRITE_REG + RF_CH, 40 ); /* 设置RF通信频率 */
    NRF24L01_Write_Reg ( NRF24L01_WRITE_REG + RX_PW_P0, RX_PLOAD_WIDTH ); /* 选择通道0的有效数据宽度 */
    NRF24L01_Write_Reg ( NRF24L01_WRITE_REG + RF_SETUP, 0x0f ); /* 设置TX发射参数，0db增益，2Mbps，低噪声增益开启 */
    NRF24L01_Write_Reg ( NRF24L01_WRITE_REG + CONFIG, 0x0f ); /* 配置基本工作模式的参数：PWR_UP、EN_CRC、16BIT_CRC、接收模式 */
    Set_NRF24L01_CE; /* CE为高时，进入接收模式 */
}

/* 初始化NRF24L01到TX模式。设置TX地址，写TX数据宽度，设置RX自动应答的地址，填充TX发送数据，选择RF频道、波特率和“LNA HCURR”、PWR_UP、CRC使能。
   当CE变高后，即进入TX模式，并可以发送数据了。CE为高大于10us，则启动发送 */
void TX_Mode ( void ) {
    Clr_NRF24L01_CE;
    NRF24L01_Write_Buf ( NRF24L01_WRITE_REG + TX_ADDR, ( u8 * ) TX_ADDRESS, TX_ADR_WIDTH ); /* 写TX节点地址 */
    NRF24L01_Write_Buf ( NRF24L01_WRITE_REG + RX_ADDR_P0, ( u8 * ) RX_ADDRESS, RX_ADR_WIDTH ); /* 设置TX节点地址，主要为了使能ACK */
    NRF24L01_Write_Reg ( NRF24L01_WRITE_REG + EN_AA, 0x00 ); /* 使能通道0的自动应答 */
    NRF24L01_Write_Reg ( NRF24L01_WRITE_REG + EN_RXADDR, 0x00 ); /* 使能通道0的接收地址 */
    NRF24L01_Write_Reg ( NRF24L01_WRITE_REG + SETUP_RETR, 0x00 ); /* 设置自动重发间隔时间“500us + 86us”；最大自动重发次数10次 */
    NRF24L01_Write_Reg ( NRF24L01_WRITE_REG + RF_CH, 40 ); /* 设置RF通道为40 */
    NRF24L01_Write_Reg ( NRF24L01_WRITE_REG + RF_SETUP, 0x0f ); /* 设置TX发射参数，0db增益,，Mbps，低噪声增益开启 */
    NRF24L01_Write_Reg ( NRF24L01_WRITE_REG + CONFIG, 0x0e ); /* 配置基本工作模式的参数：PWR_UP、EN_CRC、16BIT_CRC、接收模式、开启所有中断 */
    Set_NRF24L01_CE; /* CE为高时，10us后启动发送 */
}