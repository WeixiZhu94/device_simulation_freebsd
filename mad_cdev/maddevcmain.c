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
/*  Exe files   : maddevc.ko                                                   */ 
/*                                                                             */
/*  Module NAME : maddevcmain.c                                                */
/*                                                                             */
/*  DESCRIPTION : Main module for the MAD character-mode driver                */
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
/* $Id: maddevcmain.c, v 1.0 2021/01/01 00:00:00 htf $                         */
/*                                                                             */
/*******************************************************************************/

#define _DEVICE_DRIVER_MAIN_
#include "maddevc.h"		/* local definitions */
#include "vm_cdev.h"

//Function prototypes
static ssize_t
maddev_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos);
static ssize_t
maddev_write(struct file *filp, const char __user *buf, size_t count, loff_t *f_pos);

static long maddev_ioctl(struct file *filp, unsigned int cmd, unsigned long arg);

//Our parameters which can be set at load time.
int maddev_major = MADDEVOBJ_MAJOR;
int maddev_minor = 0;
int   maddev_max_devs = MADBUS_NUMBER_SLOTS;	
int maddev_nbr_devs = 1;	/* number of bare maddev devices */

//A set of names for multiple devices
char MadDevNames[10][20] =
     {MADDEVOBJNAME, MADDEVOBJNAME, MADDEVOBJNAME, MADDEVOBJNAME, MADDEVOBJNAME, MADDEVOBJNAME, MADDEVOBJNAME, MADDEVOBJNAME};
char MadDevNumStr[] = DEVNUMSTR;

MADREGS MadRegsRst = {0, MAD_STATUS_CACHE_INIT_MASK, 0, 0x080B0B, 0, 0, 0, 0, 0, 0, 0, 0};

struct mad_dev_obj *mad_dev_objects; /* allocated in maddev_init_module */

struct class *mad_class = NULL;
char maddrvrname[] = DRIVER_NAME;

// The structures & entry points general to a pci module
struct pci_device_id pci_ids[] =
{
	{ PCI_DEVICE(MAD_PCI_VENDOR_ID, MAD_PCI_CHAR_INT_DEVICE_ID), },
	{ PCI_DEVICE(MAD_PCI_VENDOR_ID, MAD_PCI_CHAR_MSI_DEVICE_ID), },
	{ 0, } 	//{ PCI_DEVICE(0, 0) }, //doesn't work
};
//
MODULE_DEVICE_TABLE(pci, pci_ids);

struct driver_private maddrvr_priv_data =
{
    .driver = NULL,
};

struct pci_driver maddev_driver =
{
    .driver = {
        .name = maddrvrname, 
    },
    //
    .id_table = pci_ids,
    .probe    = maddev_probe,
    .suspend  = NULL,
    .resume   = NULL,
    .remove   = maddev_remove,
    .shutdown = maddev_shutdown,
};

#include "../include/maddrvrdefs.c"

/*
 * Open and close
 */
//This is the open function for one hardware device
static int maddev_open(struct inode *inode, struct file *fp)
{
	// cdev was saved in freebsd file->si_drv1, which is saved in linux file->f_cdev by linux_dev_fdopen
	struct mad_dev_obj *pmaddevobj = 
                       container_of((struct linux_cdev *)fp->f_cdev, struct mad_dev_obj, cdev_str);

    printf("[!!!] cdev :%p, pmaddevobj : %p\n", fp->f_cdev, pmaddevobj);

	PINFO("maddev_open... dev#=%d maddev=%p inode=%p fp=%p\n",
		  (int)pmaddevobj->devnum, pmaddevobj, inode, fp);

    mutex_lock(&pmaddevobj->devmutex);
    fp->private_data = pmaddevobj; /* for other methods */
    mutex_unlock(&pmaddevobj->devmutex);

	return 0;          /* success */
}

