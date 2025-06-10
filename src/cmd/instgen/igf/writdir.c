/*
 *	wdir- creates and writes tape directory 
 */

#include	<stdio.h>
#include	"igf.h"
#include	"../com/common.h"

#define		DIRENTSZ	14

#ident	"$Header: /grp/mat/53/utils/igf/RCS/writdir.c,v 1.2 87/01/27 16:17:52 mat Exp $";

wr_dir( )
{
	register char	*cp;
	register int	dir_siz;
	register struct os_files	*osp;
	register struct ar_files	*arp;
	register struct t_dir		*dp;
	struct t_dir	*dir_base;
	int	file_no;
	int	arc_no = 1;
	int	fd;

	VERBOSE("Writing tape directory.");

	/*
	 * Calculate the size of the tape directory, rounding
	 * up to the next T_BSIZE boundary.
	 */
	
	DEBPRT1("writdir: num of dir ents = %d\n", Dirents);
	dir_siz = ( Dirents * sizeof(struct t_dir) );

	DEBPRT2("wdir: writing %x(%d) bytes\n", dir_siz, dir_siz );

	if(( dir_base = (struct t_dir *)malloc(dir_siz)) == NULL ) {
		fprintf(stderr,Nomemmsg, myname);
		return(ERROR);
	}

	/*
	 * Leave dir_base at base of buffer and use "dp" as pointer
	 * into it. dir_base will be used for writing.
	 */
	dp = dir_base;

	/* clear directory buffer */
	memset(dp,0,dir_siz);

	/*
	 * Create OS portion of directory
	 *
	 * 'file_no' starts with '3' for bootable tape and '2'
	 * for non-bootable media. This will be the first file
	 * number after the directory.
	 *
	 *	file#	bootable	non-bootable
	 *	-----	--------	------------
	 *	  0	vid		vid
	 *	  1	bootloader	directory
	 *	  2	directory	arc1
	 *	  3	os1
	 */
	file_no = (Bootable ? 3 : 2);

	if( Verbose )
		printf(" File #\tName\t\tDescription\n");

	if ( Bootable ) {
		/* Operating systems */
		for (osp=Os_head; osp; osp=osp->next,dp++,file_no++) {
			dp->number = file_no;
			dp->type = D_OS;
			if ( (cp=strrchr(osp->os_path, '/')) ) {
				cp++;
			}
			else cp = osp->os_path;
			if ( strlen(cp) > DIRENTSZ ) {
				fprintf(stderr,
				   "%s: operating system name longer than possible?\n",
				   myname);
				return(ERROR);
			}
			strcpy(dp->name, cp);
			strcpy(dp->a_desc, "OPERATING SYS");
			if ( Verbose ) {
				printf("\t% -15s %s\n", dp->name, dp->a_desc);
			}
			DEBPRT1("wr_dir: file number =  %d\n", dp->number);
			DEBPRT1("wr_dir: type        =  %d\n", dp->type);
			DEBPRT1("wr_dir: name        = [%s]\n", dp->name);
		}

		/* Create ramdisk portion of directory */
		dp->number = file_no;
		dp->type = D_RAM;
		strcpy(dp->name, "ramdisk");
		strcpy(dp->a_desc, "FILESYSTEM");
		if ( Verbose ) {
			printf("\t% -15s %s\n", dp->name, dp->a_desc);
		}
		DEBPRT1("wr_dir: file number =  %d\n", dp->number);
		DEBPRT1("wr_dir: type        =  %d\n", dp->type);
		DEBPRT1("wr_dir: name        = [%s]\n", dp->name);
		/* move file number and directory entry pointer along */
		file_no++;
		dp++;
	}

	/* Create scripts entry of directory (if it exists) */
	if ( (arp=Scripts) ) {
		dp->number = file_no++;
		dp->arnum = arc_no++;
		dp->type = D_SCR;
		strcpy(dp->name, arp->archiver);
		strcpy(dp->i_opt, arp->i_opt);
		strcpy(dp->a_desc, arp->a_desc);
		if ( Verbose ) {
			/*printf("  %d\t\t%s\n", dp->arnum, dp->a_desc);*/
			printf(" % 3d\t% -15s %s  (Scripts)\n",
			    dp->arnum, dp->name, dp->a_desc);
		}
		DEBPRT1("wr_dir: file number =  %d\n", dp->number);
		DEBPRT1("wr_dir: type        =  %d\n", dp->type);
		DEBPRT1("wr_dir: name        = [%s]\n", dp->name);
		DEBPRT1("wr_dir: dest dir    = [%s]\n", dp->dest_dir);
		DEBPRT1("wr_dir: inp opts    = [%s]\n", dp->i_opt);
		dp++;
	}

	/* Create archive portion of directory */
	for (arp=Files_head; arp; arp=arp->next,dp++,file_no++,arc_no++) {
		dp->number = file_no;
		dp->arnum = arc_no;
		dp->type = D_ARC;
		strcpy(dp->name, arp->archiver);
		strcpy(dp->i_opt, arp->i_opt);
		strcpy(dp->dest_dir, arp->dest_dir);
		strcpy(dp->a_desc, arp->a_desc);
		if ( Verbose ) {
			/*printf("  %d\t\t%s\n", dp->arnum, dp->dest_dir);*/
			printf(" % 3d\t% -15s %s\n",
			    dp->arnum, dp->name, dp->a_desc);
		}
		DEBPRT1("wr_dir: file number =  %d\n", dp->number);
		DEBPRT1("wr_dir: type        =  %d\n", dp->type);
		DEBPRT1("wr_dir: name        = [%s]\n", dp->name);
		DEBPRT1("wr_dir: dest dir    = [%s]\n", dp->dest_dir);
		DEBPRT1("wr_dir: inp opts    = [%s]\n", dp->i_opt);
	}

	/*
	 *	open the tape for append
	 */
	DEBPRT1("wr_dir: opening %s\n", Device);

	if ( (fd=opensp(M_APPEND)) == ERROR ) {
		return(ERROR);
	}

	/* write the directory block(s) */

	DEBPRT1("wdir: writing %s\n", Device);

	if( write(fd, dir_base, dir_siz) != dir_siz ) {
		fprintf( stderr, "%s: directory write error\n", myname);
		closesp();
		return(ERROR);
	}

	/* close the special file */

	DEBPRT1("wdir: closing %s\n", Device);
	closesp();

	free(dir_base);
	return(0);
}
