# DEBUG = y
KMOD= maddevc
SRCS= \
maddevcmain.c maddevcio.c \
vnode_if.h device_if.h bus_if.h pci_if.h
CFLAGS= -I/usr/src/sys/compat/linuxkpi/common/include -I../include
.include <bsd.kmod.mk>


# madbusioctls.h maddefs.h madkonsts.h \
