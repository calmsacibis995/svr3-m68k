/* sysgen.h
 *
 */

/* INCLUDES
 *
 */
#include <errno.h>			/* INTRO(2)			*/
#include <fcntl.h>			/* OPEN(2)			*/
#include <sys/signal.h>			/* SIGNAL(2)			*/
#include <sys/types.h>			/* STAT(2)			*/
#include <sys/stat.h>			/* STAT(2)			*/
#include <sys/utsname.h>		/* UNAME(2)			*/
#include <setjmp.h>			/* SETJMP(3C)			*/
#include <string.h>			/* STRING(3C)			*/
#include <stdio.h>			/* INTRO(3)			*/
#include <dirent.h>			/* DIRECTORY(3X)		*/

#ifdef ECHO				/* fix multiple definition of	*/
#undef ECHO				/* macro ECHO in sys/termio.h	*/
#endif					/* and sgtty.h			*/

#include <curses.h>			/* CURSES(3X)			*/


/* OTHER MACROS
 *
 */

/* Miscellaneous Buffer Size		This buffer size is used whenever a
 *					temporary buffer is required.  Input
 * of lines from text files, etc., must not exceed this size.
 *
 */
#define BUFSIZE 1024

/* Type Casts
 *
 */
#define CP(p) ((char *)(p))
#define CPFUNC(f) ((char *(*)())f)
#define VOIDFUNC(f) ((void (*)())f)

/* Buffers
 *
 */
extern char *mkbuf();
extern void free();

/* Strings
 *
 */
extern char *create_string();
extern void free_string();

/* Locate Integer			Locate an integer given a data block
 * Locate Pointer			(d), a pointer to a sample integer (p),
 *					and, a pointer to the sample data block
 * (s).  Works by simply subtracting the pointer to the sample data block (s)
 * from the sample integer pointer (p) found within the sample data block,
 * giving the offset within the data block to the integer in question; then
 * adding this offset to the actual data block pointer (d).  Locate a character
 * pointer in a the same manner, but with a diffent type cast.
 *
 */
#define LOCPOINTER(d,p,s) (CP(d) + (CP(p) - CP(s)))

#define LOCINTEGER(d,p,s) ((int *)LOCPOINTER(d,p,s))
#define LOCCHARPTR(d,p,s) ((char **)LOCPOINTER(d,p,s))


/* STRUCTURE MANAGEMENT
 *
 */
#define STRUCTCOUNT 8
#define GROWLISTCOUNT 4

typedef union struct_union {
	char	s_structstart;		/* beginning of structure	*/
	union struct_union *s_nextfree;	/* pointer to next struct in list */
} STRUCT;

typedef struct {
	int	s_list_size;		/* size of structure supported	*/
	int	s_list_count;		/* count of structures allocated */
	int	s_list_max;		/* maximum count		*/
	int	s_list_init;		/* initial count		*/
	STRUCT	*s_list_free;		/* pointer to pointer list	*/
} STRUCT_LIST;

#define STRUCTP(sp) ((STRUCT *)sp)

extern void init_struct();
extern char *alloc_struct();
extern void free_struct();
extern STRUCT_LIST alien_slist, cfname_slist, configfile_slist,
	configitem_slist, context_slist, desc_slist, device_slist,
	driver_slist, dup_slist, filedata_slist, force_slist,
	minor_slist, mpu_slist, param_slist, probe_slist,
	ram_slist, sysdev_slist, table_slist;

#define STRING_COUNT (22*1024)
#define ALIEN_COUNT 8
#define CFNAME_COUNT 4
#define CONFIG_FILE_COUNT 128
#define CONFIG_ITEM_COUNT 2048
#define CONTEXT_COUNT 8
#define DESC_COUNT 256
#define DEVICE_COUNT 48
#define DRIVER_COUNT 64
#define DUP_COUNT 8
#define FILEDATA_COUNT 32
#define FORCE_COUNT 2
#define MINOR_COUNT 32
#define MPU_COUNT 2
#define PARAM_COUNT 128
#define PROBE_COUNT 2
#define RAM_COUNT 2
#define SYSDEV_COUNT 8
#define TABLE_COUNT 256


