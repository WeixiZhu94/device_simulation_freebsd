#include "accelerator_runtime.h"
#include "madapplib.h"
#include <sys/stat.h>

static char MadDevName[] = MADDEVNAME;
static char MadDevPathName[100] = "";

static MADREGS MadRegs;
static PMADREGS pMapdDevRegs = NULL;

static void* pPIOregn = NULL;
static char* pLargeBufr = NULL;
static u8 RandomBufr[8192];

static int fd;


void clContextCreate(gmem_vm_mode mode)
{
    unsigned long rc;
    int devnum = 1; // By default use device 1
    // Open Device 
    rc = Build_DevName_Open(MadDevName, devnum, MADDEVNUMDX, MadDevPathName, &fd);
    if (fd < 1)
    {
        printf("[accelerator_runtime] failed to open device\n");
        return ;
    }

    // Map registers
    rc = MapDeviceRegsPio(&pMapdDevRegs, fd);
    if (pMapdDevRegs != NULL)
        pPIOregn = ((U8*)pMapdDevRegs + MAD_MAPD_READ_OFFSET);

    
    printf("Issuing ioctl for cmd %lx\n", MADDEVOBJ_IOC_INIT);
    rc = ioctl(fd, MADDEVOBJ_IOC_INIT, NULL);
    if (rc)
        printf("[accelerator runtime] INIT failed\n");

    // printf("Issuing ioctl for cmd %lx, indx = %lu\n", MADDEVOBJ_IOC_SET_READ_INDX, 1);
    // rc = ioctl(fd, MADDEVOBJ_IOC_SET_READ_INDX, 1);
    // if (rc) {
    //     printf("[accelerator runtime] MADDEVOBJ_IOC_SET_READ_INDX failed， rc = %x\n");
    //     exit(-1);
    // }

    printf("Issuing ioctl for cmd %lx, mode = %lu\n", MADDEVOBJ_IOC_CTX_CREATE, (unsigned long) mode);
    rc = ioctl(fd, MADDEVOBJ_IOC_CTX_CREATE, mode);
    if (rc) {
        printf("[accelerator runtime] heterogeneous context creation failed， rc = %lx\n", rc);
        exit(-1);
    }
    printf("Context Creation suceeeded\n");
    return;
}

// Sync launch, no kernel queue implemented
void clLaunchKernel(kernel_instance kernel, void *kernel_args)
{
    int rc;
    // struct accelerator_kernel_args *args = (struct accelerator_kernel_args *) malloc(sizeof(struct accelerator_kernel_args));
    // // struct accelerator_kernel_args args;
    // args->kernel_type = kernel;
    // args->kernel_args = kernel_args;

    printf("Issuing ioctl for cmd %lx, args = %p\n", MADDEVOBJ_IOC_LAUNCH_KERNEL, kernel_args);
    rc = ioctl(fd, MADDEVOBJ_IOC_LAUNCH_KERNEL, kernel_args);
    if (rc) {
        printf("[accelerator runtime] kernel launch failed\n");
        exit(-1);
    }
    return;
}