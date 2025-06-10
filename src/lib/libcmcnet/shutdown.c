
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
# include <CMC/user_socket.h>

shutdown (s, how)
{

	if ( ioctl (s, SIOCSHUT, how) < 0 )
		return (-1);
	else	return (0);
}
