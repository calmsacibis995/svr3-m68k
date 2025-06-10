h49449
s 00000/00000/00022
d D 1.6 86/09/16 15:28:14 fnf 6 5
c Changes to make ident directive work.
e
s 00000/00000/00022
d D 1.5 86/09/15 14:01:18 fnf 5 4
c Changes to remove multisegment capability for "init" and "lib" sections.
e
s 00000/00000/00022
d D 1.4 86/09/11 13:01:50 fnf 4 3
c Finish work for shared libraries, checkpoint changes.
e
s 00000/00000/00022
d D 1.3 86/08/18 08:48:02 fnf 3 2
c Checkpoint changes at first working shared library version
e
s 00000/00000/00022
d D 1.2 86/08/07 08:03:07 fnf 2 1
c Checkpoint changes for shared lib version.  Still does not work
c but at least accepts "init" and "lib" directives without crashing now...
e
s 00022/00000/00000
d D 1.1 86/08/04 09:38:00 fnf 1 0
c Start with baseline 5.2 68000 assembler.
e
u
U
t
T
I 1
/*	@(#)expand.h	6.1		*/
#define L_SDI	0	/* a long sdi */
#define S_SDI	1	/* a short sdi */
#define U_SDI	2	/* an sdi of unknown size */

/* structure for a sdi selection set entry */

typedef struct {
	unsigned short sdicnt;	/* this is the Nth sdi */
	short itype;	/* type of this sdi */
	long minaddr;	/* minimum address for this sdi */
	short maxaddr;	/* difference between max. addr. and min. addr. */
	symbol *labptr;	/* ptr to label appearing in the operand */
	long constant;	/* value of constant appearing in operand */
} ssentry;

/* structure that contains the span range for each sdi */

typedef struct {
	long lbound;	/* lower bound of sdi range */
	long ubound;	/* upper bound of sdi range */
} rangetag;
E 1