//This is the release function for one hardware device
static int maddev_release(struct inode *inode, struct file *fp)
{
	struct mad_dev_obj *pmaddevobj = (struct mad_dev_obj*)fp->private_data;

    printf("[!!!] cdev :%p, pmaddevobj : %p\n", fp->f_cdev, pmaddevobj);

	PINFO("maddev_release...dev#=%d inode=%p fp=%p\n",
          (int)pmaddevobj->devnum, inode, fp);

	return 0;
}

/*
 * Data management: read and write
 */
//This is the generic read function
static ssize_t
maddev_read(struct file *fp, char __user *usrbufr, size_t count, loff_t *f_pos)
{
    struct mad_dev_obj *pmaddevobj = (struct mad_dev_obj*)fp->private_data;
    ssize_t iocount = 0;                                                         

    PINFO("maddev_read... dev#=%d fp=%p count=%ld offset_arg=%ld\n",
          (int)pmaddevobj->devnum, fp, (U32)count, (U32)*f_pos);

    mutex_lock(&pmaddevobj->devmutex);

    if (count <= MAD_BUFRD_IO_MAX_SIZE)
        //Do a normal buffered read
        iocount = maddev_read_bufrd(fp, usrbufr, count, f_pos);
    else
        //Do a large buffered read using direct-io of the user buffer
        iocount = maddev_direct_io(fp, usrbufr, count, f_pos, false);

    mutex_unlock(&pmaddevobj->devmutex);

    return iocount;
}

//This is the generic write function
static ssize_t maddev_write(struct file *fp, const char __user *usrbufr, size_t count,
                            loff_t *f_pos)
{
    PMADDEVOBJ pmaddevobj = (PMADDEVOBJ)fp->private_data;
    ssize_t iocount = 0;                                                         

    PINFO("maddev_write... dev#=%d fp=%p count=%ld offset_arg=%ld\n",
          (int)pmaddevobj->devnum, fp, (U32)count, (U32)*f_pos);

    mutex_lock(&pmaddevobj->devmutex);

    if (count < MAD_BUFRD_IO_MAX_SIZE)
        //Do a normal buffered write 
        iocount = maddev_write_bufrd(fp, usrbufr, count, f_pos);
    else
        //Do a large buffered write using direct-io of the user buffer
        iocount = maddev_direct_io(fp, usrbufr, count, f_pos, true);

    mutex_unlock(&pmaddevobj->devmutex);

    return iocount;
}

/*
 * The ioctl() implementation
 */
