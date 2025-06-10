%{
#include "sysgen.h"

extern char *new_desc(), *new_configfile(), *new_configitem(),
	*new_driver(), *new_device(), *new_param(), *new_mpu(), *new_force(),
	*new_probe(), *new_alien(), *new_dup(), *new_ram(), *new_cfname(),
	*new_sysdev(), *new_minor();

int yyerror(s)
char *s;
{
	fprintf(stderr, "%s: %s(%s): line no. %d, syntax error\n",
		sysgen, yyfdp->fd_name, yyfdp->fd_uvers, yyline);
	return(0);
}

%}
%token T_DESCFILE	/* description file				*/
%token T_MACHFILE	/* machine file					*/
%token T_DESCKEY	/* description keyname				*/
%token T_DRIVER		/* driver description				*/
%token T_DRVNAME	/* driver name					*/
%token T_DRVHDLR	/* driver handler prefix			*/
%token T_DRVIVS		/* driver interrupt vectors size		*/
%token T_DRVMBDN	/* driver major block device number		*/
%token T_DRVMCDN	/* driver major character device number		*/
%token T_DRVMBRL	/* driver maximum bus request level		*/
%token T_DRVMDPC	/* driver maximum devices per controller	*/
%token T_DRVPRS		/* driver page registers size			*/
%token T_BCMASK		/* block or character device mask lines		*/
%token T_DRVSTR		/* driver is a stream handler			*/
%token T_DRVSBCH	/* separate open/close for block/char devices	*/
%token T_DRVTTY		/* driver has a tty structure			*/
%token T_DRVINIT	/* driver has an initialization handler		*/
%token T_DRVPFAIL	/* driver has a power-failure handler		*/
%token T_DRVOPEN	/* driver has an open handler			*/
%token T_DRVCLOSE	/* driver has a close handler			*/
%token T_DRVREAD	/* driver has a read handler			*/
%token T_DRVWRITE	/* driver has a write handler			*/
%token T_DRVIOCTL	/* driver has an ioctl handler			*/
%token T_FILSYS		/* filesystem handler description		*/
%token T_FSHNM		/* filesystem handler name			*/
%token T_FSHMASK	/* filesystem handler device mask lines		*/
%token T_FSHIOCTL	/* filesystem handler has an ioctl handler	*/
%token T_FSHINFO	/* filesystem handler has an info handler	*/
%token T_FSHCTRL	/* filesystem handler has a control handler	*/
%token T_FSHNOTIFY	/* filesystem handler has a notify handler	*/
%token T_FSHSATTR	/* filesystem handler has a set attr handler	*/
%token T_FSHRDMAP	/* filesystem handler has a read map handler	*/
%token T_FSHFRMAP	/* filesystem handler has a free map handler	*/
%token T_FSHALMAP	/* filesystem handler has a alloc map handler	*/
%token T_FSHGDENTS	/* filesystem handler has a get dents handler	*/
%token T_FSHACCESS	/* filesystem handler has an access handler	*/
%token T_FSHSTATUS	/* filesystem handler has a status handler	*/
%token T_FSHUPDATE	/* filesystem handler has an update handler	*/
%token T_FSHCLINOD	/* filesystem handler has a close inode handler	*/
%token T_FSHOPINOD	/* filesystem handler has an open inode handler	*/
%token T_FSHGTINOD	/* filesystem handler has a get inode handler	*/
%token T_FSHUMOUNT	/* filesystem handler has a umount handler	*/
%token T_FSHMOUNT	/* filesystem handler has a mount handler	*/
%token T_FSHNAMEI	/* filesystem handler has a namei handler	*/
%token T_FSHFSTAT	/* filesystem handler has a status file handler	*/
%token T_FSHITRUNC	/* filesystem handler has an inode trunc hdlr	*/
%token T_FSHINIT	/* filesystem handler has an init handler	*/
%token T_FSHWRINOD	/* filesystem handler has a write inode handler	*/
%token T_FSHRDINOD	/* filesystem handler has a read inode handler	*/
%token T_FSHUPINOD	/* filesystem handler has an update inode hdlr	*/
%token T_FSHIREAD	/* filesystem handler has an inode read handler	*/
%token T_FSHIPUT	/* filesystem handler has an inode put handler	*/
%token T_DRVTYPE	/* driver type information			*/
%token T_DRVCIVA	/* create interrupt vector array for this driver */
%token T_DRVCCBMN	/* create char or block major nos for this driver */
%token T_DRVCILA	/* create interrupt level array for this driver	*/
%token T_DRVONE		/* allow only one of these drivers		*/
%token T_DRVSCF		/* do not create a count field for this driver	*/
%token T_DRVSIV		/* do not create an int vector for this driver	*/
%token T_DRVREQ		/* this is a required driver			*/
%token T_DRVBLOCK	/* this is a block driver			*/
%token T_DRVCHAR	/* this is a character driver			*/
%token T_DRVIDD		/* this is an interrupt driven driver		*/
%token T_DRVSVMA	/* allow 1 vector defn with multiple addrs	*/
%token T_DRVDATA	/* driver data information			*/
%token T_DRVCTSD	/* driver config table structure declaration	*/
%token T_DEVDESC	/* device description				*/
%token T_DEVTYPE	/* device driver name				*/
%token T_DEVBRL		/* device bus request level			*/
%token T_DEVADDR	/* device address				*/
%token T_DEVIVL		/* device interrupt vector location		*/
%token T_DEVNOD		/* device number of devices			*/
%token T_DEVDNOD	/* default number of devices			*/
%token T_SYSDEV		/* system devices specification			*/
%token T_SDROOT		/* root device					*/
%token T_SDPIPE		/* pipe device					*/
%token T_SDDUMP		/* dump device					*/
%token T_SDSWAP		/* swap device					*/
%token T_MINOR		/* minor device numbers specification		*/
%token T_MROOT		/* root minor device number			*/
%token T_MPIPE		/* pipe minor device number			*/
%token T_MDUMP		/* dump minor device number			*/
%token T_MSWAP		/* swap minor device number			*/
%token T_MSWPLO		/* lowest block on device for swap		*/
%token T_MNSWAP		/* number of blocks for swap			*/
%token T_PARDESC	/* parameter description			*/
%token T_PARNAME	/* parameter name				*/
%token T_PARCONF	/* conf.c name					*/
%token T_PARDVAL	/* default value				*/
%token T_MPU		/* mpu description				*/
%token T_MPUNUM		/* mpu number					*/
%token T_FORCE		/* non-unique driver specification		*/
%token T_FRCID		/* unique identifier				*/
%token T_PROBE		/* memory probe specification			*/
%token T_PRBADDR	/* probe address				*/
%token T_PRBVAL		/* probe value					*/
%token T_ALIEN		/* alien handler entry specification		*/
%token T_ALVEC		/* normal exception vector for alien entry pt	*/
%token T_ALADDR		/* entry point for alien handler		*/
%token T_ALNAME		/* alien handler name				*/
%token T_DUP		/* multiple handler specification		*/
%token T_DUPINTRET	/* on 0, go to the normal interrupt return pt	*/
%token T_DUPALLTRAPS	/* on 0, go to the normal trap return point	*/
%token T_DUPBEQRTS	/* on 0, go to the branch equal return point	*/
%token T_DUPARG		/* argument is to be passed to handler		*/
%token T_DUPVEC		/* exception vector address			*/
%token T_DUPHDLR	/* exception handler name			*/
%token T_DUPARGVAL	/* optional handler argument			*/
%token T_RAM		/* memory configuration specification		*/
%token T_RAMNOPARTY	/* memory has no parity check			*/
%token T_RAMSINGLE	/* a single memory block may exist		*/
%token T_RAMMULTIPLE	/* multiple memory blocks may be located	*/
%token T_RAMLOW		/* low memory address				*/
%token T_RAMHIGH	/* high memory address				*/
%token T_RAMSIZE	/* memory block size				*/
%token T_CNFGFILE	/* configuration file's name			*/
%token T_CNFGVERS	/* configuration file's version			*/
%token T_CNFGITEM	/* configuration item				*/
%token T_CNFGVALU	/* configuration item value			*/
%token T_NUM		/* number					*/
%token T_STRING		/* string					*/
%token T_ERROR		/* erroneous input				*/
%%
fileformat:	T_DESCFILE string
		descriptions
	{
		/* description file */
		if (yydir == descriptions) {
			yyfdp->fd_desc = (char *)$2;
			yyfdp->fd_data = (TABLE *)$3;
		} else {
			fprintf(stderr, "%s: %s(%s): not proper type\n", sysgen,
				yyfdp->fd_name, yyfdp->fd_uvers, yyline);
		}
	}
	|	T_MACHFILE string
		configfiles
	{
		/* machine file */
		if (yydir == configs) {
			yyfdp->fd_desc = (char *)$2;
			yyfdp->fd_data = (TABLE *)$3;
		} else {
			fprintf(stderr, "%s: %s(%s): not proper type\n", sysgen,
				yyfdp->fd_name, yyfdp->fd_uvers, yyline);
		}
	}
	|	error
	{
		fprintf(stderr, "%s: %s(%s): line #%d: erroneous input\n",
			sysgen, yyfdp->fd_name, yyfdp->fd_uvers, yyline);
		return(-1);
	}
	;

