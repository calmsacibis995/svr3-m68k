
#ifndef RELEASE
#ident	upgrade -- a program to support field upgrades of sw products (x317)
#endif

#	define UPDATE 318

/*
	Writen by: David P. Schwartz

	Revision history:
WJC	23-Jun-86	(x318)	Softened the robust verification.
				Now checks only hdr.create_id ("create")
				and hdr.version (_version) of media
DS	2-Jun-86	(x317)	Added a more robust verification of media
				origin, and added a -v option to provide
				a way to verify created media
DS	9-May-86	(x316)	changed most occurrences of BSIZE to 1024
				and modified algorithms accordingly.
				(Wasn't doing as expected when compiled
				in 512 byte/blk system.)
DS	2-May-86	(x315)	when selecting a temp file system, if
				'/' is selected then the subdirectory
				prompt is disabled to prevent the user
				from entering an already disqualified
				file system name.
DS	2-May-86	(x314)	trying to find out why /tmp is allocated
				when there isn't enough space on it!
DS	1-May-86	(x313)	still trying to fix space problem.  (tried
				moving target dir space needs ahead of temp
				dir space need computation.)  Also disabled
				the display of percentages in block copy
				status line -- see 'create' for reason.
DS	29-Apr-86	(x312)	added facility to redirect TOC to a file and
				optionally audit the installation afterwards.
				("upgrade -a" can be used anytime to perform
				 this auditing function.)
DS	28-Apr-86	(x311)	fixed problem whereby if room exists on temp 
				device but not on target, then program dies.
*/

#ifndef MOD_upgrade
#	define MOD_upgrade
#endif

#define MAX_DEV 16

#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/param.h>
#include <sys/errno.h>

#include "common.h"

static char _product_id[] = "Software Product Field Upgrade Utility";

extern	char *calloc();

char *Whoami;
int	verify_mode = 0;

FILE *in_stream, *out_stream;
unsigned long volumes_needed;		/* how many disks/tapes are needed */

char prodname[32];			/* used to hold copies of the hdr values of */
char rel_id[32];			/* these fields on first volume for later checks */

int target_bsize, target_fs_blk_cnt;

#ifdef DEBUG
	int debugging = 0;
#endif


/*----------------------------------------------------------------------*/
int					mount_volume( dev, vol )
/*----------------------------------------------------------------------*/
char *dev;	/* name of device */
int vol;	/* which volume is needed */
/* returns a file descriptor pointer */
{
	char create_signature[32];

	/* this signature is used to verify that we have some media created */
	/* by the proper version of create, and not random data. */
	/* create_ver_rev_id_format is defined in common.h */
	sprintf( create_signature, create_ver_rev_id_fmt, _version, _revision );

try_again:
	errno = 0;
	if (vol == 1)
		printf("\nMount distribution media volume #%d, then hit RETURN...", vol);
	else
	{
		close( media );
		printf("\nMount distribution media volume #%d for `%s (%s)',\n then hit RETURN...", 
			vol, prodname, rel_id );
	}
	while (getchar() != '\n') ;
	if ((media = open( dev, O_RDONLY )) == -1)
	{
		sprintf(cmd , "can't open device {%s} for reading!", dev );
		if (errno == ENOENT)  /* shouldn't go back to 'Mount' prompt */
		    fatal1(cmd);
	    warn1(cmd);
	    goto try_again;
	}
	if (read_blk( media, &hdr, sizeof(hdr) ) == -1)  
	{
		warn1( "can't read header from distribution media!" );
		goto try_again;
	}

	/* check to be sure that the version of create that was used is the
	/* same as this version -- otherwise the internal data structures
	/* probably aren't compatible! */
	/* Someday...this should be a check against all known versions of
	/* create with known compatible internal structure.  For now,
	/* assume that, at least, any with the same version as us can
	/* never deviate internally */

	if (strncmp( hdr.create_id, create_signature, strlen("create") ) != 0
	    || hdr.version != _version)
	{
	    fprintf(stderr,
	    	"I can't read this media!  (incompatible internal structure)\n" );
	    goto try_again;
	}

	if (vol != hdr.vol_num)
	{
		printf( "\n\tThis is volume ---> %d <---!!!\n", hdr.vol_num );
		goto try_again;
	}

	/* be sure that the user hasn't inserted a volume from another product */
	if (vol > 1 
		&& (strcmp( hdr.prodname, prodname ) != 0 
			|| strcmp( hdr.rel_id, rel_id ) != 0))
	{
		printf( "\nThis disk is for a different product: \n\t(%s, Release %s) !!!\n",
				hdr.prodname, hdr.rel_id );
		goto try_again;
	}

	printf( "\n" );
	return( media );
}

