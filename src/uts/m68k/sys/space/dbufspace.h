/*	%W%	*/

/*
 *	This file defines the external data structures used
 *	by the dbuf.c double buffer module
 */

#include <sys/dbuf.h>

struct dbuf dbuf[DBUF];
int	dbuf_cnt = DBUF;

int	dbuf_szstatic = DBUFSZSTATIC;
char	dbuf_static[DBUFSZSTATIC*DBUF*2+1];
char	dbuf_busystatic[DBUF*2];

#undef	DBUFMAX
#undef	DBUFMIN

#undef	DBUFIOCTL
#undef	DBUFSET
#undef	DBUFGET
#undef	DBUFRETRY
#undef	DBUFFLUSH