descriptions:	/* empty */
	{
		$$ = (int)alloc_desc_table();
	}
	|	descriptions
		description
	{
		put_entry_in_desc_table($1, $2);
		$$ = $1;
	}
	|	descriptions
		error
	{
		$$ = $1;
	}
	;

configfiles:	/* empty */
	{
		/* a machine description file selection */
		$$ = (int)alloc_configfile_table();
	}
	|	configfiles
		configfile
	{
		put_entry_in_configfile_table($1, $2);
		$$ = $1;
	}
	|	configfiles
		error
	{
		$$ = $1;
	}
	;

configfile:	T_CNFGFILE string
		T_CNFGVERS string
		configitems
	{
		$$ = (int)new_configfile($2, $4, $5);
	}
	;

configitems:	/* empty */
	{
		$$ = (int)alloc_configitem_table();
	}
	|	configitems
		configitem
	{
		put_entry_in_configitem_table($1, $2);
		$$ = $1;
	}
	;

configitem:	T_CNFGITEM string
	{
		$$ = (int)new_configitem($2, CIT_NONE, 0);
	}
	|	T_CNFGITEM string
		T_CNFGVALU num
	{
		$$ = (int)new_configitem($2, CIT_NUM, $4);
	}
	|	T_CNFGITEM string
		T_CNFGVALU string
	{
		$$ = (int)new_configitem($2, CIT_STRING, $4);
	}
	;

