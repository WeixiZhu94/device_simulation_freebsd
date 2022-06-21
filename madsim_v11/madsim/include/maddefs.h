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
/*  Exe files   : madbus.ko, maddevc.ko, maddevb.ko, madsimui.exe, madtest.exe */ 
/*                                                                             */
/*  Module NAME : maddefs.h                                                    */
/*                                                                             */
/*  DESCRIPTION : Properties & Definitions for the Model-Abstract Device       */
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
/* HTF Consulting takes no responsibility for errors or fitness of use         */
/*                                                                             */
/*                                                                             */
/* $Id: madfefs.h, v 1.0 2021/01/01 00:00:00 htf $                             */
/*                                                                             */
/*******************************************************************************/

#ifndef _MADDEFS_
#define _MADDEFS_

#ifndef phys_to_pfn 
        #define    phys_to_pfn(p)  ((p) >> PAGE_SHIFT)
#endif

#define  HTFC_PCI_VENDOR_ID      0x9808
//
#define  MAD_PCI_VENDOR_ID       HTFC_PCI_VENDOR_ID
//
#define  MAD_PCI_BASE_DEVICE_ID      0x1001
#define  MAD_PCI_CHAR_DEVICE_ID      MAD_PCI_BASE_DEVICE_ID
#define  MAD_PCI_CHAR_INT_DEVICE_ID  MAD_PCI_BASE_DEVICE_ID
#define  MAD_PCI_CHAR_MSI_DEVICE_ID  (MAD_PCI_BASE_DEVICE_ID+1)
#define  MAD_PCI_BLOCK_DEVICE_ID     (MAD_PCI_BASE_DEVICE_ID+2)
#define  MAD_PCI_BLOCK_INT_DEVICE_ID (MAD_PCI_BASE_DEVICE_ID+2)
#define  MAD_PCI_BLOCK_MSI_DEVICE_ID (MAD_PCI_BASE_DEVICE_ID+3)
#define  MAD_PCI_MAX_DEVICE_ID       MAD_PCI_BLOCK_MSI_DEVICE_ID

#define  MAD_PCI_CFG_SPACE_SIZE  (4096 / 2) // 2K
#define  MAD_PCI_VENDOR_OFFSET   (MAD_PCI_CFG_SPACE_SIZE / 2) // 1K

#define  MADBUS_NUMBER_SLOTS     3

#define MAD_KMALLOC_FLAGS          GFP_KERNEL  
#define MAD_DEVICE_KMALLOC_FLAGS   GFP_DMA32

//Iconic types
typedef unsigned long      ULONG;
typedef unsigned long long ULONG64;

// #ifndef u8
// typedef unsigned char u8;
// #endif
// //
// typedef unsigned char U8;

// #ifndef u16
// typedef unsigned short u16;
// #endif
// //
typedef unsigned short U16;
typedef unsigned long U32;
typedef ULONG64 U64;

// #ifndef u64
// typedef ULONG64 u64;
// #endif
// //

#ifndef PAGE_SIZE
#define PAGE_SIZE 4096
#endif

//Configuration-Property definitions ............................................
typedef enum  _MAD_DEV_INT_MODE {ePOLLED=0, 
                                 eLEGACY, eLineBased=eLEGACY, eLevelSensitive=eLEGACY,
                                 eMSI, eLATCHED=eMSI}  MAD_DEV_INT_MODE;
//
#define MADDEV_INT_MODE eLEGACY //Not really used - unless the device driver needs to support both 

#define MAD_BASE_IRQ 30 //Devices 1..4 should use Base_irq+1 .. Base_irq+4

typedef enum {eError=-1, eIoCmplt=0, eIoReset=eIoCmplt, eIoPending} IoState;

typedef enum {eNOP=0, eBufrdRd, eBufrdWr, eLoadRdCache, eFlushWrCache,
              eAlignRdCache, eAlignWrCache, eDmaRead, eDmaWrite, eInvalid} IoMesgId;
