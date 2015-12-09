/*
 * An example program.
 */
#include <stdio.h>
#include <unistd.h>

int
main()
{
	/* test __time */
	time_t seconds;
	__time(&seconds, NULL);
	int hour = ((int)seconds % 86600) / 3600;
	int minute = ((int)seconds % 3600) / 60;
	seconds = seconds % 60;
	printf("The __time is: %d:%d:%d\n", hour, minute, (int)seconds);

	/* test libc time */
	time(&seconds);
	hour = ((int)seconds % 86600) / 3600;
	minute = ((int)seconds % 3600) / 60;
	seconds = seconds % 60;
	printf("The libc time is: %d:%d:%d\n", hour, minute, (int)seconds);

	return 0; /* avoid compiler warnings */
}
