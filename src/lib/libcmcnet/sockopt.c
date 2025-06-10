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
 *	History:
 *
 *	Initial coding October 2, 1984 by John Mullen
 */

# include <CMC/types.h>
# include <CMC/longnames.h>
# include <CMC/user_socket.h>

setsockopt (s, lev, nm, val, len)
char *val;
{
	userparm parm;
	register userparm *pb = &parm;

	pb->p_where = val;
	pb->p_len = len;
	pb->p_where2 = (char *) lev;	/* non-standard use of where2 */
	pb->p_len2 = nm;		/* ditto for len2 */

	if ( ioctl (s, SIOCSOPT, pb) < 0 )
		return (-1);
	else	return (0);
}

getsockopt (s, lev, nm, val, len)
char *val;
int *len;
{
	userparm parm;
	register userparm *pb = &parm;

	pb->p_where = val;
	pb->p_len = (int) len;		/* non-standard use len */
	pb->p_where2 = (char *) lev;	/* non-standard use of where2 */
	pb->p_len2 = nm;		/* ditto for len2 */

	if ( ioctl (s, SIOCGOPT, pb) < 0 )
		return (-1);
	else	return (0);
}
