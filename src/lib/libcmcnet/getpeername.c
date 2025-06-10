/*
 *	Copyright (c) 1984, Communication Machinery Corporation
 */

/*
 *	This file contains material and information which is
 *	proprietary to Communication Machinery Corporation, and
 *	which may not be divulged without the express written
 *	permission of Communication Machinery Corporation.
 */

# include <CMC/longnames.h>
# include <CMC/types.h>
# include <CMC/user_socket.h>

getpeername (s, name, namelen)
char *name;
int  *namelen;
{
	userparm pblock;
	register userparm *p = &pblock;

	p->p_where = 0;
	p->p_len = 0;
	p->p_where2 = name;
	p->p_len2 = (int) namelen;	/* NOTE nonstandard use of p_len2 */

	if ( ioctl (s, SIOCGNAM, p) < 0 )
		return (-1);
	else	return (0);
}

getsockname (s, name, namelen)
char *name;
int  *namelen;
{
	userparm pblock;
	register userparm *p = &pblock;

	p->p_where = name;
	p->p_len = (int) namelen;	/* NOTE nonstandard use of p_len */
	p->p_where2 = 0;
	p->p_len2 = 0;

	if ( ioctl (s, SIOCGNAM, p) < 0 )
		return (-1);
	else	return (0);
}