//Names .........................................................................
//
#ifdef PAGE_SIZE
#define LINUX_PAGE_SIZE  PAGE_SIZE
#else
#define LINUX_PAGE_SIZE  4096
#endif
//
#define PFN_AND_MASK     (0x7FFFFFFF >> (PAGE_SHIFT-1))

#define  LINUX_DEVICE_PATH_PREFIX  "/dev/"

#define  DEVNUMSTR           "0123456789ABCDEF"
#define  DEVNUMSTRLEN        16

//General MACRO definitions .................................................................
//
#define ASSERT(x) if (!x)  \
        {printk(KERN_DEBUG "assert triggered: %s, %d\n",__FILE__,__LINE__);WARN_ON(1);}

#define LINUX_SWITCH_DEFAULTCASE_ASSERT \
	    {printk(KERN_DEBUG "Undefined case value in switch statement: %s, %d\n",__FILE__,__LINE__);WARN_ON(1);} 

#define PDEBUG(fmt,args...) printk(KERN_DEBUG "%s:"fmt,DRIVER_NAME,##args)
#define PERR(fmt,args...) printk(KERN_ERR "%s:"fmt,DRIVER_NAME, ##args)
#define PWARN(fmt,args...) printk(KERN_WARNING "%s:"fmt,DRIVER_NAME, ##args)
#define PNOTICE(fmt,args...) printk(KERN_NOTICE "%s:"fmt,DRIVER_NAME, ##args)
#define PINFO(fmt,args...) printk(KERN_INFO "%s:"fmt,DRIVER_NAME, ##args)

//*** This macro is used in the assert macro Ex: ASSERT(VALID_KERNEL_ADDRESS(pX));
//*
#ifdef _X86_ //* 32-bit world
#else //* Currently treating all 64-bit platforms the same
#endif	

#ifndef max
#define max(_a, _b)     (((_a) > (_b)) ? (_a) : (_b))
#endif

//Operation-function definitions .................................................................
//
#ifdef  REAL_MODE // Not SIMULATION_MODE ... Buckle-up - we're in uncharted waters
//
#define SIMULATION_ASSERT(x)
//
#else //SIMULATION_MODE
//
    #define SIMULATION_ASSERT(x)  ASSERT(x) //Used as a sanity check to verify things known about the simulation
//
#endif //SIMULATION_MODE
//

//Power management profile ......................................
//
#undef MAD_POWER_MANAGEMENT_ENABLED
#define MAD_DEVICE_CONTROL_POWER_MNGT
//
#ifdef MAD_HOST_CONTROL_POWER_MNGT //HIPM in SATA-speak
    #ifdef MAD_DEVICE_CONTROL_POWER_MNGT //Cause the compilation to abort here 
    #error Define error. MAD_HOST_CONTROL_POWER_MNGT & MAD_DEVICE_CONTROL_POWER_MNGT are mutually exclusive *!*
    #endif 

    #define MAD_IDLE_TIMEOUT    180000; // 3 minute idle timeout in millisecs
    #define MAD_POWER_MANAGEMENT_ENABLED
#endif //MAD_HOST_CONTROL_POWER_MNGT
//
#ifdef MAD_DEVICE_CONTROL_POWER_MNGT  //DIPM in SATA-speak
    #define MAD_DFLT_POWER_CHECK_DELAY  3000 //Millisecs
    #define MAD_POWER_MANAGEMENT_ENABLED   
#endif 

//WMI statistics-related constants
#define    MAD_XFER_UNIT_SIZE               1024
#define	   MAD_MILLIWATTS_PER_XFER_UNIT      250
#define    MAD_DMA_MILLIWATTS_PER_XFER_UNIT  500 
#define    MAD_MILLIWATTS_PER_IDLE_SECOND    100

