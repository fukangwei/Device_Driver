#include <linux/errno.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/input.h>
#include <linux/init.h>
#include <linux/serio.h>
#include <linux/delay.h>
#include <linux/clk.h>
#include <linux/miscdevice.h>
#include <linux/gpio.h>
#include <asm/io.h>
#include <asm/irq.h>
#include <asm/uaccess.h>
#include <mach/regs-clock.h>
#include <plat/regs-timer.h>
#include <mach/regs-gpio.h>
#include <linux/cdev.h>

typedef unsigned int uint16 ;
typedef unsigned char uint8 ;

static int led_major = 0; /* 主设备号 */
static struct cdev LedDevs;

uint8 Tx_Buf1[32];

/* 应用程序执行ioctl(fd, cmd, arg)时的第2个参数 */
#define LED_MAGIC 'k'
#define IOCTL_LED_ON     _IOW (LED_MAGIC, 1, int)
#define IOCTL_LED_OFF    _IOW (LED_MAGIC, 2, int)
#define IOCTL_LED_RUN    _IOW (LED_MAGIC, 3, int)
#define IOCTL_LED_SHINE  _IOW (LED_MAGIC, 4, int)
#define IOCTL_LED_ALLON  _IOW (LED_MAGIC, 5, int)
#define IOCTL_LED_ALLOFF _IOW (LED_MAGIC, 6, int)

#define CSN       S3C2410_GPF(2)
#define CSN_OUTP  S3C2410_GPIO_OUTPUT
#define MOSI      S3C2410_GPF(6)
#define MOSI_OUTP S3C2410_GPIO_OUTPUT
#define IRQ       S3C2410_GPG(5)
#define IRQ_INP   S3C2410_GPIO_INPUT
#define MISO      S3C2410_GPG(1)
#define MISO_INP  S3C2410_GPIO_INPUT
#define SCK       S3C2410_GPF(4)
#define SCK_OUTP  S3C2410_GPIO_OUTPUT
#define CE        S3C2410_GPF(0)
#define CE_OUTP   S3C2410_GPIO_OUTPUT

#define TxBufSize 32

uint8 TxBuf[TxBufSize];

/* NRF24L01端口定义 */
#define CE_OUT   s3c2410_gpio_cfgpin(CE, CE_OUTP) /* 数据线设置为输出 */
#define CE_UP    s3c2410_gpio_pullup(CE, 1) /* 打开上拉电阻 */
#define CE_L     s3c2410_gpio_setpin(CE, 0) /* 拉低数据线电平 */
#define CE_H     s3c2410_gpio_setpin(CE, 1) /* 拉高数据线电平 */
#define SCK_OUT  s3c2410_gpio_cfgpin(SCK, SCK_OUTP) /* 数据线设置为输出 */
#define SCK_UP   s3c2410_gpio_pullup(SCK, 1) /* 打开上拉电阻 */
#define SCK_L    s3c2410_gpio_setpin(SCK, 0) /* 拉低数据线电平 */
#define SCK_H    s3c2410_gpio_setpin(SCK, 1) /* 拉高数据线电平 */
#define MISO_IN  s3c2410_gpio_cfgpin(MISO, MISO_INP) /* 数据线设置为输出 */
#define MISO_UP  s3c2410_gpio_pullup(MISO, 1) /* 打开上拉电阻 */
#define MISO_STU s3c2410_gpio_getpin(MISO) /* 数据状态 */
#define IRQ_IN   s3c2410_gpio_cfgpin(IRQ, IRQ_INP) /* 数据线设置为输出 */
#define IRQ_UP   s3c2410_gpio_pullup(IRQ, 1) /* 打开上拉电阻 */
#define IRQ_L    s3c2410_gpio_setpin(IRQ, 0) /* 拉低数据线电平 */
#define IRQ_H    s3c2410_gpio_setpin(IRQ, 1) /* 拉高数据线电平 */
#define MOSI_OUT s3c2410_gpio_cfgpin(MOSI, MOSI_OUTP) /* 数据线设置为输出 */
#define MOSI_UP  s3c2410_gpio_pullup(MOSI, 1) /* 打开上拉电阻 */
#define MOSI_L   s3c2410_gpio_setpin(MOSI, 0) /* 拉低数据线电平 */
#define MOSI_H   s3c2410_gpio_setpin(MOSI, 1) /* 拉高数据线电平 */
#define CSN_OUT  s3c2410_gpio_cfgpin(CSN, CSN_OUTP) /* 数据线设置为输出 */
#define CSN_UP   s3c2410_gpio_pullup(CSN, 1) /* 打开上拉电阻 */
#define CSN_L    s3c2410_gpio_setpin(CSN, 0) /* 拉低数据线电平 */
#define CSN_H    s3c2410_gpio_setpin(CSN, 1) /* 拉高数据线电平 */

