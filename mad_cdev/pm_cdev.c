#include <vm_cdev.h>
#include <vm/vm_page.h>
#include <vm/pmap.h>
#include <sys/vmem.h>

// Locks will need to be implemented in the future to protect free lists.
// Right now we don't have concurrent device faults.
struct pglist x97_activelist, x97_freelist, x97_wirelist;

vm_page_t get_victim_page() 
{
	// if (!TAILQ_EMPTY(&x97_freelist))
	// 	printf("The x97 freelist is not empty when you are reclaiming memory\n");
	vm_page_t m = TAILQ_FIRST(&x97_activelist);
	// TAILQ_REMOVE(&x97_activelist, m);
	return m;
}

// active queue: [least-recently-used, ..., most-recently-used]
void wire_x97_page(vm_page_t m)
{
	if (!TAILQ_EMPTY(&x97_activelist)) {
		TAILQ_REMOVE(&x97_activelist, m, plinks.q);
		TAILQ_INSERT_TAIL(&x97_wirelist, m, plinks.q);
	}
	else
		printf("The x97 page to wire does not exist in activelist\n");
}

// active queue: [least-recently-used, ..., most-recently-used]
void activate_x97_page(vm_page_t m)
{
	if (!TAILQ_EMPTY(&x97_freelist)) {
		TAILQ_REMOVE(&x97_freelist, m, plinks.q);
		TAILQ_INSERT_TAIL(&x97_activelist, m, plinks.q);
	}
	else
		printf("The x97 page to activate does not exist in freelist\n");
}

static inline void free_x97_page(vm_page_t m)
{
	if (!TAILQ_EMPTY(&x97_activelist)) {
		TAILQ_REMOVE(&x97_activelist, m, plinks.q);
		TAILQ_INSERT_HEAD(&x97_freelist, m, plinks.q);
	}
	else
		printf("The x97 page to free does not exist in activelist\n");
}

#define GB_PAGES 1024 * 1024 / 4
size_t npages = GB_PAGES / 10; // 100MB pages

int init_pm(struct gmem_mmu_ops *ops) {
    first_x97_page = vm_page_alloc_contig(NULL, 0, VM_ALLOC_NORMAL | VM_ALLOC_NOBUSY | VM_ALLOC_NOOBJ,
        npages, 0, 32ULL << 30, 1ULL << 30, 0, VM_MEMATTR_DEFAULT);
    if (first_x97_page == NULL)
        printf("!!! Failed to steal physical memory for the fake device!!!\n");
    else {
    	// Initing page queues
	    TAILQ_INIT(&x97_activelist);
	    TAILQ_INIT(&x97_freelist);
	    TAILQ_INIT(&x97_wirelist);

    	// These vm_page structs must be marked as NOCPU pages so that vm_fault can handle them correctly
    	// This hack should be removed if the VM system can identify device page structs at the boot time
    	for (int i = 0; i < npages; i ++) {
    		first_x97_page[i].flags |= PG_NOCPU;
    		first_x97_page[i].ref_count = 7;
    		TAILQ_INSERT_TAIL(&x97_freelist, &first_x97_page[i], plinks.q);
    	}
        last_x97_page = &first_x97_page[npages - 1];
        ops->pa_min = VM_PAGE_TO_PHYS(first_x97_page);
        ops->pa_max = VM_PAGE_TO_PHYS(last_x97_page) + PAGE_SIZE;
		pm_pool = vmem_create("x97 device private physical memory", 0, npages, 1, 16, M_NOWAIT | M_BESTFIT);
        printf("!!! Stealing physical memory for the fake device succeeded, pa_min %lx, pa_max %lx, npages: %lu\n", 
        	ops->pa_min, ops->pa_max, npages);
    }
	return 0;
}

int exit_pm() {
	vmem_destroy(pm_pool);
	return 0;
}

vm_page_t alloc_pm(dev_pmap_t *pmap)
{
	int retry = 0;
	unsigned long page_idx = 0;
	// We cannot allocate a bunch of pages but free one of them...
	// vmem_xalloc(pm_pool, npages, alignment << 12, 0, 0, VMEM_ADDR_MIN, VMEM_ADDR_MAX, M_WAITOK | M_BESTFIT, &page_idx);

	while (vmem_alloc(pm_pool, 1, M_BESTFIT | M_NOWAIT, &page_idx) != 0) {
		retry ++;
		reclaim_dev_page(pmap, 1);

		if (retry > 2) {
			printf("vmem_alloc failed, retry reclaiming dev pages %d\n", retry);
			printf("We will freeze you if retry fails after 5 times\n");
		}
		while (retry > 5) {
			// printf("[alloc_pm] goes to sleep\n");
			// tsleep(NULL, 0, "alloc pm", 1 * hz); 
			cpu_spinwait();
		}
	}

	zero_page(&first_x97_page[page_idx]);
	activate_x97_page(&first_x97_page[page_idx]);
	return &first_x97_page[page_idx];
}

gmem_error_t free_pm(vm_page_t m)
{
	int page_idx = m - first_x97_page;
	vmem_free(pm_pool, page_idx, 1);
	free_x97_page(&first_x97_page[page_idx]);
	return GMEM_OK;
}

void zero_page(vm_page_t m)
{
	pmap_zero_page(m);
}