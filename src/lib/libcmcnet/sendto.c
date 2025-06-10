/*
 *	Copyright (c) 1984, Communication Machinery Corporation
 *
 *	This file contains material and information which is
 *	proprietary to Communication Machinery Corporation, and
 *	which may not be divulged without the express written
 *	permission of Communication Machinery Corporation.
 */

/*	HISTORY:
 *		Initial coding June 1985 by Doug Miller
 */

# include <CMC/types.h>
# include <CMC/longnames.h>
# include <CMC/user_socket.h>

sendto (s, mesg, len, flags, to, tolen)
char *mesg, *to;
{
	userparm pblock;
	register userparm *pb = &pblock;
	register count;

	pb->p_where = mesg;
	pb->p_len = len;
	pb->p_len2 = flags;		/* non-standard use of len2 */
	pb->p_where3 = to;
	pb->p_len3 = tolen;

	if ((count = ioctl (s, SIOCSENDTO, pb)) < 0)
		return (-1);
	else	return (count);
}
