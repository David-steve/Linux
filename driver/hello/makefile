KVERS=$(shell uname -r)

# Kernel modules
obj-m=hello.o

# source code directory
KERNEL :=/lib/modules/$(KVERS)/build

modules:
	$(MAKE) -C $(KERNEL) M=$(CURDIR) modules
	
clean:
	$(MAKE) -C $(KERNEL) M=$(CURDIR) clean
