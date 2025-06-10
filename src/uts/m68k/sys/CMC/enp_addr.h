/*
 *	ENP address definitions:
 *
 *		ENP_VADDR	ENP (as 128k memory) in kernel's memory
 *				mapped space.
 *
 *		ENPBASE		Start of ENP memory (same as above).
 *
 *		SHAREBASE	Start of Host/ENP shared space.
 */

# define ENP_VADDR	0xdE0000

# define ENPBASE	ENP_VADDR
# define SHAREBASE	(ENPBASE + 0x1200)
