/* SPDX-License-Identifier: GPL-2.0 WITH Linux-syscall-note */
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

#endif /* LINUX_PCI_REGS_H */