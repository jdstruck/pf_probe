# To build modules outside of the kernel tree, we run "make"
# in the kernel source tree; the Makefile there then includes this
# Makefile once again.

# This conditional selects whether we are being included from the
# kernel Makefile or not.
ifeq ($(KERNELRELEASE),)

	# Assume the source tree is where the running kernel was built
	# You should set KERNELDIR in the environment if it's elsewhere
	KERNELDIR ?= /lib/modules/$(shell uname -r)/build

	# The current directory is passed to sub-makes as argument
	PWD := $(shell pwd)

modules:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules

modules_install:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules_install

clean:
	rm -rf user *.o *~ core .depend .*.cmd *.ko *.mod.c .tmp_versions Module.symvers modules.order

.PHONY: modules modules_install clean


else
	# called from kernel build system: just declare what our modules are
	obj-m += pf_probe_A.o 
	obj-m += pf_probe_B.o 
endif

kmod = pf_probe_A 

user: user.c
	gcc -Wall -g -o user user.c
run: user
	sudo ./user
reload:
	sudo rmmod $(kmod)
	make
	sudo insmod $(kmod).ko