/* POINTER TABLE DATA STRUCTURE
 *
 * Many tables must grow upon demand; the following structure
 * implements one simple method for providing this functionality.
 * The tbl_size entry specifies the total size of the pointer
 * table pointed to by tbl_ptr; tbl_count entries have been used.
 * If new entries are required; the table must be realloc'ed(3C).
 *
 */
typedef struct {
	int	tbl_count;		/* number of entries		*/
	int	tbl_size;		/* allocated size		*/
	char	**tbl_ptr;		/* pointer to table pointer	*/
	void	(*tbl_free)();		/* free entry routine		*/
	char	*(*tbl_dup)();		/* duplicate entry routine	*/
	int	(*tbl_cmp)();		/* compare routine		*/
} TABLE;

extern STRUCT_LIST table_slist;
#define alloc_table() ((TABLE *)alloc_struct(&table_slist))

#define TABLE_GROW_INCREMENT 32
#define get_table_entry(tp, i, TYPE) ((TYPE *)((TABLE *)(tp))->tbl_ptr[i])

extern TABLE *create_table();
extern char *duplicate_table();
extern void put_entry_in_table();

/* Description Table
 *
 */
#define alloc_desc_table() create_table(free_desc, dup_desc, compare_desc)
#define dup_desc_table(dtp) duplicate_table(dtp)
#define get_desc_table_entry(dtp, i) get_table_entry(dtp, i, DESC)
#define put_entry_in_desc_table(dtp, dp) put_entry_in_table(dtp, dp)
#define insert_entry_in_desc_table(dtp, i, dp) insert_table_entry(dtp, i, dp)
#define remove_desc_table_entry(dtp, i) remove_table_entry(dtp, i)
#define free_desc_table(dtp) free_table(dtp)

extern void free_desc();
extern char *dup_desc();
extern int compare_desc();

/* Configuration File Table
 *
 */
#define alloc_configfile_table() \
	create_table(free_configfile, dup_configfile, NULL)
#define dup_configfile_table(cftp) duplicate_table(cftp)
#define get_configfile_table_entry(cftp, i) \
	get_table_entry(cftp, i, CONFIG_FILE)
#define put_entry_in_configfile_table(cftp, cfp) put_entry_in_table(cftp, cfp)
#define insert_entry_in_configfile_table(cftp, i, cfp) \
	insert_table_entry(cftp, i, cfp)
#define remove_configfile_table_entry(cftp, i) remove_table_entry(cftp, i)
#define free_configfile_table(cftp) free_table(cftp)

extern void free_configfile();
extern char *dup_configfile();

/* Configuration Item Table
 *
 */
#define alloc_configitem_table() \
	create_table(free_configitem, dup_configitem, NULL)
#define dup_configitem_table(citp) duplicate_table(citp)
#define get_configitem_table_entry(citp, i) \
	get_table_entry(citp, i, CONFIG_ITEM)
#define put_entry_in_configitem_table(citp, cip) put_entry_in_table(citp, cip)
#define insert_entry_in_configitem_table(citp, i, cip) \
	insert_table_entry(citp, i, cip)
#define remove_configitem_table_entry(citp, i) remove_table_entry(citp, i)
#define free_configitem_table(citp) free_table(citp)

extern void free_configitem();
extern char *dup_configitem();

/* Configuration Name Table
 *
 */
#define alloc_configname_table() \
	create_table(free_cfname, CPFUNC(dup_cfname), NULL)
#define dup_configname_table(cfntp) duplicate_table(cfntp)
#define get_configname_table_entry(cfntp, i) get_table_entry(cfntp, i, CFNAME)
#define put_entry_in_configname_table(cfntp, cfnp) \
	put_entry_in_table(cfntp, cfnp)
#define insert_entry_in_configname_table(cfntp,i,cfnp) \
	insert_table_entry(cfntp, i, cfnp)
#define remove_configname_table_entry(cfntp, i) remove_table_entry(cfntp, i)
#define free_configname_table(cfntp) free_table(cfntp)

extern void free_cfname();
extern char *dup_cfname();

/* System Devices Table
 *
 */
#define alloc_sysdev_table() create_table(free_sysdev, CPFUNC(dup_sysdev), NULL)
#define dup_sysdev_table(sdtp) duplicate_table(sdtp)
#define get_sysdev_table_entry(sdtp, i) get_table_entry(sdtp, i, SYSDEV)
#define put_entry_in_sysdev_table(sdtp, sdp) put_entry_in_table(sdtp, sdp)
#define insert_entry_in_sysdev_table(sdtp, i, sdp) \
	insert_table_entry(sdtp, i, sdp)