static long maddev_ioctl(struct file *fp, unsigned int cmd, unsigned long arg)
{
	static MADREGS  MadRegs;

	struct mad_dev_obj *pmaddevobj = fp->private_data;
	PMADREGS        pmadregs  = (PMADREGS)pmaddevobj->pDevBase;
	PMADCTLPARMS    pCtlParms = (PMADCTLPARMS)arg;

	int err = 0;
	long retval = 0;
	U32  remains = 0;
    u32 flags1 = 0;
    struct accelerator_kernel_args kernel_launch_args;

	printf("Doing ioctl: dev#=%d fp=%p cmd=%x arg=%lx\n", (int)pmaddevobj->devnum, fp, cmd, arg);

	/* The direction is a bitmask, and VERIFY_WRITE catches R/W
	 * transfers. `Type' is user-oriented, while
	 * access_ok is kernel-oriented, so the concept of "read" and
	 * "write" is reversed */
    err = !linux_access_ok(/*VERIFY_WRITE,*/ (void __user *)arg, _IOC_SIZE(cmd));
	if (err)
        {
		PDEBUG( "maddev_ioctl returning -EINVAL\n");
		return -EINVAL;
        }

    //If the ioctl queue is not free we return
    if (pmaddevobj->ioctl_f != eIoReset)
        return -EAGAIN;

    mutex_lock(&pmaddevobj->devmutex);

    switch(cmd)
	    {
	    case MADDEVOBJ_IOC_INIT: //Initialize the device in a standard way
	    	PDEBUG( "maddev_ioctl MADDEVOBJ_IOC_INIT\n");
            maddev_acquire_lock_disable_ints(&pmaddevobj->devlock, flags1);
            memcpy_toio(&MadRegsRst, pmadregs, sizeof(MADREGS));
            maddev_enable_ints_release_lock(&pmaddevobj->devlock, flags1);
            break;

	    case MADDEVOBJ_IOC_RESET: //Reset all index registers
	    	PDEBUG("maddev_ioctl MADDEVOBJ_IOC_RESET\n");
            maddev_acquire_lock_disable_ints(&pmaddevobj->devlock, flags1);
            iowrite32(0, &pmadregs->ByteIndxRd);
	    	iowrite32(0, &pmadregs->ByteIndxWr);
	    	iowrite32(0, &pmadregs->CacheIndxRd);
	    	iowrite32(0, &pmadregs->CacheIndxWr);
	    	iowrite32(MAD_STATUS_CACHE_INIT_MASK, &pmadregs->Status);
            maddev_enable_ints_release_lock(&pmaddevobj->devlock, flags1);
            break;

	    case MADDEVOBJ_IOC_GET_DEVICE:
		    PDEBUG("maddev_ioctl... dev#=%d; MADDEVOBJ_IOC_GET_DEVICE\n",
                   (int)pmaddevobj->devnum);
		    //
		    memset(&MadRegs, 0xcc, sizeof(MADREGS));
            maddev_acquire_lock_disable_ints(&pmaddevobj->devlock, flags1);
            memcpy_fromio(&MadRegs, pmadregs, sizeof(MADREGS));
            maddev_enable_ints_release_lock(&pmaddevobj->devlock, flags1);

            remains =
		    copy_to_user(&pCtlParms->MadRegs, &MadRegs, sizeof(MADREGS)); //possibly paged memory - copy outside of spinlock
		    if (remains > 0)
                {
                PERR("maddev_ioctl:copy_to_user...  dev#=%d bytes_remaining=%ld rc=-EFAULT\n",
                     (int)pmaddevobj->devnum, remains);
	    		retval = -EFAULT;
                }
		    break;

	    case MADDEVOBJ_IOC_SET_ENABLE:
		    MadRegs.IntEnable = arg;
            maddev_acquire_lock_disable_ints(&pmaddevobj->devlock, flags1);
            iowrite32(MadRegs.IntEnable, &pmadregs->IntEnable);
            maddev_enable_ints_release_lock(&pmaddevobj->devlock, flags1);
            break;

	    case MADDEVOBJ_IOC_SET_CONTROL:
		    MadRegs.Control = arg;
            maddev_acquire_lock_disable_ints(&pmaddevobj->devlock, flags1);
            iowrite32(MadRegs.Control, &pmadregs->Control);
            maddev_enable_ints_release_lock(&pmaddevobj->devlock, flags1);
	        break;

        case MADDEVOBJ_IOC_SET_READ_INDX:
            MadRegs.ByteIndxRd = MAD_MODULO_ADJUST(arg, MAD_UNITIO_SIZE_BYTES);
            PDEBUG("maddev_ioctl... dev#=%d read_dx=%ld\n",
                   (int)pmaddevobj->devnum, pmadregs->ByteIndxRd);
            maddev_acquire_lock_disable_ints(&pmaddevobj->devlock, flags1);
		    iowrite32(MadRegs.ByteIndxRd, &pmadregs->ByteIndxRd);
            maddev_enable_ints_release_lock(&pmaddevobj->devlock, flags1);
            PDEBUG("maddev_ioctl... dev#=%d set_rd_indx=%ld\n",
                   (int)pmaddevobj->devnum, pmadregs->ByteIndxRd);
		    break;

        case MADDEVOBJ_IOC_SET_WRITE_INDX:
		    MadRegs.ByteIndxWr = MAD_MODULO_ADJUST(arg, MAD_UNITIO_SIZE_BYTES);
            PDEBUG("maddev_ioctl... dev#=%d write_dx=%ld\n",
                   (int)pmaddevobj->devnum, pmadregs->ByteIndxWr);
            maddev_acquire_lock_disable_ints(&pmaddevobj->devlock, flags1);
		    iowrite32(MadRegs.ByteIndxWr, &pmadregs->ByteIndxWr);
            maddev_enable_ints_release_lock(&pmaddevobj->devlock, flags1);
            PDEBUG("maddev_ioctl... dev#=%d set_wr_indx=%ld\n",
                   (int)pmaddevobj->devnum, pmadregs->ByteIndxWr);
		    break;

        //Not implemented
	    case MADDEVOBJ_IOC_GET_ENABLE:
	    case MADDEVOBJ_IOC_GET_CONTROL:
        case MADDEVOBJ_IOC_ALIGN_READ_CACHE:
        case MADDEVOBJ_IOC_ALIGN_WRITE_CACHE:
	        retval = -ENOSYS; 
            break;

        case MADDEVOBJ_IOC_CTX_CREATE:
            retval = setup_ctx((gmem_vm_mode) arg);
            printf("UVAS has been set up\n");
            break;

        case MADDEVOBJ_IOC_LAUNCH_KERNEL:
            copyin((void *)arg, &kernel_launch_args, sizeof(struct accelerator_kernel_args));
            // kernel_launch_args = (struct accelerator_kernel_args *) arg;
            retval = run_kernel(kernel_launch_args.kernel_type, kernel_launch_args.kernel_args);
            break;

	    default:
		    PERR("maddev_ioctl... dev#=%d rc=-EINVAL\n",
                   (int)pmaddevobj->devnum);
		    retval = -EINVAL;
	    }

    mutex_unlock(&pmaddevobj->devmutex);

	return retval;
}