description:	T_DRIVER	string
		T_DESCKEY	string
		T_DRVNAME	string
		T_DRVHDLR	string
		T_DRVIVS	num
		T_DRVMBDN	num
		T_DRVMCDN	num
		T_DRVMBRL	num
		T_DRVMDPC	num
		T_DRVPRS	num
		T_BCMASK	drvbcmask
		drvtype
		drvdata
	{
		$$ = (int)new_desc($4, $2, DT_DRIVER, new_driver($6, $8, $10,
				$12, $14, $16, $18, $20, $22, $23, $24));
	}
	|	T_FILSYS	string
		T_DESCKEY	string
		T_FSHNM		string
		T_DRVHDLR	string
		T_DRVIVS	num
		T_DRVMBDN	num
		T_DRVMCDN	num
		T_DRVMBRL	num
		T_DRVMDPC	num
		T_DRVPRS	num
		T_FSHMASK	drvfshmask
		drvtype
		drvdata
	{
		$$ = (int)new_desc($4, $2, DT_FILSYS, new_driver($6, $8, $10,
			$12, $14, $16, $18, $20, $22, $23 | 010000, $24));
	}
	|	T_DEVDESC	string
		T_DESCKEY	string
		T_DEVTYPE	string
		T_DEVBRL	num
		T_DEVADDR	num
		T_DEVIVL	num
		T_DEVDNOD	num
	{
		$$ = (int)new_desc($4, $2, DT_DEVICE,
			new_device($6, $8, $10, $12, $14));
	}
	|	T_SYSDEV	string
		T_DESCKEY	string
		sysdevs
	{
		$$ = (int)new_desc($4, $2, DT_SYSDEV, $5);
	}
	|	T_PARDESC string
		T_DESCKEY string
		T_PARNAME string
		T_PARCONF string
		T_PARDVAL string
	{
		$$ = (int)new_desc($4, $2, DT_PARAM, new_param($6, $8, $10));
	}
	|	T_MPU string
		T_DESCKEY string
		T_MPUNUM num
	{
		$$ = (int)new_desc($4, $2, DT_MPU, new_mpu($6));
	}
	|	T_FORCE		string
		T_DESCKEY	string
		T_FRCID		string
	{
		$$ = (int)new_desc($4, $2, DT_FORCE, new_force($6));
	}
	|	T_PROBE		string
		T_DESCKEY	string
		T_PRBADDR	num
		T_PRBVAL	num
	{
		$$ = (int)new_desc($4, $2, DT_PROBE, new_probe($6, $8));
	}
	|	T_ALIEN		string
		T_DESCKEY	string
		T_ALVEC		num
		T_ALADDR	num
		T_ALNAME	string
	{
		$$ = (int)new_desc($4, $2, DT_ALIEN, new_alien($6, $8, $10));
	}
	|	T_DUP		string
		T_DESCKEY	string
		dupoptions
		T_DUPVEC	num
		T_DUPHDLR	string
		duparg
	{
		$$ = (int)new_desc($4, $2, DT_DUP, new_dup($5, $7, $9, $10));
	}
	|	T_RAM		string
		T_DESCKEY	string
		ramoptions
		T_RAMLOW	num
		T_RAMHIGH	num
		ramsize
	{
		$$ = (int)new_desc($4, $2, DT_RAM, new_ram($5, $7, $9, $10));
	}
	;


