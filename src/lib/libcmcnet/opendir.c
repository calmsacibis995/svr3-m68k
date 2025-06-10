#ifndef lint
static char sccsid[] = "@(#)opendir.c	4.5 (Berkeley) 7/1/83";
#endif

#include <CMC/types.h>
#include <CMC/dir.h>

/*
 * open a directory.
 */
opendir(name)
	char *name;
{
	if ( name && (*name == 0) )
		name = ".";
	return (open(name, 0));
}
