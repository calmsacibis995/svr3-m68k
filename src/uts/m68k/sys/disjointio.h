/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-port:sys/disjointio.h	1.0"
/* 
 *		VME 1000 DMA break-up routine for any physical I/O
 */

struct djntio {
	int	addr;
	int	count;
};
