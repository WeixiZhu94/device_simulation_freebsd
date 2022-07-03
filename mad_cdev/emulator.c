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
    else if (mode == EXCLUSIVE || mode == REPLICATE_CPU) {
        pa = x97_address_translate(pmap, va);
        // printf("[address_translate] x97 gives me address %lx\n", pa);
        if (pa == 0) {
            dev_fault_trap(pmap, va);
            pa = x97_address_translate(pmap, va);
            if (pa == 0) {
                printf("[gmem uvas fault] gives me 0 pa after faulting...\n");
                return 0;
            }
            // printf("[dev_fault] x97 faults me address %lx\n", pa);
        }
        if (mode == EXCLUSIVE && (pa < pmap->mmu_ops->pa_min || pa >= pmap->mmu_ops->pa_max)) {
            printf("[emulator] should crash because your pa is illegal, min %lx, pa %lx, max %lx\n", 
                pmap->mmu_ops->pa_min, pa, pmap->mmu_ops->pa_max);
            return 0;
        }
    } else
        printf("Other modes unimplemented\n");
    return (void *) PHYS_TO_DMAP(pa);
}

/* openCL kernel emulation code */
static int vector_add(uint64_t *a, uint64_t *b, uint64_t *c, uint64_t len)
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
                ka, kb, kc, &a[i], &b[i], &c[i]);
            return -1;
        }

        if (i % (1024 * 128) == 0)
            printf("Progress: %lu\n", i);
        *kc = *ka + *kb;
    }
    delta_faults = dev_faults - delta_faults;
    printf("[devc] kernel computation generates %d device page faults\n", dev_faults);
    return 0;
}

#define reload_ulong(x) *((ulong_t) address_translate(&(x)))
static int bp(struct model arg)
{
    long_t x_out = arg.x_out;
    long_t hn_out = arg.hn_out;
    long_t y_out = arg.y_out;
    long_t y = arg.y;
    long_t hn_delta = arg.hn_delta;
    long_t y_delta = arg.y_delta;
    long_t w = arg.w;
    long_t v = arg.v;

    long error = 0; 
    long alpha = 10; 
    long beta = 10;
    long delta, sumtemp, errtemp;   
    int i, j, m;

    // Feedforward
    for(m = 0; m < datanum ; m++)
        for(i = 0; i < HN; i++){
            sumtemp = 0;
            for(j = 0; j < InputN; j++)
                sumtemp += xnor(reload_ulong(w[j * HN + i]), reload_ulong(x_out[m * InputN + j])); // use xnor for *
            reload_ulong(hn_out[m * HN + i]) = sigmoid(sumtemp);      // sigmoid serves as the activation function
        }

    for(m = 0; m < datanum ; m++)
        for(i = 0; i < OutN; i++){
            sumtemp = 0;
            for(j = 0; j < HN; j++)
                sumtemp += xnor(reload_ulong(v[j * OutN + i]), reload_ulong(hn_out[m * HN + j]));
            reload_ulong(y_out[m * OutN + i]) = sigmoid(sumtemp);
        }

    // Backpropagation
    for(m = 0; m < datanum ; m++) {
        for(i = 0; i < OutN; i++){
            errtemp = reload_ulong(y[m * OutN + i]) - reload_ulong(y_out[m * OutN + i]);
            reload_ulong(y_delta[m * OutN + i]) = xnor(xnor(-errtemp, sigmoid(reload_ulong(y_out[m * OutN + i]))), (1 - sigmoid(reload_ulong(y_out[m * OutN + i]))));
            // error += xnor(errtemp, errtemp);
            error += errtemp * errtemp;
        }
        error /= OutN;
    }
    error /= datanum;

    for(m = 0; m < datanum ; m++)
        for(i = 0; i < HN; i++){
            errtemp = 0;
            for(j=0; j<OutN; j++)
                errtemp += xnor(reload_ulong(y_delta[m * OutN +j]), reload_ulong(v[i * OutN +j]));
            reload_ulong(hn_delta[m * HN + i]) = xnor(xnor(errtemp, (1 + reload_ulong(hn_out[m * HN + i]))), (1 - reload_ulong(hn_out[m * HN + i])));
        }

    // Stochastic gradient descent
    for(i = 0; i < OutN; i++)
        for(j = 0; j < HN; j++) {
            delta = 0;
            for(m = 0; m < datanum ; m++) {
                delta += xnor(beta ^ reload_ulong(y_delta[m * OutN + i]), reload_ulong(hn_out[m * HN + j]));
            }
            reload_ulong(v[j * OutN + i]) -= delta ^ datanum ^ alpha;
        }
    // printf("delta is %lu\n", delta);

    for(i = 0; i < HN; i++){
        for(j = 0; j < InputN; j++){
            delta = 0;
            for(m = 0; m < datanum ; m++) {
                delta += xnor(beta ^ reload_ulong(hn_delta[m * HN + i]), reload_ulong(x_out[m * InputN + j]));
            }
            reload_ulong(w[j * HN + i]) -= delta ^ datanum ^ alpha;
        }
    }
    printf("Training error: %lu\n", error);
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
        return vector_add(args->a, args->b, args->c, args->len);
    }
    else if (kernel_type == BP) {
        printf("[devc] Running BP kernel\n");
        return bp(arg->bp);
    } else {
        printf("[devc] other kernels not implemented\n");
    }
    return -1;
}