/*---------------------------------------------------------------------*/
char 			*find_space( num_blks_needed, target_dir )
/*---------------------------------------------------------------------*/
int num_blks_needed;
char *target_dir;
/* returns pointer to a temp directory with > num_blks_needed free space */
{
	static char *device_p[MAX_DEV], resp[80], buf[80];	/* must be static! */
	FILE *instream;
	char res, *sz_cmd, prompt[80], *question_p, question[80], *where_p;
	int i, j, di, dsize[MAX_DEV], n_dev, min_sz, temp_device;
	int tmp_flg = -1;	/* signifies that /tmp is big enough to hold data */
	int usr_flg = -1;	/* ditto for /usr (specific interest is /usr/tmp) */
	int target_flg = -1;	/* used to hold dev index for target device */
	int select_subdir;


	/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
		This routine uses 'df | ...' to figure out how much raw space is
		available in the system.  It looks first to see if there's enough
		room for the target copy of the product.  If the product fits,
		then the file system's size is adjusted (downward!) and the system
		is checked to see if ther's room for an intermediate temp copy.
		If /tmp or /usr is big enough, then /tmp or /usr/tmp is used. 
		Otherwise, the user is asked to select another file system that 
		does have enough space.
		
		Some simplifying assumptions must be made to avoid making this
		routine excessively large.  To wit:

			(1) If enough raw file space is available, then there
				should be enough free inodes to handle things, so the
				inode needs aren't checked.

			(2) Insufficient space for either the temp copy or the
				target copy (iff on a file system different from
				the temp copy) is considered a FATAL error!
		
			(3) If neither /tmp nor /usr has enough room for holding
				the temp copy, the user is asked about each device 
				that does have enough space.

	- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

	/* do some initial stuff... */

	/* minimum size requirements = 115% of num_blks_needed */
	min_sz = num_blks_needed * 115 / 100;

	/* this command gives a list of (<file_sys_name> <free_space>) tuples  */
	/* for each file system. */
	sz_cmd = "df | sed 's/\\([^ ]*\\).*:[ ]*\\([^ ]*\\).*$/\\1	\\2/'";
	instream = popen( sz_cmd, "r" );
	n_dev = 0;
	if (instream <= 0)
		warn1( "popen failed; can't determine available space!" );
	else
	{
		device_p[0] = calloc( 1, 16 );
		for (	n_dev = 0; 
				fscanf( instream,"%s\t%d",device_p[n_dev],&dsize[n_dev] ) != EOF
				&& n_dev < MAX_DEV;
				device_p[++n_dev] = calloc( 1, 16 ) )
		{
			if (strncmp( device_p[n_dev], target_dir, sizeof(device_p[n_dev]) ) == 0)
				target_flg = n_dev;
#ifdef DEBUG
if (debugging)
			fprintf( stderr, "%d:\t%s\t%d %s\n", 
					n_dev, device_p[n_dev], dsize[n_dev],
					dsize[n_dev] >= min_sz ? "*" : "<" );
#endif
		}
		pclose( instream );
	}
#ifdef DEBUG
if (debugging)
		fprintf( stderr, "\n*** min_sz = %d\n", min_sz );
		fprintf( stderr,
			"*** %d devices mounted in this system (= n_dev)\n\n", n_dev );
#endif

					/**** now, go for it! ****/

	errno = 0;

	/* First make sure there's enough space available on the target dev */
	if (dsize[target_flg] < min_sz)
	{
		res = ask( "Does a previous version of this product already exist in your system?",
			no, yes, 0 );
		if (res == 'n')
		{
too_small:
			printf( "\nThe file system containing the intended target directory for this product\n");
			printf( "doesn't have room to hold the product!\n" );
			printf( "(%s has %d blocks free, but %s needs about %d blocks.)\n", 
				target_dir, dsize[target_flg], hdr.prodname, min_sz );
			printf( "Delete some files, then invoke Upgrade again.\n" );
			exit(1);
		}

		printf( "\nIn order to install this new version, I'll have to write over the old version.\n" );
		res = ask( "Do you want to do this?", yes, no, 0 );
		if (res == 'n')
			goto too_small;
		
		sprintf( question, "Is the old version located in the `%s' directory?", target_dir);
		res = ask( question, yes, no, 0 );
		if (res == 'y')
			where_p = target_dir;
		else
		{
			for (;;)
			{
				while (!request("Enter the name of the directory where it is located:\n--> ", resp, sizeof(resp) )) ;
				where_p = resp;

				if (chdir( resp ) == 0)
					break;
				printf( "\n`%s' is not a valid directory!\n", resp );
			}
		}
	}
	else
	{
		/* we know there's room for the target copy */
		where_p = device_p[target_flg];
		dsize[target_flg] -= min_sz;
#ifdef DEBUG
if (debugging)
		fprintf( stderr, "\n*** target size -->  %d:\t%s\t%d %s\n", 
				target_flg, device_p[target_flg], dsize[target_flg],
				dsize[target_flg] >= min_sz ? "*" : "<" );
#endif

		/* now look for space for a temp copy */
		for (i=0, j=0; i < n_dev; ++i )
		{
			if (dsize[i] >= min_sz)
			{
				++j;
				if (strcmp( device_p[i], "/tmp" ) == 0)
					tmp_flg = i;
				else
					if (strcmp( device_p[i], "/usr" ) == 0)
						usr_flg = i;
			}
		}
#ifdef DEBUG
if (debugging)
		fprintf( stderr, "*** tmp_flg = %d, usr_flg = %d\n", tmp_flg, usr_flg );
#endif

		if (j != 0)
		{
			question_p = "Do you want to use `%s' for holding a temporary copy of the product?";
			res = 'n';
			temp_device = -1;

			/* select a temp device name from names in device_p, */
			/* leaving its index in temp_device */

			if 	(res == 'n' && tmp_flg != -1)
			{
				temp_device = tmp_flg;
				res = 'y';
			}

			if (res == 'n' && usr_flg != -1)
			{
				temp_device = usr_flg;
				res = 'y';
			}

			while (res == 'n')
			{
				printf( "\nThese file systems have enough room to install this product ( >%d blks):\n\n", min_sz );
				for (i=0; i < n_dev; i++)
					if (dsize[i] >= min_sz)
						printf( "\t%s\thas %d free blks\n", device_p[i], dsize[i] );
				printf( "\n" );
	
				for (i=0; i < n_dev; i++)
					if (dsize[i] >= min_sz)
					{
						sprintf( prompt, question_p, device_p[i] );
						res = ask( prompt, no, yes, 0 );
						if (res == 'y')
							break;
					}

				if (i < n_dev)
					temp_device = i;
				else
					printf( "\n\t\tYOU MUST SELECT ONE OF THESE DEVICES!\n" );
			}
		
			/* select a subdirectory within this device */
			/* assume /tmp and /usr/tmp as normal defaults */
			where_p = strcpy( resp, device_p[temp_device] );
			if (temp_device == usr_flg)
			{
				where_p = strcat( where_p, "/tmp" );
				select_subdir = (chdir(where_p) != 0);
			}
			else
				select_subdir = (temp_device != tmp_flg);
			
			
			/* Disable subdirectory prompt when root is selected. */
			/* This prevents the user from selecting an already */
			/* disqualified file system. */
			if (strcmp( device_p[temp_device], "/" ) == 0)
				select_subdir = FALSE;

			while (select_subdir)
			{
				where_p = strcpy( resp, device_p[temp_device] );
				chdir( where_p );
				sprintf( prompt, "\nEnter a sub-directory within this file system, if desired:\n--> %s", resp );
				if (request( prompt, buf, sizeof(buf) ) > 0)
					strcat( resp, buf );
				if (strlen( buf ) == 0 || chdir( resp ) == 0)
					break;
				printf( "\n`%s' is not a valid directory!\n", resp );
			}
			where_p = strcat( where_p, "/_upgrade_dir" );
		}

		/* be sure that the directories exist */
		if (chdir( where_p ) != 0)
		{
			sprintf( buf, "mkdir %s", where_p );
			printf( "%s\n", buf );
			system( buf );
		}
		if (strcmp( where_p, target_dir ) != 0 && chdir( target_dir ) != 0)
		{
			sprintf( buf, "mkdir %s", target_dir );
			printf( "%s\n", buf );
			system( buf );
		}
	}

#ifdef DEBUG
if (debugging)
		if (ask( "Continue...", yes, no, 0) == 'n')
			exit(1);
#endif

	return( where_p );
}

