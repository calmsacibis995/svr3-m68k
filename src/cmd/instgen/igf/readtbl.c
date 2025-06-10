#include	<stdio.h>
#include	<a.out.h>
#include	<fcntl.h>
#include	<sys/types.h>
#include	<sys/stat.h>
#include	"igf.h"
#include	"../com/common.h"

#ident	"$Header: /grp/mat/53/utils/igf/RCS/readtbl.c,v 1.2 87/01/27 16:17:42 mat Exp $";

#define		INTP(p)	((int *)p)

/*
 * Possible syntax errors in table
 */
#define		E_NOSEP		1
#define		E_SEP		2
#define		E_FSENT		3
#define		E_INVALID	4
#define		E_OTHER		5

/*
 * Definitions of arg2 of access(2)
 */
#define		A_READ		04
#define		A_WRIT		02
#define		A_EXEC		01
#define		A_EXIS		00

/*
 * FILES and SCRIPTS entries used the same routines to save
 * information on their respective queues. A flag will be set
 * with one of the following values to indicate which queue the
 * routine should put the data on.
 */
#define		F_FILES		1
#define		F_SCRIPTS	2

/*
 * After reading a line from the table, we are considered
 * to be at a certain state; just read a blank line, at
 * the top of an entry or in the middle or end of it.
 */
#define		BLANK		1
#define		TOP		2
#define		INENT		4
/*
 * After we have read a line, we would like to know our previous
 * state and our current state. The HI 16 bits are the previous
 * state and the LO 16 bits are the current state.
 */
#define		BLANK_BLANK	0x00010001	/* valid state transition */
#define		BLANK_TOP	0x00010002	/* valid state transition */
#define		BLANK_IN	0x00010004	/* INVALID state transition */
#define		TOP_BLANK	0x00020001	/* valid state transition */
#define		TOP_TOP		0x00020002	/* INVALID state transition */
#define		TOP_IN		0x00020004	/* valid state transition */
#define		IN_BLANK	0x00040001	/* valid state transition */
#define		IN_TOP		0x00040002	/* INVALID state transition */
#define		IN_IN		0x00040004	/* valid state transition */

struct tabent {
	char	*tb_entstr;		/* string or name to match in table */
	short	tb_state;		/* state after reading this type line */
	short	tb_boot;		/* bool - only for bootable media ? */
	int	(*tb_savfcn)();		/* fuction ptr to save data routine */
	int	(*tb_cleanup)();	/* cleanup rtn for this entry */
};

extern int	sav_desc();
extern int	sav_pack();
extern int	sav_ver();
extern int	chk_sav_bl();
extern int	chk_sav_os();
extern int	chk_sav_ram();
extern int	chk_sav_files();
extern int	chk_sav_arc();
extern int	sav_o_opt();
extern int	sav_i_opt();
extern int	sav_a_desc();
extern int	fill_def();

/*
 * The null or blank line entry is Tabents[0] and is a special
 * case. The code depends on this fact.
 */
static struct tabent Tabents[] = {
  { "",		BLANK,	FALSE,	NULL,		NULL },
  { "DESC",	TOP,	FALSE,	sav_desc,	NULL },
  { "PACK",	TOP,	FALSE,	sav_pack,	NULL },
  { "VER",	TOP,	FALSE,	sav_ver,	NULL },
  { "BL",	TOP,	TRUE,	chk_sav_bl,	NULL },
  { "OS",	TOP,	TRUE,	chk_sav_os,	NULL },
  { "RAM",	TOP,	TRUE,	chk_sav_ram,	NULL },
  { "FILES",	TOP,	FALSE,	chk_sav_files,	fill_def },
  { "ARC",	INENT,	FALSE,	chk_sav_arc,	NULL },
  { "O_OPT",	INENT,	FALSE,	sav_o_opt,	NULL },
  { "I_OPT",	INENT,	FALSE,	sav_i_opt,	NULL },
  { "A_DESC",	INENT,	FALSE,	sav_a_desc,	NULL },
  { "SCRIPTS",	TOP,	FALSE,	chk_sav_files,	fill_def },
  { 0 }
};

