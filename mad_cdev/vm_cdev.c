#include "vm_cdev.h"
#include <sys/malloc.h>

vmem_t *pm_pool;
vm_page_t first_x97_page, last_x97_page;
dev_pmap_t *pmap; // This is the current pmap
gmem_vm_mode mode;
int dev_faults;

void dev_fault_trap(dev_pmap_t *pmap, void *va) {
    gmem_uvas_fault(pmap, (uintptr_t) va, 8, VM_PROT_READ | VM_PROT_WRITE, NULL);
    dev_faults ++;
}

static inline uint64_t *get_pte(vm_page_t pgroot, vm_offset_t va, int lvl) {
    uint64_t *pde;
    if (lvl == 0) {
        pde = (uint64_t *) PHYS_TO_DMAP(VM_PAGE_TO_PHYS(&pgroot[get_root_index(va)]));
        // printf("[get_pte] %lx %d, pg index %lu, index: %lu, pde: %p\n", va, lvl,
        //     get_root_index(va), get_lvl_index(va, 0), pde);
        return &pde[get_lvl_index(va, 0) & LVL_MASK];
    } else {
        pde = get_pte(pgroot, va, lvl - 1);
        if (*pde == 0) {
            vm_page_t m = alloc_pm(pmap);
            wire_x97_page(m);
            if (m == NULL)
                printf("!!! We are in short of memory when allocating page table...\n");
            *pde = VM_PAGE_TO_PHYS(m) | 0; // no flags. don't care
            // flush device cache.
        }
        pde = (uint64_t *) PHYS_TO_DMAP(*pde);
        // printf("[get_pte] %lx %d, index: %lu, pde: %p\n", va, lvl,
        //     get_lvl_index(va, lvl), pde);
        return &pde[get_lvl_index(va, lvl)];
    }
}

uint64_t x97_address_translate(dev_pmap_t *pmap, void *va) {
    struct x97_page_table *pgtable = (struct x97_page_table *) pmap->data;
    vm_page_t pgroot = pgtable->pgroot;
    // printf("[x97_address_translate] %p\n", va);
    uint64_t *pte = get_pte(pgroot, (vm_offset_t) va, 2);
    // printf("[x97_address_translate] done, page pa: %lx\n", *pte);
    if (*pte != 0) {
        if (*pte < pmap->mmu_ops->pa_min || *pte >= pmap->mmu_ops->pa_max)
            printf("%s %d: translate va %lx - pa %lx\n",__func__, __LINE__, (uintptr_t) va, *pte);
        return *pte | ((uintptr_t) va & ~PAGE_MASK);
    } else
        return 0;
}

static gmem_error_t x97_mmu_init(struct gmem_mmu_ops* ops)
{
    if (atomic_cmpset_int(&ops->inited, 0, 1)) {
        init_pm(ops);
    }
    return GMEM_OK;
}

static gmem_error_t x97_mmu_create(dev_pmap_t *pmap)
{
    unsigned long page_idx;
    struct x97_page_table *pgtable = malloc(sizeof(struct x97_page_table), M_DEVBUF, M_WAITOK | M_ZERO);

    mtx_init(&pgtable->lock, "x97 mmu pg table giant lock", NULL, MTX_DEF);
    if (vmem_xalloc(pm_pool, PT_LEVEL_0, 0, 0, 0, VMEM_ADDR_MIN, VMEM_ADDR_MAX, M_WAITOK | M_BESTFIT, &page_idx))
        printf("!!! x97 failed to initialize page table\n");
    else 
        printf("[x97] mmu_create initialized %d pages starting at index %lu", PT_LEVEL_0, page_idx);
    for (int i = 0; i < PT_LEVEL_0; i ++) {
        pmap_zero_page(&first_x97_page[page_idx + i]);
        activate_x97_page(&first_x97_page[page_idx + i]);
        wire_x97_page(&first_x97_page[page_idx + i]);
    }
    pgtable->pgroot = &first_x97_page[page_idx];

    pmap->data = pgtable;
    return GMEM_OK;
}

