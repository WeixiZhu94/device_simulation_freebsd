/*
 *	PCI standard defines
 *	Copyright 1994, Drew Eckhardt
 *	Copyright 1997--1999 Martin Mares <mj@ucw.cz>
 *
 *	For more information, please consult the following manuals (look at
 *	http://www.pcisig.com/ for how to get them):
 *
 *	PCI BIOS Specification
 *	PCI Local Bus Specification
 *	PCI to PCI Bridge Specification
 *	PCI System Design Guide
 *
 *	For HyperTransport information, please consult the following manuals
 *	from http://www.hypertransport.org :
 *
 *	The HyperTransport I/O Link Specification
 */

#ifndef LINUX_PCI_REGS_H
#define LINUX_PCI_REGS_H

/*
 * Conventional PCI and PCI-X Mode 1 devices have 256 bytes of
 * configuration space.  PCI-X Mode 2 and PCIe devices have 4096 bytes of
 * configuration space.
 */
#define PCI_CFG_SPACE_SIZE	256
#define PCI_CFG_SPACE_EXP_SIZE	4096



/*
 * Under PCI, each device has 256 bytes of configuration address space,
 * of which the first 64 bytes are standardized as follows:
 */
#define PCI_STD_HEADER_SIZEOF   64
#define PCI_STD_NUM_BARS    6   /* Number of standard BARs */
// #define PCI_VENDOR_ID       0x00    /* 16 bits */
#define PCI_DEVICE_ID       0x02    /* 16 bits */
// #define PCI_COMMAND     0x04    /* 16 bits */
#define  PCI_COMMAND_IO     0x1 /* Enable response in I/O space */
#define  PCI_COMMAND_MEMORY 0x2 /* Enable response in Memory space */
#define  PCI_COMMAND_MASTER 0x4 /* Enable bus mastering */
#define  PCI_COMMAND_SPECIAL    0x8 /* Enable response to special cycles */
#define  PCI_COMMAND_INVALIDATE 0x10    /* Use memory write and invalidate */
#define  PCI_COMMAND_VGA_PALETTE 0x20   /* Enable palette snooping */
#define  PCI_COMMAND_PARITY 0x40    /* Enable parity checking */
#define  PCI_COMMAND_WAIT   0x80    /* Enable address/data stepping */
#define  PCI_COMMAND_SERR   0x100   /* Enable SERR */
#define  PCI_COMMAND_FAST_BACK  0x200   /* Enable back-to-back writes */
#define  PCI_COMMAND_INTX_DISABLE 0x400 /* INTx Emulation Disable */


/*
 * Base addresses specify locations in memory or I/O space.
 * Decoded size can be determined by writing a value of
 * 0xffffffff to the register, and reading it back.  Only
 * 1 bits are decoded.
 */
#define PCI_BASE_ADDRESS_0  0x10    /* 32 bits */
#define PCI_BASE_ADDRESS_1  0x14    /* 32 bits [htype 0,1 only] */
#define PCI_BASE_ADDRESS_2  0x18    /* 32 bits [htype 0 only] */
#define PCI_BASE_ADDRESS_3  0x1c    /* 32 bits */
#define PCI_BASE_ADDRESS_4  0x20    /* 32 bits */
#define PCI_BASE_ADDRESS_5  0x24    /* 32 bits */
#define  PCI_BASE_ADDRESS_SPACE     0x01    /* 0 = memory, 1 = I/O */
#define  PCI_BASE_ADDRESS_SPACE_IO  0x01
#define  PCI_BASE_ADDRESS_SPACE_MEMORY  0x00
#define  PCI_BASE_ADDRESS_MEM_TYPE_MASK 0x06
#define  PCI_BASE_ADDRESS_MEM_TYPE_32   0x00    /* 32 bit address */
#define  PCI_BASE_ADDRESS_MEM_TYPE_1M   0x02    /* Below 1M [obsolete] */
#define  PCI_BASE_ADDRESS_MEM_TYPE_64   0x04    /* 64 bit address */
#define  PCI_BASE_ADDRESS_MEM_PREFETCH  0x08    /* prefetchable? */
#define  PCI_BASE_ADDRESS_MEM_MASK  (~0x0fUL)
#define  PCI_BASE_ADDRESS_IO_MASK   (~0x03UL)
/* bit 1 is reserved if address_space = 1 */




/* Header type 0 (normal devices) */
#define PCI_CARDBUS_CIS     0x28
#define PCI_SUBSYSTEM_VENDOR_ID 0x2c
#define PCI_SUBSYSTEM_ID    0x2e
#define PCI_ROM_ADDRESS     0x30    /* Bits 31..11 are address, 10..1 reserved */
#define  PCI_ROM_ADDRESS_ENABLE 0x01
#define PCI_ROM_ADDRESS_MASK    (~0x7ffU)


/* 0x35-0x3b are reserved */
#define PCI_INTERRUPT_LINE  0x3c    /* 8 bits */
#define PCI_INTERRUPT_PIN   0x3d    /* 8 bits */
#define PCI_MIN_GNT     0x3e    /* 8 bits */
#define PCI_MAX_LAT     0x3f    /* 8 bits */



#define PCI_CAPABILITY_LIST 0x34    /* Offset of first capability list entry */


/* Capability lists */

#define PCI_CAP_LIST_ID     0   /* Capability ID */
// #define  PCI_CAP_ID_PM      0x01    /* Power Management */
// #define  PCI_CAP_ID_AGP     0x02    /* Accelerated Graphics Port */
#define  PCI_CAP_ID_VPD     0x03    /* Vital Product Data */
#define  PCI_CAP_ID_SLOTID  0x04    /* Slot Identification */
#define  PCI_CAP_ID_MSI     0x05    /* Message Signalled Interrupts */
#define  PCI_CAP_ID_CHSWP   0x06    /* CompactPCI HotSwap */
// #define  PCI_CAP_ID_PCIX    0x07    /* PCI-X */
#define  PCI_CAP_ID_HT      0x08    /* HyperTransport */
#define  PCI_CAP_ID_VNDR    0x09    /* Vendor-Specific */
#define  PCI_CAP_ID_DBG     0x0A    /* Debug port */
#define  PCI_CAP_ID_CCRC    0x0B    /* CompactPCI Central Resource Control */
#define  PCI_CAP_ID_SHPC    0x0C    /* PCI Standard Hot-Plug Controller */
#define  PCI_CAP_ID_SSVID   0x0D    /* Bridge subsystem vendor/device ID */
#define  PCI_CAP_ID_AGP3    0x0E    /* AGP Target PCI-PCI bridge */
#define  PCI_CAP_ID_SECDEV  0x0F    /* Secure Device */
// #define  PCI_CAP_ID_EXP     0x10    /* PCI Express */
#define  PCI_CAP_ID_MSIX    0x11    /* MSI-X */
#define  PCI_CAP_ID_SATA    0x12    /* SATA Data/Index Conf. */
#define  PCI_CAP_ID_AF      0x13    /* PCI Advanced Features */
#define  PCI_CAP_ID_EA      0x14    /* PCI Enhanced Allocation */
#define  PCI_CAP_ID_MAX     PCI_CAP_ID_EA
#define PCI_CAP_LIST_NEXT   1   /* Next capability in the list */
#define PCI_CAP_FLAGS       2   /* Capability defined flags (16 bits) */
#define PCI_CAP_SIZEOF      4


#endif /* LINUX_PCI_REGS_H */