#include <inc/lib.h>

char *msg = "Now is the time for all good men to come to the aid of their party.";


void
umain(int argc, char **argv)
{
	char buf[100];
	int i, pid, fd;

	binaryname = "pipereadeof";

	if ((fd = open("/testfifo", O_RDWR)) < 0)
        cprintf("open %s: %i", "testfifo", fd);

	if ((pid = fork()) < 0)
		panic("fork: %i", pid);

	if (pid == 0) {
		cprintf("[%08x] pipereadeof readn %d\n", thisenv->env_id, fd);
		i = readn(fd, buf, sizeof buf-1);
		if (i < 0)
			panic("read: %i", i);
		buf[i] = 0;
		cprintf("i = %d\n", i);
		if (strcmp(buf, msg) == 0)
			cprintf("\npipe read closed properly\n");
		else
			cprintf("\ngot %d bytes: %s\n", i, buf);
		exit();
	} else {
		cprintf("[%08x] pipereadeof write %d\n", thisenv->env_id, fd);
		if ((i = write(fd, msg, strlen(msg))) != strlen(msg))
			panic("write: %i", i);
		close(fd);
	}
	wait(pid);

	binaryname = "pipewriteeof";
	if ((fd = open("/testfifo", O_RDWR)) < 0)
        cprintf("open %s: %i", "testfifo", fd);

	if ((pid = fork()) < 0)
		panic("fork: %i", i);

	if (pid == 0) {
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

	cprintf("pipe tests passed\n");
}