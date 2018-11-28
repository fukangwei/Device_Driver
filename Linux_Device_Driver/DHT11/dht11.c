#include <linux/miscdevice.h>
#include <linux/delay.h>
#include <asm/irq.h>
#include <mach/regs-gpio.h>
#include <mach/hardware.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/mm.h>
#include <linux/fs.h>
#include <linux/types.h>
#include <linux/delay.h>
#include <linux/moduleparam.h>
#include <linux/slab.h>
#include <linux/errno.h>
#include <linux/ioctl.h>
#include <linux/cdev.h>
#include <linux/string.h>
#include <linux/list.h>
#include <linux/pci.h>
#include <linux/gpio.h>
#include <asm/uaccess.h>
#include <asm/atomic.h>
#include <asm/unistd.h>

#define DEVICE_NAME "dht11"

static char DHT11_read_byte ( void ) {
    char DHT11_byte;
    unsigned char i;
    unsigned char temp;
    DHT11_byte = 0;

    for ( i = 0; i < 8; i ++ ) {
        temp = 0;

        while ( ! ( s3c2410_gpio_getpin ( S3C2410_GPF ( 0 ) ) ) ) {
            temp++;

            if ( temp > 12 ) {
                return 1;
            }

            udelay ( 5 );
        }

        temp = 0;

        while ( s3c2410_gpio_getpin ( S3C2410_GPF ( 0 ) ) ) {
            temp++;

            if ( temp > 20 ) {
                return 1;
            }

            udelay ( 5 );
        }

        if ( temp > 6 ) {
            DHT11_byte <<= 1;
            DHT11_byte |= 1;
        } else {
            DHT11_byte <<= 1;
            DHT11_byte |= 0;
        }
    }

    return DHT11_byte;
}

static ssize_t DHT11_read ( struct file *filp, char __user *buf, size_t count, loff_t *f_pos ) {
    unsigned char DataTemp;
    unsigned char i;
    unsigned char err;
    char tempBuf[5];
    err = 0;
    s3c2410_gpio_cfgpin ( S3C2410_GPF ( 0 ), S3C2410_GPIO_OUTPUT );
    s3c2410_gpio_setpin ( S3C2410_GPF ( 0 ), 0 );
    msleep ( 18 );
    s3c2410_gpio_setpin ( S3C2410_GPF ( 0 ), 1 );
    udelay ( 40 );
    s3c2410_gpio_cfgpin ( S3C2410_GPF ( 0 ), S3C2410_GPIO_INPUT );

    if ( !err ) {
        DataTemp = 10;

        while ( ! ( s3c2410_gpio_getpin ( S3C2410_GPF ( 0 ) ) ) && DataTemp ) {
            DataTemp--;
            udelay ( 10 );
        }

        if ( !DataTemp ) {
            err = 1;
            count = -EFAULT;
        }
    }

    if ( !err ) {
        DataTemp = 10;

        while ( ( s3c2410_gpio_getpin ( S3C2410_GPF ( 0 ) ) ) && DataTemp ) {
            DataTemp--;
            udelay ( 10 );
        }

        if ( !DataTemp ) {
            err = 1;
            count = -EFAULT;
        }
    }

    if ( !err ) {
        for ( i = 0; i < 5; i ++ ) {
            tempBuf[i] = DHT11_read_byte ();
        }

        DataTemp = 0;

        for ( i = 0; i < 4; i ++ ) {
            DataTemp += tempBuf[i];
        }

        if ( DataTemp != tempBuf[4] ) {
            count = -EFAULT;
        }

        if ( count > 5 ) {
            count = 5;
        }

        if ( copy_to_user ( buf, tempBuf, count ) ) {
            count = -EFAULT;
        }
    }

    s3c2410_gpio_cfgpin ( S3C2410_GPF ( 0 ), S3C2410_GPIO_OUTPUT );
    s3c2410_gpio_setpin ( S3C2410_GPF ( 0 ), 1 );
    return count;
}

static struct file_operations dev_fops = {
    .owner = THIS_MODULE,
    .read = DHT11_read,
};

static struct miscdevice misc = {
    .minor = MISC_DYNAMIC_MINOR,
    .name = DEVICE_NAME,
    .fops = &dev_fops,
};

static int __init DHT11_init_module ( void ) {
    int ret;
    s3c2410_gpio_cfgpin ( S3C2410_GPF ( 0 ), S3C2410_GPIO_OUTPUT );
    s3c2410_gpio_setpin ( S3C2410_GPF ( 0 ), 1 );
    ret = misc_register ( &misc );
    printk ( DEVICE_NAME"\tinitialized\n" );
    return ret;
}

static void __exit DHT11_exit_module ( void ) {
    misc_deregister ( &misc );
}

module_init ( DHT11_init_module );
module_exit ( DHT11_exit_module );

MODULE_LICENSE ( "GPL" );
MODULE_AUTHOR ( "Shihui Jin (CUMT_CS)" ); /* Here is the author, thanks! */