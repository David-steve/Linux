KVERS=$(shell uname -r)

# Kernel modules
obj-m=hello.o

KERNEL :=/lib/modules/$(KVERS)/build

modules:
	$(MAKE) -C $(KERNEL) M=$(CURDIR) modules
	
clean:
	$(MAKE) -C $(KERNEL) M=$(CURDIR) clean
