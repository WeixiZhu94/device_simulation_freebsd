/********1*********2*********3*********4*********5**********6*********7*********/
/*                                                                             */
/*  PRODUCT      : MAD Device Simulation Framework                             */
/*  COPYRIGHT    : (c) 2021 HTF Consulting                                     */
/*                                                                             */
/* This source code is provided by Dual/GPL license to the Linux open source   */ 
/* community                                                                   */
/*                                                                             */ 
/*******************************************************************************/
/*                                                                             */
/*  Exe files   : madbus.ko, maddevc.ko, maddevb.ko                            */ 
/*                                                                             */
/*  Module NAME : madbuss.h                                                    */
/*                                                                             */
/*  DESCRIPTION : Definitions for the Model-Abstract Device bus driver         */
/*                                                                             */
/*  MODULE_AUTHOR("HTF Consulting");                                           */
/*  MODULE_LICENSE("Dual/GPL");                                                */
/*                                                                             */
/* The source code in this file can be freely used, adapted, and redistributed */
/* in source or binary form, so long as an acknowledgment appears in derived   */
/* source files.  The citation should state that the source code comes from    */
/* a set of source files developed by HTF Consulting                           */
/* http://www.htfconsulting.com                                                */
/*                                                                             */
/* No warranty is attached.                                                    */
/* HTF Consulting assumes no responsibility for errors or fitness of use       */
/*                                                                             */
/*                                                                             */
/* This source file was derived from source code developed by                  */
/* ALESSANDRO RUBINI and JONATHAN CORBET.                                      */
/* Copyright (C) 2001 O'REILLY & ASSOCIATES -- appearing in the book:          */
/* "LINUX DEVICE DRIVERS" by Rubini and Corbet,                                */
/* published by O'Reilly & Associates.                                         */
/* No warranty is attached to the original source code.                        */
/*                                                                             */
/*                                                                             */
/* $Id: madbus.h, v 1.0 2021/01/01 00:00:00 htf $                              */
/*                                                                             */
/*******************************************************************************/

#include <linux/ioctl.h> /* needed for the _IOW etc stuff used later */
#include <linux/fs.h>		/* everything... */
#include <linux/cdev.h>
#include <linux/kthread.h>
#include <linux/slab.h>		/* kmalloc() */
#include <linux/module.h>
#include <linux/pci.h>
#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/mm.h>
#include <linux/page.h>
#include <linux/interrupt.h>
#include <linux/uaccess.h>

#ifdef _SIM_DRIVER_
#define DRIVER_NAME "madbus.ko"
#endif

// Linux compatibility for FreeBSD
struct klist_node {
    void            *n_klist;   /* never access directly */
    struct list_head    n_node;
    struct kref     n_ref;
};

#define UEVENT_NUM_ENVP         32  /* number of env pointers */
#define UEVENT_BUFFER_SIZE      2048    /* buffer for the variables */

struct kobj_uevent_env {
    char *argv[3];
    char *envp[UEVENT_NUM_ENVP];
    int envp_idx;
    char buf[UEVENT_BUFFER_SIZE];
    int buflen;
};

static inline int add_uevent_var(struct kobj_uevent_env *env, const char *format, ...)
{
    va_list args;
    int len;

    if (env->envp_idx >= ARRAY_SIZE(env->envp)) {
        WARN(1, KERN_ERR "add_uevent_var: too many keys\n");
        return -ENOMEM;
    }

    va_start(args, format);
    len = vsnprintf(&env->buf[env->buflen],
            sizeof(env->buf) - env->buflen,
            format, args);
    va_end(args);

    if (len >= (sizeof(env->buf) - env->buflen)) {
        WARN(1, KERN_ERR "add_uevent_var: buffer size too small\n");
        return -ENOMEM;
    }

    env->envp[env->envp_idx++] = &env->buf[env->buflen];
    env->buflen += len + 1;
    return 0;
}

#include "maddefs.h"
#include "madkonsts.h"
#include "madbusioctls.h"

#include <sys/kthread.h>

#define  MADBUSOBJNAME   "madbusobjX"
#define  MBDEVNUMDX      9 //......^

#define MADBUS_MAJOR_OBJECT_NAME     "madbus_object"
#define MADBUS_NBR_DEVS              MAD_NBR_DEVS

//A parm area to exchange information between the simulator & device driver(s)
//A device driver only cares about this in simulation mode
//
struct mad_simulator_parms
{
    void*       pmadbusobj;
    void        (*pcomplete_simulated_io)(void* pmadbusobj, PMADREGS pmaddevice); 

    void*       pInBufr;
    void*       pOutBufr;
    //
	spinlock_t* pdevlock;
	void*       pmaddevobj;
    PMADREGS    pmadregs;
};
//
typedef struct mad_simulator_parms *PMAD_SIMULATOR_PARMS;

// The device context structure for the bus-level char-mode device
struct madbus_object
{
	U32        devnum;
    U32        junk; //8-byte align below
    char       PciConfig[MAD_PCI_CFG_SPACE_SIZE];

	//struct     mad_driver *driver;
	//struct     semaphore sem;     /* mutual exclusion semaphore     */
	//spinlock_t devlock;
    struct page* pPage;
	phys_addr_t  MadDevPA;
    PMADREGS    pmaddevice;
    //
    irq_handler_t  isrfn[8];
    int            irq[8];

    //The device to be exposed in SysFs
    U32        bRegstrd;
    struct     device  sysfs_dev;

    //The PCI sumulation device
    U16        pci_devid;
    struct     pci_dev pcidev;

	struct     linux_cdev cdev_str;
    char       dummy[sizeof(struct list_head)];
	struct     proc *pThread;
	struct     mad_simulator_parms SimParms;
   	char       *name;
};
//
typedef struct madbus_object MADBUSOBJ, *PMADBUSOBJ;

#define to_madbus_object(dev) container_of(dev, struct madbus_object, dev);

extern int register_mad_device(struct madbus_object *);
extern void unregister_mad_device(struct madbus_object *);
extern int madbus_create_thread(PMADBUSOBJ pmadbusobj);
extern void madbus_dev_thread(void* pvoid);
//
void madsim_complete_simulated_io(void* vpmadbusobj, PMADREGS pmadregs);
#ifdef _SIM_DRIVER_
#include "simdrvrlib.h"
#endif

#define page_to_virt(x)     PHYS_TO_DMAP(VM_PAGE_TO_PHYS(x))
#define virt_to_phys(x)     DMAP_TO_PHYS(x)
#define phys_to_virt(x)     PHYS_TO_DMAP(x)

//


