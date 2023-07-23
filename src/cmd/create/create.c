
#ifndef RELEASE
#ident	create -- a tool to create master release media (x318)
#endif

#define UPDATE 318

/*
	Written by: David P. Schwartz
	
	Revision History:
		DS	2-Jun-86	(x318)	fixed some boundary-condition errors
		DS	9-May-86	(x317)	replaced most BSIZE things with 1024
								and adjusted computations accordingly
		DS	1-May-86	(x316)	changed some prompts; disabled display of
								percentages in copy status line.
		DS	30-Apr-86	(x315)	added auditing facilities (in /usr/AUDITS)
*/

#ifndef MOD_create
#	define MOD_create
#endif

#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/param.h>

#include "common.h"

static char _product_id[] = "Create Master Release Media Utility";
char * Whoami;

extern char *getcwd();

char src_root_dir[64];		/* path name of where product is copied from */


/*----------------------------------------------------------------------*/
int					mount_volume( dev, vol )
/*----------------------------------------------------------------------*/
char *dev;	/* name of device */
int vol;	/* which volume is needed */
/* returns a file descriptor pointer */
{
	static char format_cmd[80];
	char res;

	if (vol == 1 && can_format_media)
	{
		*format_cmd = NULL;
		printf( "\nIf you like, I will automatically format the media for you.\n" );
		res = ask( "Would you like this?", yes, no, 0 );
		if (res == 'y')
		{
			while (!request( "Enter the shell command needed to format this type of media:\n--> ", 
				format_cmd, sizeof(format_cmd) )) ;
		}
	}
	else if (vol > 1)
		close( media );

	printf("\nMount media volume #%d, then hit RETURN...", vol);
	while (getchar() != '\n') ;

	if (*format_cmd)
	{
		printf( "\n%s\n", format_cmd );
		system( format_cmd );
	}

	if ((media = open( dev, O_WRONLY )) == -1)
	{
		sprintf( cmd, "can't open output device {%s} for writing!", dev );
		fatal1( cmd );
	}
	return( media );
}

