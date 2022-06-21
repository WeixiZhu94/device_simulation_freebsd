DEBUG = y
KMOD= madbus
SRCS= madbus.h ../../include/madbusioctls.h ../../include/maddefs.h \
../../include/madkonsts.h madbusmain.c mbdevthread.c \
vnode_if.h device_if.h bus_if.h pci_if.h
CFLAGS= -I/usr/src/sys/compat/linuxkpi/common/include
.include <bsd.kmod.mk>


