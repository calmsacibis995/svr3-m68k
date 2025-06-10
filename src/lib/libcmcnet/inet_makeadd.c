/*
 *	Copyright (c) 1984, Communication Machinery Corporation
 */

/*
 *	This file contains material and information which is
 *	proprietary to Communication Machinery Corporation, and
 *	which may not be divulged without the express written
 *	permission of Communication Machinery Corporation.
 */

/*	inet_makeaddr.c	5.0	(CMC)	8/24/84	*/

#include <CMC/longnames.h>
#include <CMC/types.h>
#include <CMC/in.h>

/*
 * Formulate an Internet address from network + host.  Used in
 * building addresses stored in the ifnet structure.
 */
struct in_addr
inet_makeaddr(net, host)
	int net, host;
{
	u_long addr;

	if (net < 128)
		addr = (net << IN_CLASSA_NSHIFT) | host;
	else if (net < 65536)
		addr = (net << IN_CLASSB_NSHIFT) | host;
	else
		addr = (net << IN_CLASSC_NSHIFT) | host;
	addr = htonl(addr);
	return (*(struct in_addr *)&addr);
}
