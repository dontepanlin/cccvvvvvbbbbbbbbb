#include <inc/lib.h>


void
umain(int argc, char **argv)
{
	spawnl("client", "client", 0);
	spawnl("server", "server", 0);
    exit();
}