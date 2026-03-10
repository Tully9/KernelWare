obj-m += kernelware.o
kernelware-objs := kernelware_main.o kernelware_proc.o
all:
	make -C /lib/modules/$(shell uname -r)/build M=$(dir $(abspath $(lastword $(MAKEFILE_LIST)))) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(dir $(abspath $(lastword $(MAKEFILE_LIST)))) clean