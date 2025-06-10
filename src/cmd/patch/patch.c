	/* Patch disk program  -- copyright (c) 1984 Motorola  */

	/*
		Written by Lyndle Cummings -- ISO -- Motorola

		20-FEB-83:  Started.
		16-APR-83:  Add ability to handle read only files
		14-JUL-84:  Extend to ISOS
		28-JUL-84:  Continue extention to ISOS

	*/
#include <stdio.h>
#include <errno.h>

#define STDIN  0
#define STDOUT 1
#define STDERR 2

extern
int     errno;

int     fid     = 0,            /* File ID */
	ioflag  = 0;            /* Read only flag */

#ifdef isos
#include "ddnine.h"

extern LONG *lseek9();

#else
#include "dd68.h"

#define LONG long int

extern  LONG lseek();
#endif




















main(argc,argv)
	int   argc;
	char *argv[];
{
	int     rstat;

#ifdef isos
	brako();
#endif

	nulin();
	nulin();

		/* Open file on command line, if requested */
	if(argc > 1)
		openx(argv[1]); /* Open the file */
	else
		fid = -1;       /* No file is open */

	for(;;)                 /* Loop until finished */
	{
		write(STDERR,"> ",3);
#ifdef isos
		rstat = READLN(STDIN,inbuf,128);
#else
		rstat = read(STDIN,inbuf,128);
#endif
		if((rstat == 0) || (rstat == -1))
			exit(errno);

		switch(inbuf[0])
		{
		case 'o':  ;
		case 'O':  openfx();
			   break;

		case 's':  ;
		case 'S':  showfx();
			   break;

		case 'c':  ;
		case 'C':  chngfx();
			   break;

		case 'd':  ;
		case 'D':  filst();
			   break;

		case 'q':  ;
		case 'Q':  exit(0);

		default:
			nulin();
			twrite(msgpr1);
			twrite(msgpr2);
			twrite(msgpr3);
			twrite(msgpr4);
			twrite(msgpr5);
			twrite(msgpr6);
			nulin();
		}
	}
}
twrite(pntr)            /* Write a message to the terminal */
	char  *pntr;
{
	write(STDOUT,pntr,strlen(pntr));
}


nulin()                 /* Send a new-line to terminal */
{
#ifdef isos
	twrite("\r");
#else
	twrite("\n");
#endif
}


openfx()                /* Open a new file */
{
	char    *ptr,
		*namptr;

	if(fid != -1)
		close(fid);             /* Close old file */

	ptr = &inbuf[1];

	while(*ptr == ' ')              /* Move over spaces */
		++ptr;

	namptr = ptr;                   /* Save this value */

#ifdef isos
	while(*ptr != '\r')             /* Find end */
		++ptr;
#else
	while(*ptr != '\n')             /* Find end */
		++ptr;
#endif

	*ptr = 0;                       /* Null terminate string */

	openx(namptr);                  /* Open new file */
}

/* Attempt to open for update.  If this is not possible, attempt
   to open for input only.  If this is possible set "ioflag" */
openx(namptr)
	char    *namptr;
{
	ioflag = 0;
	fid = open(namptr,2);           /* Open for update */
	if(fid == -1)
	{
		ioflag = 1;             /* Set read-only flag */
		fid = open(namptr,0);   /* Open for input */
		if(fid == -1)
			printf(msgopn,errno);
		else
		{
			twrite(msgexc); /* Read only permission */
		}
	}
}

char
rtnhex(inchar)
	char    inchar;
{
	char    rtnchar;

	rtnchar = inchar & 0x0F;
	if(rtnchar > 9)
		rtnchar += 'A' - 10;
	else
		rtnchar += '0';
	return(rtnchar);
}