int		(*Clean_rtn)() = 0;	/* cleanup routine for current entry */
static short	CurState = BLANK;	/* current state or position in table */
static short	Ent_type = 0;		/* SCRIPTS, FILES or neither */

static char	*movetoarg();
char		*sub_device();

read_table( t_path )
char	*t_path;
{
	static char	inbuf[I_BUFSIZ];	/* input line buffer */
	register char	*bp;			/* ptr into inbuf[] */
	register struct tabent	*t;		/* ptr into Tabents[] */
	FILE	*t_fp;				/* file ptr for table */
	int	curline = 0;			/* currenl line in table */
	int	errflag;			/* syntax error in table */
	int	ret;

	/*
	 * If we are going to use standard input instead of a file
	 * or table, the user must be prompted for the differen
	 * tape components.
	 */
	if ( EQ(t_path, "-") ) {
		return( prompt_user(inbuf,Bootable) );	/* prompt for input */
	}

	if ( (t_fp=fopen(t_path, "r")) == NULL ) {
		fprintf(stderr,"%s: unable to open %s\n", myname, t_path);
		return( ERROR );
	}

	/*
	 * Read lines from table and record results. The entries that are
	 * expected to be in there are in Tabents[].
	 */
	
	while ( (ret=getline(inbuf, t_fp)) != EOF && ret != LINETOOLONG ) {
		curline++;	/* keep track of what line we're looking at */
		bp = inbuf;	/* start at beginning of input buffer */

		/* throw away comment lines */
		if ( *bp == COMMENTCHAR ) {
			continue;
		}

		/* clear any white space that shouldn't be at the beginning */
		CLEAR_WHITE(bp);

		/*
		 * See if the line read is one recognized by.
		 */
		if ( *bp == NULL ) {
			/* special case for blank lines */
			t = &Tabents[0];
		}
		else {
			for ( t=&Tabents[1]; *INTP(t); t++ ) {
				if ( EQ_N(bp, t->tb_entstr, strlen(t->tb_entstr)) ) {
					break;
				}
			}
		}

		if ( *INTP(t) == NULL ) {
			/* entry NOT found in table */
			DEBPRT1("entr not found [%s]\n", bp);
			errflag = E_INVALID;
		}
		else {
			if ( t->tb_boot && ! Bootable ) {
				fprintf(stderr,"%s:warning - %s entry not used on non-bootable media\n", myname, t->tb_entstr);
			}

			DEBPRT2("state transition - %#04x%04x\n",
			    CurState, t->tb_state);
			/*
			 * What we want to do with the data in this line depends
			 * on the state we were in before we read this line and
			 * the state we are in now that we have read it (new
			 * state in tabent structure). The value switched
			 * on will have the previous state (CurState)
			 * is in the HI 16 bits and
			 * our new state (t->tb_state) is in the LO 16 bits.
			 */
			switch( (CurState<<16)|t->tb_state ) {
			case BLANK_TOP:
				Clean_rtn = t->tb_cleanup;
				errflag = (*(t->tb_savfcn))(bp, t);
				DEBPRT2("[bt]called rtn for %s, adr %#08X\n",
				    t->tb_entstr, t->tb_savfcn);
				break;
			case TOP_IN:
			case IN_IN:
				errflag = (*(t->tb_savfcn))(bp, t);
				DEBPRT2("[ti/ii]called rtn for %s, adr %#08X\n",
				    t->tb_entstr, t->tb_savfcn);
				break;
			case TOP_BLANK:
			case IN_BLANK:
				if ( Clean_rtn != NULL ) {
					errflag = (*Clean_rtn)();
				}
				DEBPRT2("[tb/ib]?called rtn for %s, adr %#08X\n",
				    t->tb_entstr, t->tb_cleanup);
				break;
			case BLANK_BLANK:
				/* multiple blank lines OK */
				break;
			case BLANK_IN:
				errflag = E_SEP;
				break;
			case TOP_TOP:
			case IN_TOP:
				errflag = E_NOSEP;
				break;
			default:
				/* should never get here */
				fprintf(stderr,"%s: internal error\n", myname);
				fprintf(stderr,"unknown state transition %#08X\n", (CurState<<16)|t->tb_state);
				exit(-1);
			} /* END of switch() */
		} /* END of if () else if() else */

		if ( errflag ) {
			switch(errflag) {
			case E_NOSEP:
			    fprintf(stderr,"%s: This line must be in a separat entry, line %d:\n\t\"%s\"\n", myname, curline, inbuf);
			    break;
			case E_SEP:
			    fprintf(stderr,"%s: This line cannot be at the top of an entry, line %d:\n\t\"%s\"\n", myname, curline, inbuf);
			    break;
			case E_FSENT:
			    fprintf(stderr,"%s: This line must be part of a FILES/SCRIPTS entry, line %d:\n\t\"%s\"\n", myname, curline, inbuf);
			    break;
			case E_INVALID:
			    fprintf(stderr,"%s: invalid entry name - %d\n\t\"%s\"\n", myname, curline, inbuf);
			    break;
			case E_OTHER:
			    /* error message reported in child routine */
			    break;
			}
			fclose(t_fp);
			return( ERROR );
		}

		/* update current state */
		CurState = t->tb_state;
	}
	/*
	 * If last entry was a multiple line entry, some cleanup
	 * may need to be done.
	 */
	if ( Clean_rtn ) {
		if ( (*Clean_rtn)() == ERROR ) {
			fclose( t_fp );
			return( ERROR );
		}
	}

	fclose(t_fp);

	/*
	 * Check to make sure all the necessary information is specified.
	 */
	/* re-use "ret" variable */
	ret = 0;
	if( Bootable ) {
		if ( Ram == NULL ) {
			/* only a warning, may want to boot stand alone prog */
			fprintf(stderr,
			    "%s: warnning - ramdisk entry not specified\n",
			    myname);
		}
		if ( Bootloader == NULL ) {
			ret++;
			fprintf(stderr,
			    "%s: bootloader entry not specified\n",
			    myname);
		}
		if ( Os_head == NULL ) {
			ret++;
			fprintf(stderr,"%s: operating system entry not specified\n",
			    myname);
		}
	}
	if ( Files_head == NULL ) {
		/* only an error if its not bootable */
		if ( Bootable ) {
			fprintf(stderr,
			    "%s: warning - files entry not specified\n",
			    myname);
		}
		else {
			ret++;
			fprintf(stderr,"%s: files entry not specified\n",
			    myname);
		}
	}

	return( ret );
}

