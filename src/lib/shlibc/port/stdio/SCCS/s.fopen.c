h59953
s 00073/00000/00000
d D 1.1 86/07/31 11:42:44 fnf 1 0
c Initial copy from 5.3 distribution for 3b2.
e
u
U
t
T
I 1
/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libc-port:stdio/fopen.c	1.13"
/*LINTLIBRARY*/
#include "shlib.h"
#include "stdio.h"
#include <fcntl.h>

extern int open(), fclose();
extern long lseek();
extern FILE *_findiop(), *_endopen();

FILE *
fopen(file, mode)
char	*file, *mode;
{
	return (_endopen(file, mode, _findiop()));
}

FILE *
freopen(file, mode, iop)
char	*file, *mode;
register FILE *iop;
{
	(void) fclose(iop); /* doesn't matter if this fails */
	return (_endopen(file, mode, iop));
}

static FILE *
_endopen(file, mode, iop)
char	*file, *mode;
register FILE *iop;
{
	register int	plus, oflag, fd;

	if (iop == NULL || file == NULL || file[0] == '\0')
		return (NULL);
	plus = (mode[1] == '+');
	switch (mode[0]) {
	case 'w':
		oflag = (plus ? O_RDWR : O_WRONLY) | O_TRUNC | O_CREAT;
		break;
	case 'a':
		oflag = (plus ? O_RDWR : O_WRONLY) | O_APPEND | O_CREAT;
		break;
	case 'r':
		oflag = plus ? O_RDWR : O_RDONLY;
		break;
	default:
		return (NULL);
	}
	if ((fd = open(file, oflag, 0666)) < 0)
		return (NULL);
	iop->_cnt = 0;
	iop->_file = fd;
	iop->_flag = plus ? _IORW : (mode[0] == 'r') ? _IOREAD : _IOWRT;
	if (mode[0] == 'a')   {
		if (!plus)  {
			/* if update only mode, move file pointer to the end
			   of the file */
			if ((lseek(fd,0L,2)) < 0)  {
				return NULL;
			}
		}
	}
	_bufend(iop) = iop->_base = iop->_ptr = NULL;
	return (iop);
}
E 1
