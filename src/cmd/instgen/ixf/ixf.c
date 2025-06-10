#include	<stdio.h>
#include	<string.h>
#include	<ctype.h>
#include	<fcntl.h>
#include	<sys/types.h>
#include	<sys/stat.h>
#include	<sys/mvme350.h>
/*		<sys/dk.h> included by common.h> */
#include	"../com/common.h"

#ident	"$Header: /grp/mat/53/utils/ixf/RCS/ixf.c,v 1.3 87/01/27 16:10:43 mat Exp $";

#define		CANT_REWIND	2
#define		SCRIPTS_YES	13
#define		FILES		1
#define		OTHER		2
#define		SCR_DIR		"/tmp"

/*
 * argument to extract() to define which archives to bring in.
 */
#define		X_ALL		1
#define		X_SCRIPTS	2

/*
 * Modes for opening the tape device.
 */
#define		POS_ONLY	0

#define	TRUNC	(O_RDONLY|O_TRUNC)
#define	APPEND	(O_RDONLY|O_APPEND)

/*
 * Information about files to bring in off of media.
 */
struct archive {
	struct archive	*next;
	unsigned char	number;			/* media file number */
	unsigned char	arcno;			/* archive number */
	char	*archiver;
	char	*i_opt;
	char	*dest_dir;
	char	*a_desc;
};

/*
 * Global variables for ixf(1M)
 */
#ifdef DEBUG
int	Debug;
#endif /* DEBUG */
char	*Nomemmsg = { "%s: out of memory" };
char	*myname = 0;				/* name program running as */
char	Io_buf[T_BSIZE];			/* output I/O buffer */
unsigned char	Selrec[256];			/* file numbers to bring in */
/* Volume id block of tape */
static struct vidblk	Vid;
static struct t_dir	Tdir;
static struct archive	*Ar_head;
static struct archive	*O_head;
/* device or special file , file desc */
static int	sfd = 0;
static int	Allflag = TRUE;			/* bring in ALL archives */
static int	Scripts_ent = FALSE;		/* bool: scripts ent on media */

/*
 * Global command line options.
 */
int	Verbose  = FALSE;			/* -v */
int	Tflag = FALSE;				/* -t option */
int	Lflag = FALSE;				/* -l option */
int	Mflag = FALSE;				/* -m option */
char	*Tmpdir = SCR_DIR;			/* defaule for -s option */
char	*Device  = { "/dev/r41an" };		/* default output device */

static char *Usage = {
    "Usage: %s [-Vvptdncax] [-{l|m} file_list] [-s script_dir] special_file\n"
};


char	*sub_device();

extern int	errno;
extern int	optind;
extern char	*optarg;

extern char	*malloc();

