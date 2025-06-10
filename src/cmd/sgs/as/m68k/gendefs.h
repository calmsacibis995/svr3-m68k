/*	@(#)gendefs.h	6.4		*/
/*	static	char	sccsid[] = "@(#) gendefs.h: 2.4 6/23/83";			*/


#define NO		0
#define YES		1

#define TESTVAL		-2

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
#define FDINIT		14	/* Temporary (init) filename index */
#define FDLIB		15	/* Temporary (lib) filename index */
#define FDCMT		16	/* Temporary (comment) filename index */

#define NFILES		17

#define NCPS		8	/* Non-flexname character limit.	*/
#define BITSPBY		8
#define BITSPOW		16
#define BITSPW		32
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

#define NOACTION	0
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
#endif /* M68881 */
