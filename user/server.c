#include <inc/lib.h>

void
umain(int argc, char **argv)
{

    int fd;
    char *msg = "party.";
    if ((fd = open("/testfifo", O_WRONLY)) < 0)
			cprintf("open %s: %i", "testfifo", fd);

    // Write the input arr2ing on FIFO
    // and close it

    write(fd, msg, strlen(msg) + 1);
    close(fd);

    exit();
}