//#define IO_TIMER
#ifdef IO_TIMER
    #define READTIMEOUT       30 //* Seconds
    #define WRITETIMEOUT      30 //* Seconds
    #define PULSETIMEOUT      30 //* Seconds
#endif

//*** Device property definitions ................................................................
//
//Assuming the host must eXclusive-OR Status & IntID register bits on real hardware
//That is the host can't set these bits to one - bits are reset to zero by writing one (X-ORing)
//
#define XOR_REGBITS_SET_BY_DEVICE 

//Configuration Mngt parameters
//
#define MAD_DFLT_DEVICE_IRQL        9
#define MAD_DFLT_IDT_BASE_INDX      0x40 
#define MAD_DFLT_AFFINITY_ON        1 
#define MAD_DFLT_NUM_FILTERS        0
#define MAD_DFLT_DMA_ENABLED        FALSE 
#define MADDEV_LOW_PHYS_ADDR        {0, 0}
#define MADDEV_HIGH_PHYS_ADDR       {0xFFFF0000, 0} //Highest 32-bit page
#define MAD_MAX_DEVICES             9  // 1..9 = SerialNo: (One-digit name suffix may need enhancement)
#define MADDEV_TOTL_CM_RESOURCES    4  // Device registers memory(IoPort), Int Parms, PIO read memory, PIO write memory  

//* Device Memory extents & configuration 
#ifdef SECTOR_SHIFT //Linux block-dev defined                
       #define MAD_SECTOR_SHIFT     SECTOR_SHIFT
#else
       #define MAD_SECTOR_SHIFT     9 //512 bytes
#endif

#define MAD_SIZEOF_REGISTER_BLOCK    sizeof(MADREGS)
#define MAD_SECTOR_SIZE              (1 << MAD_SECTOR_SHIFT)
#define MAD_SECTORS_PER_PAGE         (LINUX_PAGE_SIZE / MAD_SECTOR_SIZE)
#define MAD_BLOCK_SIZE               MAD_SECTOR_SIZE  

#define MAD_KMALLOC_BYTES_PAGE_ORDER  9  //What we can get w/ kmalloc...  2MB
#define MAD_ALLOC_PAGES_ORDER         10 //What we can get w/ alloc_pages... 4MB  
                                         
//Contiguous Memory Allocator: requires a kernel build enabling CMA - see the readme.txt
//#define MAD_DMA_CMA_ALLOC_PAGES_ORDER 11 //Contiguous Memory Allocator... 8MB+ 
//
//Rebuild EVERYTHING when changing this parameter... the test apps must mmap & munmap properly
#define MAD_XALLOC_PAGES_ORDER       MAD_KMALLOC_BYTES_PAGE_ORDER
//MAD_ALLOC_PAGES_ORDER*/ MAD_DMA_CMA_ALLOC_PAGES_ORDER  
#define MAD_DEVICE_MAX_PAGES         (1 << MAD_XALLOC_PAGES_ORDER)
#define MAD_DEVICE_MAX_SECTORS       (MAD_DEVICE_MAX_PAGES * MAD_SECTORS_PER_PAGE)
#define MAD_TOTAL_ALLOC_SIZE         (MAD_SECTOR_SIZE * MAD_DEVICE_MAX_SECTORS)
#define MAD_SAFE_MMAP_SIZE           (1 << 16)
//
#define MAD_CACHE_NUM_SECTORS        8
#define MAD_CACHE_SIZE_BYTES         (MAD_CACHE_NUM_SECTORS * MAD_SECTOR_SIZE)
#define MAD_CACHE_NUM_PAGES          (MAD_CACHE_NUM_SECTORS / MAD_SECTORS_PER_PAGE) 
//
#define MAD_REGISTER_BLOCK_SIZE      MAD_SECTOR_SIZE
#define MAD_MAPD_READ_OFFSET         MAD_REGISTER_BLOCK_SIZE //MAPD as in mapped
#define MAD_MAPD_READ_SIZE           (MAD_SECTOR_SIZE * 1)  // * N sectors if useful
#define MAD_MAPD_WRITE_OFFSET        (MAD_MAPD_READ_OFFSET + MAD_MAPD_READ_SIZE) 
#define MAD_MAPD_WRITE_SIZE          MAD_MAPD_READ_SIZE  
//
#define MAD_CACHE_READ_OFFSET        MAD_MAPD_READ_OFFSET
#define MAD_CACHE_WRITE_OFFSET       MAD_MAPD_WRITE_OFFSET
//
#define MAD_DEVICE_DATA_OFFSET       \
        (MAD_MAPD_WRITE_OFFSET + MAD_MAPD_WRITE_SIZE + MAD_SECTOR_SIZE)