/*----------------------------------------------------------------------*/
void						verify_data( temp_dir_p )
/*----------------------------------------------------------------------*/
char *temp_dir_p;
{
	char res;

	CLEAR;
	printf( "\n\t\t%s, R%d.%d (x%03.3d)\n", 
			_product_id, _version, _revision, _update);
	printf( "\nThis product's name is `%s'\n", hdr.prodname );
	printf( "Its release identifier is: %s\n", hdr.rel_id );
	printf( "(Auditing facilities use the name: %s)\n", hdr.audit_name );
	printf( "It will occupy about %d x %d-byte disk blocks in a file system.\n",
				target_fs_blk_cnt, target_bsize );

	printf( "\nThe distribution media is a %s.\n", media_name );
	printf( "The product uses %d blocks of the media,\n", hdr.media_blk_cnt );
	printf( "\tand you should have %d volume(s) of this media.\n", 
				hdr.max_volumes );
	
	if (*hdr.pre_cmds)
		printf("\nUpgrade will execute this command string prior to installation:\n\t%s", hdr.pre_cmds );

	if (strcmp( temp_dir_p, hdr.destdir ) != 0)
	{
		printf("\nThe product will be copied into the temporary directory:\n\t%s", temp_dir_p );
		printf("\nThen it will be moved into the target directory:\n\t%s", 
				hdr.destdir );
	}
	else
		printf("\nThe product will be copied straight into the directory:\n\t%s", hdr.destdir );

	if (*hdr.post_cmds)
		printf("\nFinally, this command string will be invoked after installation:\n\t%s", hdr.post_cmds );

	printf( "\n" );
	res = ask( "Continue...", yes, no, 0 );
	if (res == 'n') exit(1);
}