#define TX_ADR_WIDTH 5 /* 5 uint8s TX address width */
#define RX_ADR_WIDTH 5 /* 5 uint8s RX address width */
#define TX_PLOAD_WIDTH 32 /* 20 uint8s TX payload */
#define RX_PLOAD_WIDTH 32 /* 20 uint8s TX payload */

uint8 const TX_ADDRESS[TX_ADR_WIDTH] = {0xE7, 0xE7, 0xE7, 0xE7, 0xE7}; /* 本地地址 */
uint8 const RX_ADDRESS[RX_ADR_WIDTH] = {0xE7, 0xE7, 0xE7, 0xE7, 0xE7}; /* 接收地址 */

/* NRF24L01寄存器指令 */
#define READ_REG    0x00 /* 读寄存器指令 */
#define WRITE_REG   0x20 /* 写寄存器指令 */
#define RD_RX_PLOAD 0x61 /* 读取接收数据指令 */
#define WR_TX_PLOAD 0xA0 /* 写待发数据指令 */
#define FLUSH_TX    0xE1 /* 冲洗发送FIFO指令 */
#define FLUSH_RX    0xE2 /* 冲洗接收FIFO指令 */
#define REUSE_TX_PL 0xE3 /* 定义重复装载数据指令 */
#define NOP         0xFF /* 保留 */

/* SPI(nRF24L01)寄存器地址 */
#define CONFIG      0x00 /* 配置收发状态，CRC校验模式以及收发状态响应方式 */
#define EN_AA       0x01 /* 自动应答功能设置 */
#define EN_RXADDR   0x02 /* 可用信道设置 */
#define SETUP_AW    0x03 /* 收发地址宽度设置 */
#define SETUP_RETR  0x04 /* 自动重发功能设置 */
#define RF_CH       0x05 /* 工作频率设置 */
#define RF_SETUP    0x06 /* 发射速率、功耗功能设置 */
#define STATUS      0x07 /* 状态寄存器 */
#define OBSERVE_TX  0x08 /* 发送监测功能 */
#define CD          0x09 /* 地址检测 */
#define RX_ADDR_P0  0x0A /* 频道0接收数据地址 */
#define RX_ADDR_P1  0x0B /* 频道1接收数据地址 */
#define RX_ADDR_P2  0x0C /* 频道2接收数据地址 */
#define RX_ADDR_P3  0x0D /* 频道3接收数据地址 */
#define RX_ADDR_P4  0x0E /* 频道4接收数据地址 */
#define RX_ADDR_P5  0x0F /* 频道5接收数据地址 */
#define TX_ADDR     0x10 /* 发送地址寄存器 */
#define RX_PW_P0    0x11 /* 接收频道0接收数据长度 */
#define RX_PW_P1    0x12 /* 接收频道1接收数据长度 */
#define RX_PW_P2    0x13 /* 接收频道2接收数据长度 */
#define RX_PW_P3    0x14 /* 接收频道3接收数据长度 */
#define RX_PW_P4    0x15 /* 接收频道4接收数据长度 */
#define RX_PW_P5    0x16 /* 接收频道5接收数据长度 */
#define FIFO_STATUS 0x17 /* FIFO栈入栈出状态寄存器设置 */

