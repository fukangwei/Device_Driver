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

static int led_major = 0; /* ���豸�� */
static struct cdev LedDevs;

uint8 Tx_Buf1[32];

/* Ӧ�ó���ִ��ioctl(fd, cmd, arg)ʱ�ĵ�2������ */
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

/* NRF24L01�˿ڶ��� */
#define CE_OUT   s3c2410_gpio_cfgpin(CE, CE_OUTP) /* ����������Ϊ��� */
#define CE_UP    s3c2410_gpio_pullup(CE, 1) /* ���������� */
#define CE_L     s3c2410_gpio_setpin(CE, 0) /* ���������ߵ�ƽ */
#define CE_H     s3c2410_gpio_setpin(CE, 1) /* ���������ߵ�ƽ */
#define SCK_OUT  s3c2410_gpio_cfgpin(SCK, SCK_OUTP) /* ����������Ϊ��� */
#define SCK_UP   s3c2410_gpio_pullup(SCK, 1) /* ���������� */
#define SCK_L    s3c2410_gpio_setpin(SCK, 0) /* ���������ߵ�ƽ */
#define SCK_H    s3c2410_gpio_setpin(SCK, 1) /* ���������ߵ�ƽ */
#define MISO_IN  s3c2410_gpio_cfgpin(MISO, MISO_INP) /* ����������Ϊ��� */
#define MISO_UP  s3c2410_gpio_pullup(MISO, 1) /* ���������� */
#define MISO_STU s3c2410_gpio_getpin(MISO) /* ����״̬ */
#define IRQ_IN   s3c2410_gpio_cfgpin(IRQ, IRQ_INP) /* ����������Ϊ��� */
#define IRQ_UP   s3c2410_gpio_pullup(IRQ, 1) /* ���������� */
#define IRQ_L    s3c2410_gpio_setpin(IRQ, 0) /* ���������ߵ�ƽ */
#define IRQ_H    s3c2410_gpio_setpin(IRQ, 1) /* ���������ߵ�ƽ */
#define MOSI_OUT s3c2410_gpio_cfgpin(MOSI, MOSI_OUTP) /* ����������Ϊ��� */
#define MOSI_UP  s3c2410_gpio_pullup(MOSI, 1) /* ���������� */
#define MOSI_L   s3c2410_gpio_setpin(MOSI, 0) /* ���������ߵ�ƽ */
#define MOSI_H   s3c2410_gpio_setpin(MOSI, 1) /* ���������ߵ�ƽ */
#define CSN_OUT  s3c2410_gpio_cfgpin(CSN, CSN_OUTP) /* ����������Ϊ��� */
#define CSN_UP   s3c2410_gpio_pullup(CSN, 1) /* ���������� */
#define CSN_L    s3c2410_gpio_setpin(CSN, 0) /* ���������ߵ�ƽ */
#define CSN_H    s3c2410_gpio_setpin(CSN, 1) /* ���������ߵ�ƽ */

#define TX_ADR_WIDTH 5 /* 5 uint8s TX address width */
#define RX_ADR_WIDTH 5 /* 5 uint8s RX address width */
#define TX_PLOAD_WIDTH 32 /* 20 uint8s TX payload */
#define RX_PLOAD_WIDTH 32 /* 20 uint8s TX payload */

uint8 const TX_ADDRESS[TX_ADR_WIDTH] = {0xE7, 0xE7, 0xE7, 0xE7, 0xE7}; /* ���ص�ַ */
uint8 const RX_ADDRESS[RX_ADR_WIDTH] = {0xE7, 0xE7, 0xE7, 0xE7, 0xE7}; /* ���յ�ַ */