fill_def()
{
	register int	cnt = 0;
	register struct ar_files	*ap;
	char	*mp1, *mp2, *mp3, *mp4;
	int	errflag = 0;

	/*
	 * Get to the least entry in the files link list. This will
	 * be the current one.
	 */
	if ( Ent_type == F_FILES ) ap = Files_head;
	else ap = Scripts;

	for ( ; ap->next; ap=ap->next) ;

	/*
	 * We know there will be a source and destination directory at this
	 * point, but we must check the archiver, input options and output
	 * options. If any one of these is specified, all of them must be,
	 * otherwise fill them in with defaults.
	 */
	if (ap->archiver) cnt++;
	if (ap->o_opt) cnt++;
	if (ap->i_opt) cnt++;

	if ( cnt == 1 || cnt == 2 ) {
		errflag = ERROR;
		fprintf(stderr,"%s: all or none of these options must be specified for FILES entries:\n\tARC, O_OPT, I_OPT\n", myname);
	}
	else if ( cnt == 0 ) {
		ap->def_opt = TRUE;
		if ( (mp1=malloc(strlen(ARCHIVER)+1)) == NULL ||
		     (mp2=sub_device(DEF_OUT)) == NULL ||
		     (mp3=malloc(strlen(DEF_INP)+1)) == NULL ) {
			errflag = ERROR;
			fprintf(stderr,Nomemmsg,myname);
		}
		else {
			strcpy(mp1, ARCHIVER);
			ap->archiver = mp1;
			ap->o_opt = mp2;
			strcpy(mp3, DEF_INP);
			ap->i_opt = mp3;
		}
	}
	/* else already filled in (cnt==3) */

	/* If not there, add a description */
	if ( ! errflag && ap->a_desc == NULL ) {
		/* re-use char *mp1, *mp2 */
		mp1 = ((Ent_type == F_FILES) ? "ARCHIVE" : "SCRIPTS");
		if ( (mp2=malloc(strlen(mp1)+1)) != NULL ) {
			/* Add in default description */
			strcpy(mp2, mp1);
			ap->a_desc = mp2;
		}
	}

	/* at the end of entry, clear global entry flag */
	Ent_type = 0;

	return(errflag);
}