uint8 init_NRF24L01 ( void );
uint8 SPI_RW ( uint8 tmp );
uint8 SPI_Read ( uint8 reg );
void SetRX_Mode ( void );
uint8 SPI_RW_Reg ( uint8 reg, uint8 value );
uint8 SPI_Read_Buf ( uint8 reg, uint8 *pBuf, uint8 uchars );
uint8 SPI_Write_Buf ( uint8 reg, uint8 *pBuf, uint8 uchars );
unsigned char nRF24L01_RxPacket ( unsigned char *rx_buf );
void nRF24L01_TxPacket ( unsigned char *tx_buf );

#define RX_DR 6
#define TX_DS 5
#define MAX_RT 4

uint8 sta; /* 状态标志 */

uint8 init_NRF24L01 ( void ) { /* NRF24L01初始化 */
    MISO_UP;
    CE_OUT;
    CSN_OUT;
    SCK_OUT;
    MOSI_OUT;
    MISO_IN;
    IRQ_IN;
    udelay ( 500 );
    CE_L; /* chip enable */
    ndelay ( 60 );
    CSN_H; /* Spi disable */
    ndelay ( 60 );
    SCK_L; /* Spi clock line init high */
    ndelay ( 60 );
    SPI_Write_Buf ( WRITE_REG + RX_ADDR_P0, RX_ADDRESS, RX_ADR_WIDTH ); /* 写接收端地址 */
    SPI_RW_Reg ( WRITE_REG + EN_AA, 0x00 ); /* 频道0自动ACK应答允许 */
    SPI_RW_Reg ( WRITE_REG + EN_RXADDR, 0x01 ); /* 允许接收地址只有频道0，如果需要多频道可以参考数据手册 */
    SPI_RW_Reg ( WRITE_REG + RF_CH, 40 ); /* 设置信道工作为2.4GHz，收发必须一致 */
    SPI_RW_Reg ( WRITE_REG + RX_PW_P0, RX_PLOAD_WIDTH ); /* 设置接收数据长度，本次设置为32字节 */
    SPI_RW_Reg ( WRITE_REG + RF_SETUP, 0x0f ); /* 设置发射速率为1MHz，发射功率为最大值0dB */
    SPI_RW_Reg ( WRITE_REG + CONFIG, 0x0f ); /* IRQ收发完成中断响应，16位CRC，主接收 */
    mdelay ( 1000 );
    nRF24L01_TxPacket ( TxBuf );
    SPI_RW_Reg ( WRITE_REG + STATUS, 0XFF );
    printk ( "test 1\n" );
    mdelay ( 1000 );
    return ( 1 );
}

uint8 SPI_RW ( uint8 tmp ) { /* NRF24L01的SPI写时序 */
    uint8 bit_ctr;

    for ( bit_ctr = 0 ; bit_ctr < 8 ; bit_ctr++ ) { /* output 8-bit */
        if ( tmp & 0x80 ) { /* output 'tmp', MSB to MOSI */
            MOSI_H;
        } else {
            MOSI_L;
        }

        tmp <<= 1; /* shift next bit into MSB */
        SCK_H; /* Set SCK high */
        ndelay ( 60 );

        if ( MISO_STU ) {
            tmp |= 0x01 ;
        }

        SCK_L; /* then set SCK low again */
        ndelay ( 60 );
    }

    return ( tmp ); /* return read tmp */
}

uint8 SPI_Read ( uint8 reg ) { /* NRF24L01的SPI读时序 */
    uint8 reg_val;
    CSN_L; /* CSN low, initialize SPI communication */
    ndelay ( 60 );
    SPI_RW ( reg ); /* Select register to read from */
    reg_val = SPI_RW ( 0 ); /* then read register value */
    CSN_H; /* CSN high, terminate SPI communication */
    ndelay ( 60 );
    return ( reg_val ); /* return register value */
}

uint8 SPI_RW_Reg ( uint8 reg, uint8 value ) { /* NRF24L01读写寄存器函数 */
    uint8 status;
    CSN_L; /* CSN low, init SPI transaction */
    ndelay ( 60 );
    status = SPI_RW ( reg ); /* select register */
    SPI_RW ( value ); /* write value to it */
    CSN_H; /* CSN high again */
    ndelay ( 60 );
    return ( status ); /* return nRF24L01 status */
}