/*---------------------------------------------------------------------*/
void					verify_destdir()
/*---------------------------------------------------------------------*/
{
	char tmpdir[80], *subpath_p;

	if (chdir( hdr.destdir ))
	{
		printf( "\nI'm creating the destination directory (%s) now...\n",
				hdr.destdir );
		strcpy( tmpdir, hdr.destdir );
#ifdef DEBUG
if (debugging)
		fprintf( stderr,
			"\n*** the destination directory (%s) doesn't exist!\n", tmpdir );
#endif
		subpath_p = strtok( tmpdir, "/" );
#ifdef DEBUG
if (debugging)
		fprintf( stderr, "*** moving into sub-directory (%s) ...\n", subpath_p);
#endif
		chdir( "/" );
		chdir( subpath_p );
		printf( "\tcd /%s\n", subpath_p );
		while (subpath_p && (subpath_p = strtok( NULL, "/" )))
		{
#ifdef DEBUG
if (debugging)
			system( "echo \"\t---> now inside directory (`pwd`)\"" );
			fprintf( stderr,
				"\t*** moving into sub-directory (%s)\n", subpath_p);
#endif
			if (chdir( subpath_p ))
			{
#ifdef DEBUG
if (debugging)
				fprintf( stderr,
					"\t\t+++ making sub-directory (%s) ***\n\t", subpath_p );
#endif
				sprintf(cmd, "mkdir %s; chmod 777 %s; cd %s",
						subpath_p, subpath_p, subpath_p);
				printf( "\t%s\n", cmd );
				system( cmd );

				/* the following chdir is needed because the cd in above cmd */
				/* has no effect outside of the system() call! */
				chdir( subpath_p );

#ifdef DEBUG
if (debugging)
				system( "echo \"\t\t+++> now inside directory (`pwd`)\"");
#endif
			}
		}
		printf( "\n" );
#ifdef DEBUG
if (debugging)
		if (ask( "Continue...", yes, no, 0) == 'n')
			exit(1);
#endif
	}
#ifdef DEBUG
	else
		if (debugging)
			fprintf( stderr,
				"\n*** the destination directory (%s) already exists!\n", 
				hdr.destdir );
#endif
}