chk_sav_bl( p, tp )
register char	*p;
register struct tabent	*tp;
{
	register char	*mp;

	/* Get past keyword and "=" sign */
	if ( (p = movetoarg(p)) == (char *)E_INVALID ) {
		return( E_INVALID );
	}

	/*
	 * Check that the file exists and is a regular a.out file
	 */
	if ( ! chkaout(p) ) {
		fprintf(stderr,
		    "%s: %s does not exist or is an invalid bootloader file\n",
		    myname, p);
		return(E_OTHER);
	}

	if ( (mp=malloc(strlen(p)+1)) == NULL ) {
		fprintf(stderr,Nomemmsg, myname);
		return(E_OTHER);
	}

	/* set up global bootloader variable */
	Bootloader = strcpy(mp, p);

	return(0);
}

chk_sav_os( p, tp )
register char	*p;
register struct tabent	*tp;
{
	register char	*mp;
	register struct os_files	*osp, *nosp;

	/* Get past keyword and "=" sign */
	if ( (p = movetoarg(p)) == (char *)E_INVALID ) {
		return( E_INVALID );
	}

	/*
	 * Check that the file exists and is a regular a.out file
	 */
	if ( ! chkaout(p) ) {
		fprintf(stderr,
		    "%s: %s does not exist or is an invalid bootloader file\n",
		    myname, p);
		return(E_OTHER);
	}

	/*
	 * Since there may be multiple operating systems, tack this one
	 * on to the end of the link list.
	 */
	if ( (mp=malloc(strlen(p)+1)) == NULL ) {
		fprintf(stderr,Nomemmsg, myname);
		return(E_OTHER);
	}

	if ((nosp=(struct os_files *)malloc(sizeof(struct os_files))) == NULL) {
		fprintf(stderr,Nomemmsg, myname);
		return(E_OTHER);
	}

	/* link the new on onto the end of the list and save name */
	if (Os_head) {
		for (osp = Os_head; osp->next; osp=osp->next) ;
		osp->next = nosp;
	}
	else {
		Os_head = nosp;
	}
	nosp->os_path = strcpy(mp, p);
	nosp->next = NULL;

	return(0);
}

chk_sav_ram( p, tp )
register char	*p;
register struct tabent	*tp;
{
	register char	*mp;

	/* Get past keyword and "=" sign */
	if ( (p = movetoarg(p)) == (char *)E_INVALID ) {
		return( E_INVALID );
	}

	/*
	 * Check that the file exists and is a regular file
	 */
	if ( access(p, A_READ) == ERROR ) {
		fprintf(stderr,
		    "%s: %s does not exist or is an invalid ramdisk\n",
		    myname, p);
		return(E_OTHER);
	}

	if ( (mp=malloc(strlen(p)+1)) == NULL ) {
		fprintf(stderr,Nomemmsg, myname);
		return(E_OTHER);
	}

	/* set up global bootloader variable */
	Ram = strcpy(mp, p);

	return(0);
}

