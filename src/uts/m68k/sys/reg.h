/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-port:sys/reg.h	10.2"

/*
 * Location of the users' stored registers relative to R0.
 * Usage is u.u_ar0[XX].
 */
#define	D0	0
#define	D1	1
#define	D2	2
#define	D3	3
#define	D4	4
#define	D5	5
#define	D6	6
#define	D7	7

#define	A0	8
#define	A1	9
#define	A2	10
#define	A3	11
#define	A4	12
#define	A5	13
#define	A6	14
#define A7	(-1)

#define SP	A7
#define	FP	A6
#define	PS	15
#define	SR	15
#define	PC	16

#define NREGS	18

#define	R0	0
#define	R1	1
#define	R2	2
#define	R3	3
#define	R4	4
#define	R5	5
#define	R6	6
#define	R7	7
#define	R8	8
#define	R9	9
#define	R10	10
#define	R11	11
#define	R12	12
#define	R13	13
#define	R14	14
#define	R15	A7

/*
 *	Special registers for bus errors and address errors
 */

#define	SSW	19
#define	DCFA	21
#define	SBA	26
