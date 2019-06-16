#include <inc/lib.h>

void
umain(int argc, char **argv)
{
    int fd;
    char buf[6];
    if ((fd = open("/testfifo", O_RDONLY)) < 0)
        cprintf("open %s: %i", "testfifo", fd);

    // Read from FIFO
    read(fd, buf, sizeof(6));

    // Print the read message
    printf("User2: %s\n", buf);
    close(fd);

    exit();
}