chk_sav_files( p, tp )
register char	*p;
register struct tabent	*tp;
{
	register char	*src, *dst;
	register struct ar_files	*arp, *narp;
	int	nargs;
	int	scrent;			/* type of entry is SCRIPTS */
	char	srcbuf[I_BUFSIZ],	/* tmp buffers for pathlists */
		dstbuf[I_BUFSIZ];
	struct stat	st;

	/* Get past keyword and "=" sign */
	if ( (p = movetoarg(p)) == (char *)E_INVALID ) {
		return( E_INVALID );
	}

	nargs = sscanf(p, "%s%s", srcbuf, dstbuf);

	/*
	 * Should only be one SCRIPTS entry.
	 */
	if ( EQ_N("SCRIPTS", tp->tb_entstr, strlen(tp->tb_entstr)) ) {
		scrent=TRUE;
		if ( Scripts != NULL ) {
			/* already this is the second SCRIPTS entry */
			fprintf(stderr,
			    "%s: more than one SCRIPTS entry\n", myname);
			return( E_OTHER );
		}
		else if ( nargs != 1 ) {
			fprintf(stderr,
			    "%s: only source directory for SCRIPTS entry\n",
			    myname);
			return( E_OTHER );
		}
	}
	else {
		scrent=FALSE;
		if ( nargs != 2 ) {
			fprintf(stderr,
			    "%s: need source and destination directories for FILES entry\n",
			    myname);
			return( E_OTHER );
		}
	}

	/*
	 * Check that the source directories exist and is read/searchable
	 */
	if ( stat(srcbuf, &st) == ERROR || ! ISDIR(st) ) {
		fprintf(stderr,
		    "%s: source directory %s does not exist or is not searchable/readable\n",
		    myname, srcbuf);
		return(E_OTHER);
	}

	/* check length of destination directory */
	if ( strlen(dstbuf) >= D_OPTSZ ) {
		fprintf(stderr,
		    "%s: destination directory \"%s\" too long for tape directory\n",
		    myname, dstbuf);
		return(E_OTHER);
	}

	if ( ! scrent && dstbuf[0] != '/' ) {
		fprintf(stderr,
		  "%s: warning - destination directory \"%s\" in not rooted at /\n",
		  myname, dstbuf);
	}

	/*
	 * Since there may be multiple archives, tack this one
	 * on to the end of the link list.
	 */
	if ((narp=(struct ar_files *)malloc(sizeof(struct ar_files))) == NULL) {
		fprintf(stderr,Nomemmsg, myname);
		return(E_OTHER);
	}

	if ( (src=malloc(strlen(srcbuf)+1)) == NULL ) {
		fprintf(stderr,Nomemmsg, myname);
		return(E_OTHER);
	}

	if ( !scrent && (dst=malloc(strlen(dstbuf)+1)) == NULL ) {
		fprintf(stderr,Nomemmsg, myname);
		return(E_OTHER);
	}

	/*
	 * This routine is used for FILES and SCRIPTS entries.
	 */
	if ( scrent == FALSE ) {
		Ent_type = F_FILES;
		arp = Files_head;
	}
	else {
		Ent_type = F_SCRIPTS;
		arp = Scripts;
	}

	/* link the new one on to the end of the list and save name */
	if (arp) {
		for ( ; arp->next; arp=arp->next) ;	/* get to last one */
		arp->next = narp;			/* link on new one */
	}
	else {
		if ( Ent_type == F_FILES ) Files_head = narp;
		else Scripts = narp;
	}
	narp->src_dir = strcpy(src, srcbuf);
	narp->dest_dir = scrent ? NULL : strcpy(dst, dstbuf);
	narp->archiver = NULL;
	narp->o_opt = NULL;
	narp->i_opt = NULL;
	narp->next = NULL;
	DEBPRT2("chk_sav_files: SRC [%s] - DST [%s]\n", src, dst);

	return(0);
}