/*------------------------------------------------------------*/
void					get_info( disclimit )
/*------------------------------------------------------------*/
unsigned long disclimit;
{
	int i, j, blks;
	unsigned long cnt;
	char filnam[10];  /* holds the file name */
	char prompt[128], *prompt_p;
	char *cwd, *junkstr, *xhdr;

	xhdr = (char *) &hdr;
	for (i = 0; i < sizeof(hdr); *xhdr++ = 0, i++) ;

	while (!request( "What is the name of this product? --> ", 
						hdr.prodname, sizeof(hdr.prodname) ));

	while (!request( "What is this product's new release identifier? --> ",
						hdr.rel_id, sizeof(hdr.rel_id) ));

	prompt_p = "Enter a file name (< 11 chars; similar to the product name) for auditing needs\n--> ";
	while (TRUE)
	{
		while (!request(  prompt_p, hdr.audit_name, sizeof(hdr.audit_name) ));
		if (strpbrk( hdr.audit_name, " /|\\`'\"*:;?{}[]()<>~$#!" ) == NULL)
			break;
		warn1( "This name must conform to UNIX file naming conventions!!!" );
	}

	sprintf( toc_file, toc_file_fmt, hdr.audit_name );
	sprintf( audit_file, create_audit_file_fmt, hdr.audit_name );
	cwd = getcwd( (char *) NULL, 64 );
	if (chdir( audit_dir_name )) system( make_audit_dir_cmd );

	chdir( cwd );
	sprintf( prompt,
		"Enter the pathname for the directory where this product is found\n(you're now in %s)\n--> ",
		cwd );
	while (TRUE)
	{
		while (!request( prompt, src_root_dir, sizeof(src_root_dir) ));
		if (chdir( src_root_dir ) == 0)
		{
			junkstr = getcwd( src_root_dir, sizeof(src_root_dir) );
			break;
		}
		sprintf( cmd,"`%s' is an invalid directory path!!!\n", src_root_dir);
		warn1( cmd );
	}
	printf( "(The product's root pathname is %s)\n", src_root_dir );
	chdir( cwd );

	for (errno = 0;;)
	{
		prompt_p="Into what directory (on the TARGET SYSTEM) should Upgrade copy this product?\n--> ";
		while (!request( prompt_p, hdr.destdir, sizeof(hdr.destdir) )) ;
		if (*hdr.destdir != '/')
			warn1( "The target directory name must begin with a '/'!" );
		else if (strlen(hdr.destdir) == 1)
			warn1( "The target directory cannot be just a '/'!" );
		else
			break;
	}

	prompt_p="Enter a command string (<128 chars) to be executed at START of Upgrade:\n--> ";
	request( prompt_p, hdr.pre_cmds, sizeof(hdr.pre_cmds) );

	prompt_p="Enter a command string (<128 chars) to be executed at END of Upgrade:\n--> ";
	request( prompt_p, hdr.post_cmds, sizeof(hdr.post_cmds) );

	printf("\n\tPlease stand-by for a moment while I figure a few things out...");

	sprintf( hdr.create_id, create_ver_rev_id_fmt, _version, _revision );
	hdr.version = _version;
	hdr.revision = _revision;

	hdr.src_bsize = get_bsize( src_root_dir );

	/* get total number of blocks the product occupies in the file system */
	sprintf( cmd, "du -s %s", src_root_dir );
	in_stream = popen( cmd, "r" );
	fscanf( in_stream, "%ld", &hdr.fs_blk_cnt );
	pclose( in_stream );

	/* get number of inodes used by the product in a file system */
	sprintf( cmd, "find %s -print | wc -l", src_root_dir );
	in_stream = popen( cmd, "r" );
	fscanf( in_stream, "%d", &hdr.fs_inode_cnt );
	pclose( in_stream );

	/* figure out how much raw space is needed on master media */
	/* and create a table-of-contents file */
	sprintf( cmd, "find %s -print | cpio -ov 2>%s", src_root_dir, toc_file );
#ifdef DEBUG
	fprintf( stderr, "\n*** %s\n", cmd );
#endif
	in_stream = popen( cmd, "r" );
	blks = 0;
	printf( "\n\n" );
	while ((i = fread( buf_io, 1, 1024, in_stream)) > 0)
		if (!(++blks % 10))
			printf( "(%ld media blocks needed)...      \r", blks );
	pclose(in_stream);

	sprintf( cmd, "crc -cfd %s >%s 2>/dev/null", toc_file, audit_file );
#ifdef DEBUG
	fprintf( stderr, "\n*** %s\n", cmd );
#endif
	printf( "(deriving an audit file now)...    \r" );
	system( cmd );

	i = 0;
	sprintf( cmd, "du -a %s", audit_file );
	in_stream = popen( cmd, "r" );
	if (fscanf( in_stream, "%d", &i ) == EOF)
		warn1( "can't determine size of audit file!" );
	pclose( in_stream );

	hdr.media_blk_cnt = blks + ( (BSIZE == 1024) ? i : (i + 1) / 2 );
	printf( "(%d media blocks needed)         \n", hdr.media_blk_cnt );
	hdr.max_volumes = (int) (hdr.media_blk_cnt * 1024 / disclimit) + 1;
}

/*----------------------------------------------------------------------*/
int						verify_input()
/*----------------------------------------------------------------------*/
{
	char res, *bufp;

	CLEAR;
	printf( "\n\t\t%s, R%d.%d (x%03.3d)\n", 
			_product_id, _version, _revision, _update);
	printf( "\nThis product's name is `%s'\n", hdr.prodname );
	printf( "Its release identifier is: %s\n", hdr.rel_id );
	printf( "(Auditing facilities will use the name: %s)\n", hdr.audit_name);
	printf( "It is located in the directory rooted at `%s'.\n", src_root_dir );
	printf( "It occupies about %d x %d-byte disk blocks in the file system,\n",
				hdr.fs_blk_cnt, hdr.src_bsize );
	printf( "\tand requires %d (1024-byte) blocks of distribution media.\n",
				hdr.media_blk_cnt );

	printf( "\nThe distribution media was specified as %s.\n", media_name );
	printf( "So, %d volume(s) of this media will be needed to distribute this product.\n", hdr.max_volumes );
	
	printf("\nUpgrade will execute this command string prior to installation:\n\t%s\n", 
		*hdr.pre_cmds ? hdr.pre_cmds : "--- none specified ---" );
	printf("It will then install the product into the directory named:\n\t%s\n", hdr.destdir );
	printf("Finally, this command string will be invoked after installation:\n\t%s\n", 
		*hdr.post_cmds ? hdr.post_cmds : "--- none specified ---" );
	
	res = ask( "Are these input parameters correct?", "yes", "no", 0 );
	if (res == 'y') return( TRUE );
	
	res = ask( "\nDo you want to redefine them, accept them, or quit now?",
			"redefine", "accept", "quit", 0);

	if (res == 'q') exit(0);

	if (res == 'r') close( media );

	return( res == 'a' );
}