static gmem_error_t x97_mmu_destroy(dev_pmap_t *pmap)
{
    struct x97_page_table *pgtable = (struct x97_page_table *) pmap->data;

    vmem_xfree(pm_pool, pgtable->pgroot - first_x97_page, PT_LEVEL_0);
    mtx_destroy(&pgtable->lock);

    free(pgtable, M_DEVBUF);
    return GMEM_OK;
}

static gmem_error_t x97_mmu_enter(dev_pmap_t *pmap, vm_offset_t va, vm_size_t size, 
        vm_paddr_t pa, u_int prot, u_int mem_flags)
{
    struct x97_page_table *pgtable = (struct x97_page_table *) pmap->data;
    uint64_t *pte;
    if (va + size > VA_MASK) {
        printf("!!! VA out of range\n");
        return -1;
    }

    X97_PT_LOCK(pgtable);
    vm_page_t pgroot = pgtable->pgroot;

    for (vm_offset_t va_i = va; va_i < va + size; va_i += PAGE_SIZE, pa += PAGE_SIZE) {
        pte = get_pte(pgroot, va_i, 2);
        *pte = (pa & PAGE_MASK) | 0; // No memory protection flags, don't care.

        // printf("[x97_mmu_enter] maps va %lx to pa %lx, last level pte %p, *pte is %lx\n", 
        //     va_i, pa, pte, *pte);
        if (*pte < pmap->mmu_ops->pa_min || *pte >= pmap->mmu_ops->pa_max)
            printf("[x97_mmu_enter] installed out of range pa %lx\n", *pte);
        // flush device cache
    }

    X97_PT_UNLOCK(pgtable);
    return GMEM_OK;
}

static gmem_error_t x97_mmu_release(dev_pmap_t *pmap, vm_offset_t va, vm_size_t size)
{
    return GMEM_OK;
}

static void x97_mmu_tlb_invl(dev_pmap_t *pmap, vm_offset_t va, vm_size_t size)
{
    return;
}

struct gmem_mmu_ops x97_mmu_ops = {
    .inited = 0,
    .mmu_init               = x97_mmu_init,
    .mmu_pmap_create        = x97_mmu_create,
    .mmu_pmap_destroy       = x97_mmu_destroy,
    .mmu_pmap_enter         = x97_mmu_enter,
    .mmu_pmap_release       = x97_mmu_release,
    .mmu_tlb_invl           = x97_mmu_tlb_invl,

    // Hack for simulation, should be removed once the OS can acknowledge the device physical memory at boot time
    .alloc_page             = alloc_pm,
    .free_page              = free_pm,
    .zero_page              = pmap_zero_page,
    .get_victim_page        = get_victim_page,
};



/* VM code based on GMEM driver API */
int setup_ctx(gmem_vm_mode running_mode)
{
    int error;

    dev_faults = 0;
    if (running_mode == SHARE_CPU) {
        mode = running_mode;
        printf("[devc] setting ctx as share_cpu, save my pmap to address %p\n", &pmap);
        error = gmem_uvas_create(NULL, &pmap, NULL, NULL, NULL, NULL, GMEM_UVAS_SHARE_CPU,
            0, 0, 0, 0); // SHARE mode should not care about the last 4 args
        if (error == GMEM_OK)
            gmem_uvas_set_pmap_policy(pmap, false, false, 1); // page order = 1
        // pmap->data should now contain the CPU pmap
        // Save your context with pmap->data now.
        return error;
    }
    else if (running_mode == EXCLUSIVE) {
        mode = running_mode;
        printf("[devc] setting ctx as exclusive, save my pmap to address %p\n", &pmap);
        // gmem_dev_t *dev;
        // if (!is_gmem_dev(device)) 
        //     dev = gmem_dev_add(device);
        // else
        //     dev = device_get_gmem_dev(device);

        error = gmem_uvas_create(NULL, &pmap, NULL, &x97_mmu_ops, NULL, NULL, GMEM_UVAS_EXCLUSIVE,
            0, 0, 0, 0); // exclusive mode should not care about the last 4 args
        if (error == GMEM_OK)
            gmem_uvas_set_pmap_policy(pmap, false, false, 1); // page order = 1
        return error;
    } else
        printf("Other GMEM UVAS modes not supported yet\n");
    return -1;
}