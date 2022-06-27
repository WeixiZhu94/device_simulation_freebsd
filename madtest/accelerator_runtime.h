#ifndef  _ACCELERATOR_RUNTIME_H_
#define  _ACCELERATOR_RUNTIME_H_
//
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <fstream>
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <assert.h>

typedef signed char s8;
typedef unsigned char u8;

typedef signed short s16;
typedef unsigned short u16;

typedef signed int s32;
typedef unsigned int u32;

typedef signed long long s64;
typedef unsigned long long u64;

//
#include "maddefs.h"
#include "madapplib.h"
#include "madkonsts.h"
#include "maddevioctls.h"


// !!! Must keep it synchronized with gmem.h, otherwise bugs will go wild
enum gmem_vm_mode {
	UNIQUE = 0,
	REPLICATE,
	SHARE,
	REPLICATE_CPU,
	SHARE_CPU
};
typedef enum gmem_vm_mode gmem_vm_mode;

#ifdef BIO //Block-mode device
#define  MADDEVNAME      "maddevb_objX"
#else //Char-mode device
#define  MADDEVNAME      "maddevcx"
#endif
#define  MADDEVNUMDX     7 //.......^

typedef PMADREGS* PPMADREGS; //just an env test

#define nudef kSA
//

#define kPRC  15
#define kPWC  16
#define kARC  17
#define kAWC  18
//
#define kMGT  19
#define kPR   20
#define kPW   21
#define kRB   22
#define kWB   23
#define kRBA  24
#define kWBA  25
#define kRBQ  26
#define kWBQ  27
#define kRDI  28
#define kWDI  29
#define kRDR  30
#define kWRR  31

// library-internal calls
bool Parse_Cmd(int argc, char **argv,
   		int* devnum, int* op, long* val, long *offset, void* *parm);

int Process_Cmd(int fd, int op, long val, long offset, void* parm);

void display_error_w_help(char* pChar);
void display_help();
int GetBufr(char** ppBufr, size_t IoSize);
void InitData(char* pBufr, size_t Len);
void DisplayData(char* pBufr, size_t Len);
int MapDeviceRegs(PMADREGS *ppMadRegs, int fd);
void DisplayDevRegs(PMADREGS pMadRegs);
ssize_t Async_Io(int fd, u8* pBufr, size_t DataLen, size_t offset, u8 bWrite);
ssize_t Queued_Io(int fd, u8* pBufr, size_t DataLen, u8 bWrite);

// Library APIs (something like openCL)
void clContextCreate(gmem_vm_mode mode);
void clLaunchKernel(kernel_instance kernel, void *args);
// void clPrefetch(); // do we need to exercise this? maybe we can try and see how this could be implemented by gmem api.
int generate_test(kernel_instance kernel, void **args);
int validate_test(kernel_instance kernel, void *args);
#endif
