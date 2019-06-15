#include <inc/lib.h>

void
umain(int argc, char **argv)
{
	mkfifo("testfifo");
	exit();
}

