/*
 *	Copyright (c) 1984, Communication Machinery Corporation
 */

/*
 *	This file contains material and information which is
 *	proprietary to Communication Machinery Corporation, and
 *	which may not be divulged without the express written
 *	permission of Communication Machinery Corporation.
 */

/*	inet.h	4.1	83/05/28	*/

/*
 * External definitions for
 * functions in inet(3N)
 */
struct	in_addr inet_addr();
char	*inet_ntoa();
struct	in_addr inet_makeaddr();
unsigned long inet_network();
