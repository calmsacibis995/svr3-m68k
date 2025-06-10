#include	<stdio.h>
#include	"igf.h"
#include	"../com/common.h"

#ident	"$Header: /grp/mat/53/utils/igf/RCS/writarc.c,v 1.2 87/01/27 16:17:49 mat Exp $";

wr_arc()
{
	register struct ar_files	*arp;
	register char	*cur_dir;
	extern char	*getcwd();

	if( Files_head == NULL && Scripts == NULL )
		/* no archives specified */
		return(0);

	if ( Verbose )
		printf("Copying archives.\n");

	/*
	 * Always return to current directory after each archive
	 * command so relative pathlists will work.
	 */
	DEBPRT0("getting Current Working Directory = %s\n");
	if ( (cur_dir=getcwd((char *)NULL, 128)) == NULL ) {
		fprintf(stderr,"%s: cannot obtain current working directory\n",
		    myname);
		return(ERROR);
	}
	DEBPRT1("Current Working Directory = %s\n",cur_dir);

	/*
	 * construct and execute commands. SCRIPTS archives first,
	 * then FILES archives.
	 */
	for( arp = Scripts; arp ; arp = arp->next ) {
		if ( archive(arp, cur_dir) == ERROR ) {
			free(cur_dir);
			return(ERROR);
		}
	}
	for( arp = Files_head; arp ; arp = arp->next ) {
		if ( archive(arp, cur_dir) == ERROR ) {
			free(cur_dir);
			return(ERROR);
		}
	}
	free(cur_dir);
	return(0);
}

archive( arp, cwd )
register struct ar_files	*arp;
char	*cwd;
{
	register char *shcmd;

	/*
	 * Use global I/O buffer for shell command. It will be
	 * plenty big enough.
	 */
	shcmd = Io_buf;

	/*
	 * construct and execute commands.
	 * change to proper directory
	 */
	if ( chdir(arp->src_dir) == ERROR) {
		fprintf(stderr,"%s: cannot changed to directory %s\n",
		    myname, arp->src_dir);
		return(ERROR);
	}
	if ( arp->def_opt ) {
		strcpy(shcmd, "find . -print | ");
	}
	else {
		strcpy(shcmd, "exec ");
	}
	strcat(shcmd, arp->archiver);
	strcat(shcmd, " ");
	strcat(shcmd, arp->o_opt);
	DEBPRT1("SH -> \"%s\"\n", shcmd);

	if ( Verbose ) {
		printf("... %s\n", arp->src_dir);
	}

	if( system(shcmd) < 0 ) {
		fprintf(stderr,"%s: %s error on %s\n",
		    myname, arp->archiver, arp->src_dir);
		return(ERROR);
	}

	/*
	 * Change back to current directory.
	 */
	if ( chdir(cwd) == ERROR) {
		fprintf(stderr,"%s: cannot change back to current directory\n",
		    myname);
		return(ERROR);
	}
	return(0);
}