/*---------------------------------------------------------------------*/
long			write_to_media( disclimit, cnt, eof_flag_p )
/*---------------------------------------------------------------------*/
unsigned long disclimit;
unsigned long *cnt;		/* outputs total bytes transferred */
int *eof_flag_p;		/* outputs TRUE when we reach end of input stream */
/*	returns crc of data written */
{
	unsigned long crcout, ltmp;
	int blks, i, rd_cnt, wrt_cnt, running_pct;
	static int tot_pct;

	if (hdr.vol_num == 1)
		tot_pct = 0;

	if ((*cnt = write_blk( media, &hdr, sizeof(hdr), NULL )) == -1 )
		fatal1("error encountered while writing header info");

	/* initialize the output checksum crc variable */
	crcout = checksum(crctable, buf_io, 0, -1);
			
#ifdef DEBUG
	fprintf( stderr, "*** wrote header: cnt = %ld ***\n", *cnt );
#endif

	blks = 0;
	while ((*cnt < disclimit) 
		&& ((rd_cnt = fread( buf_io, 1, 1024, in_stream )) > 0))
	{
		if ((wrt_cnt = write_blk( media, buf_io, rd_cnt, &crcout )) <= 0)
			fatal1( "error writing to output media!" );
		*cnt += (long) wrt_cnt;
		running_pct = blks * 100 / hdr.media_blk_cnt;
		if (!(++blks % 5))
/*			printf( "%d blocks = %d%% written to vol #%d from %s...   \r",  */
/*				blks, running_pct + tot_pct, hdr.vol_num, src_root_dir); */
			printf( "%d blocks written to vol #%d from %s...   \r", 
				blks, hdr.vol_num, src_root_dir);

		/*--- note: the block count percentage is turned off until we can
		/*--- figure out why we don't get 100% as a final count for
		/*--- big products. */
	}
	if (rd_cnt == -1)
		fatal1( "error reading from input data stream!" );

	*eof_flag_p = (rd_cnt == 0);

	ltmp = (long) blks * (long) hdr.vol_num * 100L;
	ltmp /= hdr.media_blk_cnt;
	tot_pct = (int) ltmp;

/*	printf( "%d blocks = %d%% written to vol #%d from %s.   \n",	*/
/*			blks, tot_pct, hdr.vol_num, src_root_dir);				*/
	printf( "%d blocks written to vol #%d from %s.   \n",
			blks, hdr.vol_num, src_root_dir);


#ifdef DEBUG
	fprintf( stderr, "*** wrote data: cnt, crc = %ld, %lx ***\n", *cnt, crcout);
#endif

	for (i=0; i < NELTS(eov_blk); i++)
		eov_blk[i] = EOV_FLAG;
	if ((wrt_cnt = write_blk( media, eov_blk, sizeof(eov_blk), NULL )) == -1)
		fatal1("error encountered while writing end-of-volume flag!"); 
	*cnt += wrt_cnt;
	
	if ((wrt_cnt = write_blk( media, &crcout, sizeof(crcout), NULL )) == -1)
		fatal1("error encountered while writing checksum to the media!"); 
	*cnt += wrt_cnt;

#ifdef DEBUG
	fprintf( stderr, "*** wrote checksum: cnt = %ld ***\n", *cnt);
#endif

	return( crcout );
}