/*---------------------------------------------------------------------*/
unsigned long		copy_to_tempdir( disclimit, cnt_p, temp_dir_name )
/*---------------------------------------------------------------------*/
unsigned long disclimit;
unsigned long *cnt_p;		/* outputs total bytes transferred */
char *temp_dir_name;
/*	returns crc of data written */
{
	unsigned long crcin, ltmp;
	int blks, i, rd_cnt, wrt_cnt, running_pct;
	static int tot_pct;

	if (hdr.vol_num == 1)
		tot_pct = 0;

	/* initialize the input checksum crc variable */
	crcin = checksum(crctable, buf_io, 0, -1);
			
	*cnt_p = 0L;
	blks = 0;
	while ((*cnt_p < disclimit) 
		&& (rd_cnt = read_blk( media, buf_io, 1024 )))
	{
		if (rd_cnt == -1)
			fatal1( "error reading from distribution media!" );
		if (is_eov(buf_io))
			break;
		if ((wrt_cnt = fwrite( buf_io, 1, 1024, out_stream)) == -1)
			fatal1( "error writing to cpio data stream!" );
		*cnt_p += (long) wrt_cnt;
		running_pct = blks * 100 / hdr.media_blk_cnt;
		if (!(++blks % 5))
/*	printf( "%d blocks = %d%% transferred from volume #%d to %s... \r",	*/
/*	blks, running_pct + tot_pct, hdr.vol_num, temp_dir_name );			*/
			printf( "%d blocks transferred from volume #%d to %s... \r",
				blks, hdr.vol_num, temp_dir_name );
		crcin = checksum(crctable, buf_io, rd_cnt, crcin);
	}
	ltmp = (long) blks * (long) hdr.vol_num * 100L;
	ltmp /= (long) hdr.media_blk_cnt;
	tot_pct = (int) ltmp;

/*	printf( "%d blocks = %d%% transferred from volume #%d to %s.   \n", */
/*		blks, tot_pct, hdr.vol_num, temp_dir_name );					*/
	printf( "%d blocks transferred from volume #%d to %s.   \n", 
		blks, hdr.vol_num, temp_dir_name );


#ifdef DEBUG
if (debugging)
	fprintf( stderr,"*** read data: cnt, crc = %ld, %lx ***\n", *cnt_p, crcin);
#endif

	return( crcin );
}