//
#define MAD_DEVICE_MEM_SIZE_NODATA   MAD_DEVICE_DATA_OFFSET
//
#define MAD_DEVICE_DATA_SIZE        \
        (MAD_TOTAL_ALLOC_SIZE - MAD_DEVICE_DATA_OFFSET)
#define MAD_DEVICE_DATA_SECTORS     (MAD_DEVICE_DATA_SIZE / MAD_SECTOR_SIZE)

#define MAD_DEVICE_MAP_MEM_SIZE     MAD_TOTAL_ALLOC_SIZE
//
#define MAD_64KB_ALIGNMENT            0x10000
#define MAD_64KB_ALIGN_MASK           0xFFFFFFFF0000

//Device I-O count & size defines  
#define MAD_SGDMA_MAX_SECTORS        256 
#define MAD_SGDMA_MAX_BYTES          (MAD_SGDMA_MAX_SECTORS * MAD_SECTOR_SIZE)
#define MAD_SGDMA_MAX_PAGES          (MAD_SGDMA_MAX_SECTORS / MAD_SECTORS_PER_PAGE)

#define MAD_UNITIO_SIZE_BYTES        16
#define MAD_BYTE_COUNT_MULT          MAD_UNITIO_SIZE_BYTES
#define MAD_CONTROL_REG_COUNT_BITS   4
#define MAD_BUFRD_IO_COUNT_BITS      MAD_CONTROL_REG_COUNT_BITS
#define MAD_BUFRD_IO_MAX_SIZE        (MAD_UNITIO_SIZE_BYTES * (1 << MAD_BUFRD_IO_COUNT_BITS))
#define MAD_DIRECT_XFER_MAX_SECTORS  (1 << MAD_CONTROL_REG_COUNT_BITS)
#define MAD_DIRECT_XFER_MAX_PAGES    (MAD_DIRECT_XFER_MAX_SECTORS / MAD_SECTORS_PER_PAGE)  
#define MAD_ONE_BLOCK_DMA_MAX_PAGES  MAD_DIRECT_XFER_MAX_PAGES
//
#define MAD_MODULO_ADJUST(X, alignment)       ((X/alignment) * alignment)

// The larger our device extent the coarser the granularity of the cache alignment/indexing
// given that we have N bits (4) for offseting/indexing
//
#define MAD_CACHE_OFFSET_BITS        4
#define MAD_CACHE_OFFSET_RANGE       (1 << MAD_CACHE_OFFSET_BITS) // 2 ^ CACHE_OFFSET_BITS
#define MAD_CACHE_GRANULARITY        (MAD_DEVICE_MAX_SECTORS / MAD_CACHE_OFFSET_RANGE)
#define MAD_CACHE_XFER_MAX_SIZE      (MAD_SECTOR_SIZE * MAD_CACHE_OFFSET_RANGE)
#define MAD_CACHE_XFER_MAX_PAGES     (MAD_CACHE_XFER_MAX_SIZE / LINUX_PAGE_SIZE)

