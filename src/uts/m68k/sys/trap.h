/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-port:sys/trap.h	1.0"

#define	NSYSENT	88
#define	USER	512

/*
 *  Exception Vector definitions for M68020
 */

#define		BUSERR		8	/*  Bus error			*/
#define		ADDRERR		12	/*  Address error		*/
#define		INSTERR		16	/*  Illegal instruction		*/
#define		ZDVDERR		20	/*  Divide by zero		*/
#define		CHKTRAP		24	/*  chk instruction		*/
#define		TRAPVFT		28	/*  trapv instruction		*/
#define		PRIVFLT		32	/*  priviledged instruction 	*/
#define		TRCTRAP		36	/*  trace trap  		*/
#define         EMT10FT		40	/*  emulator 1010 trap		*/
#define         EMT11FT		44	/*  emulator 1111 trap		*/
#define		COPROTV		52	/*  coprocessor violation	*/
#define		FMTERR		56	/*  format error		*/
#define		UNINTR		60	/*  uninitialized interrupt	*/
#define		STRAYFT		96	/*  stray interupt		*/

/*
 *	Special TRAP instruction vectors
 */
					/* TRAP No.			*/
#define		SYSCALL		128	/*  0 system call trap		*/
#define		BPTFLT		132	/*  1 software breakpoint entry */
#define		IOTTRAP		136	/*  2 DEC iot trap		*/
#define		EMTTRAP		140	/*  3 DEC emt trap		*/
#define		FPETRAP		144	/*  4 Floating point exeception	*/

/*
 * 	The following are 68881 specific floating point exceptions
 */

#define		FPTBSUN		192	/* Branch or Set on UNordered cond.*/
#define		FPTINEX		196	/* INEXact result		*/
#define		FPTZDVD		200	/* Divide by Zero		*/
#define		FPTDZ		200	/* Divide by Zero		*/
#define		FPTUNFL		204	/* UNderFLow			*/
#define		FPTOPER		208	/* OPerand ERror		*/
#define		FPTOVFL		212	/* OVerFLow			*/
#define		FPTSNAN		216	/* Signalling NAN		*/

#define		PMMUCONF	224	/*  PMMU configuration		*/
#define		PMMUILOP	228	/*  PMMU illegal operation	*/
#define		PMMUALV		232	/*  PMMU access level violation	*/

/*
 *	The following are software defined exceptions
 */

#define		RESCHED		256	/*  pseudo trap to signal cpu	*/
					/*  rescheduling. 		*/
#define		ACFAIL		260	/*  VME power failure		*/
#define		SYSFAIL		268	/*  VME bus SYSFAIL asserted	*/
