h21764
s 00000/00000/00023
d D 1.6 86/09/16 15:25:29 fnf 6 5
c Changes to make ident directive work.
e
s 00000/00000/00023
d D 1.5 86/09/15 14:00:19 fnf 5 4
c Changes to remove multisegment capability for "init" and "lib" sections.
e
s 00000/00000/00023
d D 1.4 86/09/11 13:00:03 fnf 4 3
c Finish work for shared libraries, checkpoint changes.
e
s 00000/00000/00023
d D 1.3 86/08/18 08:47:45 fnf 3 2
c Checkpoint changes at first working shared library version
e
s 00000/00000/00023
d D 1.2 86/08/07 08:05:38 fnf 2 1
c Checkpoint shared library changes so far.  Still does not work
c but accepts "init" and "lib" section directives without crashing...
e
s 00023/00000/00000
d D 1.1 86/08/04 09:39:33 fnf 1 0
c Start with baseline 5.2 68000 assembler.
e
u
U
t
T
I 1
/*	@(#)systems.h	6.1		*/
/*	static	char	sccsid[] = "@(#) systems.h: 2.4 7/1/83";			*/


#define M4ON		1
#define NODEFS		1
#define DEBUG		0
#define TRANVEC		0
#define DCODGEN		0
#define ONEPROC		1
#define FLDUPDT		1	/* "field update" is really complete
				** function information
				*/
#define VAX		0
#define MACRO		0
#define MULTSECT	1
#define MC68		1
#define FLEXNAMES	1


/* Extend size of span-dependent optimizations table */

#define	MAXSS	350
E 1
