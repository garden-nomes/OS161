#include <types.h>
#include <lib.h>
#include <test.h>


/*
 * Gives the user a friendly hello and pat on the back.
 */
void
hello(void)
{
	kprintf("Hello! You are an awesome person. Have a beer! You deserve it.\n\n");
	kprintf("\t     oOOOOOo\n"
			  "\t    ,|    oO\n"
			  "\t   //|     |\n"
			"\t   \\\\|     |\n"
			  "\t    `|     |\n"
			  "\t     `-----`\n\n");
}
