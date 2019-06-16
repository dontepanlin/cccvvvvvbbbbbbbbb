#include <inc/lib.h>

char *msg = "Now is the time for all good men to come to the aid of their party.";

void
umain(int argc, char **argv)
{
    char buf[100];
	int fd = 0, pid, i;

    binaryname = "fiforeadeof";

    if ((pid = fork()) < 0)
		panic("fork: %i", pid);

	if (pid == 0) {
		close(fd);
		if ((fd = open("/testfifo", O_RDONLY)) < 0)
			cprintf("open %s: %i", "testfifo", fd);
		cprintf("Open fifo to read %d\n", fd);
		i = readn(fd, buf, sizeof buf-1);
		if (i < 0)
			panic("read: %i", i);
		buf[i] = 0;
		if (strcmp(buf, msg) == 0)
			cprintf("\npipe read closed properly\n");
		else
			cprintf("\ngot %d bytes: %s\n", i, buf);
		close(fd);
		exit();
	} else {
		close(fd);
		if ((fd = open("/testfifo", O_WRONLY)) < 0)
			cprintf("open %s: %i", "testfifo", fd);
		cprintf("file descriptor %d\n", fd);
		if ((i = write(fd, msg, strlen(msg))) != strlen(msg))
			panic("write: %i", i);
		close(fd);
	}
	wait(pid);

	binaryname = "fifowriteeof";

	if ((pid = fork()) < 0)
		panic("fork: %i", pid);

	if (pid == 0) {
		close(fd);
		if ((fd = open("/testfifo", O_WRONLY)) < 0)
			cprintf("open %s: %i", "testfifo", fd);
		while (1) {
			cprintf(".");

			if (write(fd, "x", 1) != 1)
				break;
		}
		cprintf("\npipe write closed properly\n");
		exit();
	}

	close(fd);
	wait(pid);

	cprintf("fifo tests passed\n");
}