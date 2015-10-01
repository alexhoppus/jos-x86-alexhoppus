// implement fork from user space

#include <inc/string.h>
#include <inc/lib.h>

// PTE_COW marks copy-on-write page table entries.
// It is one of the bits explicitly allocated to user processes (PTE_AVAIL).
#define PTE_COW		0x800
#define US_PTE_ADDR(addr) (unsigned long *)((PDX(UVPT) << 22) | (PDX(addr) << 12) | (PTX(addr) << 2))
//
// Custom page fault handler - if faulting page is copy-on-write,
// map in our own private writable copy.
//
static void
pgfault(struct UTrapframe *utf)
{
	void *addr = (void *) utf->utf_fault_va;
	uint32_t err = utf->utf_err;
	int r;

	// Check that the faulting access was (1) a write, and (2) to a
	// copy-on-write page.  If not, panic.
	// Hint:
	//   Use the read-only page table mappings at uvpt
	//   (see <inc/memlayout.h>).

	// LAB 4: Your code here.

	// Allocate a new page, map it at a temporary location (PFTEMP),
	// copy the data from the old page to the new page, then move the new
	// page to the old page's address.
	// Hint:
	//   You should make three system calls.
	//   No need to explicitly delete the old page's mapping.

	// LAB 4: Your code here.
	unsigned long *pgd_base = (unsigned long *)UVPT;
        unsigned long *pte_addr = (unsigned long *)US_PTE_ADDR(addr);
        unsigned long perm = (unsigned long)((*pte_addr) & 0xFFF);
	//cprintf("page fault %p\n", addr);
	//cprintf("Pte %lx pteaddr %p\n", *pte_addr, pte_addr);
        if ((perm & PTE_COW) && (err & FEC_WR)) {
		int r;
		if ((r = sys_page_alloc(0, PFTEMP, PTE_P|PTE_U|PTE_W)) < 0)
			panic("sys_page_alloc: %e", r);
		memcpy(PFTEMP, ROUNDDOWN(addr, PGSIZE), PGSIZE);
		if ((r = sys_page_map(0, PFTEMP, 0, ROUNDDOWN(addr, PGSIZE), PTE_P|PTE_U|PTE_W)) < 0)
 			panic("sys_page_map: %e", r);
 		if ((r = sys_page_unmap(0, PFTEMP)) < 0)
 			panic("sys_page_unmap: %e", r);
	} else {
		panic("Unhandled pagefault at addr %p perm %lx\n", addr, perm);
	}
}


//
// Map our virtual page pn (address pn*PGSIZE) into the target envid
// at the same virtual address.  If the page is writable or copy-on-write,
// the new mapping must be created copy-on-write, and then our mapping must be
// marked copy-on-write as well.  (Exercise: Why do we need to mark ours
// copy-on-write again if it was already copy-on-write at the beginning of
// this function?)
//
// Returns: 0 on success, < 0 on error.
// It is also OK to panic on error.
//
static int
duppage(envid_t envid, unsigned long addr)
{
	int r;
	unsigned long *pgd_base = (unsigned long *)UVPT;
	unsigned long *pte_addr = (unsigned long *)((PDX(UVPT) << 22) | (PDX(addr) << 12) | (PTX(addr) << 2));
	unsigned long perm = (unsigned long)((*pte_addr) & 0xFFF);
	//cprintf("addr %lx perm %lx\n", addr, perm);
	if (perm & PTE_W || perm & PTE_COW) {
		if ((r = sys_page_map(0, (void *)addr, envid, (void *)addr, PTE_P|PTE_U|PTE_COW)) < 0)
			panic("sys_page_map\n");
		if ((r = sys_page_map(0, (void *)addr, 0, (void *)addr, PTE_P|PTE_U|PTE_COW)) < 0)
			panic("sys_page_map\n");
	} else {
                if ((r = sys_page_map(0, (void *)addr, envid, (void *)addr, PTE_P|PTE_U)) < 0)
                        panic("sys_page_map\n");
	}
	return 0;
}

//
// User-level fork with copy-on-write.
// Set up our page fault handler appropriately.
// Create a child.
// Copy our address space and page fault handler setup to the child.
// Then mark the child as runnable and return.
//
// Returns: child's envid to the parent, 0 to the child, < 0 on error.
// It is also OK to panic on error.
//
// Hint:
//   Use uvpd, uvpt, and duppage.
//   Remember to fix "thisenv" in the child process.
//   Neither user exception stack should ever be marked copy-on-write,
//   so you must allocate a new page for the child's user exception stack.
//
envid_t
fork(void)
{
	int r;
	envid_t envid;
	unsigned long addr;
	extern unsigned char end[];
	// LAB 4: Your code here.
	set_pgfault_handler(pgfault);
	// Allocate a new child environment.
        // The kernel will initialize it with a copy of our register state,
        // so that the child will appear to have called sys_exofork() too -
        // except that in the child, this "fake" call to sys_exofork()
        // will return 0 instead of the envid of the child.
        envid = sys_exofork();
        if (envid < 0)
                panic("sys_exofork: %e", envid);
        if (envid == 0) {
                // We're the child.
                // The copied value of the global variable 'thisenv'
                // is no longer valid (it refers to the parent!).
                // Fix it and return 0.
                thisenv = &envs[ENVX(sys_getenvid())];
                return 0;
        }
	// We're the parent.
        // Eagerly copy our entire address space into the child.
        // This is NOT what you should do in your fork implementation.
        for (addr = (unsigned long) UTEXT; addr < (unsigned long)end; addr += PGSIZE)
 		duppage(envid, addr);

	duppage(envid, (unsigned long) (USTACKTOP - PGSIZE));
	if ((r = sys_page_alloc(envid, (void *)(UXSTACKTOP - PGSIZE), PTE_P|PTE_U|PTE_W)) < 0) {
 		panic("sys_page_alloc: %e", r);
 	}
        // Start the child environment running
        if ((r = sys_env_set_status(envid, ENV_RUNNABLE)) < 0)
                panic("sys_env_set_status: %e", r);
	return envid;
}

// Challenge!
int
sfork(void)
{
	panic("sfork not implemented");
	return -E_INVAL;
}