/* 用于读数据，reg为寄存器地址，pBuf为待读出数据地址，uchars为读出数据的个数 */
uint8 SPI_Read_Buf ( uint8 reg, uint8 *pBuf, uint8 uchars ) {
    uint8 status, uint8_ctr;
    CSN_L; /* Set CSN low, init SPI tranaction */
    ndelay ( 60 );
    status = SPI_RW ( reg ); /* Select register to write to and read status */

    for ( uint8_ctr = 0; uint8_ctr < uchars; uint8_ctr++ ) {
        pBuf[uint8_ctr] = SPI_RW ( 0 );
        ndelay ( 20 );
    }

    CSN_H;
    ndelay ( 60 );
    return ( status ); /* return nRF24L01 status */
}

/* 用于写数据，reg为寄存器地址，pBuf为待写入数据地址，uchars写入数据的个数 */
uint8 SPI_Write_Buf ( uint8 reg, uint8 *pBuf, uint8 uchars ) {
    uint8 status, uint8_ctr;
    CSN_L;
    ndelay ( 60 );
    status = SPI_RW ( reg );

    for ( uint8_ctr = 0; uint8_ctr < uchars; uint8_ctr++ ) {
        SPI_RW ( *pBuf++ );
        ndelay ( 20 );
    }

    CSN_H;
    ndelay ( 60 );
    return ( status );
}

void SetRX_Mode ( void ) { /* 数据接收配置 */
    CE_L;
    ndelay ( 60 );
    SPI_Write_Buf ( WRITE_REG + RX_ADDR_P0, RX_ADDRESS, RX_ADR_WIDTH ); /* 写接收端地址 */
    SPI_RW_Reg ( WRITE_REG + EN_AA, 0x00 ); /* 频道0自动ACK应答允许 */
    SPI_RW_Reg ( WRITE_REG + EN_RXADDR, 0x01 ); /* 允许接收地址只有频道0，如果需要多频道可以参考数据手册 */
    SPI_RW_Reg ( WRITE_REG + RF_CH, 40 ); /* 设置信道工作为2.4GHz，收发必须一致 */
    SPI_RW_Reg ( WRITE_REG + RX_PW_P0, RX_PLOAD_WIDTH ); /* 设置接收数据长度，本次设置为32字节 */
    SPI_RW_Reg ( WRITE_REG + RF_SETUP, 0x0f ); /* 设置发射速率为1MHz，发射功率为最大值0dB */
    SPI_RW_Reg ( WRITE_REG + CONFIG, 0x0f ); /* IRQ收发完成中断响应，16位CRC，主接收 */
    CE_H;
    udelay ( 130 );
}

unsigned char nRF24L01_RxPacket ( unsigned char *rx_buf ) { /* 数据读取后放入rx_buf接收缓冲区中 */
    unsigned char revale = 0;
    sta = SPI_Read ( STATUS ); /* 读取状态寄存其来判断数据接收状况 */

    if ( sta & ( 1 << RX_DR ) ) { /* 判断是否接收到数据 */
        CE_L;
        udelay ( 50 );
        SPI_Read_Buf ( RD_RX_PLOAD, rx_buf, TX_PLOAD_WIDTH ); /* read receive payload from RX_FIFO buffer */
        printk ( "I get the data!\n" );
        printk ( "The data is %s\n", TxBuf );
        revale = 1; /* 读取数据完成标志 */
    }

    SPI_RW_Reg ( WRITE_REG + STATUS, sta ); /* 接收到数据后，RX_DR、TX_DS和MAX_PT都置为高，通过写1来清除中断标志 */
    return revale;
}

void nRF24L01_TxPacket ( unsigned char *tx_buf ) { /* 发送tx_buf中的数据 */
    CE_L;
    ndelay ( 60 );
    SPI_Write_Buf ( WRITE_REG + RX_ADDR_P0, TX_ADDRESS, TX_ADR_WIDTH ); /* 装载接收端地址 */
    SPI_Write_Buf ( WR_TX_PLOAD, tx_buf, TX_PLOAD_WIDTH ); /* 装载数据 */
    SPI_RW_Reg ( WRITE_REG + CONFIG, 0x0e ); /* IRQ收发完成中断响应，16位CRC，主发送 */
    CE_H; /* 置高CE，发送数据 */

    while ( s3c2410_gpio_getpin ( IRQ ) != 0 );

    udelay ( 10 );
}

