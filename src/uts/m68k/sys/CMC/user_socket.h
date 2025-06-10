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
 *	User-level information for dealing with the CMC socket
 *	emulation driver.
 *
 *	This file should be included by applications that want
 *	access to the socket mechanism without using the 4.2
 *	user-level entries.  In general, that should only be
 *	library-type routines seeking to emulate a 4.2 function.
 */

typedef
struct userparm {
	char	*p_where;
	long	 p_len;
	char	*p_where2;
	long	 p_len2;
	char	*p_where3;
	long	 p_len3;
} userparm;

# ifndef INKERNEL
# define NSOCKETS 256
# endif

# define mkioctl(type,value) (('type'<<8)|value)
# define SIOCBIND mkioctl(S,1)
# define SIOCLISN mkioctl(S,2)
# define SIOCCONN mkioctl(S,3)
# define SIOCRESV mkioctl(S,4)
# define SIOCACPT mkioctl(S,5)
# define SIOCRECV mkioctl(S,6)
# define SIOCSEND mkioctl(S,7)
# define SIOCGNAM mkioctl(S,8)
# define SIOCSOPT mkioctl(S,9)
# define SIOCGOPT mkioctl(S,10)
# define SIOCSHUT mkioctl(S,11)
# define SIOCSENDTO mkioctl(S,12)
# define SIOCRECVFROM mkioctl(S,13)

/*
 *	User-visible ioctl's
 */

# define SIOCATMARK mkioctl(S,126)
# define SIOCSPGRP  mkioctl(S,125)
# define SIOCGPGRP  mkioctl(S,124)

/*
 *	For "generic" socket names - as used by sock_driver
 *	and related routines.
 *	I_NET Guys will cast as sockaddr_in
 *	ETH_NET Guys will cast as sockaddr_eth
 */

typedef struct	sockaddr {
	ushort	sa_family;		/* Protocol family	*/
	char	sa_data[14];		/* Generic "name"	*/
} sockaddr;

