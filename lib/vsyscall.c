#include <inc/vsyscall.h>
#include <inc/lib.h>

static inline int32_t
vsyscall(int num)
{
	// LAB 12: Your code here.
	if (num == VSYS_gettime) {
		return vsys[VSYS_gettime];
	} else {
		return -E_INVAL;
	}
}

int vsys_gettime(void)
{
	return vsyscall(VSYS_gettime);
}