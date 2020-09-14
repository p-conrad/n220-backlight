ifneq ($(KERNELRELEASE),)
	obj-m := n220-backlight.o
else
	KDIR := /lib/modules/$(shell uname -r)/build
	PWD  := $(shell pwd)
default:
	$(MAKE) -C $(KDIR) M=$(PWD) modules
endif

clean:
	@rm -rf *~ *.o *.mod.c .*.cmd .tmp_versions *.ko *.mod *.mod.o *.cmd modules.order Module.symvers
