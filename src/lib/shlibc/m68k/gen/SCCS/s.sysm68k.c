h41949
s 00030/00000/00000
d D 1.1 86/07/31 12:17:10 fnf 1 0
c Initial copy from 5.2 Generic M68000 distribution.
e
u
U
t
T
I 1
/*	@(#)sysm68k.c	6.1		*/
/*
 * sysm68k - Motorola specific system call
 */
sysm68k( cmd, arg1, arg2 )
{
	extern _sysm68k();

	switch( cmd )
	{
 	case 1:		/* grow/shrink stack */
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
	default:
		return( -1 );
	}
}
E 1
