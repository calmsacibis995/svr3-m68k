h24775
s 00114/00000/00000
d D 1.1 86/07/29 14:34:47 fnf 1 0
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

#ident	"@(#)sgs-inc:common/ldfcn.h	1.8"
/*
 */

#ifndef LDFILE
struct	ldfile {
	int	_fnum_;		/* so each instance of an LDFILE is unique */
	FILE	*ioptr;		/* system I/O pointer value */
	long	offset;		/* absolute offset to the start of the file */
	FILHDR	header;		/* the file header of the opened file */
	unsigned short	type;		/* indicator of the type of the file */
};


/*
	provide a structure "type" definition, and the associated
	"attributes"
*/

#define	LDFILE		struct ldfile
#define IOPTR(x)	x->ioptr
#define OFFSET(x)	x->offset
#define TYPE(x)		x->type
#define	HEADER(x)	x->header
#define LDFSZ		sizeof(LDFILE)

/*
	define various values of TYPE(ldptr)
*/

#define TVTYPE	TVMAGIC	
#if u3b5
#define USH_ARTYPE	ARTYPE
#else
#define USH_ARTYPE	(unsigned short) ARTYPE
#endif
#define ARTYPE 	0177545

/*
	define symbolic positioning information for FSEEK (and fseek)
*/

#define BEGINNING	0
#define CURRENT		1
#define END		2

/*
	define a structure "type" for an archive header
*/

#if defined(PORTAR) || defined(PORT5AR)
typedef struct
{
	char ar_name[16];
	long ar_date;
	int ar_uid;
	int ar_gid;
	long ar_mode;
	long ar_size;
} archdr;

#define	ARCHDR	archdr
#else
#define	ARCHDR	struct ar_hdr	/* ARCHIVE is defined in ar.h */
#endif
#define ARCHSZ	sizeof(ARCHDR)


/*
	define some useful symbolic constants
*/

#define SYMTBL	0	/* section nnumber and/or section name of the Symbol Table */
#define SYMESZ 18  	/* Size of symbol table entry */

#define	SUCCESS	 1
#define	CLOSED	 1
#define	FAILURE	 0
#define	NOCLOSE	 0
#define	BADINDEX	-1L

#define	OKFSEEK	0

/*
	define macros to permit the direct use of LDFILE pointers with the
	standard I/O library procedures
*/

LDFILE *ldopen();
LDFILE *ldaopen();

#define GETC(ldptr)	getc(IOPTR(ldptr))
#define GETW(ldptr)	getw(IOPTR(ldptr))
#define FEOF(ldptr)	feof(IOPTR(ldptr))
#define FERROR(ldptr)	ferror(IOPTR(ldptr))
#define FGETC(ldptr)	fgetc(IOPTR(ldptr))
#define FGETS(s,n,ldptr)	fgets(s,n,IOPTR(ldptr))
#define FILENO(ldptr)	fileno(IOPTR(ldptr))
#define FREAD(p,s,n,ldptr)	fread(p,s,n,IOPTR(ldptr))
#define FSEEK(ldptr,o,p)	fseek(IOPTR(ldptr),(p==BEGINNING)?(OFFSET(ldptr)+o):o,p)
#define FTELL(ldptr)	ftell(IOPTR(ldptr))
#define FWRITE(p,s,n,ldptr)       fwrite(p,s,n,IOPTR(ldptr))
#define REWIND(ldptr)	rewind(IOPTR(ldptr))
#define SETBUF(ldptr,b)	setbuf(IOPTR(ldptr),b)
#define UNGETC(c,ldptr)		ungetc(c,IOPTR(ldptr))
#define STROFFSET(ldptr) (HEADER(ldptr).f_symptr + HEADER(ldptr).f_nsyms * SYMESZ) 
#endif
E 1
