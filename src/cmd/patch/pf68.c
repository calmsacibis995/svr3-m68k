/***********************************************************/
/*  Special files for UNIX                                 */
/***********************************************************/

#include <sys/types.h>
#include <sys/stat.h>

#define LONG long int
#define STDIN  0
#define STDOUT 1

extern  int
	errno,
	fid,
	ioflag;

extern  char
	inbuf[],
	dkbuf[],
	otbuf[],
	msgnof[],
	msgexc[],
	msginv[],
	msgskk[],
	msgwrt[],
	msgttl[],
	msgrdd[];

char
	msgdir[]  = "Directory",
	msgchr[]  = "Character special",
	msgblk[]  = "Block special",
	msgreg[]  = "Regular",
	msgfifo[] = "Fifo",
	msgunk[]  = "Unknown";





















	/* GETHEX()
		Converts a hex value to a long integer.
		It sets errno to non-zero if it encounters
		a non-hex character except space and new-line.
	*/

char *
gethex(pntr,lpnt)
	char    *pntr;
	LONG    *lpnt;
{
	char     tchar;
	LONG     ltemp;

	ltemp = 0;                      /* Initialize return value */
	while(*pntr == ' ')             /* Move over spaces */
		++pntr;

	for(;;)
	{
		tchar = *pntr & 0x7F;   /* Get next character */
		if((tchar >= '0') && (tchar <= '9'))
			tchar -= '0';
		else
		{
			if((tchar >= 'A') && (tchar <= 'F'))
				tchar += (10 - 'A');
			else
			{
				if((tchar >= 'a') && (tchar <= 'f'))
					tchar += (10 - 'a');
				else
				{
					break;  /* Non-hex */
				}
			}
		}
		ltemp <<= 4;
		ltemp += tchar;
		++pntr;
	}

	if((*pntr == ' ') || (tchar == '\n'))
		errno = 0;              /* Success */
	else
		errno = 1;              /* Bad */

	*lpnt = ltemp;
	return(pntr);
}






chngfx()                        /* Change bytes */
{
	int      ii,
		 rstat;
	LONG     lstart, lstat,
		 ltemp;
	char    *p1,
		*p2;

	if(fid == -1)
	{
		twrite(msgnof);
		return;
	}

	if(ioflag)              /* Check for read only */
	{
		twrite(msgexc);
		return;
	}

	p1 = &inbuf[1];
	p2 = gethex(p1,&lstart);
	if(p1 == p2)
	{
		twrite(msginv);
		return;
	}

	ii = 0;
	for(;;)
	{
		p1 = p2;
		p2 = gethex(p1,&ltemp);
		if(p1 == p2)
			break;
		dkbuf[ii] = ltemp;
		++ii;
	}

	if(ii == 0)
	{
		twrite(" No change\n");
		return;
	}

	if(errno)
	{
		twrite(msginv);
		return;
	}

		/* Input was OK, so write to disk */

	lstat = lseek(fid,lstart,0);
	if(lstat == -1)
	{
		printf(msgskk,errno);
		return;
	}

	rstat = write(fid,dkbuf,ii);
	if(rstat != ii)
	{
		printf(msgwrt,errno);
	}
}


showfx()                /* Display (show) */
{
	LONG    lstart,
		lstop,
		lstat,
		ltemp;

	char    *p1,
		*p2,
		 tchar;

	int     ii,
		jj,
		shift,
		rstat;

	if(fid == -1)
	{
		twrite(msgnof);
		return;
	}

		/* Get start address */

	p1 = &inbuf[1];
	p2 = gethex(p1,&lstart);
	if(p1 == p2)
	{
		twrite(msginv);
		return;
	}
	p1 = p2;

		/* Get ending address */

	p2 = gethex(p1,&lstop);
	if(p1 == p2)
	{
		lstop = lstart;
	}

		/* Normalize start and ending addresses */

	lstart &= 0xFFFFFFF0;
	lstop  |= 0xF;
	ltemp = lstart;

		/* Seek to starting slot */

	lstat = lseek(fid,lstart,0);
	if(lstat == -1)
	{
		printf(msgskk,errno);
		return;
	}

		/* Make title */

	nulin();
	twrite(msgttl);
	otbuf[78] = '\n';
	mfill(otbuf,78,'-');
	write(STDOUT,otbuf,79);
	nulin();

		/* Loop through */

	while(ltemp < lstop)
	{
		mfill(otbuf,78,' ');            /* Fill spaces */
		rstat = read(fid,dkbuf,16);
		if(rstat == -1)
		{
			printf(msgrdd,errno);
			return;
		}

			/* Current address to output buffer */
		shift = 28;
		for(ii = 0;ii<9; ++ii)
		{
			if(ii != 4)             /* Leaves a space */
			{
				tchar = ltemp >> shift;
				otbuf[ii] = rtnhex(tchar);
				shift -= 4;
			}
		}

			/* Write out the 16 bytes */

		if(rstat)       /* If not end-of-file */
		{
			jj = 11;
			for(ii = 0; ii < rstat; ++ii)
			{
				tchar = dkbuf[ii] >> 4;
				otbuf[jj] = rtnhex(tchar);
				tchar = dkbuf[ii];
				otbuf[jj + 1] = rtnhex(tchar);
				jj += 3;
			}

				/* Write out as ASCII */

			jj = 61;
			for(ii = 0; ii < rstat; ++ii)
			{
				tchar = dkbuf[ii] & 0x7F;
				if((tchar < ' ') || (tchar > '~'))
				tchar = '.';
				otbuf[jj] = tchar;
				++jj;
			}

			write(STDOUT,otbuf,79);

		}
		ltemp += 16;

		if(rstat != 16)
		{
			twrite("End of file detected\n");
			return;
		}
	}
}


mfill(pntr,count,valu)
	char    *pntr;
	int      count;
	char     valu;
{
	while(count)
	{
		*pntr = valu;
		++pntr;
		--count;
	}
}


	/* Displays the characteristics of a file */


struct stat sbuf;

filst()
{
	register int   ii;
	register char *mptr;

	if(fid == -1)
	{
		twrite(msgnof);
		return;
	}

	if(fstat(fid,&sbuf) == -1)
	{
		printf("Status error: %d\n",errno);
		return;
	}

	ii = sbuf.st_mode;
	switch(ii & S_IFMT)             /* Get file type */
	{
	case S_IFDIR:   mptr = msgdir;  /* Directory */
			break;
	case S_IFCHR:   mptr = msgchr;  /* Character special */
			break;
	case S_IFBLK:   mptr = msgblk;  /* Block special */
			break;
	case S_IFREG:   mptr = msgreg;  /* Regular */
			break;
	case S_IFIFO:   mptr = msgfifo; /* Fifo */
			break;
	default:        mptr = msgunk;
	}
	printf("Type of file:   %s\n",mptr);

	printf("Permissions:    %o\n",ii & 0777);

	ii = sbuf.st_dev;
	printf("Device:         %d\n",ii);

	ii = sbuf.st_ino;
	printf("Inode:          %d\n",ii);

	ii = sbuf.st_nlink;
	printf("Links:          %d\n",ii);

	ii = sbuf.st_uid;
	printf("User ID:        %d\n",ii);

	ii = sbuf.st_gid;
	printf("Group ID:       %d\n",ii);

	ii = sbuf.st_rdev;
	printf("R-Device:       %d\n",ii);

	ii = sbuf.st_size;
	printf("Size:           %d\n",ii);
}
