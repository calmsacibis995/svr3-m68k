h28164
s 00001/00001/00086
d D 1.7 86/11/17 11:11:32 fnf 7 6
c New preprocess does not accept random junk after #endif.
e
s 00004/00001/00083
d D 1.6 86/09/16 15:25:19 fnf 6 5
c Changes to make ident directive work.
e
s 00003/00009/00081
d D 1.5 86/09/15 14:00:05 fnf 5 4
c Changes to remove multisegment capability for "init" and "lib" sections.
e
s 00000/00000/00090
d D 1.4 86/09/11 12:59:55 fnf 4 3
c Finish work for shared libraries, checkpoint changes.
e
s 00000/00000/00090
d D 1.3 86/08/18 08:47:40 fnf 3 2
c Checkpoint changes at first working shared library version
e
s 00027/00001/00063
d D 1.2 86/08/07 08:05:26 fnf 2 1
c Checkpoint shared library changes so far.  Still does not work
c but accepts "init" and "lib" section directives without crashing...
e
s 00064/00000/00000
d D 1.1 86/08/04 09:39:27 fnf 1 0
c Start with baseline 5.2 68000 assembler.
e
u
U
t
T
I 1
/*	@(#)gendefs.h	6.4		*/
/*	static	char	sccsid[] = "@(#) gendefs.h: 2.4 6/23/83";			*/


#define NO		0
#define YES		1

#define TESTVAL		-2

D 2
#define NFILES		15
E 2
I 2
#define FDIN		0	/* Input file name in filenames[0] */
#define FDOUT		1	/* Output filename index */
#define FDTEXT		2	/* Temporary (text) filename index */
#define FDGSYMB		2	/* (pass 2, gbl sym file, share with FDTEXT */
#define FDDATA		3	/* Temporary (data) filename index */
#define FDSTAB		4	/* Temporary (symtab) filename index */
#define FDLINE		4	/* (lineno file, 2nd pass, share w FDSTAB) */
#define FDLONG		5	/* Temporary (sdi) filename index */
#define FDREL		5	/* (rel file, 2nd pass, shared with FDLONG */
#define FDSTRING	6	/* Temporary (string) filename index */
#define FDSYMB		7	/* Temporary (symbol) filename index */
#define FDTXT1		8	/* Temporary (text 1) filename index */
#define FDTXT2		9	/* Temporary (text 2) filename index */
#define FDTXT3		10	/* Temporary (text 3) filename index */
#define FDDAT1		11	/* Temporary (data 1) filename index */
#define FDDAT2		12	/* Temporary (data 2) filename index */
#define FDDAT3		13	/* Temporary (data 3) filename index */
D 5
#define FDINIT1		14	/* Temporary (init 1) filename index */
#define FDINIT2		15	/* Temporary (init 2) filename index */
#define FDINIT3		16	/* Temporary (init 3) filename index */
#define FDLIB1		17	/* Temporary (lib 1) filename index */
#define FDLIB2		18	/* Temporary (lib 2) filename index */
#define FDLIB3		19	/* Temporary (lib 3) filename index */
#define FDINIT		20	/* Temporary (init) filename index */
#define FDLIB		21	/* Temporary (lib) filename index */
E 5
I 5
#define FDINIT		14	/* Temporary (init) filename index */
#define FDLIB		15	/* Temporary (lib) filename index */
I 6
#define FDCMT		16	/* Temporary (comment) filename index */
E 6
E 5

D 5
#define NFILES		22
E 5
I 5
D 6
#define NFILES		16
E 6
I 6
#define NFILES		17
E 6
E 5
E 2

#define NCPS		8	/* Non-flexname character limit.	*/
#define BITSPBY		8
#define BITSPOW		16
I 6
#define BITSPW		32
E 6
#define OUTWTYPE	unsigned short
#define OUT(a,b)	fwrite((char *)(&a),sizeof(OUTWTYPE),1,b)

#ifdef M68020
#define SCTALIGN	4L	/* Byte alignment for sections (M68020).*/
#else
#define SCTALIGN	2L	/* Byte alignment for sections.		*/
#endif
#define TXTFILL		0x4e71	/* Text region "fill" nop instruction.	*/
#define FILL		0L
#define NULLSYM		((symbol *) NULL)


/* Index of action routines in modes array.				 */

I 6
#define NOACTION	0
E 6
#define	SETFILE		1
#define	LINENBR		2
#define	LINENUM		3
#define	LINEVAL		4
#define	DEFINE		5
#define	ENDEF		6
#define	SETVAL		7
#define	SETTYP		8
#define	SETLNO		9
#define	SETSCL		10
#define	SETTAG		11
#define	SETSIZ		12
#define	SETDIM1		13
#define	SETDIM2		14
#define	NEWSTMT		15
#define GENRELOC	16
#define BRLOC		17
#define BRAOPT		18
#define BCCOPT		19
#define BSROPT		20
#define ABSOPT		21
#define SWBEG		22
#define MOVE		23
#define IOPT		24
#define INSTI		25
#define ABSBR		26
#define NDXRELOC	27
#define IABSBR		28

#ifndef  M68881
#define NACTION		28
#else
#define CPBCCOPT	29
#define NACTION		29
D 7
#endif M68881
E 7
I 7
#endif /* M68881 */
E 7
E 1
