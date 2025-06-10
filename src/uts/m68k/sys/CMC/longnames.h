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
 * This file contains defines to convert long names into names which are
 * unique in the first 7 characters.  This allows systems where the
 * preprocessor handles long names but the compiler doesn't to compile
 * files properly.  If the target system supports long names in the
 * compiler, these defines can be removed.
 *
 * Created August 3, 1984		D. S. Brown, CMC
 */

#define sockaddr_eth sadd_eth
# define sockaddr_in sadd_in

#define inet_addr	in_addr
#define inet_lnaof	in_lnaof
#define inet_makeaddr	in_makeaddr
#define inet_netof	in_netof
#define inet_network	in_network
#define inet_ntoa	in_ntoa

#define gethostbyaddr		gthstbaddr
#define gethostbyname		gthstbname
#define gethostent		gthstent
#define getnetbyaddr		gtnetbaddr
#define getnetbyname		gtnetbname
#define getnetent		gtnetent
#define getprotobyname		gtprobname
#define getprotobynumber	gtprobnumber
#define getprotoent		gtproent
#define getservbyname		gtsrvbname
#define getservbyport		gtsrvbport
#define getservent		gtsrvent

#define gethostname		gthstname
#define gettimeofday		gettod

#define getsockname		gtsoname
#define getpeername		gtprname
#define getsockopt		gtsoopt

#define setsockopt		stsoopt