//This is the open function for the virtual memory area struct
static void maddev_vma_open(struct vm_area_struct* vma)
{
	struct mad_dev_obj *pmaddevobj = vma->vm_private_data;

	PINFO( "maddev_vma_open... dev#=%d\n", (int)pmaddevobj->devnum);
}

//This is the close function for the virtual memory area struct
static void maddev_vma_close(struct vm_area_struct* vma)
{
	struct mad_dev_obj *pmaddevobj = vma->vm_private_data;

	PINFO( "maddev_vma_close... dev#=%d\n", (int)pmaddevobj->devnum);
}

// static int maddev_vma_fault(struct vm_fault *vmf)
// {
// 	struct vm_area_struct *vma = vmf->vma;
// 	struct mad_dev_obj *pmaddevobj = (struct mad_dev_obj *)vma->vm_private_data;
// 	//
// 	struct page *page_str = NULL;
// 	unsigned char *pageptr = NULL; /* default to "missing" */
// 	unsigned long ofset = 0;
// 	int retval = 0;

// 	PDEBUG( "maddev_vma_fault... dev#=%d, vma=%p, vmf=%p\n", (int)pmaddevobj->devnum, vma, vmf);

// 	down(&pmaddevobj->devsem);
// 	//
// 	ofset = (unsigned long)(vma->vm_pgoff << PAGE_SHIFT) + (vmf->address - vma->vm_start);
// 	pageptr = (void *)pmaddevobj->pDevBase;
// 	pageptr += ofset;
// 	page_str = virt_to_page(pageptr);
// 	//
// 	PDEBUG( "maddev_vma_fault... dev#=%d, pageptr=%p, PhysAddr=0x%0X\n",
// 		   (int)pmaddevobj->devnum, pageptr, (unsigned int)__pa(pageptr));

// 	/* got it, now increment the count */
// 	get_page(page_str);
// 	vmf->page = page_str;
// 	//
// 	up(&pmaddevobj->devsem);

// 	return retval;
// } 
  
//This table specifies the entry points for VM mapping operations
static struct vm_operations_struct maddev_remap_vm_ops =
{
		.open  = maddev_vma_open,
		.close = maddev_vma_close,
		//.fault = maddev_vma_fault,
};

