TARGET_MODULE := maddevb.ko
DEBUG = y
DEPS = madbus.h madioctls.h simdrvrlib.h maddefs.h madkonsts.h maddevb.h

maddevb-objs := maddevbmain.o maddevb_blk_utils.o maddevbio.o maddevb_zoned.o

obj-m		:= maddevb.o 
KERN_SRC	:= /lib/modules/$(shell uname -r)/build/
PWD			:= $(shell pwd)

KBUILD_EXTRA_SYMBOLS := /root/device_simulation_freebsd/madsim_v11/madsim/madbus/KERN_SRC/Module.symvers

modules:
	make -C $(KERN_SRC) M=$(PWD) modules

install:
	make -C $(KERN_SRC) M=$(PWD) modules_install
	depmod -a

clean:
	make -C $(KERN_SRC) M=$(PWD) clean