/*
 * Supplies the function provided by the gethostname system call on
 * 4.2 systems.
 *
 * gethostname (name, lth)
 * char *name;
 * int lth;
 *
 * Gets the host name from the system file /etc/HOST.
 *
 * Initial coding - 24 August 1984 - Deb Brown, CMC
 */

#include <CMC/longnames.h>
#include <sys/utsname.h>

#define HOSTFILE	"/etc/host"
#define READ		0
#define BLANK		040

gethostname (name, lth)
register char *name;
register int lth;
{
struct utsname sysname;
int fid;
int l;

/* First try to get name from the system. */

	if ((uname (&sysname) != -1) && sysname.nodename[0])
	{
		l = lth < sizeof (sysname.nodename) ?
			 lth : sizeof (sysname.nodename);
		memcpy (name, sysname.nodename, l);
		if (l < lth)
			memset (&(name[l]), '\0', lth-l);
		return (0);
	}

/* If that didn't work, read system name from known file */

	fid = open (HOSTFILE, READ);
	if (fid < 0)
	{
		name [0] = 0;
		return (-1);
	}
	l = read (fid, name, lth);
	if (l <= 0)
		*name = 0;
	else
	{
		for (; name [--l] <= BLANK;);
		++l;
		memset (&(name[l]), '\0', lth-l);
	}
	close (fid);
	return (0);
}
