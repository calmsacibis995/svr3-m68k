h26310
s 00001/00001/00067
d D 1.8 86/11/17 11:14:52 fnf 8 7
c New preprocessor does not accept random junk after #endif.
e
s 00001/00000/00067
d D 1.7 86/09/16 15:28:57 fnf 7 6
c Changes to make ident directive work.
e
s 00000/00000/00067
d D 1.6 86/09/15 14:01:52 fnf 6 5
c Changes to remove multisegment capability for "init" and "lib" sections.
e
s 00000/00000/00067
d D 1.5 86/09/11 13:02:38 fnf 5 4
c Finish work for shared libraries, checkpoint changes.
e
s 00000/00000/00067
d D 1.4 86/08/18 08:48:32 fnf 4 3
c Checkpoint changes at first working shared library version
e
s 00000/00000/00067
d D 1.3 86/08/07 08:03:41 fnf 3 2
c Checkpoint changes for shared lib version.  Still does not work
c but at least accepts "init" and "lib" directives without crashing now...
e
s 00002/00000/00065
d D 1.2 86/08/04 15:45:29 fnf 2 1
c Initial changes to support "init" and "lib" coff sections for
c shared libraries.
e
s 00065/00000/00000
d D 1.1 86/08/04 09:38:08 fnf 1 0
c Start with baseline 5.2 68000 assembler.
e
u
U
t
T
I 1
/*	@(#)symbols.h	6.2		*/

#include	"systems.h"
typedef char BYTE;

#define UNDEF	000
#define ABS	001
#define TXT	002
#define DAT	003
#define BSS	004
I 2
#define INIT	005		/* .init section for shared libraries */
#define LIB	006		/* .lib section for shared libraries */
I 7
#define CMT	007		/* .comment section for time/date stamping */
E 7
E 2
#define TYPE	(UNDEF|ABS|TXT|DAT|BSS)
#define TVDEF	020
#define EXTERN	040
#if N3B
#define	FUNC	0100
#endif

typedef	union
	{
		char	name[9];
		struct
		{
			long	zeroes;
			long	offset;
		} tabentry;
	} name_union;

typedef	struct
	{
		name_union	_name;
		BYTE	tag;
		short	styp;
		long	value;
#ifdef M68881
		long	value2;
		long	value3;
D 8
#endif M68881
E 8
I 8
#endif /* M68881 */
E 8
		short	maxval;
		short	sectnum;
	} symbol;

#define SYMBOLL	sizeof(symbol)

typedef	struct
	{
		char	name[sizeof(name_union)];
		BYTE	tag;
		BYTE	val;
		BYTE	nbits;
		long	opcode;
		symbol	*snext;
	} instr;

#define INSTR sizeof(instr);

#define INSTALL	1
#define N_INSTALL	0
#define USRNAME	0
#define MNEMON	1

typedef	union
	{
		symbol	*stp;
		instr	*itp;
	} upsymins;
E 1
