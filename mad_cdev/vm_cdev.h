#ifndef _VM_CDEV_H_
#define _VM_CDEV_H_

// GMEM API USAGE:
#include <vm/gmem.h>
#include <vm/gmem_dev.h>
#include <vm/gmem_uvas.h>

#include <vm/vm_map.h>
// Save our time of implementing an x86-comptaible page walker
#include <vm/pmap.h>

// emulator code
int run_kernel(kernel_instance kernel_type, void *args)

// The code below is shared
dev_pmap_t *pmap; // This is the current pmap
gmem_vm_mode mode;
int dev_faults;
void dev_fault_trap(dev_pmap_t *pmap, void *va);
int setup_ctx(device_t device, gmem_vm_mode running_mode);

// The code below is specific for x97 mmu
vmem_t *pm_pool;
vm_page_t first_x97_page, last_x97_page;
uint64_t x97_address_translate(dev_pmap_t *pmap, vm_offset_t va);
int init_pm(struct gmem_mmu_ops *ops);
vm_page_t alloc_pm(void *);
gmem_error_t free_pm(vm_page_t m);
void zero_page(vm_page_t m);

#define X97_PT_LOCK(x)    mtx_lock((x)->lock)
#define X97_PT_UNLOCK(x)  mtx_unlock((x)->lock)
struct x97_page_table {
    struct mtx lock;
    vm_page_t pgroot;
};

#define VA_MASK 0xffffffffffff

// x97 MMU supports translating 48-bit VA to a 48-bit PA
// It has 3-levels:
// Level 0: 18-bit (An array of 64 * 8 * 4KB pages)
// Level 1: 9-bit (A 4KB page)
// Level 2: 9-bit (A 4KB page)

// VA: [ 18 bit ] [ 9 bits ] [ 9 bits ] [ 12 bits ]
#define PT_LEVEL_0 64 * 8
#define get_lvl_index(x, lvl) (x >> (9 * (2 - lvl) + 12))
// #define PAGE_MASK 0xffff
#define LVL_MASK 0x1ff

#endif