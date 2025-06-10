/*
 *	Copyright (c) 1984, Communication Machinery Corporation
 */

/*
 *	This file contains material and information which is
 *	proprietary to Communication Machinery Corporation, and
 *	which may not be divulged without the express written
 *	permission of Communication Machinery Corporation.
 */

/*
 * This program downloads Motorola S-record format files into the ENP
 * and causes the ENP to start executing them.  It is structured to support
 * any ENP on any of the systems we know about.  The differences between
 * systems and ENPs are handled by setting various defined values described
 * below.
 *
 * Usage is:
 *	ldenp [-l|-x] enp<n> file ...
 * where
 *	-l says to load ENP but not start executing
 *	-x says to start execution at entry point of last file, but don't
 *	   do a download
 *	-r reset ENP regardless of socket status
 *	enp<n> is exactly one of "enp0" or "enp1"
 * There may be any number of files, all of which must be in S-record format.
 *
 * Assumptions: Bytes are at least 8 bits.
 *	Byte order between chars, ints, and longs are handled properly.
 *
 * Several values depend on the ENP being loaded, and the particular system
 * we're running on.  These values are specified as defines at compile
 * time.  These defines, and their default values are:
 *		ENP_DEV	"/dev/enpram"
 *			Name of ENP device
 *		ENP0	0x000000
 *			Host's address of first ENP.  Generally will be 0
 *			for systems which use /dev/enpram.  Systems which use
 *			/dev/kmem to access ENP memory should define ENP0 to
 *			be 0x1000 plus the base address of the ENP in the
 *			host's memory space.
 *		ENP1	0x000000
 *			Host's address of second enp (if any).  See note for
 *			ENP0.  (Currently unused)
 *		ENP_RAM	0xF01000
 *			Enp's internal RAM address plus 0x1000.
 *
 * May 6, 1984 - Modified to use accept	S-record formats.
 *						Deb Brown, CMC
 * Aug 25, 1986 - add check for open sockets. Abort reset unless force
 *		  option is active. - dtk
 */
/**/
#include <fcntl.h>
#include <stdio.h>
#include <CMC/enpram.h>

# define TRUE	-1
# define FALSE	 0
# define MXBUF	512
# define dcd(p,i)	(dcd_tab[((p)[i])&0x7F])
# define dcode(p)	((dcd(p,0) << 4) + dcd(p,1))

/* -------------------- E N P - S P E C I F I C S --------------------- */

# ifndef ENP0
#	define	ENP0	0x0		/* ENP 0's memory		*/
# endif

# ifndef ENP1
#	define	ENP1	0x0		/* ENP 1's memory		*/
# endif

# ifndef ENP_RAM
#	define	ENP_RAM	0xF01000	/* ENP's internal RAM addr	*/
# endif

# ifndef ENP_DEV
#	define	ENP_DEV	"/dev/enpram"
# endif
/**/
/* ---------------- S T R U C T U R E S   &   T A B L E S ------------- */

/*
 *		Structure needed to read S-record file
 */

struct S_rec
{
	long	saddr;		/* where to write this info		*/
	long	scnt;		/* number of bytes to write		*/
	char	*sbuf;		/* buffer for the data			*/
};

/*
 *		ENP "device" table (so we can load multiple ENPs)
 *		See ENP/xx User Guide
*/

typedef struct	enp
{
	char	name[8];		/* Device's "name"		*/
	long	mstart;			/* Host's idea of enp mem org	*/
} ENP_TAB;

ENP_TAB	enp_tab[] = {
	{ "enp0", ENP0, },
     /* { "enp1", ENP1, }, */
};

int dcd_tab [] = {
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 0, 0, 0, 0, 0,
	0,10,11,12,13,14,15, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0,10,11,12,13,14,15, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};

/* -------------------- G L O B A L S --------------------------------- */

char		buf[MXBUF];			/* r/w buffer		*/
char		cbuf[MXBUF*2];			/* read data as chard	*/
int		doex = 1;			/* assume will execute	*/
int		dold = 1;			/* assume doing load	*/
unsigned long	enp_go = 0;			/* Start up vector	*/
short		enpfid = -1;			/* dev "enpram" fid	*/
FILE		*enpsid;			/* enpram as stream	*/
char		enpsbuf [BUFSIZ];		/* stream output buffer	*/
ENP_TAB		*ep;				/* enp_tab pointer	*/
char		*file_name;			/* name of current file	*/
ENP_TAB		*getenp();
int		line_num = 1;			/* line in Srec files	*/
short		num_enp = sizeof(enp_tab) / sizeof(enp_tab[0]);
char		*prog_name;			/* name of this program	*/

