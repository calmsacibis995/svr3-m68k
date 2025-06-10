/*	@(#)tpoke00.c	6.3	*/
/* timepoke() routine for machines which do not allow simulation of
a software interrupt to defer execution of timeintr()   */

int NO_TMPK;		/* force variable */

timepoke ()
	{
	static int busy = 0;

	/* this prevents recursive calls of timeintr but there is a <very
	   small> possibility of a new call coming in between the decrement
	   of busy and the test.  However, the next clock interrupt will 
	   take care of it so it's not worth preventing */

	if (busy++)
		return;

	do	{
		timein();
		} while (--busy);
	}