chk_sav_arc( p, tp )
register char	*p;
register struct tabent	*tp;
{
	register char	*mp;
	register struct ar_files	*arp;

	/* Get past keyword and "=" sign */
	if ( (p = movetoarg(p)) == (char *)E_INVALID ) {
		return( E_INVALID );
	}

	/*
	 * Because of space constraints in the tape directory, the archiver
	 * cannot be a pathlist. When the tape is extracted, the $PATH
	 * environment variable will be used and can be set up to force
	 * looking in certain directories for the archiver utility.
	 */
	if ( strchr(p, '/') != NULL ) {
		fprintf(stderr,
		    "%s: pathlist \"%s\" invalid; must only be utility name\n",
		    myname, p);
		return(E_OTHER);
	}

	/*
	 * Search for archiver utility by looking through $PATH
	 * and make sure it is executable.
	 */
	if ( ! inpath(p) ) {
		fprintf(stderr,
		  "%s: archiving utility %s not in path or is not executable\n",
		  myname, p);
		return( E_OTHER );
	}

	if ( (mp=malloc(strlen(p)+1)) == NULL ) {
		fprintf(stderr,Nomemmsg, myname);
		return(E_OTHER);
	}

	/*
	 * Find proper FILES/SCRIPTS structure. This is guarenteed to be
	 * the last one on the link list.
	 */
	if ( Ent_type == F_FILES ) arp = Files_head;
	else arp = Scripts;

	for ( ; arp->next; arp=arp->next) ;

	arp->archiver = strcpy(mp, p);
	
	return(0);
}

sav_o_opt( p, tp )
register char	*p;
register struct tabent	*tp;
{
	register char	*mp;
	register struct ar_files	*arp;

	/* Get past keyword and "=" sign */
	if ( (p = movetoarg(p)) == (char *)E_INVALID ) {
		return( E_INVALID );
	}

	/*
	 * Pass string to sub_device() which will substitute
	 * the string "$DEVICE" with the actual output device
	 */
	if ( (mp=sub_device(p)) == 0 ) {
		fprintf(stderr,Nomemmsg, myname);
		return(E_OTHER);
	}

	/*
	 * Find proper FILES/SCRIPTS structure. This is guarenteed to be
	 * the last one on the link list.
	 */
	if ( Ent_type == F_FILES ) arp = Files_head;
	else arp = Scripts;

	for ( ; arp->next; arp=arp->next) ;

	arp->o_opt = mp;
	
	return(0);
}

static char *
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

sav_i_opt( p, tp )
register char	*p;
register struct tabent	*tp;
{
	register char	*mp;
	register struct ar_files	*arp;

	/* Get past keyword and "=" sign */
	if ( (p = movetoarg(p)) == (char *)E_INVALID ) {
		return( E_INVALID );
	}

	/* check length of input options */
	if ( strlen(p) >= D_OPTSZ ) {
		fprintf(stderr,
		    "%s: input options \"%s\" too long for tape directory\n",
		    myname, p);
		return(E_OTHER);
	}

	if ( (mp=malloc(strlen(p)+1)) == NULL ) {
		fprintf(stderr,Nomemmsg, myname);
		return(E_OTHER);
	}

	/*
	 * Find proper FILES/SCRIPTS structure. This is guarenteed to be
	 * the last one on the link list.
	 */
	if ( Ent_type == F_FILES ) arp = Files_head;
	else arp = Scripts;

	for ( ; arp->next; arp=arp->next) ;

	arp->i_opt = strcpy(mp, p);
	
	return(0);
}

