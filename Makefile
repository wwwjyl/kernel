#for linux kernel module

ifneq ($(KERNELRELEASE),)
obj-m:=block_read.o
else
KDIR=/home/ls/studio/src/linux-2.6.35.6/
MDIR=`pwd`

default:
	$(MAKE) -C $(KDIR) M=$(MDIR) modules

clean:
	rm -fr *.o *.mod.c *.ko *.mod.o

endif