/*---------------------------------------------------------------------*/
int							check_crc( crc )
/*---------------------------------------------------------------------*/
unsigned long crc;
/* 
	Reads crc block from media and compares it with specified crc value.
	TRUE is returned iff they match.
*/
{
	unsigned long media_crc;

	if ((read_blk( media, &media_crc, sizeof(media_crc) )) == -1)
		warn1("error encountered while reading checksum from distribution media!"); 

#ifdef DEBUG
if (debugging)
	fprintf( stderr, "*** read checksum: crc = %lx ***\n", media_crc );
#endif

	return( media_crc == crc );
}

/*---------------------------------------------------------------------*/
void					do_some_initial_stuff()
/*---------------------------------------------------------------------*/
{
	/* this procedure exists simply to avoid duplication of code */

	disclimit = get_media_size( &devname );

	media = mount_volume( devname, 1 );
	strcpy( prodname, hdr.prodname );
	strcpy( rel_id, hdr.rel_id );

	sprintf( toc_file, toc_file_fmt, hdr.audit_name );
	sprintf( audit_file, upgrade_audit_file_fmt, hdr.audit_name );
	if (chdir( audit_dir_name ))
		system( make_audit_dir_cmd );
}


/*---------------------------------------------------------------------*/
void						do_audit( standalone_flag )
/*---------------------------------------------------------------------*/
int standalone_flag;
{
	char cmd[80];
	int i;

	if (standalone_flag)
		do_some_initial_stuff();

	/* check to see if the toc_file exists */
	if ((i = open( toc_file )) == -1)
		fatal1( "you'll have to (re)install this product before auditing!\n" );
	else
		close( i );

	printf( "\n\t--- NOW EXTRACTING CRC'S FROM PRODUCT FILES ---\n" );
	printf( "\nThis may take a minute or two... please be patient!\n\n" );
	sprintf( cmd, "crc -cdf %s >%s 2>/dev/null", toc_file, audit_file );
	system( cmd );
	printf("\nThe following files contain auditing data for this product:\n\n");
	sprintf( cmd, "ls -l %s/%s.*", audit_dir_name, hdr.audit_name );
	system( cmd );
	printf("\n\nIf you want to delete these files, print them out first and put\n");
	printf( "the print-out away with the distribution media for future reference.\n\n" );
}

/*=====================================================================*/
							main( argc, argv )
