/*%W%	%Q%	%G%*/
/*IH*********************** INCLUDE HEADER ****************************
**
**  INCLUDE NAME:	types.h
**
**  DESCRIPTION:	This file contains data type definitions for
**			use with VRTX networking software.
**
**  NOTES:
**
**  UPDATE LOG:
**  Date	Name		V/R	Comments
**  --------	-----------   	---	-----------------------------
**  07/10/85	Roy Larsen	1.0	Original submital to SCCS
**  01/13/86	Roy Larsen	1.0	Changed BOOLEAN to a UBYTE type
**					and VOID to void
**  05/19/86	B. Volquardsen	1.0	Changed SUCCESS & FAILURE defs.
**					 to match those in ldfcn.h.
**
**********************************************************************/

#define	BOOLEAN	unsigned char		/* Two valued (TRUE/FALSE)   */
#define	DEFAULT	int			/* Default size		     */

#define	BYTE	char			/* Signed byte (8 bits)	     */
#define	UBYTE	unsigned char		/* Unsigned byte	     */

#define	SHORT	short			/* Signed word (16 bits)     */
#define	USHORT	unsigned short		/* Unsigned word	     */

#define	LONG	long			/* Signed long (32 bits)     */
#define	ULONG	unsigned long		/* Unsigned long	     */

#define	VOID	void			/* Void function return	     */
#ifndef NOT_USED
typedef	VOID	(*PTF)();  		/* Pointer to function type  */
#endif

/*
**  The following are null definitions and may be used with
**  the previously defined declarators for clarity.
*/

#define	LOCAL	/**/			/* Local variable	     */
#define	GLOBAL	/**/			/* Global variable	     */
#define	MODULE	/**/			/* Module variable	     */

/*
**  Miscellaneous definitions.
*/
#define SUCCESS  1
#define FAILURE  0

#define	TRUE	(1)			/* Function TRUE value	     */
#define	FALSE	(0)			/* Function FALSE value	     */

#define	YES	1			/* "TRUE"		     */
#define	NO	0			/* "FALSE"		     */

#define	ON	1			/* "TRUE"		     */
#define	OFF	0			/* "FALSE"		     */

#define	FOREVER	1			/* while ( FOREVER ) loop    */

#define	EOF	(-1)			/* EOF value		     */
#define	NULL	0			/* Null value		     */
