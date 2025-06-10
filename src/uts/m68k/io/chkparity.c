/* chkparity.c
 *
 * call parity handling routines as found in
 * the parity check routines table set-up in sys/space.h
 */
#include "sys/param.h"

/* mode argument values */
#define KERNEL 0
#define USER 1

extern int (*chkpartbl[])();

int chkparity(loc, mode)
char *loc;
int mode;
{
	register int i, status, (*chkpar)();

	status = 0;
	for (i = 0; (chkpar = chkpartbl[i]) != NULL; i++)
		status |= (*chkpar)(loc, mode);
	return(status);
}