/*=====================================================================*/
int argc;
char *argv[];
{
	int i, more_prod, eof_flag, this_vol;
	char res, *temp_dir_p;

	Whoami = argv[0];
	CLEAR;

#ifdef RELEASE
	printf( "\n%s, R%d.%d\n", 
		_product_id, _version, _revision );
#else
	printf( "\n%s, R%d.%d (x%03.3d)\n",
		_product_id, _version, _revision, _update );
#endif
	printf( "%s\n", _copyright );

#ifndef DEBUG
	/* check to see if in superuser mode */
	if ((i=setuid(0)) == -1)
		fatal1("You must be logged in as root or have root's setuid permission set!\n");
#endif

	if (argc > 1 && *argv[1] == '-' && argv[1][1] == 'a')
	{
		do_audit( TRUE );
		exit(0);
	}

	if (argc > 1 && *argv[1] == '-' && argv[1][1] == 'v')
	{
		++verify_mode;
		printf( "\n%s: VERIFY-MODE selected!\n", Whoami );
	}

#ifdef DEBUG
	if (argc > 1 && *argv[1] == '-' && argv[1][1] == 'd')
		++debugging;
#endif

	/* initialize the checksum crc table */
	make_crctable( crctable );

	for (more_prod = TRUE; more_prod; )
	{
		do_some_initial_stuff();	/* assigns disclimit and media */

		target_fs_blk_cnt = hdr.fs_blk_cnt;
		if ((target_bsize = get_bsize( hdr.destdir )) == -1)
			warn1( "can't accurately determine block-size of target device!" );
		else
		{
			if (target_bsize > hdr.src_bsize)
				target_fs_blk_cnt /= 2;
			else if (target_bsize < hdr.src_bsize)
				target_fs_blk_cnt *= 2;
		}
		temp_dir_p = find_space( target_fs_blk_cnt, hdr.destdir );

		if (verify_mode && strcmp( temp_dir_p, hdr.destdir ) == 0)
		{
			printf( "%s: VERIFY-MODE WARNING: the temporary directory == destination directory!\n", Whoami );
			if (ask( "Do you want to continue?", no, yes, 0) == 'n')
				exit(1);
		}

		verify_data( temp_dir_p );

		if (!verify_mode)
			verify_destdir();

		printf("\n\t\t------ PRODUCT UPGRADE IN PROGRESS ------\n\n");

		if (!verify_mode && *hdr.pre_cmds)
			system( hdr.pre_cmds );

		chdir( temp_dir_p );

		sprintf( cmd, "cpio -idvum >%s 2>/dev/null", toc_file );
#ifdef DEBUG
if (debugging)
		fprintf( stderr, "*** %s\n", cmd );
#endif
		if ((out_stream = popen(cmd,"w")) <= 0)
			fatal1( "can't popen output stream!" );
	 
		for (this_vol = 1; this_vol <= hdr.max_volumes; this_vol++)
		{
			if (this_vol > 1)
				media = mount_volume( devname, this_vol );

			crc = copy_to_tempdir( disclimit, &cnt, temp_dir_p );
			if (!check_crc( crc ))
			{
				warn1( "The checksums don't match for this volume!" );
				if (ask( "\nDo you want to continue?", no, yes, 0) == 'n')
				{
					pclose( out_stream );
					if (strcmp( temp_dir_p, hdr.destdir ) != 0)
					{
						chdir( "/" );
						sprintf( cmd, "rm -rf %s", temp_dir_p );
						fprintf( stderr, "\n%s\n\n", cmd );
						system( cmd );
					}
					exit(1);
				}
			}
			else if (verify_mode)
				printf( "\nCreated media has been verified as OK!\n" );
		}
		pclose( out_stream );
		close( media );

		/* if the temp_dir isn't same as target dir, move stuff there now */
		if (strcmp( temp_dir_p, hdr.destdir ) != 0)
		{
			if (verify_mode)
				goto clean_up;

			printf("\nNow moving files into the target directory.\n" );
			printf("The name of each file will be displayed as it is copied...\n" );

			chdir( temp_dir_p );
			sprintf(cmd, 
				"find . -print | cpio -pdvum %s | tee %s", 
				hdr.destdir, toc_file );
#ifdef DEBUG
if (debugging)
			fprintf( stderr, "\n*** %s\n", cmd );
#endif
			system( cmd );

clean_up:	chdir( "/" );
			sprintf( cmd, "rm -rf %s ", temp_dir_p );
			if (verify_mode)
				strcat( cmd, toc_file );
			printf( "\n%s\n\n", cmd );
			system( cmd );
		}

		if (verify_mode) 
		{
			printf("Finished verifying `%s (%s).'\n", hdr.prodname, hdr.rel_id);
			res = ask( "Do you want to verify another product's media?", 
					no, yes, 0);
			if (more_prod = (res == 'y'))
				CLEAR;
			continue;	/* escapes the loop here */
		}

		if (*hdr.post_cmds)
			system( hdr.post_cmds );

		printf( "\nA table-of-contents listing of this product is contained in:\n\t`%s'\n",
			toc_file );

		res = ask("Would you like me to generate audit files for this product installation now?",
			yes, no, 0 );

		if (res == 'y')
			do_audit( FALSE );
		else
			printf( "You can use 'upgrade -a' to do the audit later!\n\n" );

		printf("Finished upgrading `%s (%s).'\n", hdr.prodname, hdr.rel_id );

		res = ask( "Do you want to upgrade another product?", no, yes, 0);

		if (more_prod = (res == 'y'))
			CLEAR;
	}
	printf( "\n" );
	exit(0);
}
