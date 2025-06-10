/*		Copyright (c) 1985 AT&T		*/
/*		All Rights Reserved		*/
/*	@(#)rand.h	7.2		*/
/* rand.h */
/*	@(#) rand.h:	2.1 83/07/08	*/

/*	MC68000 Optimizer	*/

/* Definitions for operand data */

/* Operand types */

#define	RT_REG		1	/* register direct:	%d0	*/
#define	RT_IMMNUM	2	/* immediate numeric:	&1	*/
#define	RT_IMMED	3	/* immediate other:	&xyz	*/
#define	RT_IREG		4	/* register indirect:	(%a0)	*/
#define	RT_INCR		5	/* register post-incr:	(%a0)+	*/
#define	RT_DECR		6	/* register pre-decr:	-(%a0)	*/
#define	RT_OREG		7	/* offset from reg:	4(%a0)	*/
#define	RT_DBLIND	8	/* reg. double ind.:	4(%a0,%d0.w) */
#define	RT_MEM		9	/* memory direct:	xyz	*/

#define	RT_RTSUB	10	/* name of a special run-time support
				** subroutine which expects %d0
				*/
#define	RT_LABEL	11	/* label */
#define	RT_OTHER	12	/* other:  for pseudo-ops, etc. */

#define RT_SUB01	13	/* name of a special run-time subroutine
				** which expects arguments in %d0 and %d1
				*/

typedef	unsigned char RTYPE;	/* operand type */
/* Register names and values.  The values are mutually exclusive bits. */

/* REG represents a register.  It must be a long to hold all the bits. */

typedef	long	REG;

#define	REG_NONE ((REG) 0)		/* no register */
#define	REG_D0	((REG) 01)		/* %d0 */
#define	REG_D1	((REG) 02)		/* %d1 */
#define	REG_D2	((REG) 04)		/* %d2 */
#define	REG_D3	((REG) 010)		/* %d3 */
#define	REG_D4	((REG) 020)		/* %d4 */
#define	REG_D5	((REG) 040)		/* %d5 */
#define	REG_D6	((REG) 0100)		/* %d6 */
#define	REG_D7	((REG) 0200)		/* %d7 */
#define	REG_A0	((REG) 0400)		/* %a0 */
#define	REG_A1	((REG) 01000)		/* %a1 */
#define	REG_A2	((REG) 02000)		/* %a2 */
#define	REG_A3	((REG) 04000)		/* %a3 */
#define	REG_A4	((REG) 010000)		/* %a4 */
#define	REG_A5	((REG) 020000)		/* %a5 */
#define	REG_A6	((REG) 040000)		/* %a6 */
#define	REG_A7	((REG) 0100000)		/* %a7 */
#define	REG_PC	((REG) 0200000)		/* %pc */
#define	REG_CC	((REG) 0400000)		/* %cc */
#define	REG_SR	((REG) 01000000)	/* %sr */
#define	REG_USP	((REG) 02000000)	/* %usp */
#define	REG_FP	REG_A6			/* %fp is %a6 */
#define	REG_SP	REG_A7			/* %sp is %a7 */

#define	NREGS	20		/* number of unique registers above */

/* Declare register classes */

#define	REG_AREG \
	(REG_A0 | REG_A1 | REG_A2 | REG_A3 | REG_A4 | REG_A5 | REG_A6 | REG_A7)

#define	REG_DREG \
	(REG_D0 | REG_D1 | REG_D2 | REG_D3 | REG_D4 | REG_D5 | REG_D6 | REG_D7)

/* These definitions are for live/dead analysis */

#define	REGS	( REG_AREG | REG_DREG | REG_CC | REG_SR | REG_PC | REG_USP )
#define	LIVEREGS	( REG_PC | REG_SP | REG_FP )
/* Data are returned in %d0, but pointers are returned in %a0 */
#define	RETREG	( REG_D0 | REG_A0 )
#define	LIVEDEAD	NREGS

/* This next definition is highly dependent on the compiler.  It states
** which registers the compiler thinks it can use for scratch registers.
*/

#define	SCRATCHREGS	(REG_D0 | REG_D1 | REG_A0 | REG_A1) 
/* RAND contains all of the information that the optimizer requires
** for each operand.  Two operands with the same operand text string
** share the same RAND structure.  Thus two operands are the same if
** the pointer to their RAND structures is the same.
*/

struct rand
{
    char * rname;		/* operand char string */
    RTYPE rtype;		/* operand type (one of the types above) */
    REG ruses;			/* registers used by operand */
};

typedef struct rand	RAND;

#define	RNULL	((RAND *) 0)	/* a null operand pointer */

RAND * saverand();
RAND * findrand();
