/*	@(#)sysm68k.c	6.1		*/
/*
 * sysm68k - Motorola specific system call
 */

#include "sys/sysm68k.h"

sysm68k( cmd, arg1, arg2 )
{
	extern _sysm68k();

	switch( cmd )
	{
 	case S68STACK:		/* grow/shrink stack */
		{
			register x;
#ifdef DEBUG
			printf("\n\tcmd = %x\targ1 = %x\targ2 = %x\t*arg2 = %x\n",
				cmd, arg1, arg2, *arg2 );

#endif
			if( ( x = _sysm68k( 1, arg1 ) ) == -1 )
				return( -1 );
#ifdef DEBUG
			printf("\n\tx = %x\n", x);
#endif
			*(int *)arg2 = x;
			return( 0 );
		}
		break;
#ifdef M68881
 	case 3:		/* Fetch offending MC68881 exception operand */
		{
			register x;
#ifdef DEBUG
			printf("\n\tcmd = %x\targ1 = %x\n", cmd, arg1);

#endif
			if( ( x = _sysm68k( 3, arg1 ) ) == -1 )
				return( -1 );
#ifdef DEBUG
			printf("\n\tx = %x\n", x);
#endif
			return( 0 );
		}
		break;
#endif

	case S68RTODC:		/* read TODC */
	case S68STIME:		/* set internal time, not hardware clock */
	case S68SETNAME:	/* rename the system */
	case S68MEMSIZE:	/*  return the size of memory */
	case S68SWAP:		/* General swap add/del/chk interface
	case S68ICACHEON:	/* Enable internal cache of the MC-68020 */
	case S68ICACHEOFF:	/* Disable internal cache of the MC-68020 */
	case S68DELMEM:
	case S68ADDMEM:		/* Add back previously deleted memory */
	case S68FPHW:		/* Tell user whether we have a coprocessor  */

		return (_sysm68k (cmd, arg1, arg2));
		break;
	default:
		return( -1 );
	}
}
