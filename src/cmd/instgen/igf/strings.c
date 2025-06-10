#include	<stdio.h>
#include	"igf.h"
#include	"../com/common.h"

#ident	"$Header: /grp/mat/53/utils/igf/RCS/strings.c,v 1.2 87/01/27 16:17:48 mat Exp $";

getline(buf, fp)
register char	*buf;
register FILE	*fp;
{
	register int	cnt = 0;
	register char	c, *p;

	for (p=buf; (c=getc(fp)) != EOF && c != EOL;  ) {
		if ( ++cnt == I_BUFSIZ ) {
			return( LINETOOLONG );
		}
		*p++ = c;
	}
	*p = NULL;

	/*
	 * Remove any trailing spaces or tabs.
	 */
	while ( cnt ) {
		--p;
		if ( *p == ' ' || *p == '\t' ) {
			*p = NULL;
		} else break;
	}
	DEBPRT2("getline: [%d] %s\n", strlen(buf), buf);

	/* if the end of line was reached, return success */
	return( c == EOL ? 0 : c );
}
