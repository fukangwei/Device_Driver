#include <linux/init.h>
#include <linux/module.h>
#include <linux/delay.h>
#include <linux/kernel.h>
#include <linux/moduleparam.h>
#include <linux/init.h>
#include <linux/types.h>
#include <linux/fs.h>
#include <mach/regs-gpio.h>
#include <mach/hardware.h>
#include <linux/cdev.h>
#include <asm/uaccess.h>
#include <linux/errno.h>
#include <linux/gpio.h>
#include <linux/device.h>

/* 相关引脚定义，方便以后移植 */
#define DQ      S3C2410_GPF(3)
#define CFG_IN  S3C2410_GPIO_INPUT
#define CFG_OUT S3C2410_GPIO_OUTPUT

/* ds18b20主次设备号(动态分配) */
static int ds18b20_major = 0;
static int ds18b20_minor = 0;
static int ds18b20_nr_devs = 1;

static struct ds18b20_device { /* 定义设备类型 */
    struct cdev cdev;
};

struct ds18b20_device *ds18b20_devp; /* 设备结构体指针 */

static struct class *ds18b20_class;
static struct class_device *ds18b20_class_dev;

/* 函数声明 */
static int ds18b20_open ( struct inode *inode, struct file *filp );
static int ds18b20_init ( void );
static void write_byte ( unsigned char data );
static unsigned char read_byte ( void );
static ssize_t ds18b20_read ( struct file *filp, char __user *buf, size_t count, loff_t *f_pos );
void ds18b20_setup_cdev ( struct ds18b20_device *dev, int index );

static int ds18b20_open ( struct inode *inode, struct file *filp ) {
    int flag = 0;
    flag = ds18b20_init();

    if ( flag & 0x01 ) {
        printk ( KERN_WARNING "open ds18b20 failed\n" );
        return -1;
    }

    printk ( KERN_NOTICE "open ds18b20 successful\n" );
    return 0;
}

static int ds18b20_init ( void ) {
    int retval = 0;
    s3c2410_gpio_cfgpin ( DQ, CFG_OUT );
    s3c2410_gpio_pullup ( DQ, 0 );
    s3c2410_gpio_setpin ( DQ, 1 );
    udelay ( 2 );
    s3c2410_gpio_setpin ( DQ, 0 ); /* 拉低ds18b20总线，复位ds18b20 */
    udelay ( 500 ); /* 保持复位电平500us */
    s3c2410_gpio_setpin ( DQ, 1 ); /* 释放ds18b20总线 */
    udelay ( 60 );
    /* 若复位成功，ds18b20发出存在脉冲(低电平，持续60至240us) */
    s3c2410_gpio_cfgpin ( DQ, CFG_IN );
    retval = s3c2410_gpio_getpin ( DQ );
    udelay ( 500 );
    s3c2410_gpio_cfgpin ( DQ, CFG_OUT );
    s3c2410_gpio_pullup ( DQ, 0 );
    s3c2410_gpio_setpin ( DQ, 1 ); /* 释放总线 */
    return retval;
}

static void write_byte ( unsigned char data ) {
    int i = 0;
    s3c2410_gpio_cfgpin ( DQ, CFG_OUT );
    s3c2410_gpio_pullup ( DQ, 1 );

    for ( i = 0; i < 8; i++ ) {
        /* 总线从高拉至低电平时，就产生写时序 */
        s3c2410_gpio_setpin ( DQ, 1 );
        udelay ( 2 );
        s3c2410_gpio_setpin ( DQ, 0 );
        s3c2410_gpio_setpin ( DQ, data & 0x01 );
        udelay ( 60 );
        data >>= 1;
    }

    s3c2410_gpio_setpin ( DQ, 1 ); /* 重新释放ds18b20总线 */
}

static unsigned char read_byte ( void ) {
    int i;
    unsigned char data = 0;

    for ( i = 0; i < 8; i++ ) {
        /* 总线从高拉至低，只需维持低电平17us，再把总线拉高，就产生读时序 */
        s3c2410_gpio_cfgpin ( DQ, CFG_OUT );
        s3c2410_gpio_pullup ( DQ, 0 );
        s3c2410_gpio_setpin ( DQ, 1 );
        udelay ( 2 );
        s3c2410_gpio_setpin ( DQ, 0 );
        udelay ( 2 );
        s3c2410_gpio_setpin ( DQ, 1 );
        udelay ( 8 );
        data >>= 1;
        s3c2410_gpio_cfgpin ( DQ, CFG_IN );

        if ( s3c2410_gpio_getpin ( DQ ) ) {
            data |= 0x80;
        }

        udelay ( 50 );
    }

    s3c2410_gpio_cfgpin ( DQ, CFG_OUT );
    s3c2410_gpio_pullup ( DQ, 0 );
    s3c2410_gpio_setpin ( DQ, 1 ); /* 释放ds18b20总线 */
    return data;
}