#define remove_sysdev_table_entry(sdtp, i) remove_table_entry(sdtp, i)
#define free_sysdev_table(sdtp) free_table(sdtp)

extern void free_sysdev();
extern char *dup_sysdev();

/* Minor Devices Table
 *
 */
#define alloc_minor_table() create_table(free_minor, CPFUNC(dup_minor), NULL)
#define dup_minor_table(mtp) duplicate_table(mtp)
#define get_minor_table_entry(mtp, i) get_table_entry(mtp, i, MINOR)
#define put_entry_in_minor_table(mtp, mp) put_entry_in_table(mtp, mp)
#define insert_entry_in_minor_table(mtp, i, mp) insert_table_entry(mtp, i, mp)
#define remove_minor_table_entry(mtp, i) remove_table_entry(mtp, i)
#define free_minor_table(mtp) free_table(mtp)

extern char *dup_minor();
extern void free_minor();

/* File Data Table
 *
 */
#define alloc_filedata_table() \
	create_table(free_filedata, CPFUNC(dup_filedata), compare_filedata)
#define dup_filedata_table(fdtp) duplicate_table(fdtp)
#define get_filedata_table_entry(fdtp, i) get_table_entry(fdtp, i, FILEDATA)
#define put_entry_in_filedata_table(fdtp, fdp) put_entry_in_table(fdtp, fdp)
#define insert_entry_in_filedata_table(fdtp, i, fdp) \
	insert_table_entry(fdtp, i, fdp)
#define remove_filedata_table_entry(fdtp, i) remove_table_entry(fdtp, i)
#define free_filedata_table(fdtp) free_table(fdtp)

extern void free_filedata();
extern char *dup_filedata();
extern int compare_filedata();


/* FILE DATA STRUCTURE
 *
 * This table is used to represent the files present in a given directory.
 * Each entry in the table has the file's name, latest version and comment
 * as defined under SCCS.
 *
 */
typedef struct {			/* File Name Data Structure	*/
	char	*fd_name;		/* file name			*/
	char	*fd_desc;		/* file description		*/
	char	*fd_uvers;		/* user version			*/
	char	*fd_svers;		/* SCCS version			*/
	char	*fd_comm;		/* file comment			*/
	TABLE	*fd_data;		/* file data			*/
	int	fd_type;		/* file type			*/
	int	fd_issccs;		/* sccs archive flag		*/
	int	fd_error;		/* error code			*/
	int	fd_select;		/* file has been selected flag	*/
	int	fd_modified;		/* file was modified		*/
} FILEDATA;

#define FT_TEXT		0		/* Text File			*/
#define FT_DESC		1		/* Description File		*/
#define FT_MACH		2		/* Machine File			*/

extern STRUCT_LIST filedata_slist;
#define alloc_filedata() ((FILEDATA *)alloc_struct(&filedata_slist))

FILEDATA *yyfdp, sfiledata;


/* CONFIGURATION FILE DATA
 *
 * This data structure contains the name and version of a description file,
 * as well as, a table of description item names (cf_citbl).  A configuration
 * file data structure exists for each description file which contributes
 * one or more items of information to the configuration.
 *
 */
typedef struct {			/* Select File Structure	*/
	char	*cf_name;		/* file name 			*/
	char	*cf_vers;		/* version name			*/
	TABLE	*cf_citbl;		/* select name table		*/
} CONFIG_FILE;

CONFIG_FILE sconfigfile;
extern STRUCT_LIST configfile_slist;

#define alloc_configfile() ((CONFIG_FILE *)alloc_struct(&configfile_slist))


/* CONFIGURATION ITEM DATA STRUCTURE
 *
 * This data structure contains the keyword associated to a description item
 * which has been selected for a given configuration.
 *
 */
typedef struct {
	char	*ci_keyword;		/* name selected		*/
	int	ci_assn;		/* name has assigned value	*/

	union {				/* assigned value		*/
	  int	 ciu_num;		/* number value			*/
	  char	*ciu_string;		/* character string value	*/
	}	ci_union;

} CONFIG_ITEM;