FILE		*fopen ();
FILE		*fdopen ();
/**/
/* -------------------- M A I N --------------------------------------- */

main( argc,argv )
int	argc;
char	**argv;
{
	register short	fcnt,i;		/* Files to load		*/
	register int	force_reset = 0;/* flag for forced reset 	*/
	register int	cnt;		/* I/O count			*/
	FILE		*in_sid;	/* stream for input file	*/
	static char	insbuf[BUFSIZ];	/* input buffer for stream	*/


/* Check for options */

	prog_name = argv[0];
	if (argc > 1)
	{
		++argv;
		--argc;
		while (argv[0][0] == '-')
		{
			switch (argv[0][1])
			{
			case 'x': dold = 0; break;
			case 'l': doex = 0; break;
			case 'r': force_reset = 1; break;
			}
			++argv;
			--argc;
		}
	}
	if ((!dold) && (!doex))
		fatal ("Only one of -x and -l can be specified\n");

/* Check usage, that can access ENP, etc. */

	if( argc <= 1 )
		fatal( "usage: enpload [-x|-l] dev file...\n" );
	if( (enpfid = open(ENP_DEV, O_WRONLY)) == -1 )
		fatal( "Can't access %s!\n", ENP_DEV );
	enpsid = fdopen (enpfid, "w");
	setbuf (enpsid, enpsbuf);
	if( (ep = getenp( *argv++ )) == 0 )
		fatal( "Bad ENP device name!\n" );
	--argc;

/* Reset ENP. */

/* if reset return error - outstanding sockets would lose context from
   reset - abort reset after message
   */
	if (dold) {
		if (reset_enp(force_reset) == -1) {
			fprintf(stderr,"Outstanding Sockets - reset Aborted\n");
			exit(1);
		}
	}

/* Read each input file, and write it to ENP */

	for( i = 0; i < argc; i++ )
	{
		file_name = argv [i];
		if( (in_sid = fopen( file_name,"r" )) == (FILE *) -1 )
			fatal( "Can't open %s!\n",file_name );
		setbuf( in_sid, insbuf );
		if(( buf[0] = getc( in_sid )) == -1)
			fatal( "Can't read %s!\n",file_name );
		if( buf[0] != 'S' )
			fatal( "%s not an S-record file\n",file_name );
		do_sfile( in_sid, enpfid );
		fclose( in_sid );
		if( !dold )
			printf( "...Execute only; no load\n" );
	}
	fflush (enpsid);

/*	Last file, we exec this one	*/

	if( doex )
	{
		printf( "Starting ENP execution at %X ... ",enp_go );
		start_enp();
		printf( "done!\n" );
	}
	else
		printf( "...Load only; no execute\n" );
	exit(0);
}

/* -------------------- C H K S U M ----------------------------------- */
/*
 * Calc Srec-type check sum
 * Usage:	if (chksum (cnt, ptr)) error;
 * Returns:	0 - match
 *		-1 - differ
 */

chksum (cnt, ptr)
int	cnt;
char	*ptr;
{
int	sum;
int	csum;

	if (cnt <= 0)
		return (-1);
	sum = cnt;
	csum = ptr[--cnt];
	for (; cnt-- > 0; ++ptr)
		sum += *ptr;
	if (((~sum) & 0xFF) != (csum & 0xFF))
		return (-1);
	else
		return (0);
}

/* -------------------- D C D _ A D D R ------------------------------- */
/*
 * Build a long from bytes in a char array.
 * Usage:	result = dcd_addr (buf, lth);
 */

long
dcd_addr (buf, lth)
register char	*buf;
register int	lth;
{
register long	l;

	for (l = 0; lth > 0; --lth)
		l = (l << 8) + ((*buf++) & 0xFF);
	return (l);
}

/* -------------------- D O _ S F I L E ------------------------------- */
/*
 * Download a file in S-record format.
 *
 * Takes stream (FILE *) for input file, and file descriptor for
 * output (ENP) file.
 */

do_sfile (isid, ofid)
FILE	*isid;
int	ofid;
{
long	num;			/* num data bytes read */
long	do_srec ();
struct S_rec	srec;		/* struct for holding decoded S record */

	printf ("\n%s:\n", file_name);
	fseek (isid, (long) 0, (int) 0);	/* rewind input file */
	srec.sbuf = buf;			/* set up buf to use */
	while (TRUE)				/* read thru file */
	{
		if ((num = get_srec (isid, &srec)) < 0)
			break;
		if (dold && srec.scnt > 0)
			write_enp (ofid, srec.sbuf, srec.scnt,
			    srec.saddr - ENP_RAM + ep -> mstart);
		++ line_num;
	}
	write_enp (ofid, srec.sbuf, 0, 0);
}