//*** Device registers & register mask values ....................................................
//
// Using the Universe_II:Tundra controller chip for the VmeBus as the model for our DMA controller
//
typedef struct _MAD_DMA_CHAIN_ELEMENT
        {
		ULONG64   HostAddr;     //* Physical address of the host data
		ULONG     DevLoclAddr;  //* Device-relative address for data xfers
        ULONG     DmaCntl;      //* DMA Control 
        ULONG     DXBC;         //* DMA Transfer Byte Count
        ULONG64   CDPP;         //* Chained-DMA Packet Pointer to the next element
        } __attribute__((aligned(8),packed))
        MAD_DMA_CHAIN_ELEMENT,  *PMAD_DMA_CHAIN_ELEMENT;

#ifdef __cplusplus //Because C won't allow this
typedef struct _EMPTY_STRUCT {} EmptyStruct;
#endif

//DMA Controller register mask defines
//
#define MAD_DMA_CNTL_INIT        0x00440044 //Address space & address alignment bits (arbitrary - not really used)
#define MAD_DMA_CNTL_H2D         0x80000000 //Indicates host to device (dma-write)
//
#define MAD_DMA_CDPP_END         0x00000001 //Chained-DMA End-of-list

//*** Define the device registers in memory  
//*
typedef struct _MADREGS
    {
    ULONG    MesgID;
    ULONG    Control;
    ULONG    Status;
    ULONG    IntEnable;
    ULONG    IntID;
    ULONG    IoTag;
    //
    ULONG    PioCacheReadLen;    
	ULONG    PioCacheWriteLen;   
    ULONG    CacheIndxRd;   //Which sector
    ULONG    CacheIndxWr;   //Which sector
    //
    ULONG    ByteIndxRd;
    ULONG    ByteIndxWr;
    //  
    ULONG    PowerState;
    ULONG    Devnum;

// Here we have 64-bit alignment because we have an even # of 32-bit regs above (12)
// (Assuming pagesize(+) alignment of the whole memory allocation)
//  
// This aggregate of registers must match the chain element above so that we can do
// a bulk transfer of registers across the bus. 
// This is NOT workable if the real device register layout doesn't cooperate  
//
	#ifdef __cplusplus
	EmptyStruct      OneBlockDmaRegs; //This has the same offset as the following registers
    #endif
	// 
    ULONG64          HostPA;          //* Physical address of the host data
    ULONG64          DevLoclAddr;     //* Local (system/pci) adddress
    ULONG            DmaCntl;         //* DMA Control-Status
    ULONG            DTBC;            //* DMA Xfer byte count
    ULONG64          BCDPP;           //* Base Chained-DMA Pkt Pntr
    //MAD_DMA_CHAIN_ELEMENT SgDmaBaseElement;
    }
    MADREGS, *PMADREGS;

// Register offsets 	
//*
#define MADREG_MESG_ID        offsetof(MADREGS, MesgId)
#define MADREG_CNTL           offsetof(MADREGS, Control)
#define MADREG_CMD            offsetof(MADREGS, Control)
#define MADREG_CNTL1          offsetof(MADREGS, Control)
#define MADREG_CMD1           offsetof(MADREGS, Control)
#define MADREG_CNTL2          (offsetof(MADREGS, Control)+1)
#define MADREG_CMD2           (offsetof(MADREGS, Control)+1)
#define MADREG_STAT           offsetof(MADREGS, Status)
#define MADREG_STAT1          offsetof(MADREGS, Status)
#define MADREG_STAT2          (offsetof(MADREGS, Status)+1)
#define MADREG_INT_ACTV       offsetof(MADREGS, IntEnable)
#define MADREG_INT_ACTV1      offsetof(MADREGS, IntEnable) 
#define MADREG_INT_ACTV2      (offsetof(MADREGS, IntEnable)+1) 
#define MADREG_INT_ID         offsetof(MADREGS, IntId) 
#define MADREG_INT_ID1        offsetof(MADREGS, IntId) 
#define MADREG_INT_ID2        (offsetof(MADREGS, IntId)+1) 
//
#define MADREG_PIO_READ_LEN    offsetof(MADREGS, PioCacheReadLen)
#define MADREG_PIO_READ_LEN1   offsetof(MADREGS, PioCacheReadLen)
#define MADREG_PIO_READ_LEN2   (offsetof(MADREGS, PioCacheReadLen)+1)
#define MADREG_PIO_WRITE_LEN   offsetof(MADREGS, PioCacheWriteLen)
#define MADREG_PIO_WRITE_LEN1  offsetof(MADREGS, PioCacheWriteLen)
#define MADREG_PIO_WRITE_LEN2  (offsetof(MADREGS, PioCacheWriteLen)+1)
#define MADREG_POWER_STATE     offsetof(MADREGS, PowerState)

