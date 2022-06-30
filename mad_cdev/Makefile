# DEBUG = y
KMOD= maddevc
SRCS= \
maddevcmain.c maddevcio.c emulator.c pm_cdev.c vm_cdev.c vm_cdev.h \
vnode_if.h device_if.h bus_if.h pci_if.h
CFLAGS= -I/usr/src/sys/compat/linuxkpi/common/include -I../include
.include <bsd.kmod.mk>


# madbusioctls.h maddefs.h madkonsts.h \
