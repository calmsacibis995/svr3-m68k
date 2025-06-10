h65357
s 00000/00000/00024
d D 1.3 86/08/18 08:44:49 fnf 3 2
c Checkpoint changes at first working shared library version
e
s 00000/00000/00024
d D 1.2 86/07/30 14:21:23 fnf 2 1
c Add macro based C debugging package macros.
e
s 00024/00000/00000
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

#ident	"@(#)ld:common/instr.h	1.5"
/*
 */

#if PERF_DATA
extern long	nwalks, nfwalks, ncolisns, maxchain;

extern long	allocspc;

extern long ttime;
extern struct tbuffer {
	long proc_user_time;
	long proc_system_time;
	long child_user_time;
	long child_system_time;
	} ptimes;
#endif
E 1