/* -------------------- F A T A L ------------------------------------- */
/*
 * Print our error message and quit
 */

fatal( fmt, parm )
char	*fmt;
char	*parm;
{
	fprintf (stderr, fmt, parm);
	exit (1);
}

/* -------------------- G E T _ S R E C ------------------------------- */
/*
 * Read and decode one record from S-rec file.
 * Usage:	get_srec (isid, s_rec);
 * Returns	0 - record OK
 *		-1 - hit EOF
 * Also returns values in s_rec structure.
 * Aborts processing on any syntax error in s-record.
 */

get_srec (isid, s_rec)
FILE	*isid;
struct S_rec	*s_rec;
{
register int	cnt;		/* num bytes in record			*/
	 int	type;		/* type of addr field; 1->4 bytes, 2->6 */
	 int	sz;		/* size of addr field			*/
register int	i;

	while (((buf [0] = getc (isid)) != -1) && buf[0] != 'S');
	if (buf[0] != 'S')		/* check for EOF		*/
		return (-1);
	if (fread (buf, sizeof (char), 3, isid) != 3) /* type and count	*/
		gsr_err ("Short line");
	type = dcd (buf,0);
	cnt = dcode (&(buf[1]));
	if (fread (cbuf, sizeof (char), 2*cnt, isid) != 2*cnt)	/* data	*/
		gsr_err ("Short data");
	for (i = 0; i < cnt; ++i)	/* convert data to binary	*/
		buf[i] = dcode (&(cbuf[2*i]));
	if (chksum (cnt, buf))		/* verify checksum		*/
		gsr_err ("Checksum");
	switch (type)
	{
	case 0:				/* Signon card; ignore it	*/
		s_rec -> scnt = 0;
		break;
	case 1:				/* data card; 4 digit address	*/
	case 2:				/* data card; 6 digit address	*/
		sz = type == 2 ? 3 : 2;
		s_rec -> saddr = dcd_addr (buf, sz);
		s_rec -> sbuf = buf + sz;
		s_rec -> scnt = cnt - 1 - sz;
		break;
	case 8:
		s_rec -> scnt = 0;
		enp_go = dcd_addr (buf, 3);
		break;
	default:
		gsr_err ("Bad type");
	}
	return (0);
}

/* -------------------- G E T E N P ----------------------------------- */

ENP_TAB *
getenp( np )
register char	*np;
{
	register ENP_TAB	*ep;

	for( ep = enp_tab; ep < &enp_tab[num_enp]; ep++ )
	if( strcmp( np,ep->name ) == 0 )
		return( ep );
	return( 0 );
}

/* -------------------- G S R _ E R R --------------------------------- */

gsr_err (msg)
char	*msg;
{
	fprintf (stderr, "ERROR: %s - %s, line %d\n", msg, file_name, line_num);
	exit (1);
}

/* -------------------- R E S E T _ E N P ----------------------------- */
/*
 * Reset the ENP.
 */

reset_enp (force)
{
	if (force)
		ioctl (enpfid, ENPIOC_FRESET, 0);
	else
		ioctl (enpfid, ENPIOC_RESET, 0);
}

/* -------------------- S T A R T _ E N P ----------------------------- */

/* Cause the ENP to start executing it's code */

start_enp()
{
	ioctl( enpfid, ENPIOC_JUMP, enp_go );
}

/* -------------------- W R I T E _ E N P ----------------------------- */
/*
 * Do an (internally) buffered write to the enp.
 *
 * Takes file descriptor for ENP file, ptr to output buf, length of buf,
 * and position in the file where this buffer is to be written.
 */

write_enp (fid, buf, lth, loc)
int fid, lth;
char *buf;
long loc;
{
static char	my_buf [MXBUF];
static int	my_lth = 0;
static long	my_loc = 0;

	if ((loc != my_loc + my_lth) || (my_lth + lth > MXBUF))
	{
		lseek (fid, my_loc, 0);
		if (write (fid, my_buf, my_lth) != my_lth)
		{
			perror (file_name);
			exit (1);
		}
		my_loc = loc;
		my_lth = 0;
	}
	if (my_lth + lth <= MXBUF)
	{
		memcpy (my_buf + my_lth, buf, lth);
		my_lth += lth;
	}
	else
	{
		lseek (fid, loc, 0);
		if (write (fid, buf, lth) != lth)
		{
			perror (file_name);
			exit (1);
		}
		my_loc = loc + lth;
	}
}
