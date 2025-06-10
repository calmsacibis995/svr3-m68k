/*
 * Modified to read System 5 (and earlier) style directories.
 *			Deb Brown, CMC		August 20, 1984
 */

#ifndef lint
static char sccsid[] = "@(#)readdir.c	4.5 (Berkeley) 7/1/83";
#endif

#include <CMC/types.h>
#include <CMC/dir.h>
#include <memory.h>

/*
 * get next entry in a directory.
 */
struct direct *
readdir(dirid)
	int dirid;
{
	static struct direct dp;
	struct dir_s5 d5;

	for (;;)
	{
		if (read (dirid, &d5, sizeof (d5)) != sizeof (d5))
			return (struct direct *) NULL;
		if (d5.d5_ino != 0)
		{
			dp.d_ino = d5.d5_ino;
			memcpy (dp.d_name, d5.d5_name, MAXNAMLEN);
			dp.d_name [MAXNAMLEN] = NULL;
			return (&dp);
		}
	}
}