main(argc,argv)
int	argc;
char	*argv[];
{
	register char	*p;
	register int	let;		/* option letter */
	register struct volumeid *vp = &Vid.vid;
	int	exitflg = FALSE,
		verflag = FALSE,	/* -V option */
		nflag = FALSE,		/* -V option */
		cflag = FALSE,		/* -c option */
		pflag = FALSE,		/* -p option */
		xflag = FALSE,		/* -x option */
		dflag = FALSE;		/* -d option */
	struct stat	stbuf;		/* for checking of -s option */

	/*
	 * make program name instead of pathlist
	 */
	if ( (p=strrchr(argv[0], '/')) == NULL ) {
		myname = argv[0];
	}
	else {
		myname = p + 1;
	}

#ifdef DEBUG
	while ( (let=getopt(argc, argv, "pcVvtdm:l:nas:xD")) != EOF ) {
#else
	while ( (let=getopt(argc, argv, "pcVvtdm:l:nas:x")) != EOF ) {
#endif
		switch(let) {
			case 'V':
				/* show version of media */
				verflag = TRUE;
				break;
			case 'p':
				/* show package name */
				pflag = TRUE;
				break;
			case 'x':
				/*
				 * Use special exit value if Scripts
				 * entry exists on media.
				 */
				xflag = TRUE;
				break;
			case 'v':
				/* verbose mode */
				Verbose = TRUE;
				break;
			case 't':
				/* show table of contents of tape */
				Tflag = TRUE;
				break;
			case 'd':
				/* show volume description */
				dflag = TRUE;
				break;
			case 'l':
				if ( Lflag || Mflag ) {
					fprintf(stderr,"%s: -m and -l options can only be used once\n", myname);
					exitflg = TRUE;
					break;
				}
				Allflag = FALSE;
				Lflag = TRUE;
				/* use this list of archive files to bring in */
				if(parselopt(optarg, INT(Vid.vid.vid_dirents)))
					exitflg = TRUE;
				break;
			case 'm':
				if ( Lflag || Mflag ) {
					fprintf(stderr,"%s: -m and -l options can only be used once\n", myname);
					exitflg = TRUE;
					break;
				}
				Mflag = TRUE;
				Allflag = FALSE;
				/* use this list of archive files to bring in */
				if(parselopt(optarg, INT(Vid.vid.vid_dirents)))
					exitflg = TRUE;
				break;
			case 'c':
				/* full table of contents */
				cflag = TRUE;
				break;
			case 'a':
				/* all information */
				Tflag = verflag = pflag = dflag = cflag = TRUE;
				break;
			case 'n':
				/* do not rewind tape before exiting */
				nflag = TRUE;
				break;
			case 's':
				/* directory for Scripts */
				if ( stat(optarg, &stbuf)==0 && ISDIR(stbuf) ) {
					Tmpdir = optarg;
				}
				else {
					fprintf(stderr,"%s: directory for -s option does not exist\n",
					    myname, optarg);
					exitflg = TRUE;
				}
				break;
#ifdef DEBUG
			case 'D':
				/* turn on debugging if compiled in */
				Debug = TRUE;
				break;
#endif
			case '?':
				exitflg = TRUE;
				break;
			default:
				break;
		}
		if (exitflg) {
			break;
		}
	}

	if ( exitflg || argc != (optind + 1) ) {
		fprintf(stderr,Usage,myname);
		exit(1);			/* syntax error */
	}

	Device = argv[optind];

	/*
	 * Rewind tape to make sure we are at the beginning
	 * so the VID can be read.
	 */
	if (rewind_t() == ERROR) {
		exit(2);
	}

	/*
	 * Read volume id from media for information about the
	 * rest of the tape.
	 */
	if ( getvid( &Vid ) == ERROR ) {
		if ( ! nflag ) {
			rewind_t();f ( cflag ) {
		if (Verbose) {
			printf("Creation Date - ");
		}
		printf("%s", ctime(vp->vid_ctime));
	}

	/*
	 * If any of the above information was printed, and a table
	 * of contents was not requested, we will exit here so no
	 * unneccessary is done.
	 */
	if ( (cflag || dflag || pflag || verflag) && !Tflag ) {
		rewind_t();
		exit( getexitval(0, xflag, &Vid) );
	}

	/*
	 * Get information about all or selected (-l opt) files
	 * from the media directory.
	 */
	if ( getmedia_dir( &Vid ) == ERROR ) {
ption - ");
			}
			prtchrs(vp->vid_desc, DESCSIZ);
			putchar('\n');
		}
		else {
			if (Verbose) {
				printf("No Volume Description\n");
			}
		}
	}

	/*
	 * Print out version in the same manner as the volume desc.
	 */
	if ( verflag ) {
		if ( vp->vid_version != NULL ) {
			if (Verbose) {
				printf("Version - ");
			}
			prtchrs(vp->vid_version, VERSIZ);
			putchar('\n');
		}
		else {
			if (Verbose) {
				printf("No Version\n");
			}
		}
	}

	/*
	 * Print out time the install media was created
	 */
	if ( cflag ) {
		if (Verbose) {
			printf("Creation Date - ");
		}
		printf("%s", ctime(vp->vid_ctime));
	}

	/*
	 * If any of the above information was printed, and a table
	 * of contents was not requested, we will exit here so no
	 * unneccessary is done.
	 */
	if ( (cflag || dflag || pflag || verflag) && !Tflag ) {
		rewind_t();
		exit( getexitval(0, xflag, &Vid) );
	}

	/*
	 * Get information about all or selected (-l opt) files
	 * from the media directory.
	 */
	if ( getmedia_dir( &Vid ) == ERROR ) {
		rewind_t();
		exit(2);
	}

	if ( Tflag ) {
		/*
		 * Print table of contents. If doing TOC, not
		 * extracting archives.
		 */
		prt_toc();
	}
	else {
		exitflg = extract( ((!Scripts_ent||Mflag||Lflag)?X_ALL:X_SCRIPTS) );
	}

	if ( ! nflag ) {
		rewind_t();
	}

	/*
	 * Exit value decided on these factors:
	 *	-x option used
	 *	SCRIPTS entry on media
	 *	has an error occurred
	 */
	exit( getexitval(exitflg, xflag, &Vid) );
}

getexitval(err, xflg, vidp)
int	err, xflg;
struct vidblk	*vidp;
{
	/*
	 *  If there is an error (err), return value of 1, otherwise
	 *  see if user requested -x option asking for special exit code
	 *  signifying whether or not a SCRIPTS entry is on the media. If
	 *  this option requested and there is a SCRIPTS entry on the media
	 *  return that special value (SCRIPTS_YES) otherwise return zero.
	 */
	if ( err ) {
		return(1);
	}
	else if ( xflg && (*vidp->vid.vid_infoflg & I_SCRIPT) ) {
		return(SCRIPTS_YES);
	}
	else {
		return(0);
	}
}
getvid( vid )
struct vidblk	*vid;
{
	int	fd;

	if ( (fd=opensp(M_TRUNC, 0)) == ERROR ) {
		return(ERROR);
	}
	DEBPRT1("getvid: file descriptor %d\n", fd);

	DEBPRT3("getvid: read(%d, %#08x, %d)\n",
	    fd, vid, sizeof(struct vidblk));
	if ( read(fd, vid, sizeof(struct vidblk)) == ERROR ) {
		fprintf(stderr,"%s: cannot read volume id block\n",myname);
		DEBPRT1("getvid: read errno = %d\n", errno);
		closesp();
		return(ERROR);
	}

	closesp();

	DEBPRT1("getvid: mot string [%s]\n", vid->vid.vid_mac);
	if ( ! EQ_N("MOTOROLA", vid->vid.vid_mac, 8) ||
	     ! EQ_N("UNIX", &vid->vid.vid_vol, 4) ) {
		fprintf(stderr,"%s: invalid volume id block\n", myname);
		return(ERROR);
	}

	if ( vid->vid.vid_dirtype[0] != D_NEWF ) {
		fprintf(stderr,
		    "%s: media made with incompatable version of igf(1M)\n",
		    myname);
		return(ERROR);
	}

	return(0);
}

getmedia_dir( vidblk )
struct vidblk	*vidblk;
{
	register struct volumeid	*vp = &vidblk->vid;
	register int	i, max, fd, cnt;
	int	dirf;

	if ( *vp->vid_infoflg & I_BOOT ) {
		dirf = 2;
	}
	else dirf = 1;

	DEBPRT1("getmedia_dir: opening media directory file %d\n", dirf);
	if ( (fd=opensp(M_APPEND, dirf)) == ERROR ) {
		fprintf(stderr,"%s: cannot open media directory\n");
		return(ERROR);
	}

	for ( i=0, max=INT(vp->vid_dirents); i < max; i++ ) {
		if ( (cnt=read(fd, &Tdir, sizeof(struct t_dir))) == ERROR ) {
			fprintf(stderr,"%s: cannot read media directory\n");
			closesp();
			return(ERROR);
		}
		DEBPRT1("getmedia_dir: dirent type is %d\n", Tdir.type);
		if ( Tdir.type == NULL ) {
			/* empty entry */
		}
		else if ( Tdir.type != D_ARC && Tdir.type != D_SCR ) {
			if ( Verbose ) {
				sav_ent(&Tdir, OTHER);
			}
		}
		else {
			if ( inlist(Tdir.arnum) ) {
				DEBPRT1("getmedia_dir: saving archive # %d\n",
				    Tdir.number);
				if ( Mflag ) {
					mod_opts( &Tdir );
				}
				if ( sav_ent(&Tdir, FILES) == ERROR ) {
					closesp();
					return(ERROR);
				}
				if ( Tdir.type == D_SCR ) {
					Scripts_ent = TRUE;
				}
			}
		}
	}
	closesp();
	return(0);
}

mod_opts( tp )
struct t_dir	*tp;
{
	char		ibuf[D_OPTSZ];

	/* heading info */
	printf("\nArchive number: %d\t\"%s\"%s", tp->arnum,
	    (tp->type==D_SCR) ? SCR_DIR : tp->a_desc,
	    (tp->type==D_SCR) ? " (Scripts)\n" : "\n");
	printf("Archive utility: %s\n", tp->name);

	/* change destination directory ? */
	printf("  Destination directory (default = %s): ",
	    (tp->type==D_SCR) ? SCR_DIR : tp->a_desc);
	if ( fgets(ibuf, D_PATHSZ, stdin) == NULL ) {
		return(0);
	}
	savstring(tp->dest_dir, ibuf);

	/* change input options ? */
	printf("  Input options (default = %s): ", tp->i_opt);
	if ( fgets(ibuf, D_OPTSZ, stdin) == NULL ) {
		return(0);
	}
	savstring(tp->i_opt, ibuf);
	return(0);
}

savstring( to, from )
register char	*to, *from;
{
	register char	*p;
	register int	cnt;

	/*
	 * copy string from "from" to "to" only if there is
	 * anything other than a newline in "from".
	 */
	for (p=from, cnt=0; *p; p++, cnt++) {
		/* get to end of string */ ;
	}
	if ( *--p == '\n' ) {
		*p = NULL; cnt--;
	}
	if ( cnt != 0 ) {
		strcpy(to, from);
	}
}

sav_ent( tp, type )
struct t_dir	*tp;
int	type;
{
	register struct archive	*ap, *lnkp;
	register char		*dirp;

	if ( (ap=(struct archive *)malloc(sizeof(struct archive))) == NULL ) {
		fprintf(stderr,Nomemmsg,myname);
		return(ERROR);
	}

	ap->arcno = tp->arnum;
	ap->number = tp->number;
	ap->next = NULL;

	if ( (ap->archiver=malloc(strlen(tp->name)+1)) == NULL ) {
		fprintf(stderr,Nomemmsg,myname);
		return(ERROR);
	}
	strcpy(ap->archiver, tp->name);

	if ( (ap->a_desc=malloc(strlen(tp->a_desc)+1)) == NULL ) {
		fprintf(stderr,Nomemmsg,myname);
		return(ERROR);
	}
	strcpy(ap->a_desc, tp->a_desc);

	if ( type == FILES ) {
		if ( (ap->i_opt=sub_device(tp->i_opt)) == NULL ) {
			fprintf(stderr,Nomemmsg,myname);
			return(ERROR);
		}

		dirp = ((tp->type == D_SCR) ? Tmpdir : tp->dest_dir);
		if ( (ap->dest_dir=malloc(strlen(dirp)+1)) == NULL ) {
			fprintf(stderr,Nomemmsg,myname);
			return(ERROR);
		}
		strcpy(ap->dest_dir, dirp);

		/*
		 * And finally, add to archive link list
		 */
		if ( Ar_head == NULL ) {
			Ar_head = ap;
		}
		else {
			for ( lnkp=Ar_head; lnkp->next; lnkp = lnkp->next ) ;

			lnkp->next = ap;
		}
	}
	else {
		/*
		 * And finally, add to OTHER link list
		 */
		if ( O_head == NULL ) {
			O_head = ap;
		}
		else {
			for ( lnkp=O_head; lnkp->next; lnkp = lnkp->next ) ;

			lnkp->next = ap;
		}
	}
	DEBPRT1("sav_ent: saved:  file number %d\n", ap->number);
	DEBPRT1("sav_ent: saved:  archiver %s\n", ap->archiver);
	DEBPRT1("sav_ent: saved:  input iptions %s\n", ap->i_opt);
	DEBPRT1("sav_ent: saved:  dest dir %s\n", ap->dest_dir);
	return(0);
}

inlist(num)
register int	num;
{
	register unsigned char	*p;

	if ( Allflag ) {
		return(TRUE);
	}

	for ( p=Selrec; *p; p++ ) {
		if ( *p == (unsigned char)num ) {
			return(TRUE);
		}
	}
	return(FALSE);
}

/*
 *	open special file
 */
opensp(mode, fno)
int	mode;
{
	static int	curfilno;

#ifdef	DEBUG
	if( sfd != 0 ) {
		printf("Warning: special file not closed\n");
		close(sfd);
	}
#endif

	if (fno < curfilno) {
		fprintf(stderr,"%s: request reverse file skip, illegal\n",
		    myname);
		return(ERROR);
	}

	if (fno > curfilno) {
		if ( skip(fno - curfilno) == ERROR ) {
			fprintf(stderr,"%s: cannot skip to file number %d\n",
			    myname, (fno - curfilno));
			return(ERROR);
		}
	}
	/*
	 * After the close, the current file number will be (fno+1)
	 */
	curfilno = fno + 1;

	/*
	 * Routine called only to position at file number, not open
	 * it.
	 */
	if ( mode == POS_ONLY ) {
		return(0);
	}

	if(( sfd = open(Device,(mode==M_TRUNC ? TRUNC : APPEND)))==ERROR) {
		fprintf( stderr, "%s: can't open %s\n", myname, Device );
		return(ERROR);
	}
	return(sfd);
}

skip(filcnt)
register int	filcnt;
{
	int	fd;

	DEBPRT1("skip: skipping %d files\n", filcnt);
	while ( filcnt-- ) {
		if ( (fd=open(Device, APPEND)) == ERROR ) {
			return(ERROR);
		}
		if ( read(fd, Io_buf, T_BSIZE) == ERROR ) {
			close(fd);
			return(ERROR);
		}
		/*
		 * The close will cause the driver to posn at the
		 * beginning of the next file.
		 */
		close(fd);
	}
	return(0);
}

/*
 *	close special file
 */
closesp()
{
	close(sfd);
	sfd = 0;
}

/*
 * rewind streamer tape
 */
rewind_t()
{
	register int	fd;		/* fd for special device */
	register int	errflag = 0;

	if ( (fd=open(Device, (O_NDELAY|O_RDONLY))) == ERROR ) {
		fprintf(stderr,"%s: cannot open %s\n",myname,Device);
		errflag = ERROR;
	}
	else if ( ioctl(fd, M350REWIND, 0) == ERROR ) {
		fprintf(stderr,"%s: warning: unable to rewind media\n",myname);
		errflag = CANT_REWIND;
	}
	(void)close(fd);
	return(errflag);
}

prt_toc()
{
	register struct archive	*lnkp;

	if (Verbose && O_head) {
		printf("\nArchive   Tape File   Name           Description\n");
		printf("Number    Number\n");
		for ( lnkp=O_head; lnkp; lnkp = lnkp->next ) {
			printf("           % 3d        % -14s %s\n",
			    lnkp->number, lnkp->archiver,
			    lnkp->a_desc);
		}
	}

	if ( Verbose ) {
	  printf("\nArc  File #   Destination   Input Command/Description\n");
	  printf("Num  Num      Directory\n");
	  for ( lnkp=Ar_head; lnkp; lnkp = lnkp->next ) {
		printf("% 3d  % 3d      % -13s %s %s   \"%s\"\n",
		    lnkp->arcno, lnkp->number, lnkp->dest_dir, lnkp->archiver,
		    lnkp->i_opt, lnkp->a_desc);
	  }
	}
	else {
	  printf("\nArchive   Destination   Input Command/Description\n");
	  printf("Number    Directory\n");
	  for ( lnkp=Ar_head; lnkp; lnkp = lnkp->next ) {
		printf(" % 3d      % -13s %s %s   \"%s\"\n",
		    lnkp->arcno, lnkp->dest_dir, lnkp->archiver,
		    lnkp->i_opt, lnkp->a_desc);
	  }
	}
}

extract(what)
int	what;
{
	register struct archive	*lnkp;

	if ( what == X_SCRIPTS ) {
		/* just bring in the first archive, SCRIPTS entry */
		return( exarc(Ar_head) );
	}
	else {	/* X_ALL */
		for ( lnkp=Ar_head; lnkp; lnkp = lnkp->next ) {
			if ( exarc(lnkp) == ERROR ) {
				return(ERROR);
			}
		}
	}
}

exarc( lnkp )
register struct archive	*lnkp;
{
	register char	*shcmd;

	/*
	 * Use global I/O buffer for shell command. It will be
	 * plenty big enough.
	 */
	shcmd = Io_buf;

	/*
	 * change to proper directory
	 */
	if ( chdir(lnkp->dest_dir) == ERROR) {
		fprintf(stderr,"%s: cannot changed to directory %s\n",
		    myname, lnkp->dest_dir);
		return(ERROR);
	}

	/* position tape */
	if ( opensp(POS_ONLY, lnkp->number) != 0 ) {
		fprintf(stderr,
		    "%s: cannot position media to file number %d\n",
		    myname, lnkp->number);
		return(ERROR);
	}
	strcpy(shcmd, "exec ");
	strcat(shcmd, lnkp->archiver);
	strcat(shcmd, " ");
	strcat(shcmd, lnkp->i_opt);
	DEBPRT1("SH -> \"%s\"\n", shcmd);

	if ( Verbose ) {
		printf("Reading files into %s\n", lnkp->dest_dir);
	}

	if( system(shcmd) < 0 ) {
		fprintf(stderr,"%s: %s error on %s\n",
		    myname, lnkp->archiver, lnkp->dest_dir);
		return(ERROR);
	}
	DEBPRT0("exarc: good return - 0\n");
	return(0);
}

parselopt( flist, dents )
char	*flist;
int	dents;
{
	register char	*p;
	register int	i = 0;

	if ( strcspn(flist, "1234567890,") > 0 ) {
		fprintf(stderr,"%s: illegal character in -l option argument\n");
		DEBPRT1("parselopt: bad -l option [%s]\n", flist);
		return(ERROR);
	}

	if ( (p = strtok(flist, ",")) == NULL ) {
		Selrec[i++] = ((unsigned char)atoi(flist) & 0xff);
		Selrec[i] = NULL;
		DEBPRT1("parselopt: file %d registered to be extracted\n",
		    Selrec[i-1]);
		return(0);
	}
	else {
		do {
			if (i > 0xff) {
				fprintf(stderr,"%s: file list too large\n",
				    myname);
			}
			Selrec[i++] = ((unsigned char)atoi(p) & 0xff);
			DEBPRT1("parselopt: file %d regs to be extracted\n",
			    Selrec[i-1]);
		} while( (p=strtok(NULL, ",")) );
	}
	return(0);
}

prtchrs(s, cnt)
register char	*s;
register int	cnt;
{
	while ( cnt-- && *s && isprint(*s) ) {
		putchar(*s++);
	}
}

char	*
sub_device( p )
register char	*p;
{
	char	loc_buf[I_BUFSIZ];
	register char	*lp = loc_buf;		/* local ptr into loc_buf */
	register char	*devp;			/* ptr to global device string*/

	DEBPRT1("sub_device: before device substitution - [%s]\n", p);
	while ( *p ) {
		if ( *p == '$' && strncmp(*(p+1), DEVSTR, strlen(DEVSTR))) {
			/* move p past "$DEVICE" for rest of copy */
			p += (strlen(DEVSTR) +1);	/* +1 for '$' sign */
			for ( devp=Device; *devp; ) {
				/* substitute read device name */
				*lp++ = *devp++;
			}
		}
		else {
			*lp++ = *p++;
		}
	}
	*lp = NULL;

	/* allocate memory for this new string and return a ptr to it */
	/* we can now re-use the "lp" variable */
	if ( (lp=malloc(strlen(loc_buf)+1)) == NULL ) {
		return(0);
	}

	strcpy(lp,loc_buf);
	DEBPRT1("sub_device: after device substitution - [%s]\n", lp);
	return( lp );
}
