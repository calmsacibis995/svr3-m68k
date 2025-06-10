/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-port:sys/sysm68k.h	10.2"

/*
 * Commands for sysm68k system call 
 */

#define	S68STACK	1	/* Grow user stack on the MC68000 	*/
#define	S68CONT		2	/* Instr. cont. after signal handlers	*/
#define S68FPEX		3	/* MC68881 floating point exception	*/
#define	S68STIME	4	/* Set internal time only 		*/
#define	S68SETNAME	5	/* Rename the sytem			*/
#define	S68RTODC	6	/* read time of day clock		*/
#define	S68ICACHEON	7	/* Turn internal chip cache on 		*/
#define	S68ICACHEOFF	8	/* Turn internal chip cache off		*/
#define	S68DELMEM	9	/* Delete available memory for testing	*/
#define	S68ADDMEM	10	/* Return previously deleted memory	*/
#define	S68MEMSIZE	11	/* return the memory size of system	*/
#define	S68SWAP		12	/* Manipulate swap space (see swap.h)	*/
#define	S68FPHW		13	/* status of floating point hardware	*/
#define	S68BCACHEON	14	/* Turn MVME131XT board cache on	*/
#define	S68BCACHEOFF	15	/* Turn MVME131XT board cache off	*/

/*
 *	Arguments and return values for above system calls
 */

#define	NOFPHW		0	/* No floating point hardware present	*/
#define	MC68881		1	/* MC-68881 floating point coprocessor	*/
