#ident	"$Header: /grp/mat/53/utils/igf/RCS/igf.h,v 1.2 87/01/27 16:18:00 mat Exp $"
#define		I_BUFSIZ	1024		/* input read buffer size */
#define		EOL		'\n'		/* End Of Line character */
#define		COMMENTCHAR	'#'
#define		FOREVER		TRUE
#define		T_BSIZE		512		/* TAPE block size */
#define		LINETOOLONG	ERROR

/*
 * Defauld archiver and options.
 */
#define		ARCHIVER	"cpio"
#define		DEF_OUT		"-ocB >> $DEVICE"
#define		DEF_INP		"-icBdu < $DEVICE"
#define		DEF_NULL	""

/*
 * Information about each archive of files that will be
 * on the installation media. They will be maintained in a
 * link list, starting at "Files_head".
 */
struct ar_files {
	struct ar_files	*next;
	char	*src_dir;
	char	*dest_dir;
	char	*archiver;
	char	*o_opt;
	char	*i_opt;
	char	*a_desc;		/* desc of this particular archive */
	short	def_opt;
};

/*
 * This is a link list of operating systems that will be o
 * the installation tape. The head is at "Os_head".
 */
struct os_files {
	struct os_files	*next;
	char	*os_path;
};

extern struct ar_files	*Files_head;	/* head of archive-info link list */
extern struct ar_files	*Scripts;	/* head of scripts link list */
extern struct os_files	*Os_head;	/* head of operating system link list */
extern char	*Ram;			/* ramdisk file */
extern char	*Bootloader;		/* bootloader file */
extern char	*myname;		/* name program running as */
extern char	*Nomemmsg;		/* "no memory" message */
extern char	*Device;		/* default output device */
extern char	*Desc;			/* media description */
extern char	*Package;		/* package name */
extern char	*Version;		/* version of this software */
extern int	Verbose;			/* -v */
extern int	Bootable;			/* -b */
extern int	Dirents;
extern char	Io_buf[];		/* I/O buffer */
#ifdef DEBUG
extern int	Debug;
#endif

extern char	*malloc(),
		*strcat(),
		*strcpy(),
		*getenv(),
		*strchr(),
		*strrchr(),
		*strtok();
