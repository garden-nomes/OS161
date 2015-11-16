/*
 * First user program!
 * 
 * -Noah, 10/21/15 (Back to the Future Day) 
 */

#include <types.h>
#include <lib.h>
#include <test.h>


int
haiku(int argc, char** argv)
{
	const char *haiku_text =
		"\tcompiling kernels\n"
		"\tsomeday read the man pages\n"
		"\tthis doesn't work yet\n";

	/* make the compiler happy */
	(void)argc;
	(void)argv;

	kprintf("\n%s\n", haiku_text);
	return 0;
}