#define ci_num		ci_union.ciu_num
#define ci_string	ci_union.ciu_string

extern CONFIG_ITEM *create_configitem();
extern STRUCT_LIST configitem_slist;
#define alloc_configitem() ((CONFIG_ITEM *)alloc_struct(&configitem_slist))
CONFIG_ITEM sconfigitem;


/* SCCS OPEN FILE STRUCTURE
 *
 */
typedef struct {			/* open SCCS file structure	*/
	int	s_arflag;		/* is SCCS archive flag		*/
	FILE	*s_file;		/* stream pointer		*/
	char	*s_path;		/* path name pointer		*/
	char	*s_vers;		/* version name pointer		*/
	char	*s_type;		/* access type			*/
} SFILE;


/* Access(2) Values
 *
 */
#define A_READ 04			/* read	access			*/
#define A_WRIT 02			/* write access			*/
#define A_EXEC 01			/* execute (search) access	*/
#define A_EXIS 00			/* check existence of file	*/


/* Select Field Values
 *
 * The Following macros are used in conjuction with the select fields (_select)
 * found in the FILEDATA and DESC data structures.
 *
 */
#define SELECTED_FALSE	0
#define SELECTED_TRUE	1
#define SELECTED_ERROR	2


/* FORMAT STRUCTURE
 *
 * The following structure and definitions are used to output the various
 * files used by sysgen.
 *
 */
typedef struct {			/* Ouput Format Structure	*/
	int	f_format;		/* format code			*/
	int	f_token;		/* line token			*/
	char	*f_sdata;		/* sample data pointer		*/
} FORMAT;

/* f_format field values: */
#define FMT_END		0		/* no more lines		*/
#define FMT_PROMPT	1		/* no argument			*/
#define	FMT_STRING	2		/* string argument		*/
#define FMT_DECIMAL	3		/* decimal argument		*/
#define FMT_HEXADEC	4		/* hexadecimal argument		*/
#define FMT_OCTAL	5		/* octal argument		*/

typedef struct {
	int	fb_token;
	int	fb_mask;
} BFORMAT;


/* MULTIPLE HANDLER SPECIFICATION FLAG VALUES
 *
 */
#define DF_INTRET	001		/* on 0, go to intret		*/
#define	DF_ALLTRAPS	002		/* on 0, go to alltraps		*/
#define DF_BEQRTS	004		/* on 0, go to beq return	*/
#define	DF_ARG		010		/* argument is to be passed	*/


/* MEMORY CONFIGURATION SPECIFICATION FLAG VALUES
 *
 */
#define RF_NOPARTY	1		/* memory has no parity check	*/
#define RF_SINGLE	2		/* a single memory block	*/
#define RF_MULTIPLE	4		/* multiple memory blocks	*/


/* DESCRIPTION TYPES
 *
 */
#define DT_NONE		0		/* not yet defined		*/
#define DT_DRIVER	1		/* device description		*/
#define DT_FILSYS	2		/* file system handler desc	*/
#define DT_DEVICE	3		/* device description		*/
#define DT_SYSDEV	4		/* system devices		*/
#define DT_PARAM	5		/* parameter description	*/
#define DT_MPU		6		/* system mpu declaration	*/
#define DT_FORCE	7		/* non-unique driver spec	*/
#define DT_PROBE	8		/* memory probe specification	*/
#define DT_ALIEN	9		/* alien controller spec	*/
#define DT_DUP		10		/* multiple handler spec	*/
#define DT_RAM		11		/* memory configuration spec	*/

#define DT_COUNT	12		/* description type count	*/


/* CONFIGURATION ITEM ASSIGNMENT TYPES
 *
 */
#define CIT_NONE	0		/* No assigned value		*/
#define CIT_NUM		1		/* Numeric assigned value	*/
#define CIT_STRING	2		/* Character string assign val	*/


/* LIBRARY ROUTINES
 *
 */