/* 应用程序对设备文件(/dev/led)执行open时，就会调用s3c2440_leds_open函数 */
static int s3c2440_leds_open ( struct inode *inode, struct file *file ) {
    init_NRF24L01();
    return 0;
}

/* 应用程序对设备文件(/dev/led)执行ioctl时，就会调用s3c24xx_leds_ioctl函数 */
static int s3c2440_leds_ioctl ( struct inode *inode, struct file *file, unsigned int cmd, unsigned long arg ) {
    unsigned int data;

    if ( __get_user ( data, ( unsigned int __user * ) arg ) ) { /* 指针参数传递 */
        return -EFAULT;
    }

    switch ( cmd ) {
        case IOCTL_LED_ON:
            SetRX_Mode();
            nRF24L01_RxPacket ( TxBuf );
            mdelay ( 500 );
            return 0;

        case IOCTL_LED_OFF:
            return 0;

        case IOCTL_LED_RUN:
            return 0;

        case IOCTL_LED_SHINE:
            return 0;

        case IOCTL_LED_ALLON:
            return 0;

        case IOCTL_LED_ALLOFF:
            return 0;

        default:
            return -EINVAL;
    }
}

/* 这个结构是字符设备驱动程序的核心，当应用程序操作设备文件时所调用的open、read、write等函数，最终会调用这个结构中指定的对应函数 */
static struct file_operations s3c2440_leds_fops = {
    .owner = THIS_MODULE, /* 这是一个宏，推向编译模块时自动创建的“__this_module”变量 */
    .open = s3c2440_leds_open,
    .ioctl = s3c2440_leds_ioctl,
};

static void led_setup_cdev ( struct cdev *dev, int minor, struct file_operations *fops ) { /* Set up the cdev structure for a device */
    int err, devno = MKDEV ( led_major, minor );
    cdev_init ( dev, fops ); /* 设备的注册 */
    dev->owner = THIS_MODULE;
    dev->ops = fops;
    err = cdev_add ( dev, devno, 1 );

    if ( err ) { /* Fail gracefully if need be */
        printk ( KERN_NOTICE "Error %d adding Led%d", err, minor );
    }
}

static int __init s3c2440_leds_init ( void ) { /* 执行“insmod s3c24xx_leds.ko”命令时调用的函数 */
    int result;
    dev_t dev = MKDEV ( led_major, 0 );
    char dev_name[] = "led"; /* 加载驱动后，执行“cat /proc/devices”命令看到的设备名称 */

    if ( led_major ) { /* Figure out our device number */
        result = register_chrdev_region ( dev, 1, dev_name );
    } else {
        result = alloc_chrdev_region ( &dev, 0, 1, dev_name );
        led_major = MAJOR ( dev );
    }

    if ( result < 0 ) {
        printk ( KERN_WARNING "leds: unable to get major %d\n", led_major );
        return result;
    }

    if ( led_major == 0 ) {
        led_major = result;
    }

    /* Now set up cdev */
    led_setup_cdev ( &LedDevs, 0, &s3c2440_leds_fops );
    printk ( "Led device installed, with major %d\n", led_major );
    printk ( "The device name is: %s\n", dev_name );
    return 0;
}

static void __exit s3c2440_leds_exit ( void ) { /* 执行“rmmod s3c24xx_leds”命令时调用的函数，即卸载驱动程序 */
    cdev_del ( &LedDevs );
    unregister_chrdev_region ( MKDEV ( led_major, 0 ), 1 );
    printk ( "Led device uninstalled\n" );
}

/* 这两行指定驱动程序的初始化函数和卸载函数 */
module_init ( s3c2440_leds_init );
module_exit ( s3c2440_leds_exit );

/* 描述驱动程序的一些信息，不是必需的 */
MODULE_AUTHOR ( "http://embedclub.taobao.com" ); /* 驱动程序的作者 */
MODULE_DESCRIPTION ( "s3c2440 LED Driver" ); /* 一些描述信息 */
MODULE_LICENSE ( "Dual BSD/GPL" ); /* 遵循的协议 */