
#include <stdio.h>
#include <CMC/errno.h>

char *nsys_errlist[] = {	/* network error messages */
	"Operation would block",
	"Operation now in progress",
	"Operation already in progress",
	"Socket operation on non-socket",
	"Destination address required",
	"Message too long",
	"Protocol wrong type for socket",
	"Protocol not available",
	"Protocol not supported",
	"Socket type not supported",
	"Operation not supported on socket",
	"Protocol family not supported",
	"Address family not supported by protocol family",
	"Address already in use",
	"Can't assign requested address",
	"Network is down",
	"Network is unreachable",
	"Network dropped connection on reset",
	"Software caused connection abort",
	"Connection reset by peer",
	"No buffer space available",
	"Socket is already connected",
	"Socket is not connected",
	"Can't send after socket shutdown",
	"Too many references: can't splice",
	"Connection timed out",
	"Connection refused",
	"Too many levels of symbolic links",
	"File name too long",
	"Host is down",
	"Host is unreachable",
};
int nsys_nerr = (sizeof(nsys_errlist)/sizeof(nsys_errlist[0]));

void
perror (s)
char *s;
{
extern int	errno;
extern int	sys_nerr;	/* num std err messages */
extern char	*sys_errlist[];	/* text for std messages */
char		*colon = ": ";


	if (s == 0)
		s = "";
	if (*s == 0)
		colon = "";
	if (EWOULDBLOCK <= errno && errno < nsys_nerr+EWOULDBLOCK)
		fprintf (stderr, "%s%s%s\n", 
			s, colon, nsys_errlist[errno-EWOULDBLOCK]);
	else
	if (errno > 0 && errno < sys_nerr)
		fprintf (stderr, "%s%s%s\n", s, colon, sys_errlist[errno]);
	else
		fprintf (stderr, "%s%sError %d\n", s, colon, errno);
}
