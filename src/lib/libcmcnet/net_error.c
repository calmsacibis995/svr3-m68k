/*
 *	Copyright (c) 1984, Communication Machinery Corporation
 */

/*
 *	This file contains material and information which is
 *	proprietary to Communication Machinery Corporation, and
 *	which may not be divulged without the express written
 *	permission of Communication Machinery Corporation.
 */

/*
 * Get a pointer to the message for a given error code.
 */

#include <stdio.h>
#include <CMC/errno.h>

char *
net_error (errnum)
int errnum;
{
extern int	sys_nerr;	/* num std err messages */
extern char	*sys_errlist[];	/* text for std messages */
extern char	*nsys_errlist[];/* text for network messages */
extern int	nsys_nerr;	/* num network messages */
static char	unknown[48];


	if (EWOULDBLOCK <= errnum && errnum < nsys_nerr+EWOULDBLOCK)
		return (nsys_errlist[errnum-EWOULDBLOCK]);
	else
	if (errnum > 0 && errnum < sys_nerr)
		return (sys_errlist[errnum]);
	else {
		sprintf ( unknown, "Unknown error code %d", errnum );
		return (unknown);
	}
}
