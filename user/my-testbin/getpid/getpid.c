/*
 * An example program.
 */
#include <stdio.h>
#include <unistd.h>

int
main()
{
	int pid = getpid();
	printf("My pid is: %d\n", pid);
	return 0; /* avoid compiler warnings */
}
