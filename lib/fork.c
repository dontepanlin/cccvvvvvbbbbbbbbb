// implement fork from user space

#include <inc/string.h>
#include <inc/lib.h>

// PTE_COW marks copy-on-write page table entries.
// It is one of the bits explicitly allocated to user processes (PTE_AVAIL).
#define PTE_COW		0x800

// extern volatile pte_t uvpt[];
// extern volatile pde_t uvpd[];
// void _pgfault_upcall(void);

#define UVPT_IDX(x)	((uintptr_t)addr >> PGSHIFT)

extern void _pgfault_upcall();
//
// Custom page fault handler - if faulting page is copy-on-write,
// map in our own private writable copy.
//
static void
pgfault(struct UTrapframe *utf)
{
	void *addr = (void *) utf->utf_fault_va;
	uint32_t err = utf->utf_err;

	// Check that the faulting access was (1) a write, and (2) to a
	// copy-on-write page.  If not, panic.
	// Hint:
	//   Use the read-only page table mappings at uvpt
	//   (see <inc/memlayout.h>).

	// LAB 9: Your code here.
	pte_t entry = uvpt[UVPT_IDX(addr)];
	if (! (err & FEC_WR)) {
		panic("Access does not have PTE_W");
	}
	if (! (entry & PTE_COW)) {
		panic("Page was not COW");
	}

	uint32_t permissions = PTE_P | PTE_U | PTE_W;
	int32_t retval = sys_page_alloc(0, PFTEMP, permissions);
	if (retval < 0) {
		return;
	}

	void* pg_aligned_addr = ROUNDDOWN(addr, PGSIZE);
	memmove(PFTEMP, pg_aligned_addr, PGSIZE);

	retval = sys_page_map(0,  PFTEMP, 0, pg_aligned_addr, permissions);
	if (retval < 0) {
		return;
	}

	retval = sys_page_unmap(0, PFTEMP);
	if (retval < 0) {
		return;
	}
	// Allocate a new page, map it at a temporary location (PFTEMP),
	// copy the data from the old page to the new page, then move the new
	// page to the old page's address.
	// Hint:
	//   You should make three system calls.
	//   No need to explicitly delete the old page's mapping.
	//   Make sure you DO NOT use sanitized memcpy/memset routines when using UASAN.

	// LAB 9: Your code here.

	// panic("pgfault not implemented");
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
duppage(envid_t envid, unsigned pn)
{
	// LAB 9: Your code here.
	//panic("duppage not implemented");
	void* va = (void *)(pn * PGSIZE);

	pte_t entry = uvpt[pn];

	int32_t retval = 0;
	if (! (entry & PTE_W || entry & PTE_COW)) {
		retval = sys_page_map(thisenv->env_id, va, envid, va, entry & PTE_SYSCALL);
		if (retval < 0) {
			panic("sys_page_map: %d", retval);
		}
		return 0;
	}

	entry &= ~PTE_W;
	entry |= PTE_COW;

	retval = sys_page_map(thisenv->env_id, va, envid, va, entry & PTE_SYSCALL);
	if (retval < 0) {
		panic("sys_page_map: %d", retval);
	}

	retval = sys_page_map(thisenv->env_id, va, thisenv->env_id, va, entry & PTE_SYSCALL);
	if (retval < 0) {
		panic("sys_page_map: %d", retval);
	}

	return 0;
}

static void duplicate_pages(int32_t envid)
{
	uint32_t va = 0;
	for (va = 0; va < UTOP ; va += PGSIZE) {
		uint32_t pde_index_debug = PDX(va);
		uint32_t page_number = PGNUM(va);
		uint32_t uvpd_entry = uvpd[pde_index_debug];
		if ( !(uvpd_entry & PTE_P)) {
			va += PGSIZE * (NPTENTRIES - 1);
			continue;
		}
		uint32_t low_addr = UXSTACKTOP - PGSIZE;
		uint32_t high_addr = UXSTACKTOP;
		if (low_addr <= (uint32_t)va && (uint32_t)va < high_addr) {
			continue;
		}
		uint32_t uvpt_entry = uvpt[page_number];
		if (uvpt_entry & PTE_P) {
			duppage(envid, page_number);
		}
	}

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
	// LAB 9: Your code here.
	set_pgfault_handler(pgfault);

	envid_t ret_envid = sys_exofork();
	if (ret_envid < 0) {
		panic("Could not exofork");
	}
	if (ret_envid == 0) {
		thisenv = &envs[ENVX(sys_getenvid())];
		return ret_envid;
	}


	duplicate_pages(ret_envid);

	sys_env_set_pgfault_upcall(ret_envid, _pgfault_upcall);

	sys_page_alloc(ret_envid, (void *)(UXSTACKTOP - PGSIZE), PTE_W | PTE_U);

	sys_env_set_status(ret_envid, ENV_RUNNABLE);

	return ret_envid;
}

// Challenge!
int
sfork(void)
{
	panic("sfork not implemented");
	return -E_INVAL;
}