//* REGISTER MASK BITS 
//*
#define MAD_INT_BUFRD_INPUT_BIT          0x00000001
#define MAD_INT_DMA_INPUT_BIT            0x00000002  //Enable-indicate DMA input
#define MAD_INT_ALIGN_INPUT_BIT          0x00000008  //Enable-indicate read cache alignment
#define MAD_INT_INPUT_MASK        \
        (MAD_INT_BUFRD_INPUT_BIT | MAD_INT_DMA_INPUT_BIT | MAD_INT_ALIGN_INPUT_BIT) 
//
#define MAD_INT_BUFRD_OUTPUT_BIT         0x00000010  //Enable-indicate buffered output
#define MAD_INT_DMA_OUTPUT_BIT           0x00000020  //Enable-indicate DMA ioutput
#define MAD_INT_ALIGN_OUTPUT_BIT         0x00000080  //Enable-indicate write cache alignment
#define MAD_INT_OUTPUT_MASK        \
        (MAD_INT_BUFRD_OUTPUT_BIT | MAD_INT_DMA_OUTPUT_BIT | MAD_INT_ALIGN_OUTPUT_BIT) 

#define MAD_INT_STATUS_ALERT_BIT         0x00008000  // The "Oh S**t!" indicator
#define MAD_INT_ALL_VALID_MASK           \
        (MAD_INT_STATUS_ALERT_BIT | MAD_INT_INPUT_MASK | MAD_INT_OUTPUT_MASK)
#define MAD_INT_ALL_INVALID_MASK         ~MAD_INT_ALL_VALID_MASK

//Any / all undefined int conditions
#define MAD_INT_INVALID_BYTEMODE_MASK    \
        (MAD_INT_ALL_INVALID_MASK | MAD_INT_DMA_INPUT_BIT | MAD_INT_DMA_OUTPUT_BIT)

#define MAD_ALL_INTS_ENABLED_MASK        \
        (MAD_INT_INPUT_MASK | MAD_INT_OUTPUT_MASK | MAD_INT_STATUS_ALERT_BIT)
#define MAD_ALL_INTS_DISABLED            0x00000000
#define MAD_ALL_INTS_CLEARED             MAD_ALL_INTS_DISABLED
//
#define MAD_STATUS_NO_ERROR_MASK         0x00000000
#define MAD_STATUS_GENERAL_ERR_BIT       0x00000001
#define MAD_STATUS_OVER_UNDER_ERR_BIT    0x00000002  //Status 
#define MAD_STATUS_DEVICE_BUSY_BIT       0x00000004  //Status 
#define MAD_STATUS_DEVICE_FAILURE_BIT    0x00000008  //Status
#define MAD_STATUS_INVALID_IO_BIT        0x00000010  //Status ... not visible in the UI
#define MAD_STATUS_RESOURCE_ERROR_BIT    0x00000020  //Status ... not visible in the UI
#define MAD_STATUS_BUS_ERROR_BIT         MAD_STATUS_RESOURCE_ERROR_BIT
//
#define MAD_STATUS_LO_ERROR_MASK         \
(MAD_STATUS_GENERAL_ERR_BIT | MAD_STATUS_OVER_UNDER_ERR_BIT | MAD_STATUS_DEVICE_BUSY_BIT)
#define MAD_STATUS_HI_ERROR_MASK         \
        (MAD_STATUS_DEVICE_FAILURE_BIT | MAD_STATUS_INVALID_IO_BIT | MAD_STATUS_RESOURCE_ERROR_BIT)
