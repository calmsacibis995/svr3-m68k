/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-port:sys/psw.h	1.0"

/*
 *  	processor status register for 68020
 */

#define	PS_C	0x0001		/* carry bit */
#define	PS_V	0x0002		/* overflow bit */
#define	PS_Z	0x0004		/* zero bit */
#define	PS_N	0x0008		/* negative bit */
#define PS_EXT	0x0010		/* extend bit */

#define	PS_IPL	0x0700		/* interrupt priority level */
#define	PS_M	0x1000		/* Master/interrupt state */
#define	PS_CUR	0x2000		/* current mode. Supervisor if set */
#define	PS_T0	0x4000		/* trace enable bit 0 */
#define	PS_T1	0x8000		/* trace enable bit 1 */
#define	PS_T	PS_T1		/* trace enable bit 1 */

/*
 * Structure defining the two shorts that are passed to
 * exception processing routines.
 */

struct ex_status {
	short	vec_num;	/* exception type and vector number */
	short	psw;		/* process status word on stack */
};

/*
 * ex_status vector number bus types
 */

#define	SHRTBUSSTKFRM	(short)0xa008 /* short bus stack frame vector number */
#define LONGBUSSTKFRM	(short)0xb008 /* Long  bus stack frame vector number */

/*
 * Special Status Word for bus fault exception stack frame information
 */

#define	SSW_FC	0x8000		/* fault on stage C of instruction Pipe */
#define	SSW_FB	0x4000		/* fault on stage B of instruction Pipe */
#define	SSW_RC	0x2000		/* rerun stage C of instruction pipe */
#define	SSW_RB	0x1000		/* rerun stage B of instruction pipe */
#define SSW_DF	0x0100		/* Fault/rerun for data cycle */
#define	SSW_RM	0x0080		/* read-modify-write on data cycle */
#define	SSW_RW	0x0040		/* read/write for data cycle */
#define SSW_SZ	0x0030		/* size coe for data cycle */
#define	SSW_FCD	0x0007		/* function code - address space */

#define	SZ_BYTE		0x1	/* byte transfer */
#define	SZ_WORD		0x2	/* word transfer */
#define	SZ_3BYTE	0x3	/* 3 byte transfer */
#define	SZ_LONG		0x0	/* long transfer */

#define	FCD_USER	0x0	/* User address space */
#define	FCD_KERNEL	0x4	/* Kernel address space */
#define	FCD_CPU		0x7	/* CPU address space */
#define FCD_DATA	0x1	/* Data space */
#define	FCD_PROG	0x2	/* Program space */