drvbcmask:	/* empty */
	{
		/* driver is a block or character device */
		$$ = 0;
	}
	|	drvbcmask drvbcline
	{
		$$ = $1 | $2;
	}
	;

drvbcline:	T_DRVSTR
	{
		/* driver is a stream handler */
		$$ = 001000;
	}
	|	T_DRVSBCH
	{
		/* separate open and close for block and character devices */
		$$ = 000400;
	}
	|	T_DRVTTY
	{
		/* driver has a tty structure */
		$$ = 000200;
	}
	|	T_DRVINIT
	{
		/* driver has an initialization handler */
		$$ = 000100;
	}
	|	T_DRVPFAIL
	{
		/* driver has a power-failure handler */
		$$ = 000040;
	}
	|	T_DRVOPEN
	{
		/* driver has an open handler */
		$$ = 000020;
	}
	|	T_DRVCLOSE
	{
		/* driver has a close handler */
		$$ = 000010;
	}
	|	T_DRVREAD
	{
		/* driver has a read handler */
		$$ = 000004;
	}
	|	T_DRVWRITE
	{
		/* driver has a write handler */
		$$ = 000002;
	}
	|	T_DRVIOCTL
	{
		/* driver has an ioctl handler */
		$$ = 000001;
	}
	;

drvfshmask:	/* empty */
	{
		/* driver is a filesystem handler */
		$$ = 0;
	}
	|	drvfshmask drvfshline
	{
		$$ = $1 | $2;
	}
	;

drvfshline:	T_FSHIOCTL
	{
		/* filesystem handler has an ioctl handler */
		$$ = 0400000000;
	}
	|	T_FSHINFO
	{
		/* filesystem handler has an information handler */
		$$ = 0200000000;
	}
	|	T_FSHCTRL
	{
		/* filesystem handler has a control handler */
		$$ = 0100000000;
	}
	|	T_FSHNOTIFY
	{
		/* filesystem handler has a notify handler */
		$$ = 0040000000;
	}
	|	T_FSHSATTR
	{
		/* filesystem handler has a set attributes handler */
		$$ = 0020000000;
	}
	|	T_FSHRDMAP
	{
		/* filesystem handler has a read map handler */
		$$ = 0010000000;
	}
	|	T_FSHFRMAP
	{
		/* filesystem handler has a free map handler */
		$$ = 0004000000;
	}
	|	T_FSHALMAP
	{
		/* filesystem handler has a allocate map handler */
		$$ = 0002000000;
	}
	|	T_FSHGDENTS
	{
		/* filesystem handler has a get dents handler */
		$$ = 0001000000;
	}
	|	T_FSHACCESS
	{
		/* filesystem handler has an access handler */
		$$ = 0000400000;
	}
	|	T_FSHSTATUS
	{
		/* filesystem handler has a status handler */
		$$ = 0000200000;
	}
	|	T_FSHUPDATE
	{
		/* filesystem handler has an update handler */
		$$ = 0000100000;
	}
	|	T_FSHCLINOD
	{
		/* filesystem handler has a close inode handler */
		$$ = 0000040000;
	}
	|	T_FSHOPINOD
	{
		/* filesystem handler has an open inode handler */
		$$ = 0000020000;
	}
	|	T_FSHGTINOD
	{
		/* filesystem handler has a get inode handler */
		$$ = 0000010000;
	}
	|	T_FSHUMOUNT
	{
		/* filesystem handler has a umount handler */
		$$ = 0000004000;
	}
	|	T_FSHMOUNT
	{
		/* filesystem handler has a mount handler */
		$$ = 0000002000;
	}
	|	T_FSHNAMEI
	{
		/* filesystem handler has a namei handler */
		$$ = 0000001000;
	}
	|	T_FSHFSTAT
	{
		/* filesystem handler has a status file handler */
		$$ = 0000000400;
	}
	|	T_FSHITRUNC
	{
		/* filesystem handler has an inode truncate handler */
		$$ = 0000000200;
	}
	|	T_FSHINIT
	{
		/* filesystem handler has an initialization handler */
		$$ = 0000000100;
	}
	|	T_FSHWRINOD
	{
		/* filesystem handler has a write inode handler */
		$$ = 0000000040;
	}
	|	T_FSHRDINOD
	{
		/* filesystem handler has a read inode handler */
		$$ = 0000000020;
	}
	|	T_FSHUPINOD
	{
		/* filesystem handler has an update inode handler */
		$$ = 0000000010;
	}
	|	T_FSHIREAD
	{
		/* filesystem handler has an inode read handler */
		$$ = 0000000002;
	}
	|	T_FSHIPUT
	{
		/* filesystem handler has an inode put handler */
		$$ = 0000000001;
	}
	;

drvtype	:	T_DRVTYPE
	{
		/* driver type information */
		$$ = 0;
	}
	|	drvtype drvtline
	{
		$$ = $1 | $2;
	}
	;

drvtline:	T_DRVCIVA
	{
		/* create interrupt vector array for this driver */
		$$ = 004000;
	}
	|	T_DRVCCBMN
	{
		/* create character or block major numbers for this driver */
		$$ = 002000;
	}
	|	T_DRVCILA
	{
		/* create interrupt level array for this driver */
		$$ = 001000;
	}
	|	T_DRVONE
	{
		/* allow only one of these drivers */
		$$ = 000200;
	}
	|	T_DRVSCF
	{
		/* do not create a count field for this driver */
		$$ = 000100;
	}
	|	T_DRVSIV
	{
		/* do not create an interrupt vector for this driver */
		$$ = 000040;
	}
	|	T_DRVREQ
	{
		/* this is a required driver */
		$$ = 000020;
	}
	|	T_DRVBLOCK
	{
		/* this is a block device */
		$$ = 000010;
	}
	|	T_DRVCHAR
	{
		/* this is a character device */
		$$ = 000004;
	}
	|	T_DRVIDD
	{
		/* this is an interrupt driven device */
		$$ = 000002;
	}
	|	T_DRVSVMA
	{
		/* allow for a single vector defn with multiple addresses */
		$$ = 000001;
	}
	;

drvdata	:	T_DRVDATA
	{
		$$ = (int)alloc_configname_table();
	}
	|	drvdata
		T_DRVCTSD string
	{
		put_entry_in_configname_table($1, new_cfname($3));
		$$ = $1;
	}
	;

sysdevs:	/* empty */
	{
		$$ = (int)alloc_sysdev_table();
	}
	|	sysdevs
		T_SDROOT	string
		T_SDPIPE	string
		T_SDDUMP	string
		T_SDSWAP	string
		minors
	{
		put_entry_in_sysdev_table($1,
			new_sysdev($3, $5, $7, $9, $10));
		$$ = $1;
	}
	;

minors:		/* empty */
	{
		$$ = (int)alloc_minor_table();
	}
	|	minors
		T_MINOR
		T_MROOT		num
		T_MPIPE		num
		T_MDUMP		num
		T_MSWAP		num
		T_MSWPLO	num
		T_MNSWAP	num
	{
		put_entry_in_minor_table($1,
			new_minor($4, $6, $8, $10, $12, $14));
		$$ = $1;
	}
	;

dupoptions:	/* empty */
	{
		$$ = 0;
	}
	|	dupoptions dupoption
	{
		$$ = $1 | $2;
	}
	;

dupoption:	T_DUPINTRET
	{
		$$ = DF_INTRET;
	}
	|	T_DUPALLTRAPS
	{
		$$ = DF_ALLTRAPS;
	}
	|	T_DUPBEQRTS
	{
		$$ = DF_BEQRTS;
	}
	|	T_DUPARG
	{
		$$ = DF_ARG;
	}
	;

duparg:	/* empty */
	{
		$$ = 0;
	}
	|	T_DUPARGVAL num
	{
		$$ = $2;
	}
	;

ramoptions:	/* empty */
	{
		$$ = 0;
	}
	|	ramoptions ramoption
	{
		$$ = $1 | $2;
	}
	;

ramoption:	T_RAMNOPARTY
	{
		$$ = RF_NOPARTY;
	}
	|	T_RAMSINGLE
	{
		$$ = RF_SINGLE;
	}
	|	T_RAMMULTIPLE
	{
		$$ = RF_MULTIPLE;
	}
	;

ramsize:	/* empty */
	{
		$$ = 0;
	}
	|	T_RAMSIZE num
	{
		$$ = $2;
	}
	;

num	:	T_NUM
	{
		$$ = yylval;
	}
	;

string	:	T_STRING
	{
		$$ = yylval;
	}
	;
%%