//This is the function invoked when an application calls the mmap function
//on the device. It returns a virtual mode address for the memory-mapped device
static int maddev_mmap(struct file *fp, struct vm_area_struct* vma)
{
	struct mad_dev_obj *pmaddevobj = fp->private_data;
	// struct inode* inode_str = fp->f_inode;
    U32    pfn              = phys_to_pfn(pmaddevobj->MadDevPA);
    size_t MapSize          = vma->vm_end - vma->vm_start;
	//
	int rc = 0;

	PINFO("maddev_mmap... dev#=%d fp=%px pfn=0x%lX PA=x%lX MapSize=%ld\n",
          (int)pmaddevobj->devnum, fp, pfn, pmaddevobj->MadDevPA, MapSize);

    mutex_lock(&pmaddevobj->devmutex);

    //Map/remap the Page Frame Number of the phys addr of the device into
    //user mode virtual addr. space
    rc = remap_pfn_range(vma, vma->vm_start, pfn, MapSize, vma->vm_page_prot);
    if (rc != 0)
        {
        mutex_unlock(&pmaddevobj->devmutex);
        PERR("maddev_mmap:remap_pfn_range... dev#=%d rc=%d\n",
             (int)pmaddevobj->devnum, rc);
        return rc;
        }

	vma->vm_ops = &maddev_remap_vm_ops;
	vma->vm_flags |= VM_IO; //RESERVED;
	vma->vm_private_data = fp->private_data;

    //Increment the reference count on first use
	maddev_vma_open(vma);
    mutex_unlock(&pmaddevobj->devmutex);

    PDEBUG("maddev_mmap:remap_pfn_range... dev#=%d start=%lx rc=%d\n",
           (int)pmaddevobj->devnum, vma->vm_start, rc);

    return rc;
}

//This is the table of entry points for device i/o operations
static struct file_operations maddev_fops = 
{
	.owner          = THIS_MODULE,
	.llseek         = no_llseek,
	.read           = maddev_read,
	.write          = maddev_write,
    // .read_iter      = maddev_queued_read,
    // .write_iter     = maddev_queued_write,
    .unlocked_ioctl = maddev_ioctl,
    .mmap           = maddev_mmap,
    .open           = maddev_open,
	.release        = maddev_release,
};

/*
 * Set up the char_dev structure for this device.
 */
static int maddev_setup_cdev(void* pvoid, int indx)
{
    struct mad_dev_obj* pmaddevobj = (struct mad_dev_obj*)pvoid;
	int devno = MKDEV(maddev_major, maddev_minor + indx);
	int rc = 0;
        
	PINFO("maddev_setup_cdev... dev#=%d maddev_major=%d maddev_minor=%d cdev_no=x%X\n",
		   indx, maddev_major, (maddev_minor+indx), devno);

    //Initialize the cdev structure
	cdev_init(&pmaddevobj->cdev_str, &maddev_fops);
	pmaddevobj->cdev_str.owner = THIS_MODULE;

    // set name
    kobject_set_name(&pmaddevobj->cdev_str.kobj, "maddevc%d", pmaddevobj->devnum);

    //Introduce the device to the kernel
	rc = cdev_add(&pmaddevobj->cdev_str, devno, 1);
	printf("[!!!] adding cdev %p to madobj %p\n", &pmaddevobj->cdev_str, pmaddevobj);
    PDEBUG("maddev_setup_cdev... dev#=%d rc=%d\n", (int)pmaddevobj->devnum, rc);

    return rc;
}

