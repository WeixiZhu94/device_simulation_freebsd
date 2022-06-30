#include <vm_cdev.h>
#include <vm/vm_page.h>
#include <vm/pmap.h>
#include <sys/vmem.h>


size_t npages = 1024 * 1024 / 4;

int init_pm(struct gmem_mmu_ops *ops) {
    first_x97_page = vm_page_alloc_contig(NULL, 0, VM_ALLOC_NORMAL | VM_ALLOC_NOBUSY | VM_ALLOC_NOOBJ,
        npages, 0, 32ULL << 30, 1ULL << 30, 0, VM_MEMATTR_DEFAULT);
    if (first_x97_page == NULL)
        printf("!!! Failed to steal physical memory for the fake device!!!\n");
    else {
    	// These vm_page structs must be marked as NOCPU pages so that vm_fault can handle them correctly
    	// This hack should be removed if the VM system can identify device page structs at the boot time
    	for (int i = 0; i < npages; i ++)
    		first_x97_page[i].flags |= PG_NOCPU;
        last_x97_page = &first_x97_page[npages - 1];
        ops->pa_min = VM_PAGE_TO_PHYS(first_x97_page);
        ops->pa_max = VM_PAGE_TO_PHYS(last_x97_page);
		pm_pool = vmem_create("x97 device private physical memory", 0, npages, 1, 16, M_WAITOK | M_BESTFIT);
        printf("!!! Stealing physical memory for the fake device succeeded, pa_min %lx, pa_max %lx\n", ops->pa_min, ops->pa_max);
    }
	return 0;
}

int exit_pm() {
	vmem_destroy(pm_pool);
	return 0;
}

vm_page_t alloc_pm()
{
	unsigned long page_idx = 0;
	// We cannot allocate a bunch of pages but free one of them...
	// vmem_xalloc(pm_pool, npages, alignment << 12, 0, 0, VMEM_ADDR_MIN, VMEM_ADDR_MAX, M_WAITOK | M_BESTFIT, &page_idx);
	if (vmem_alloc(pm_pool, 1, M_BESTFIT | M_WAITOK, &page_idx) == 0) {
		pmap_zero_page(&first_x97_page[page_idx]);
		return &first_x97_page[page_idx];
	}
	else
		return NULL;
}

gmem_error_t free_pm(vm_page_t m)
{
	int page_idx = m - first_x97_page;
	vmem_free(pm_pool, page_idx, 1);
	return GMEM_OK;
}

void zero_page(vm_page_t m)
{
	pmap_zero_page(m);
}