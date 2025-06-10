/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)xenv:m68k/paths.h	1.12"

/************************************************************************/
/*									*/
/*			      W A R N I N G				*/
/*									*/
/*	Do not directly edit this file unless you are editing the	*/
/*	master copy under <somewhere>/src/cmd/sgs/xenv!  Otherwise	*/
/*	your changes will be lost when the next pathedit is run.	*/
/*									*/
/************************************************************************/

/*
 *	Directory containing libraries and executable files
 *	(e.g. assembler pass 1)
 */

#ifndef LIBDIR
#define LIBDIR	"/lib"
#endif

#ifndef LLIBDIR1
#define LLIBDIR1 "/usr/lib"
#endif

#ifndef NDELDIRS
#define NDELDIRS 2
#endif

/*
 *	Directory containing executable ("bin") files
 */

#ifndef BINDIR
#define BINDIR	"/bin"
#endif

/*
 *	Directory containing include ("header") files for building tools
 */

#ifndef INCDIR
#define INCDIR	"/tmp"
#endif

/*
 *	Directory for "temp"  files
 */

#ifndef TMPDIR
#define TMPDIR	"/tmp"
#endif

/*
 *	Default name of output object file
 */

#define A_OUT	"a.out"

#define M4	"/usr/bin/m4"			/* macro preprocessor */

#ifdef NEW_FRONT_END	/* Not yet converted from old to new... (fnf) */

/*
 *	The following pathnames will be used by the "cc" command
 *
 *	m68k/m68kmau cross compiler
 */

#define CPP	"/lib/cpp"

/*
 *	Directory containing include ("header") files for users' use
 */

#define INCLDIR	"-I/tmp"
#define COMP	"/lib/comp"
#define C0	"/lib/front"
#define C1	"/lib/back"
#define OPTIM	"/lib/optim"

/*
 *	m68k cross assembler
 */

#define AS	"/bin/as"
#define AS1	"/lib/as1"	/* assembler pass 1 */
#define AS2	"/lib/as2"	/* assembler pass 2 */
#define CM4DEFS	"/lib/cm4defs"	/* C interface macros */
#define CM4TVDEFS "/lib/cm4tvdefs"	/* C macros with 'tv' call */

/*
 *	m68k link editor
 */

#define LD	"/bin/ld"
#define LD2	"/lib/ld2"	/* link editor pass 2 */

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

