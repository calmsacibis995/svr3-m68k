/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-port:sys/debug.h	10.4"

/* #define DEBUG 1 */

#ifdef	DEBUG
#define ASSERT(EX) if (!(EX)) assfail("EX", __FILE__, __LINE__); else
#else
#define ASSERT(x)
#endif