sav_a_desc( p, tp )
register char	*p;
register struct tabent	*tp;
{
	register char	*mp;
	register struct ar_files	*arp;

	/* Get past keyword and "=" sign */
	if ( (p = movetoarg(p)) == (char *)E_INVALID ) {
		return( E_INVALID );
	}

	/* check length of input options */
	if ( strlen(p) >= D_DSCSZ ) {
		fprintf(stderr,
		    "%s: warning - archive description \"%s\" too long for tape directory, truncating\n",
		    myname, p);
		*(p+(D_DSCSZ-1)) = NULL;
	}

	if ( (mp=malloc(strlen(p)+1)) == NULL ) {
		fprintf(stderr,Nomemmsg, myname);
		return(E_OTHER);
	}

	/*
	 * Find proper FILES/SCRIPTS structure. This is guarenteed to be
	 * the last one on the link list.
	 */
	if ( Ent_type == F_FILES ) arp = Files_head;
	else arp = Scripts;

	for ( ; arp->next; arp=arp->next) ;

	arp->a_desc = strcpy(mp, p);
	
	return(0);
}

sav_desc( p, tp )
register char	*p;
register struct tabent	*tp;
{
	register int	len;

	/* Get past keyword and "=" sign */
	if ( (p = movetoarg(p)) == (char *)E_INVALID ) {
		return( E_INVALID );
	}

	if ( (len=strlen(p)) > DESCSIZ) {
		fprintf(stderr,"truncating version to %d characters\n",DESCSIZ);
		len = DESCSIZ;
	}

	if ( (Desc=malloc(len)) == NULL ) {
		fprintf(stderr, Nomemmsg, myname);
		return(E_INVALID);
	}
	strncpy(Desc, p, len);
	return(0);
}

sav_pack( p, tp )
register char	*p;
register struct tabent	*tp;
{
	register int	len;

	/* Get past keyword and "=" sign */
	if ( (p = movetoarg(p)) == (char *)E_INVALID ) {
		return( E_INVALID );
	}

	if ( (len=strlen(p)) > PACKSIZ) {
		fprintf(stderr,"truncating version to %d characters\n",PACKSIZ);
		len = PACKSIZ;
	}

	if ( (Package=malloc(len)) == NULL ) {
		fprintf(stderr, Nomemmsg, myname);
		return(E_INVALID);
	}
	strncpy(Package, p, len);
	return(0);
}

sav_ver( p, tp )
register char	*p;
register struct tabent	*tp;
{
	register int	len;

	/* Get past keyword and "=" sign */
	if ( (p = movetoarg(p)) == (char *)E_INVALID ) {
		return( E_INVALID );
	}

	if ( (len=strlen(p)) > VERSIZ) {
		fprintf(stderr,"truncating version to %d characters\n",VERSIZ);
		len = VERSIZ;
	}

	if ( (Version=malloc(len)) == NULL ) {
		fprintf(stderr, Nomemmsg, myname);
		return(E_INVALID);
	}
	strncpy(Version, p, len);
	return(0);
}

static char *
movetoarg( p )
register char	*p;
{

	/* advance to "=" in string */
	while (*p != '=' && *p != NULL) p++;
	if (*p == NULL) {
		return( (char *)E_INVALID );
	}

	p++;		/* get past '=' sign */

	/* get to first non-white character */
	while ( (*p == ' ' || *p == '\t') && *p != NULL ) p++;
	if (*p == NULL) {
		return( (char *)E_INVALID );
	}
	return(p);
}

chkaout(f)
char	*f;
{
	register od;
	register ret = 0;
	struct filehdr fhdr;

	if( (od=open(f, O_RDONLY)) == ERROR )
		return(0);

	if (read(od, &fhdr, sizeof(fhdr)) == sizeof(fhdr) ) {
#ifdef	m68k
		if( fhdr.f_magic == MC68MAGIC ) {
			ret = 1;
		}
#endif
	}

	close(od);
	return(ret);
}

