#include <inc/lib.h>

void
umain(int argc, char **argv)
{
	int r = mkfifo("testfifo");
	if (r < 0)
	{
		cprintf("Failed to create fifo testfifo %d \n", r);
	}
	exit();
}