#define MAD_STATUS_ERROR_MASK            \
        (MAD_STATUS_LO_ERROR_MASK | MAD_STATUS_HI_ERROR_MASK)
//
#define MAD_STATUS_READ_CACHE_MT_BIT     0x00000040
#define MAD_STATUS_WRITE_CACHE_MT_BIT    0x00000080
#define MAD_STATUS_CACHE_INIT_MASK       \
        (MAD_STATUS_READ_CACHE_MT_BIT | MAD_STATUS_WRITE_CACHE_MT_BIT)
//#define MAD_STATUS_TIMEOUT_ERROR_BIT     0x00000800  //Status ... not visible in the UI
//
#define MAD_STATUS_READ_COUNT_MASK       0x00000F00  //Count of completed input - up to 16
#define MAD_STATUS_READ_COUNT_SHIFT               8
//
#define MAD_STATUS_WRITE_COUNT_MASK      0x0000F000  //Count of completed output - up to 16
#define MAD_STATUS_WRITE_COUNT_SHIFT             12
#define MAD_STATUS_RW_COUNT_MASK         \
        (MAD_STATUS_READ_COUNT_MASK | MAD_STATUS_WRITE_COUNT_MASK)
#define MAD_STATUS_DEAD_DEVICE_MASK      (~MAD_STATUS_RW_COUNT_MASK)
//
#define MAD_CONTROL_IOSIZE_BYTES_BIT     0x00000001 //Control: Io count(Size) is in Bytes not sectors
#define MAD_CONTROL_BUFRD_GO_BIT         0x00000008 //Control: BUFRD Go bit
//
#define MAD_CONTROL_CACHE_XFER_BIT       0x00000010 //Control: Buffered I/O through the R/W cache
#define MAD_CONTROL_CHAINED_DMA_BIT      0x00000040 //Control: DMA is chained
#define MAD_CONTROL_DMA_GO_BIT           0x00000080 //Control: DMA Go bit
//
#define MAD_CONTROL_RESET_STATE          0x00000000
//
#define MAD_CONTROL_IO_OFFSET_MASK       0x00000F00
#define MAD_CONTROL_IO_ALIGN_MASK        MAD_CONTROL_IO_OFFSET_MASK
#define MAD_CONTROL_IO_OFFSET_SHIFT               8
#define MAD_CONTROL_IO_ALIGN_SHIFT       MAD_CONTROL_IO_OFFSET_SHIFT
//
#define MAD_CONTROL_IO_COUNT_MASK        0x0000F000
#define MAD_CONTROL_IO_COUNT_SHIFT               12
//
#define REGISTER_MASK_ALL_BITS_HIGH      0xFFFFFFFF
//
// The generic mask bit defines for the Simulator-UI 
//
#define MADMASK_BIT0             0x0001
#define MADMASK_BIT1             0x0002
#define MADMASK_BIT2             0x0004
#define MADMASK_BIT3             0x0008
#define MADMASK_BIT4             0x0010
#define MADMASK_BIT5             0x0020
#define MADMASK_BIT6             0x0040
#define MADMASK_BIT7             0x0080
//
#define MADMASK_BIT8             0x0100
#define MADMASK_BIT9             0x0200
#define MADMASK_BIT10            0x0400
#define MADMASK_BIT11            0x0800
#define MADMASK_BIT12            0x1000
#define MADMASK_BIT13            0x2000
#define MADMASK_BIT14            0x4000
#define MADMASK_BIT15            0x8000
//

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