static ssize_t ds18b20_read ( struct file *filp, char __user *buf, size_t count, loff_t *f_pos ) {
    int flag;
    unsigned long err;
    unsigned char result[2] = { 0x00, 0x00 };
    flag = ds18b20_init();

    if ( flag & 0x01 ) {
        printk ( KERN_WARNING "ds18b20 init failed\n" );
        return -1;
    }

    write_byte ( 0xcc );
    write_byte ( 0x44 );
    flag = ds18b20_init();

    if ( flag & 0x01 ) {
        return -1;
    }

    write_byte ( 0xcc );
    write_byte ( 0xbe );
    result[0] = read_byte(); /* 温度低八位 */
    result[1] = read_byte(); /* 温度高八位 */
    err = copy_to_user ( buf, &result, sizeof ( result ) );
    return err ? -EFAULT : min ( sizeof ( result ), count );
}

static struct file_operations ds18b20_dev_fops = {
    .owner = THIS_MODULE,
    .open = ds18b20_open,
    .read = ds18b20_read,
};

void ds18b20_setup_cdev ( struct ds18b20_device *dev, int index ) {
    int err, devno = MKDEV ( ds18b20_major, ds18b20_minor + index );
    cdev_init ( &dev->cdev, &ds18b20_dev_fops );
    dev->cdev.owner = THIS_MODULE;
    err = cdev_add ( & ( dev->cdev ), devno, 1 );

    if ( err ) {
        printk ( KERN_NOTICE "ERROR %d add ds18b20\n", err );
    }
}

static int __init ds18b20_dev_init ( void ) {
    int result;
    dev_t dev = 0;
    dev = MKDEV ( ds18b20_major, ds18b20_minor );

    if ( ds18b20_major ) {
        result = register_chrdev_region ( dev, ds18b20_nr_devs, "ds18b20" );
    } else {
        result = alloc_chrdev_region ( &dev, ds18b20_minor, ds18b20_nr_devs, "ds18b20" );
        ds18b20_major = MAJOR ( dev );
    }

    if ( result < 0 ) {
        printk ( KERN_WARNING "ds18b20: failed to get major\n" );
        return result;
    }

    ds18b20_devp = kmalloc ( sizeof ( struct ds18b20_device ), GFP_KERNEL ); /* 为新设备分配内存和初始化 */

    if ( !ds18b20_devp ) { /* 申请失败 */
        result = -ENOMEM;
        goto fail_malloc;
    }

    memset ( ds18b20_devp, 0, sizeof ( struct ds18b20_device ) );
    ds18b20_setup_cdev ( ds18b20_devp, 0 );
    ds18b20_class = class_create ( THIS_MODULE, "ds18b20_sys_class" ); /* 自动创建设备节点 */

    if ( IS_ERR ( ds18b20_class ) ) {
        return PTR_ERR ( ds18b20_class );
    }

    ds18b20_class_dev = device_create ( ds18b20_class, NULL, MKDEV ( ds18b20_major, 0 ), NULL, "ds18b20" );

    if ( unlikely ( IS_ERR ( ds18b20_class_dev ) ) ) {
        return PTR_ERR ( ds18b20_class_dev );
    }

    return 0;
fail_malloc:
    unregister_chrdev_region ( dev, 1 );
    return result;
}

static void __exit ds18b20_dev_exit ( void ) {
    cdev_del ( &ds18b20_devp->cdev ); /* 注销cdev */
    kfree ( ds18b20_devp ); /* 释放设备结构体内存 */
    unregister_chrdev_region ( MKDEV ( ds18b20_major, 0 ), ds18b20_nr_devs ); /* 释放设备号 */
    device_unregister ( ds18b20_class_dev );
    class_destroy ( ds18b20_class );
}

module_init ( ds18b20_dev_init );
module_exit ( ds18b20_dev_exit );
MODULE_LICENSE ( "Dual BSD/GPL" );
MODULE_AUTHOR ( "sg131971@qq.com" ); /* Here is the author, thanks! */