/*---------------------------------------------------------------------*/
unsigned long			verify_media( cnt )
/*---------------------------------------------------------------------*/
unsigned long cnt;	/* total bytes written */
/* return crc of data on media */
{
	unsigned long crcin, crctemp, tot_valid, rd_cnt;
	int i, blks, eov;

	close( media );
	if ((media = open(devname, O_RDONLY)) == -1)
		fatal1("unable to open output media for reading");

	/* initialize the crc */
	crcin = checksum(crctable, buf_io, 0, -1);

	/* re-read the header */
	if ((tot_valid = read_blk( media, &hdr, sizeof(hdr) )) == -1 )
		fatal1("error encountered while reading header");

#ifdef DEBUG
	fprintf( stderr, "\n*** verify header: tot_valid = %ld ***\n", tot_valid );
#endif

	blks = 0;
	/* note: the 'cnt' value includes two extra blocks:  */
	/* the eov flag block, and the final checksum block size, */
	/* which we don't want to read yet! */
	while (tot_valid < cnt - 2 * 1024)
	{
		if ((rd_cnt = read_blk( media, buf_io, 1024 )) == -1)
			fatal1( "error occurred attempting to read data from media!" );
		tot_valid += (long) rd_cnt;
		if (!(++blks % 5))
			printf( "%d blocks being verified on media...  \r", blks );
		crcin = checksum( crctable, buf_io, rd_cnt, crcin );
	}
	printf( "%d media blocks verified!            \n", blks );

#ifdef DEBUG
	fprintf( stderr, "*** verify data: tot_valid, crc = %ld, %lx ***\n", tot_valid, crcin );
#endif

	if ((rd_cnt = read_blk( media, eov_blk, sizeof(eov_blk) )) == -1)
		warn1( "can't read/verify end-of-volume flag from media!" );
	else
	{
		if (is_eov(eov_blk))
			tot_valid += rd_cnt;
		else
		{
#ifdef DEBUG
			for (i=0; i < NELTS(eov_blk); i++)
			{
				if (!(i / 8))
					printf( "\n%d\t", i );
				if (!(i % 8))
					printf( "%lx ", eov_blk[i] );
			}
			printf( "\n\n" );
#endif
			fatal1( "The end-of-volume flag block is corrupted!" );
		}
	}

	if ((rd_cnt = read_blk( media, &crctemp, sizeof(crctemp) )) == -1)
		warn1( "can't read/verify checksum block from media!" );
	else
		tot_valid += rd_cnt;
	close(media);

#ifdef DEBUG
	fprintf( stderr, "*** verify checksum: tot_valid = %ld, crc is/should be %lx/%lx\n", tot_valid, crcin, crctemp );
#endif

	return( (crcin == crctemp) ? crcin : -1L );
}

/*=======================================================================*/
							    main(argc, argv)
/*=======================================================================*/
int argc;
char * argv[];
{
	int i, more_prod, eof_flag;
	char res;


	errno = 0;
	Whoami = argv[0];
	CLEAR;

#ifdef RELEASE
	printf( "\n%s, R%d.%d\n", 
		_product_id, _version, _revision );
#else
	printf( "\n%s, R%d.%d (x%03.3d)\n", 
		_product_id, _version, _revision, _update);
#endif
	printf( "%s\n", _copyright );

#ifndef DEBUG
	/* check to see if in superuser mode */
	if ((i=setuid(0)) == -1)
		fatal1("You must be logged in as root or have root's setuid permission set!\n");
#endif

	/* initialize the checksum crc table */
	make_crctable( crctable );

	for (more_prod = TRUE; more_prod; )
	{
		do {
			disclimit = get_media_size( &devname );
			media = mount_volume( devname, 1 );
			get_info( disclimit );
		} while (!verify_input());

		printf("\t\t------ FILE COPY IN PROGRESS ------\n\n");

		/* move to specified directory */
		if (chdir(src_root_dir))
		{
			sprintf( cmd, "cannot chdir to '%s'!", src_root_dir);
			fatal1( cmd );
		}

		/* open input stream */
		sprintf( cmd, "(echo %s; find . -print) | cpio -o 2>/dev/null",
				audit_file );
#ifdef DEBUG
		fprintf( stderr, "\n*** %s\n", cmd );
#endif
		in_stream = popen( cmd, "r" );
	
		for (	eof_flag = FALSE, hdr.vol_num=1; 
				!eof_flag && hdr.vol_num <= hdr.max_volumes; 
				hdr.vol_num++ )
		{
			errno = 0;
			if (hdr.vol_num > 1)
				media = mount_volume( devname, hdr.vol_num );

			crc = write_to_media( disclimit, &cnt, &eof_flag );
			if (verify_media( cnt ) != crc)
				warn1( "The media failed verification!" );
		}
		pclose(in_stream);

		printf("\nFinished creating master(s) for `%s (%s)'.\n", 
				hdr.prodname, hdr.rel_id );

		printf( "\n(A table-of-contents listing of this product (with crc's) is contained in:\n\t`%s')\n",
				audit_file );

		res = ask( "Do you wish to create a master copy of another product?",
					"no", "yes", 0 );

		if (more_prod = (res == 'y'))
			CLEAR;
	}
	printf( "\n" );
	exit(0);
}
