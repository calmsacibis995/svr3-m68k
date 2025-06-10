h01472
s 00019/00019/00105
d D 1.6 86/11/19 12:03:43 fnf 6 5
c First cut at pathedit integration for m68k (native only).
e
s 00017/00000/00107
d D 1.5 86/11/19 10:56:14 fnf 5 4
c Changes for use with pathedit and m68k.
e
s 00002/00001/00105
d D 1.4 86/11/18 14:31:21 fnf 4 3
c Checkpoint changes.
e
s 00037/00010/00069
d D 1.3 86/11/12 16:51:09 fnf 3 2
c Changes from integration of latest m68k compiler.
e
s 00009/00000/00070
d D 1.2 86/09/08 08:24:19 fnf 2 1
c Change so LIBDIRS can be specified on compilation line.
e
s 00070/00000/00000
d D 1.1 86/07/29 14:35:35 fnf 1 0
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

D 3
#ident	"@(#)xenv:m32/paths.h	1.12"
/*
 *
 *
 *	Pathnames for m32/m32mau
 */
E 3
I 3
#ident	"@(#)xenv:m68k/paths.h	1.12"
E 3

I 5
/************************************************************************/
/*									*/
/*			      W A R N I N G				*/
/*									*/
/*	Do not directly edit this file unless you are editing the	*/
/*	master copy under <somewhere>/src/cmd/sgs/xenv!  Otherwise	*/
/*	your changes will be lost when the next pathedit is run.	*/
/*									*/
/************************************************************************/

E 5
/*
 *	Directory containing libraries and executable files
 *	(e.g. assembler pass 1)
 */
I 2

#ifndef LIBDIR
E 2
D 6
#define LIBDIR	"/lib"
E 6
I 6
#define LIBDIR	"M68KLIBDIR"
E 6
I 2
#endif

#ifndef LLIBDIR1
E 2
D 6
#define LLIBDIR1 "/usr/lib"
E 6
I 6
#define LLIBDIR1 "M68KUSRLIB"
E 6
I 2
#endif

#ifndef NDELDIRS
E 2
#define NDELDIRS 2
I 2
#endif
E 2

/*
 *	Directory containing executable ("bin") files
 */
I 3

I 5
#ifndef BINDIR
E 5
E 3
D 6
#define BINDIR	"/bin"
E 6
I 6
#define BINDIR	"M68KBINDIR"
E 6
I 5
#endif
E 5

/*
 *	Directory containing include ("header") files for building tools
 */
I 3

I 5
#ifndef INCDIR
E 5
E 3
D 6
#define INCDIR	"/tmp"
E 6
I 6
#define INCDIR	"M68KINCDIR"
E 6
I 5
#endif
E 5

/*
 *	Directory for "temp"  files
 */
D 3
#define TMPDIR	"/tmp"
E 3

I 5
#ifndef TMPDIR
E 5
I 3
D 6
#define TMPDIR	"/usr/tmp"
E 6
I 6
#define TMPDIR	"M68KTMPDIR"
E 6
I 5
#endif
E 5

E 3
/*
 *	Default name of output object file
 */
I 3

E 3
D 6
#define A_OUT	"a.out"
E 6
I 6
#define A_OUT	"SGSa.out"
E 6

I 4
#define M4	"/usr/bin/m4"			/* macro preprocessor */

E 4
I 3
#ifdef NEW_FRONT_END	/* Not yet converted from old to new... (fnf) */

E 3
/*
 *	The following pathnames will be used by the "cc" command
 *
D 3
 *	m32/m32mau cross compiler
E 3
I 3
 *	m68k/m68kmau cross compiler
E 3
 */
I 3

E 3
D 6
#define CPP	"/lib/cpp"
E 6
I 6
#define CPP	"M68KCPP"
E 6
I 3

E 3
/*
 *	Directory containing include ("header") files for users' use
 */
I 3

E 3
D 6
#define INCLDIR	"-I/tmp"
#define COMP	"/lib/comp"
#define C0	"/lib/front"
#define C1	"/lib/back"
#define OPTIM	"/lib/optim"
E 6
I 6
#define INCLDIR	"-IM68KINCDIR"
#define COMP	"M68KLIBDIR/comp"
#define C0	"M68KLIBDIR/front"
#define C1	"M68KLIBDIR/back"
#define OPTIM	"M68KLIBDIR/optim"
E 6
I 3

E 3
/*
D 3
 *	m32/m32mau cross assembler
E 3
I 3
 *	m68k cross assembler
E 3
 */
I 3

E 3
D 6
#define AS	"/bin/as"
#define AS1	"/lib/as1"	/* assembler pass 1 */
#define AS2	"/lib/as2"	/* assembler pass 2 */
D 4
#define M4	"/usr/bin/m4"			/* macro preprocessor */
E 4
#define CM4DEFS	"/lib/cm4defs"	/* C interface macros */
#define CM4TVDEFS "/lib/cm4tvdefs"	/* C macros with 'tv' call */
E 6
I 6
#define AS	"M68KBINDIR/SGSas"
#define AS1	"M68KLIBDIR/SGSas1"	/* assembler pass 1 */
#define AS2	"M68KLIBDIR/SGSas2"	/* assembler pass 2 */
#define CM4DEFS	"M68KLIBDIR/SGScm4defs"	/* C interface macros */
#define CM4TVDEFS "M68KLIBDIR/SGScm4tvdefs"	/* C macros with 'tv' call */
E 6
I 3

E 3
/*
D 3
 *	m32/m32mau link editor
E 3
I 3
 *	m68k link editor
E 3
 */
I 3

E 3
D 6
#define LD	"/bin/ld"
#define LD2	"/lib/ld2"	/* link editor pass 2 */
E 6
I 6
#define LD	"M68KBINDIR/SGSld"
#define LD2	"M68KLIBDIR/SGSld2"	/* link editor pass 2 */
E 6
I 3

#else	/* !NEW_FRONT_END */

/*
** Names of various pieces called by the cc command.  The names are
** formed by prepending either BINDIR (B) or LIBDIR (L) and the cc
** command prefix (e.g., "mc68" in "mc68cc").  See cc command for
** details.
*/

#define NAMEcpp		"cpp"		/* C preprocessor:  L		*/
#define NAMEccom	"ccom"		/* C compiler:  L		*/
#define NAMEoptim	"optim"		/* optimizer:  L		*/
#define NAMEas		"as"		/* assembler:  B		*/
#define NAMEld		"ld"		/* loader:  B			*/
#define NAMEcrt0	"crt0.o"	/* C run time startoff:  L	*/
#define NAMEmcrt0	"mcrt0.o"	/* profiling startoff:  L	*/

#endif	/* NEW_FRONT_END */
I 5

E 5
E 3
E 1