inpath( util )
char	*util;
{
	char	*path,
		*elm;		/* element of $PATH variable */
	char	path_buf[I_BUFSIZ];
	char	our_path[I_BUFSIZ];
	int	found = FALSE;	/* bool: util found in $PATH dirs */
	struct stat	st;

	if ( (path=getenv("PATH")) == NULL ) {
		return(FALSE);
	}

	/*
	 * Make a local copy of PATH so strtok does not write in
	 * the environment space.
	 */
	strcpy(our_path, path);

	if ( (elm = strtok(our_path, ":")) == NULL ) {
		return(FALSE);
	}

	/*
	 * Check to see if the utility is in current path and is
	 * executable.
	 */
	do {
		strcpy(path_buf, elm);
		strcat(path_buf, "/");
		strcat(path_buf, util);
		DEBPRT1("looking for util --> %s\n", path_buf);
		if ( access(path_buf, A_EXEC) == 0 ) {
			DEBPRT1("executable utility found --> %s\n", path_buf);
			found=TRUE;
			break;	/* out of do-while loop */
		}
	} while ( (elm=strtok((char *)NULL, ":")) );

	if ( ! found ) {
		return(FALSE);
	}

	return(TRUE);
}

prompt_user(lnbuf, bootflag)
char	*lnbuf;
int	bootflag;
{
	register char	*buf;		/* ptr into lnbuf */
	register int	use_def;	/* default archiving utility and opts */
	int	goodcnt;

	/*
	 * Make string read from user look like one from the table/file
	 * so the same chk_sav_* routines can be used. This is done by
	 * placing an '=' sign at the beginning of the string and tacking
	 * user input onto it.
	 */
	buf = lnbuf;
	*buf++ = '=';

	if ( bootflag ) {
		printf("\nEnter bootloader pathlist:\n\t ");
		getline(buf, stdin);
		if (chk_sav_bl(lnbuf)) {
			return(ERROR);
		}

		goodcnt = 0;
		while ( FOREVER ) {
			printf("Enter operating system(s) pathlist(s) [RETURN TO END]:\n\t ");
			getline(buf, stdin);
			/*
			 * No response is OK if at least one good operating
			 * system was given previously.
			 */
			if ( *buf == NULL && goodcnt > 0 ) {
				break;		/* out of while loop */
			}
			if (chk_sav_os(lnbuf)) {
				return(ERROR);
			}
			goodcnt++;
		}

		printf("Enter ramdisk file or device:\n\t ");
		getline(buf, stdin);
		if (chk_sav_ram(lnbuf)) {
			return(ERROR);
		}
	}

	goodcnt = 0;
	while ( FOREVER ) {
		if ( ! bootflag ) {
			printf("\n");
		}
		printf("Enter source and destination directories for archives [RETURN TO END]:\n\t");
		getline(buf, stdin);
		if ( *buf == NULL && goodcnt > 0 ) {
			break;		/* out of while loop */
		}
		if (chk_sav_files(lnbuf)) {
			return(ERROR);
		}

		printf("Enter archive utility [ %s ]:\n\t ", ARCHIVER);
		getline(buf, stdin);
		if ( *buf == NULL ) {
			use_def = TRUE;
			strcpy(buf, ARCHIVER);
		}
		else use_def = FALSE;
		if (chk_sav_arc(lnbuf)) {
			return(ERROR);
		}

		printf("Enter archive utility's output options [ %s ]:\n\t ",
		    use_def ? DEF_OUT : DEF_NULL);
		getline(buf, stdin);
		if ( *buf == NULL ) {
			strcpy(buf, use_def ? DEF_OUT : DEF_NULL);
		}
		if (sav_o_opt(lnbuf)) {
			return(ERROR);
		}

		printf("Enter archive utility's input options [ %s ]:\n\t ",
		    use_def ? DEF_INP : DEF_NULL);
		getline(buf, stdin);
		if ( *buf == NULL ) {
			strcpy(buf, use_def ? DEF_INP : DEF_NULL);
		}
		if (sav_i_opt(lnbuf)) {
			return(ERROR);
		}
		
		goodcnt++;
	}
	return(0);
}