/**
 * struct bus_type - The bus type of the device
 *
 * @name:   The name of the bus.
 * @dev_name:   Used for subsystems to enumerate devices like ("foo%u", dev->id).
 * @dev_root:   Default device to use as the parent.
 * @bus_groups: Default attributes of the bus.
 * @dev_groups: Default attributes of the devices on the bus.
 * @drv_groups: Default attributes of the device drivers on the bus.
 * @match:  Called, perhaps multiple times, whenever a new device or driver
 *      is added for this bus. It should return a positive value if the
 *      given device can be handled by the given driver and zero
 *      otherwise. It may also return error code if determining that
 *      the driver supports the device is not possible. In case of
 *      -EPROBE_DEFER it will queue the device for deferred probing.
 * @uevent: Called when a device is added, removed, or a few other things
 *      that generate uevents to add the environment variables.
 * @probe:  Called when a new device or driver add to this bus, and callback
 *      the specific driver's probe to initial the matched device.
 * @sync_state: Called to sync device state to software state after all the
 *      state tracking consumers linked to this device (present at
 *      the time of late_initcall) have successfully bound to a
 *      driver. If the device has no consumers, this function will
 *      be called at late_initcall_sync level. If the device has
 *      consumers that are never bound to a driver, this function
 *      will never get called until they do.
 * @remove: Called when a device removed from this bus.
 * @shutdown:   Called at shut-down time to quiesce the device.
 *
 * @online: Called to put the device back online (after offlining it).
 * @offline:    Called to put the device offline for hot-removal. May fail.
 *
 * @suspend:    Called when a device on this bus wants to go to sleep mode.
 * @resume: Called to bring a device on this bus out of sleep mode.
 * @num_vf: Called to find out how many virtual functions a device on this
 *      bus supports.
 * @dma_configure:  Called to setup DMA configuration on a device on
 *          this bus.
 * @pm:     Power management operations of this bus, callback the specific
 *      device driver's pm-ops.
 * @iommu_ops:  IOMMU specific operations for this bus, used to attach IOMMU
 *              driver implementations to a bus and allow the driver to do
 *              bus-specific setup
 * @p:      The private data of the driver core, only the driver core can
 *      touch this.
 * @lock_key:   Lock class key for use by the lock validator
 * @need_parent_lock:   When probing or removing a device on this bus, the
 *          device core should lock the device's parent.
 *
 * A bus is a channel between the processor and one or more devices. For the
 * purposes of the device model, all devices are connected via a bus, even if
 * it is an internal, virtual, "platform" bus. Buses can plug into each other.
 * A USB controller is usually a PCI device, for example. The device model
 * represents the actual connections between buses and the devices they control.
 * A bus is represented by the bus_type structure. It contains the name, the
 * default attributes, the bus' methods, PM operations, and the driver core's
 * private data.
 */
struct bus_type {
    const char      *name;
    const char      *dev_name;
    struct device       *dev_root;
    const struct attribute_group **bus_groups;
    const struct attribute_group **dev_groups;
    const struct attribute_group **drv_groups;

    int (*match)(struct device *dev, struct device_driver *drv);
    int (*uevent)(struct device *dev, struct kobj_uevent_env *env);
    int (*probe)(struct device *dev);
    void (*sync_state)(struct device *dev);
    int (*remove)(struct device *dev);
    void (*shutdown)(struct device *dev);

    int (*online)(struct device *dev);
    int (*offline)(struct device *dev);

    int (*suspend)(struct device *dev, pm_message_t state);
    int (*resume)(struct device *dev);

    int (*num_vf)(struct device *dev);

    int (*dma_configure)(struct device *dev);

    const struct dev_pm_ops *pm;

    const struct iommu_ops *iommu_ops;

    struct subsys_private *p;
    struct lock_class_key lock_key;

    bool need_parent_lock;
};


#endif //_MADDEFS_
