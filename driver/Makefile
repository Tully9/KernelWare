obj-m += kernelware.o
kernelware-objs := kernelware_main.o kernelware_proc.o
all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean