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