/* NRF24L01�Ĵ���ָ�� */
#define READ_REG    0x00 /* ���Ĵ���ָ�� */
#define WRITE_REG   0x20 /* д�Ĵ���ָ�� */
#define RD_RX_PLOAD 0x61 /* ��ȡ��������ָ�� */
#define WR_TX_PLOAD 0xA0 /* д��������ָ�� */
#define FLUSH_TX    0xE1 /* ��ϴ����FIFOָ�� */
#define FLUSH_RX    0xE2 /* ��ϴ����FIFOָ�� */
#define REUSE_TX_PL 0xE3 /* �����ظ�װ������ָ�� */
#define NOP         0xFF /* ���� */

/* SPI(nRF24L01)�Ĵ�����ַ */
#define CONFIG      0x00 /* �����շ�״̬��CRCУ��ģʽ�Լ��շ�״̬��Ӧ��ʽ */
#define EN_AA       0x01 /* �Զ�Ӧ�������� */
#define EN_RXADDR   0x02 /* �����ŵ����� */
#define SETUP_AW    0x03 /* �շ���ַ������� */
#define SETUP_RETR  0x04 /* �Զ��ط��������� */
#define RF_CH       0x05 /* ����Ƶ������ */
#define RF_SETUP    0x06 /* �������ʡ����Ĺ������� */
#define STATUS      0x07 /* ״̬�Ĵ��� */
#define OBSERVE_TX  0x08 /* ���ͼ�⹦�� */
#define CD          0x09 /* ��ַ��� */
#define RX_ADDR_P0  0x0A /* Ƶ��0�������ݵ�ַ */
#define RX_ADDR_P1  0x0B /* Ƶ��1�������ݵ�ַ */
#define RX_ADDR_P2  0x0C /* Ƶ��2�������ݵ�ַ */
#define RX_ADDR_P3  0x0D /* Ƶ��3�������ݵ�ַ */
#define RX_ADDR_P4  0x0E /* Ƶ��4�������ݵ�ַ */
#define RX_ADDR_P5  0x0F /* Ƶ��5�������ݵ�ַ */
#define TX_ADDR     0x10 /* ���͵�ַ�Ĵ��� */
#define RX_PW_P0    0x11 /* ����Ƶ��0�������ݳ��� */
#define RX_PW_P1    0x12 /* ����Ƶ��1�������ݳ��� */
#define RX_PW_P2    0x13 /* ����Ƶ��2�������ݳ��� */
#define RX_PW_P3    0x14 /* ����Ƶ��3�������ݳ��� */
#define RX_PW_P4    0x15 /* ����Ƶ��4�������ݳ��� */
#define RX_PW_P5    0x16 /* ����Ƶ��5�������ݳ��� */
#define FIFO_STATUS 0x17 /* FIFOջ��ջ��״̬�Ĵ������� */

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

uint8 sta; /* ״̬��־ */

uint8 init_NRF24L01 ( void ) { /* NRF24L01��ʼ�� */
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
    SPI_Write_Buf ( WRITE_REG + RX_ADDR_P0, RX_ADDRESS, RX_ADR_WIDTH ); /* д���ն˵�ַ */
    SPI_RW_Reg ( WRITE_REG + EN_AA, 0x00 ); /* Ƶ��0�Զ�ACKӦ������ */
    SPI_RW_Reg ( WRITE_REG + EN_RXADDR, 0x01 ); /* ������յ�ַֻ��Ƶ��0�������Ҫ��Ƶ�����Բο������ֲ� */
    SPI_RW_Reg ( WRITE_REG + RF_CH, 40 ); /* �����ŵ�����Ϊ2.4GHz���շ�����һ�� */
    SPI_RW_Reg ( WRITE_REG + RX_PW_P0, RX_PLOAD_WIDTH ); /* ���ý������ݳ��ȣ���������Ϊ32�ֽ� */
    SPI_RW_Reg ( WRITE_REG + RF_SETUP, 0x0f ); /* ���÷�������Ϊ1MHz�����书��Ϊ���ֵ0dB */
    SPI_RW_Reg ( WRITE_REG + CONFIG, 0x0f ); /* IRQ�շ�����ж���Ӧ��16λCRC�������� */
    mdelay ( 1000 );
    nRF24L01_TxPacket ( TxBuf );
    SPI_RW_Reg ( WRITE_REG + STATUS, 0XFF );
    printk ( "test 1\n" );
    mdelay ( 1000 );
    return ( 1 );
}