// This is the driver init function
// It allocates memory and initializes all static device objects
static int maddev_init_module(void)
{
	int   rc = 0;
    int   i;
    dev_t dev = 0;
	PMADDEVOBJ pmaddevobj = NULL;
	U32    devcount = 0;
    U32    len;
    // size_t SetSize = (maddev_max_devs + 1) * PAGE_SIZE;
    struct pci_dev* pPciDvTmp = NULL;
 
	PINFO("maddev_init_module... mjr=%d mnr=%d\n", maddev_major, maddev_minor);

	if (maddev_major == 0)
    {// Get a range of minor numbers, asking for a dynamic major number 
        rc = 
        alloc_chrdev_region(&dev, maddev_minor, maddev_nbr_devs, MAD_MAJOR_OBJECT_NAME);
        if (rc == 0)
        {
            maddev_major = MAJOR(dev);
	        PINFO("maddev_init_module... allocated major number (%d)\n", maddev_major);
        }
    }
    else
	{// Get a range of minor numbers - using the assigned major #
	    dev = MKDEV(maddev_major, maddev_minor);
		rc = 
        register_chrdev_region(maddev_major, maddev_nbr_devs, MAD_MAJOR_OBJECT_NAME);
	}

	if (rc < 0)
	{
		PERR("maddev_init_module: can't register/alloc chrdev region... rc=%d\n", rc);
		goto InitFail;
	}

    //Create a class for creating device nodes for hotplug devices
    len = strlen(MadDevNames[0]);
    MadDevNames[0][len-1] = 0x00;
    mad_class = class_create(THIS_MODULE, MadDevNames[0]);
    if (IS_ERR(mad_class))
    {
        rc = PTR_ERR(mad_class);
        PERR("maddev_init_module:class_create returned %d\n", rc);
        rc = 0; //Let's continue
    }

	mad_dev_objects = kzalloc(sizeof(MADDEVOBJ) * (maddev_max_devs + 1), MAD_KMALLOC_FLAGS);
	if (!mad_dev_objects)
	{
		rc = -ENOMEM;
        PERR("maddev_init_module failing to get memory... rc=-ENOMEM\n");
		goto InitFail;  
	}

    //* Register with the driver core through the bus driver API
    rc = pci_register_driver(&maddev_driver);
    if (rc != 0)
    {
        PERR("maddev_init_module:pci_register_driver error!... rc=%d\n", rc);
		goto InitFail;  
	}

    /* Create & initialize each device. */
	for (i = 1; i <= maddev_nbr_devs; i++)
	{
        pmaddevobj = &mad_dev_objects[i];
        pmaddevobj->devnum = i;
        rc = maddev_setup_device(pmaddevobj, &pPciDvTmp, false); 
        if (rc != 0)
            {
            PERR("maddev_init_module:maddev_setup_device... dev#=%d rc=%d - continuing\n",
                 (int)pmaddevobj->devnum, rc);
            continue;
            }

        devcount++;
	}

#ifdef MADDEVOBJ_DEBUG /* only when debugging */
	maddev_create_proc();
#endif

    if (maddev_nbr_devs == 0) //No static devices
        {rc = 0;} 
    else //Did at least one static device init succeed
        {rc = (devcount > 0) ? 0 : -ENODEV;}
 
    return rc;

InitFail:
	maddev_cleanup_module();
	PERR( "maddev_init_module failure - returning (%d)\n", rc);

	return rc;
}

static int fake_device_event_handler(struct module *module,
    int event_type, void *arg) 
{
    int retval = 0;

    switch (event_type) {

        case MOD_LOAD:
            uprintf("simulated character dev started\n");
            maddev_init_module();
            break;

        case MOD_UNLOAD:
            uprintf("simulated character dev terminated\n");
            maddev_cleanup_module();
            break;

        default:
            retval = EOPNOTSUPP;
            break;
    }

    return retval;
}

static moduledata_t fake_dev = {
    "simulate_character_dev",
    fake_device_event_handler,
    NULL
};

DECLARE_MODULE(maddevc, fake_dev, SI_SUB_DRIVERS, SI_ORDER_MIDDLE);
MODULE_VERSION(maddevc, 1);
MODULE_DEPEND(maddevc, madbus, 1, 1, 1);