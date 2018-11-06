CROSS_COMPILE := arm-linux-
CC :=$(CROSS_COMPILE)gcc
KERNEL_DIR := /home/fukangwei/samba/linux-2.6.32.2

obj-m := NRF24L01_drv.o

PWD := $(shell pwd)
all:
	make -C  $(KERNEL_DIR) M=$(PWD) modules
clean:
	rm -fr *.o *.ko *.mod.o *.mod.c .*.cmd *.order *.*s *~ .tmp*