uint8 SPI_RW ( uint8 tmp ) { /* NRF24L01��SPIдʱ�� */
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

uint8 SPI_Read ( uint8 reg ) { /* NRF24L01��SPI��ʱ�� */
    uint8 reg_val;
    CSN_L; /* CSN low, initialize SPI communication */
    ndelay ( 60 );
    SPI_RW ( reg ); /* Select register to read from */
    reg_val = SPI_RW ( 0 ); /* then read register value */
    CSN_H; /* CSN high, terminate SPI communication */
    ndelay ( 60 );
    return ( reg_val ); /* return register value */
}

uint8 SPI_RW_Reg ( uint8 reg, uint8 value ) { /* NRF24L01��д�Ĵ������� */
    uint8 status;
    CSN_L; /* CSN low, init SPI transaction */
    ndelay ( 60 );
    status = SPI_RW ( reg ); /* select register */
    SPI_RW ( value ); /* write value to it */
    CSN_H; /* CSN high again */
    ndelay ( 60 );
    return ( status ); /* return nRF24L01 status */
}

/* ���ڶ����ݣ�regΪ�Ĵ�����ַ��pBufΪ���������ݵ�ַ��ucharsΪ�������ݵĸ��� */
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

/* ����д���ݣ�regΪ�Ĵ�����ַ��pBufΪ��д�����ݵ�ַ��ucharsд�����ݵĸ��� */
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

void SetRX_Mode ( void ) { /* ���ݽ������� */
    CE_L;
    ndelay ( 60 );
    SPI_Write_Buf ( WRITE_REG + RX_ADDR_P0, RX_ADDRESS, RX_ADR_WIDTH ); /* д���ն˵�ַ */
    SPI_RW_Reg ( WRITE_REG + EN_AA, 0x00 ); /* Ƶ��0�Զ�ACKӦ������ */
    SPI_RW_Reg ( WRITE_REG + EN_RXADDR, 0x01 ); /* ������յ�ַֻ��Ƶ��0�������Ҫ��Ƶ�����Բο������ֲ� */
    SPI_RW_Reg ( WRITE_REG + RF_CH, 40 ); /* �����ŵ�����Ϊ2.4GHz���շ�����һ�� */
    SPI_RW_Reg ( WRITE_REG + RX_PW_P0, RX_PLOAD_WIDTH ); /* ���ý������ݳ��ȣ���������Ϊ32�ֽ� */
    SPI_RW_Reg ( WRITE_REG + RF_SETUP, 0x0f ); /* ���÷�������Ϊ1MHz�����书��Ϊ���ֵ0dB */
    SPI_RW_Reg ( WRITE_REG + CONFIG, 0x0f ); /* IRQ�շ�����ж���Ӧ��16λCRC�������� */
    CE_H;
    udelay ( 130 );
}

unsigned char nRF24L01_RxPacket ( unsigned char *rx_buf ) { /* ���ݶ�ȡ�����rx_buf���ջ������� */
    unsigned char revale = 0;
    sta = SPI_Read ( STATUS ); /* ��ȡ״̬�Ĵ������ж����ݽ���״�� */

    if ( sta & ( 1 << RX_DR ) ) { /* �ж��Ƿ���յ����� */
        CE_L;
        udelay ( 50 );
        SPI_Read_Buf ( RD_RX_PLOAD, rx_buf, TX_PLOAD_WIDTH ); /* read receive payload from RX_FIFO buffer */
        printk ( "I get the data!\n" );
        printk ( "The data is %s\n", TxBuf );
        revale = 1; /* ��ȡ������ɱ�־ */
    }

    SPI_RW_Reg ( WRITE_REG + STATUS, sta ); /* ���յ����ݺ�RX_DR��TX_DS��MAX_PT����Ϊ�ߣ�ͨ��д1������жϱ�־ */
    return revale;
}

void nRF24L01_TxPacket ( unsigned char *tx_buf ) { /* ����tx_buf�е����� */
    CE_L;
    ndelay ( 60 );
    SPI_Write_Buf ( WRITE_REG + RX_ADDR_P0, TX_ADDRESS, TX_ADR_WIDTH ); /* װ�ؽ��ն˵�ַ */
    SPI_Write_Buf ( WR_TX_PLOAD, tx_buf, TX_PLOAD_WIDTH ); /* װ������ */
    SPI_RW_Reg ( WRITE_REG + CONFIG, 0x0e ); /* IRQ�շ�����ж���Ӧ��16λCRC�������� */
    CE_H; /* �ø�CE���������� */

    while ( s3c2410_gpio_getpin ( IRQ ) != 0 );

    udelay ( 10 );
}

/* Ӧ�ó�����豸�ļ�(/dev/led)ִ��openʱ���ͻ����s3c2440_leds_open���� */
static int s3c2440_leds_open ( struct inode *inode, struct file *file ) {
    init_NRF24L01();
    return 0;
}

/* Ӧ�ó�����豸�ļ�(/dev/led)ִ��ioctlʱ���ͻ����s3c24xx_leds_ioctl���� */
static int s3c2440_leds_ioctl ( struct inode *inode, struct file *file, unsigned int cmd, unsigned long arg ) {
    unsigned int data;

    if ( __get_user ( data, ( unsigned int __user * ) arg ) ) { /* ָ��������� */
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

/* ����ṹ���ַ��豸��������ĺ��ģ���Ӧ�ó�������豸�ļ�ʱ�����õ�open��read��write�Ⱥ��������ջ��������ṹ��ָ���Ķ�Ӧ���� */
static struct file_operations s3c2440_leds_fops = {
    .owner = THIS_MODULE, /* ����һ���꣬�������ģ��ʱ�Զ������ġ�__this_module������ */
    .open = s3c2440_leds_open,
    .ioctl = s3c2440_leds_ioctl,
};

static void led_setup_cdev ( struct cdev *dev, int minor, struct file_operations *fops ) { /* Set up the cdev structure for a device */
    int err, devno = MKDEV ( led_major, minor );
    cdev_init ( dev, fops ); /* �豸��ע�� */
    dev->owner = THIS_MODULE;
    dev->ops = fops;
    err = cdev_add ( dev, devno, 1 );

    if ( err ) { /* Fail gracefully if need be */
        printk ( KERN_NOTICE "Error %d adding Led%d", err, minor );
    }
}

static int __init s3c2440_leds_init ( void ) { /* ִ�С�insmod s3c24xx_leds.ko������ʱ���õĺ��� */
    int result;
    dev_t dev = MKDEV ( led_major, 0 );
    char dev_name[] = "led"; /* ����������ִ�С�cat /proc/devices����������豸���� */

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

static void __exit s3c2440_leds_exit ( void ) { /* ִ�С�rmmod s3c24xx_leds������ʱ���õĺ�������ж���������� */
    cdev_del ( &LedDevs );
    unregister_chrdev_region ( MKDEV ( led_major, 0 ), 1 );
    printk ( "Led device uninstalled\n" );
}

/* ������ָ����������ĳ�ʼ��������ж�غ��� */
module_init ( s3c2440_leds_init );
module_exit ( s3c2440_leds_exit );

/* �������������һЩ��Ϣ�����Ǳ���� */
MODULE_AUTHOR ( "http://embedclub.taobao.com" ); /* ������������� */
MODULE_DESCRIPTION ( "s3c2440 LED Driver" ); /* һЩ������Ϣ */
MODULE_LICENSE ( "Dual BSD/GPL" ); /* ��ѭ��Э�� */