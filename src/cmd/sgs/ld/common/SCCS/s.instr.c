h55472
s 00000/00000/00035
d D 1.3 86/08/18 08:44:47 fnf 3 2
c Checkpoint changes at first working shared library version
e
s 00000/00000/00035
d D 1.2 86/07/30 14:21:21 fnf 2 1
c Add macro based C debugging package macros.
e
s 00035/00000/00000
d D 1.1 86/07/29 14:31:40 fnf 1 0
c Baseline code from 5.3 release for 3b2
e
u
U
t
T
I 1
/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)ld:common/instr.c	1.6"

/*
 * Define various statistic-collection variables used to monitor the
 * internal workings of ld
 *
 *
 * Statistics are collected if the compile time define PERF_DATA
 * is turned on.  Output of these variables
 * is controlled by the presence of the UN*X file "instr.data". If
 * this file exists and can be opened for "append", these variables
 * are output.  Output occurs only on complete, successful of ld.
 */

#if PERF_DATA
long	nwalks = 0L,		/* number of collision chain walks  */
	nfwalks = 0L,		/* number of unsuccessful walks	    */
	ncolisns = 0L,		/* number of hashtab[] collisions   */
	maxchain = 0L;		/* max length of a collision chain  */

long	ttime;			/* total elapsed time		    */
struct tbuffer {		/* process and child execution	    */
	long proc_user_time;	/*	times, as returned by the   */
	long proc_system_time;	/*	times() function	    */
	long child_user_time;
	long child_system_time;
	} ptimes;
#endif
E 1
