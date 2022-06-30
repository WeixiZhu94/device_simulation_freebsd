#include "maddevc.h"        /* local definitions */
#include "vm_cdev.h"

/* Device MMU emulation code */
// emulate a hardware memory access
// VA must be translated by device pmap, then we obtain the PA. Calculation is based on DMAP.
static void* address_translate(void *va)
{
    vm_paddr_t pa = 0;

    // No TLB emulation, fall through to page walk
    if (mode == SHARE_CPU) {
        pa = pmap_extract(((vm_map_t) pmap->data)->pmap, (uintptr_t) va);
        if (pa == 0) {
            dev_fault_trap(pmap, va);
            pa = pmap_extract(((vm_map_t) pmap->data)->pmap, (uintptr_t) va);
            if (pa == 0) {
                printf("[gmem uvas fault] gives me 0 pa after faulting...\n");
                return 0;
            }
        }
    }
    else if (mode == EXCLUSIVE) {
        pa = x97_address_translate(pmap, va);
        if (pa == 0) {
            dev_fault_trap(pmap, va);
            pa = x97_address_translate(pmap, va);
            if (pa == 0) {
                printf("[gmem uvas fault] gives me 0 pa after faulting...\n");
                return 0;
            }
        }
    } else
        printf("Other modes unimplemented\n");
    return (void *) PHYS_TO_DMAP(pa);
}

/* openCL kernel emulation code */
static void vector_add(uint64_t *a, uint64_t *b, uint64_t *c, uint64_t len)
{
    uint64_t *ka, *kb, *kc;
    int delta_faults = dev_faults;
    // depending on your mode, a, b, c could be different pa values.
    for (uint64_t i = 0; i < len; i ++) // Let's assume this for-loop is paralleled by many accelerator cores
    {
        ka = address_translate(&a[i]);
        kb = address_translate(&b[i]);
        kc = address_translate(&c[i]);
        if (ka == 0 || kb == 0 || kc == 0) {
            printf("[devc] kernel failed with 0 pa, %p %p %p %p %p %p\n",
                a, b, c, &a[i], &b[i], &c[i]);
            return;
        }

        *kc = *ka + *kb;
    }
    delta_faults = dev_faults - delta_faults;
    printf("[devc] kernel computation generates %d device page faults\n", dev_faults);
}

int run_kernel(void *arg)
{
    struct accelerator_kernel_args *kernel_launch_args;
    kernel_instance kernel_type; 
    struct vector_add_args *args;

    kernel_launch_args = arg;
    // copyin(arg, &kernel_launch_args, sizeof(struct accelerator_kernel_args));
    kernel_type = kernel_launch_args->kernel_type;
    args = &kernel_launch_args->vector_add;

    printf("[devc] running kernel, karg %p, type %u, args %p\n", arg, kernel_type, args);
    // Do we need to translate user-space va to kernel space va?
    if (kernel_type == SUM) {
        printf("[devc] simulating kernel for vector add, a %p, b %p, c %p, len %lu\n", 
            args->a, args->b, args->c, args->len);
        vector_add(args->a, args->b, args->c, args->len);
        return 0;
    }
    else
        printf("[devc] other kernels not implemented\n");
    return -1;
}