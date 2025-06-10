#include	<stdio.h>
#include	"igf.h"
#include	"../com/common.h"

#ident	"$Header: /grp/mat/53/utils/igf/RCS/igf.c,v 1.2 87/01/27 16:17:38 mat Exp $";

/*
 * Global variables for igf(1M)
 */
struct ar_files	*Files_head = 0;	/* pointer to above */
struct ar_files	*Scripts;		/* head of scripts link list */
struct os_files	*Os_head = 0;		/* pointer to above */
char	*Ram = 0;			/* ramdisk file */
char	*Bootloader = 0;		/* bootloader file */
char	*myname = 0;			/* name program running as */
char	*Nomemmsg = { "%s: out of memory\n" };
char	Io_buf[T_BSIZE];		/* output I/O buffer */
int	Dirents;			/* number of dir entries on media */
int	Debug = 0;

/*
 * Global command line options.
 */
int	Verbose  = FALSE;		/* -v */
int	Bootable = FALSE;		/* -b */
char	*Desc 	 = NULL;		/* media description */
char	*Package = NULL;		/* package name */
char	*Version = NULL;		/* version of this software */
char	*Device  = { "/dev/r41an" };	/* default output device */

static char *Usage = {
	"Usage: %s [-vb] [-f special_file] {table | - }\n"
};

extern int	optind;
extern char	*optarg;

main(argc,argv)
int	argc;
char	*argv[];
{
	register char	*p;
	register int	let;		/* option letter */
	int	errflag = FALSE;

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
	while ( (let=getopt(argc, argv, "Dvbf:")) != EOF ) {
#else
	while ( (let=getopt(argc, argv, "vbf:")) != EOF ) {
#endif
		switch(let) {
			case 'v':
				/* verbose mode */
				Verbose = TRUE;
				break;
			case 'b':
				/* creating bootable media */
				Bootable = TRUE;
				break;
			case 'f':
				/* use this special file instead of default */
				Device = optarg;
				break;
#ifdef DEBUG
			case 'D':
				/* turn on debugging if compiled in */
				Debug = TRUE;
				break;
#endif
			case '?':
				errflag = TRUE;
				break;
			default:
				break;
		}
	}

	if ( optind == argc || errflag ) {
		fprintf(stderr,Usage,myname);
		exit(1);			/* syntax error */
	}

	/*
	 * Read table or prompt user for bootloader, os's
	 * archive files, etc...
	 */
	if ( read_table(argv[optind]) == ERROR ) {
		/* error in table or user input */
		exit(2);
	}

	/* Create the volume id on output device */
	DEBPRT0("main: calling wr_vid\n");
	if ( wr_vid() == ERROR ) {
		/* unable to write vid */
		exit(3);
	}

	if ( Bootable ) {
		DEBPRT0("main: calling wr_boot\n");
		/* write bootloader on output device */
		if ( wr_boot(Bootloader) == ERROR ) {
			/* unable to write bootloader */
			exit(4);
		}
	}

	/* Create the tape directory on output device */
	DEBPRT0("main: calling wr_dir\n");
	if ( wr_dir() == ERROR ) {
		/* unable to write vid */
		exit(5);
	}

	if ( Bootable ) {
		DEBPRT0("main: calling wr_os\n");
		/* write operating systems on output device */
		if ( wr_os() == ERROR ) {
			/* unable to write bootloader */
			exit(6);
		}
		DEBPRT0("main: calling wr_ram\n");
		/* write operating systems on output device */
		if ( wr_ram() == ERROR ) {
			/* unable to write bootloader */
			exit(7);
		}
	}

	/* Create the file archives on output device */
	DEBPRT0("main: calling wr_arc\n");
	if ( wr_arc() == ERROR ) {
		/* unable to write vid */
		exit(8);
	}

	/*
	 * Rewind output media
	 */
	DEBPRT0("main: calling rewind_t\n");
	if ( Verbose ) {
		printf("Rewinding tape\n");
	}
	if ( rewind_t() == ERROR ) {
		fprintf(stderr,"%s: unable to rewind tape\n", myname);
		exit(9);
	}

	exit(0);		/* good exit value */
}
