h24068
s 00000/00000/00023
d D 1.3 86/08/18 08:45:09 fnf 3 2
c Checkpoint changes at first working shared library version
e
s 00000/00000/00023
d D 1.2 86/07/30 14:23:05 fnf 2 1
c Add macro based C debugging package macros.
e
s 00023/00000/00000
d D 1.1 86/07/29 14:31:49 fnf 1 0
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

#ident	"@(#)ld:common/list.h	1.4"
/*
 */

#define l_AI 1		/* list of ACTITEMs			*/
#define l_IF 2		/* list of Input Files (INFILIST)	*/
#define l_IS 3		/* list of Input Sections (INSEC List)	*/
#define l_OS 4		/* list of Output Sections (OUTSEC List) */
#define l_INC 5		/* list of input sections included in an
				output sect			*/
#define l_REG 6		/* list of REGIONs			*/
#define l_MEM 7		/* list of MEMTYPEs			*/
#define l_ADR 8		/* list of ANODEs off of REGION		*/
#define l_DS 9		/* list of dummy output sections (DSECT list) */
#define l_GRP 10	/* list of output sections in a "group"	*/
#define l_CM 11		/* list of pseudo COMMON sections in ifile */
E 1