extern char	*calloc();		/* MALLOC(3C)			*/
extern int	close();		/* CLOSE(2)			*/
extern int	errno;			/* INTRO(2)			*/
extern void	exit();			/* EXIT(2)			*/
extern int	fclose();		/* FCLOSE(3S)			*/
extern void	flash();		/* CURSES(3X)			*/
extern void	free();			/* MALLOC(3C)			*/
extern int	fprintf();		/* PRINTF(3S)			*/
extern int	fstat();		/* STAT(2)			*/
extern char	*getenv();		/* GETENV(3C)			*/
extern char	*fgets();		/* GETS(3S)			*/
extern FILE	*fopen();		/* FOPEN(3S)			*/
extern int	fprintf();		/* PRINTF(3S)			*/
extern int	getopt();		/* GETOPT(3C)			*/
extern void	longjmp();		/* SETJMP(3C)			*/
extern char	*malloc();		/* MALLOC(3C)			*/
extern char	*memchr();		/* MEMORY(3C)			*/
extern char	*memset();		/* MEMORY(3C)			*/
extern int	open();			/* OPEN(2)			*/
extern int	optind;			/* GETOPT(3C)			*/
extern char	*optarg;		/* GETOPT(3C)			*/
extern void	perror();		/* PERROR(3C)			*/
extern int	pclose();		/* POPEN(3S)			*/
extern FILE	*popen();		/* POPEN(3S)			*/
extern int	read();			/* READ(2)			*/
extern char	*realloc();		/* MALLOC(3C)			*/
extern void	(*sigset())();		/* SIGSET(2)			*/
extern int	sprintf();		/* PRINTF(3S)			*/
extern int	stat();			/* STAT(2)			*/
extern int	strcmp();		/* STRING(3C)			*/
extern char	*strchr();		/* STRING(3C)			*/
extern char	*strcpy();		/* STRING(3C)			*/
extern int	strlen();		/* STRING(3C)			*/
extern char	*strncpy();		/* STRING(3C)			*/
extern char	*strrchr();		/* STRING(3C)			*/
extern long	strtol();		/* STRTOL(3C)			*/
extern int	system();		/* SYSTEM(3S)			*/
extern char	*sys_errlist[];		/* PERROR(3C)			*/
extern int	sys_nerr;		/* PERROR(3C)			*/
extern int	yyparse();		/* YACC(1)			*/
extern FILE	*yyin;			/* LEX(1)			*/
extern char	yytext[];		/* LEX(1)			*/
extern int	yylval;			/* YACC(1)			*/


/* BSS DATA ITEMS
 *
 */
char		*descname;		/* description file name	*/
TABLE		*ttbl_fdtp;		/* text file data table		*/
TABLE		*dtbl_fdtp;		/* description file data table	*/
TABLE		*cftbl_fdtp;		/* configuration file data table */
int		alwaysno;		/* always give neg. confirm.	*/
int		alwaysyes;		/* go with throttle up flag	*/
TABLE		contbl;			/* context table		*/
char		eraser;			/* erase character		*/
char		**error;		/* error messages		*/
int		intflag;		/* interrupt occured flag	*/
char		killer;			/* line kill character		*/
int		mylastline;		/* reserved last line		*/
char		*libname;		/* sysgen library name		*/
jmp_buf		outenv;			/* exit environment		*/
char		*sccserror;		/* SCCS error message pointer	*/
SFILE		sccsfile[_NFILE];	/* open SCCS files		*/
int		scrollhalf;		/* scroll up/down line count	*/
int		scrollfull;		/* scroll fwd/back line count	*/
char		*sysgen;		/* facility invokation name	*/
char		*uconfig;		/* user configuration name	*/
int		yyline;			/* line number field		*/
char		*yydir;			/* current directory		*/


/* FUNCTIONS
 *
 */
char		*pcat();		/* pathname concatenate		*/
char		*libpath();		/* library pathname		*/
char		*justname();		/* get name portion of pathname	*/
char		*dpstring();		/* duplicate string (non-macro)	*/
char		*nxtsccs();		/* next sccs version		*/
char		*sccsget();		/* get SCCS version		*/
FILE		*sccsopen();		/* open SCCS file		*/
char		*sccsvers();		/* SCCS version			*/
char		*sccscomm();		/* SCCS comment			*/
void		write_format();
void		write_bit_lines();


/* INITIALIZED DATA
 *
 */
extern char	*null;			/* null character string	*/
extern char	*syslibdir[];		/* library directory names	*/
extern char	*keyword[];		/* keyword array		*/
extern char	*descriptions;		/* descriptions directory name	*/
extern char	*configs;		/* configurations directory name */
