/* sysgen.c
 *
 */
#include "sysgen.h"
#include "sgyacc.h"
#include "sgkey.h"
#include "sgform.h"


/* CHARACTER STRINGS
 *
 */
char *null = "";			/* the null string		*/
char *sg_lib = "/usr/src/uts/m68k/sysgen"; /* default library pathname	*/
char *datafile = "sgdata";		/* sysgen data file		*/
char *descriptions = "descriptions";	/* descriptions directory	*/
char *configs = "machines";		/* configurations directory	*/
int exit_status = 0;
char *master = "master";
char *dfile = "dfile";

void remove_table_entry();

/* KEY WORD ARRAY
 *
 */
char *keyword[] = {
	S_DESCFILE,	S_MACHFILE,	S_DESCKEY,
	S_DRVDESC,	S_DRVNAME,	S_DRVHDLR,	S_DRVIVS,
	S_DRVMBDN,	S_DRVMCDN,	S_DRVMBRL,	S_DRVMDPC,
	S_DRVPRS,	S_BCMASK,	S_DRVSTR,	S_DRVSBCH,
	S_DRVTTY,	S_DRVINIT,	S_DRVPFAIL,	S_DRVOPEN,
	S_DRVCLOSE,	S_DRVREAD,	S_DRVWRITE,	S_DRVIOCTL,
	S_FILSYS,	S_FSHNM,	S_FSHMASK,	S_FSHIOCTL,
	S_FSHINFO,	S_FSHCTRL,
	S_FSHNOTIFY,	S_FSHSATTR,	S_FSHRDMAP,	S_FSHFRMAP,
	S_FSHALMAP,	S_FSHGDENTS,	S_FSHACCESS,	S_FSHSTATUS,
	S_FSHUPDATE,	S_FSHCLINOD,	S_FSHOPINOD, S_FSHGTINOD,
	S_FSHUMOUNT,	S_FSHMOUNT,	S_FSHNAMEI,	S_FSHFSTAT,
	S_FSHITRUNC,	S_FSHINIT,	S_FSHWRINOD,	S_FSHRDINOD,
	S_FSHUPINOD,	S_FSHIREAD,	S_FSHIPUT,	S_DRVTYPE,
	S_DRVCIVA,	S_DRVCCBMN,	S_DRVCILA,
	S_DRVONE,	S_DRVSCF,	S_DRVSIV,	S_DRVREQ,
	S_DRVBLOCK,	S_DRVCHAR,	S_DRVIDD,	S_DRVSVMA,
	S_DRVDATA,	S_DRVCTSD,	S_DEVDESC,	S_DEVTYPE,
	S_DEVBRL,	S_DEVADDR,	S_DEVIVL,	S_DEVNOD,
	S_DEVDNOD,	S_SYSDEV,	S_SDROOT,	S_SDPIPE,
	S_SDDUMP,	S_SDSWAP,	S_MINOR,	S_MROOT,
	S_MPIPE,	S_MDUMP,	S_MSWAP,	S_MSWPLO,
	S_MNSWAP,
	S_PARDESC,	S_PARNAME,	S_PARCONF,	S_PARDVAL,
	S_MPU,		S_MPUNUM,
	S_FORCE,	S_FRID,
	S_PROBE,	S_PRBADDR,	S_PRBVAL,
	S_ALIEN,	S_ALVEC,	S_ALADDR,	S_ALNAME,
	S_DUP,		S_DUPINTRET,	S_DUPALLTRAPS,	S_DUPBEQRTS,	
	S_DUPARG,	S_DUPVEC,	S_DUPHDLR,	S_DUPARGVAL,	
	S_RAM,		S_RAMNOPARTY,	S_RAMSINGLE,	S_RAMMULTIPLE,	
	S_RAMLOW,	S_RAMHIGH,	S_RAMSIZE,
	S_CNFGFILE,	S_CNFGVERS,	S_CNFGITEM,	S_CNFGVALU
};


/* SYSGEN FUNCTION SUPPORT ROUTINES
 *
 */

/* Print Error Message
 *
 */
void efile(name)
char *name;
{
	fprintf(stderr, "%s: %s: ", sysgen, name);
	perror(NULL);
}

/* Create And Initialize Buffer
 *
 */
char *mkbuf(size)
unsigned size;
{
	register char	*p;

	if ((p = calloc(sizeof(char), size)) == NULL) {
		perror(sysgen);
		exit(1);
	}
	return(p);
}

/* Concatenate Names W/ Intervening '/'
 *
 */
char *pcat(path, name)
char *path, *name;
{
	register char *pathname;

	pathname = mkbuf(strlen(path) + sizeof('/') + strlen(name)
				+ sizeof('\0'));
	sprintf(pathname, "%s/%s", path, name);
	return(pathname);
}

/* Generate Library Pathname
 *
 */
char *libpath(name)
char *name;
{
	return(pcat(libname, name));
}

/* Create String
 *
 */
int string_count;

char *create_string(string)
char *string;
{
	static char initial_string_buffer[STRING_COUNT];

	static char *string_buffer = initial_string_buffer;
	static int free_count = STRING_COUNT;

	register char *p;
	register int i;

	if ((i = strlen(string) + 1) > free_count) {
		free_count = (i < BUFSIZE) ? BUFSIZE : i;
		string_buffer = mkbuf(free_count);
	}

	p = string_buffer;

	free_count -= i;
	string_buffer += i;

	string_count += i;

	return(strcpy(p, string));
}

/* De-Allocate String
 *
 */
void free_string(string)
char *string;
{
}


/* SYSGEN SUB-ROUTINES
 *
 */

/* Update Sysgen Data File
 *
 * Final function after other function routines
 *
 */
void out_sgdata_line(prompt, value, file)
char *prompt, *value;
FILE *file;
{
	fprintf(file, "%-20s%s\n", prompt, value);
}

static int no_build = 0;

void update_sgdata(config_name, rebuild, install)
char *config_name;
int rebuild, install;
{
	static char *true = "true", *false = "false";

	register char *path;
	register FILE *file;

	path = libpath(datafile);
	if ((file = fopen(path, "w")) != NULL) {
		out_sgdata_line("Configuration:", config_name, file);
		out_sgdata_line("Rebuild:", (rebuild) ? true : false, file);
		out_sgdata_line("Install:", (install) ? true : false, file);
		fclose(file);
	} else {
		perror(sysgen);
		exit_status = 1;
	}

	free_string(path);
}

/* Check For Master(4) And Dfile(4)
 *
 */
static int master_dfile_exist()
{
	register char *master_path, *dfile_path;
	register int result;

	master_path = libpath(master);
	dfile_path = libpath(dfile);

	result = (!((access(master_path, A_EXIS)) ||
			(access(dfile_path, A_EXIS))));

	free_string(master_path);
	free_string(dfile_path);
	return(result);
}

/* Remove Master(4) and Dfile(4)
 *
 */
static void remove_master_dfile()
{
	char *master_path, *dfile_path;

	master_path = libpath(master);
	dfile_path = libpath(dfile);

	if (unlink(master_path) | unlink(dfile_path))
		exit_status = 1;

	free_string(master_path);
	free_string(dfile_path);
}

/* Display Structure Counts
 *
 */
int write_counts_flag = 0;

void write_counts()
{
	static struct {
		STRUCT_LIST	*slist;
		char		*name;
	} thelist[] = {
		{ &alien_slist, "ALIEN" },
		{ &cfname_slist, "CFNAME" },
		{ &configfile_slist, "CONFIG_FILE" },
		{ &configitem_slist, "CONFIG_ITEM" },
		{ &context_slist, "CONTEXT" },
		{ &desc_slist, "DESC" },
		{ &device_slist, "DEVICE" },
		{ &driver_slist, "DRIVER" },
		{ &dup_slist, "DUP" },
		{ &filedata_slist, "FILEDATA" },
		{ &force_slist, "FORCE" },
		{ &minor_slist, "MINOR" },
		{ &mpu_slist, "MPU" },
		{ &param_slist, "PARAM" },
		{ &probe_slist, "PROBE" },
		{ &ram_slist, "RAM" },
		{ &sysdev_slist, "SYSDEV" },
		{ &table_slist, "TABLE" },
		{ NULL, NULL }
	};

	register int i;

	if (!write_counts_flag)
		return;

	for (i = 0; thelist[i].slist != NULL; i++)
		fprintf(stderr, "%s: %d %s structure(s) allocated\n", sysgen,
			thelist[i].slist->s_list_count, thelist[i].name);

	fprintf(stderr, "%s: %d string characters allocated\n", sysgen,
			string_count);
}

/* Interrupt Signal Handler
 *
 * Set global interrupt signal flag and
 * continue execution.
 *
 */
void interrupt(sig)
int sig;
{
	--intflag;
}

/* Quit Signal Handler
 *
 * Exit immediately, perform no disk updates.
 *
 */
void quit(sig)
int sig;
{
	longjmp(outenv, errno);
}

/* Initialize Curses(3X)
 *
 */
void cursesinit()
{
	initscr();		/* curses: init terminal settings	*/
	raw();			/* curses: break for each character	*/
	nonl();			/* curses: disable CR to LF translation	*/
	noecho();		/* curses: disable input echoing	*/
	idlok(stdscr, TRUE);	/* curses: use ins/del-line features	*/
	keypad(stdscr, TRUE);	/* curses: interpret function keys	*/
	killer = killchar();	/* curses: obtain kill line character	*/
	eraser = erasechar();	/* curses: obtain erase character	*/
	refresh();		/* curses: clear the screen		*/

	mylastline = --LINES;
	scrollhalf = LINES / 2;
	scrollfull = LINES;
	COLS--;

	if ((sigset(SIGINT, interrupt) == SIG_ERR) ||
	    (sigset(SIGQUIT, quit) == SIG_ERR)) {
		perror(sysgen);
		exit(1);
	}
}

/* Header Psuedo-Form
 *
 */
#define HEADER_TEXT "SYSGEN V1.0"

#define display_header_title_field display_prompt_field
#define change_header_title_field flash
#define open_header_title_field flash

static PROMPT_FIELD header_title_field = {
	CENTER(HEADER_TEXT),
	LENGTH(HEADER_TEXT),
	display_header_title_field,
	change_header_title_field,
	open_header_title_field,
	HEADER_TEXT
};

#define quit_header_form flash

#define add_header_form_line flash
#define del_header_form_line flash
#define dup_header_form_line flash

static FIELD *header_fields[] = {
	ISFIELD(header_title_field)
};

static LINE header_form_lines[]= {
	{ 0, 1, header_fields }
};

static FORM header_form = {
	NULL,
	NULL,
	quit_header_form,
	1,
	add_header_form_line,
	del_header_form_line,
	dup_header_form_line,
	header_form_lines
};

/* Edit File Data Table
 *
 */
void edit(fdtp, open_type_fdtbl_form, first_name, second_name)
TABLE *fdtp;
void (*open_type_fdtbl_form)();
char *first_name, *second_name;
{
	register FILEDATA *fdp;
	register int i;

	if (first_name != NULL) {
		if ((i = search_fdtbl(fdtp, first_name)) < 0) {
			fprintf(stderr, "%s: %s: %s\n", sysgen, first_name,
				sys_errlist[ENOENT]);
			exit(1);
		}
	} else 
		i = -1;

	if (!(setjmp(outenv))) {
		cursesinit();

		acp = alloc_context();
		acp->c_form = &header_form;
		acp->c_line = &header_form_lines[0];
		acp->c_field = ISFIELD(header_title_field);

		(*open_type_fdtbl_form)(i);

		if ((i < 0) && (second_name != NULL) &&
		    ((i = search_fdtbl(fdtp, second_name)) > 0)) {
			display_current_form_line();
			acp->c_curline = i;
			update_context();
		}
		read_form();
	}
}


/* SYSGEN FUNCTION ROUTINES
 *
 */

/* Update System Configuration Files
 *
 */
int function_mask = 0;

#define GENERATE 1
#define BUILD    2
#define INSTALL  4

void generate_build_install(argc, argv)
int argc;
char *argv[];
{
	register FILEDATA *cftbl_fdp;
	register char *config_name;
	register int i;

	config_name = (optind < argc) ? argv[optind] : uconfig;
	if ((i = search_fdtbl(cftbl_fdtp, config_name)) < 0) {
		fprintf(stderr, "%s: %s: %s\n", sysgen, config_name,
			sys_errlist[ENOENT]);
		exit(1);
	}
	cftbl_fdp = get_filedata_table_entry(cftbl_fdtp, i);

	if ((!(function_mask & GENERATE)) &&
	    (strcmp(cftbl_fdp->fd_name, uconfig))) {
		fprintf(stderr, "%s: %s: configuration files not found.\n",
			sysgen, cftbl_fdp->fd_name);
		i = 0;
		function_mask = 0;
	} else {
		if (function_mask & GENERATE) {
			out_master_dfile(cftbl_fdp);
			uconfig = cftbl_fdp->fd_name;
		}
		i = 0;
		if (function_mask & BUILD) {
			if (master_dfile_exist())
				i = -1;
			else {
				fprintf(stderr,
			"%s: %s: configuration files not found.\n",
					sysgen, cftbl_fdp->fd_name);
				if (function_mask & INSTALL) {
					fprintf(stderr,
			"%s: operating system not installed.", sysgen);
					function_mask = 0;
				}
			}
		}
	}
	update_sgdata(uconfig, i, function_mask & INSTALL);
}

/* Select/De-select/Change Comment
 *
 */
static char *new_comment = NULL;

static TABLE enable_table, disable_table;
#define get_name_table_entry(ntp, i) get_table_entry(ntp, i, char)
#define put_entry_in_name_table(ntp, np) put_entry_in_table(ntp, np)

#define ALREADY_ENABLED "%s: %s: %s: already enabled.\n"
#define NOT_ENABLED "%s: %s: %s: not enabled.\n"

static int enable_entries(cftbl_fdp)
register FILEDATA *cftbl_fdp;
{
	register FILEDATA *dtbl_fdp;
	register char *desc_table_name;
	register int i, changes;

	changes = 0;
	for (i = 0; i < enable_table.tbl_count; i++) {
		desc_table_name = get_name_table_entry(&enable_table, i);
		if (search_cftbl(cftbl_fdp->fd_data, desc_table_name) >= 0) {
			fprintf(stderr, ALREADY_ENABLED, sysgen,
				cftbl_fdp->fd_name, desc_table_name);
			continue;
		}
		put_entry_in_configfile_table(cftbl_fdp->fd_data,
			get_configfile(get_filedata_table_entry(dtbl_fdtp,
				search_fdtbl(dtbl_fdtp, desc_table_name))));
		--changes;
	}
	return(changes);
}

static int disable_entries(cftbl_fdp)
register FILEDATA *cftbl_fdp;
{
	register FILEDATA *dtbl_fdp;
	register char *desc_table_name;
	register int i, j, changes;

	changes = 0;
	for (i = 0; i < disable_table.tbl_count; i++) {
		desc_table_name = get_name_table_entry(&disable_table, i);
		j = search_cftbl(cftbl_fdp->fd_data, desc_table_name);
		if (j < 0) {
			fprintf(stderr, NOT_ENABLED, sysgen,
				cftbl_fdp->fd_name, desc_table_name);
			continue;
		}
		remove_configfile_table_entry(cftbl_fdp->fd_data, j);
		changes--;
	}
	return(changes);
}

static void enable_disable_change_comment(argc, argv)
int argc;
char *argv[];
{
	extern void write_cftbl();
	extern FORMAT cftbl_format[];

	register char *config_name, *desc_table_name;
	register FILEDATA *cftbl_fdp;
	register int i, desc_table_subscript, error_flag, changes;

	error_flag = 0;
	for (i = 0; i < enable_table.tbl_count; i++) {
		desc_table_name = get_name_table_entry(&enable_table, i);
		if (search_fdtbl(dtbl_fdtp, desc_table_name) < 0) {
			fprintf(stderr, "%s: %s: %s\n", sysgen,
				desc_table_name, sys_errlist[ENOENT]);
			error_flag--;
		}
	}

	if (error_flag) {
		fprintf(stderr, "%s: no action taken.", sysgen);
		exit(1);
	}

	do {
		changes = 0;
		config_name = (optind < argc) ? argv[optind++] : uconfig;
		if ((i = search_fdtbl(cftbl_fdtp, config_name)) < 0) {
			fprintf(stderr, "%s: %s: %s\n", sysgen,
				config_name, sys_errlist[ENOENT]);
			continue;
		}
		cftbl_fdp = get_filedata_table_entry(cftbl_fdtp, i);

		changes = enable_entries(cftbl_fdp) |
				disable_entries(cftbl_fdp);

		if (new_comment != NULL) {
			free_string(cftbl_fdp->fd_desc);
			cftbl_fdp->fd_desc = create_string(new_comment);
			changes--;
		}

		if (changes) {
			write_filedata(cftbl_fdp, configs,
				cftbl_format, write_cftbl);
			if (strcmp(cftbl_fdp->fd_name, uconfig))
				remove_master_dfile();
		}
	} while (optind < argc);

	update_sgdata(uconfig, 0, 0);
}

/* Edit Description Table File Data Table
 *
 */
void edit_desc_table_filedata_table(argc, argv)
int argc;
char *argv[];
{
	extern void open_dtbl_fdtbl_form(), save_dtbl_fdtbl();

	register char *path;
	register FILE *file;

	edit(dtbl_fdtp, open_dtbl_fdtbl_form,
		(argc > optind) ? argv[optind] : NULL, NULL);
	save_dtbl_fdtbl();
	refresh();
	endwin();

	update_sgdata(uconfig, 0, 0);
}

/* Edit Configuration File Table File Data Table
 *
 */
void edit_configfile_table_filedata_table(argc, argv)
int argc;
char *argv[];
{
	extern void open_cftbl_fdtbl_form(), save_cftbl_fdtbl();

#define INSTALLQ \
	"Install the new operating system to be used on the next reboot"

	register FILE *file;
	register FILEDATA *fdp;
	register int i, j;

	edit(cftbl_fdtp, open_cftbl_fdtbl_form,
		(argc > optind) ? argv[optind] : NULL, uconfig);

	save_cftbl_fdtbl();
	fdp = get_filedata_table_entry(acp->c_fdtbl, acp->c_curline);

	if ((strcmp(fdp->fd_name, uconfig)) || (fdp->fd_modified)) {
		if (confirm("Update system configuration files")) {
			out_master_dfile(fdp);
			uconfig = fdp->fd_name;
		} else {
			i = search_fdtbl(cftbl_fdtp, uconfig);
			if ((i >= 0) &&
		(get_filedata_table_entry(cftbl_fdtp, i)->fd_modified))
				remove_master_dfile();
		}
	}

	i = ((!strcmp(fdp->fd_name, uconfig)) && (master_dfile_exist())) ?
		confirm("Rebuild the operating system") : 0;
	j = (i) ? confirm(INSTALLQ) : 0;
	refresh();
	endwin();

	update_sgdata(uconfig, i, j);
}

/* Main Entry Point
 *
 */
int main(argc, argv, envp)
int argc;
char *argv[], *envp[];
{
	static char *usage = "Usage:\n\
\t%s [-l path] [configuration]\n\
\t%s [-l path] -d [collection]\n\
\t%s [-l path] -gbi [configuration]\n\
\t%s [-l path] [-s/r collection]... [-c comment] [configuration]\n";

	static STRUCT_LIST *slist[] = {
		&alien_slist, &cfname_slist, &configfile_slist,
		&configitem_slist, &context_slist, &desc_slist,
		&device_slist, &driver_slist, &dup_slist,
		&filedata_slist, &force_slist, &minor_slist,
		&mpu_slist, &param_slist, &probe_slist,
		&ram_slist, &sysdev_slist, &table_slist,
		NULL
	};

	extern TABLE *read_fdtbl();

	register int c, status;
	register char *p, *q;
	register TABLE *fdtp;
	register void (*function)();
	register FILE *file;

	p = strrchr(argv[0], '/');
	sysgen = (p != NULL) ? p + 1 : argv[0];

	if ((libname = getenv("SG_LIB")) == NULL)
		libname = sg_lib;

	status = 0;
	function = edit_configfile_table_filedata_table;

	while ((c = getopt(argc, argv, "c:dgbil:D:s:v")) != -1)
		switch (c) {
		case 'c':
			function = enable_disable_change_comment;
			new_comment = optarg;
			break;
		case 's':
			function = enable_disable_change_comment;
			put_entry_in_name_table(&enable_table, optarg);
			break;
		case 'D':
			function = enable_disable_change_comment;
			put_entry_in_name_table(&disable_table, optarg);
			break;
		case 'd':
			function = edit_desc_table_filedata_table;
			break;
		case 'g':
			function = generate_build_install;
			function_mask |= GENERATE;
			break;
		case 'b':
			function = generate_build_install;
			function_mask |= BUILD;
			break;
		case 'i':
			function = generate_build_install;
			function_mask |= INSTALL;
			break;
		case 'l':	/* specify sysgen library directory */
			libname = optarg;
			break;
		case 'n':
			--alwaysno;
			if (alwaysyes)
				--status;
			break;
		case 'y':
			--alwaysyes;
			if (alwaysno)
				--status;
			break;
		case 'v':
			--write_counts_flag;
			break;
		case '?':
		default:
			--status;
			break;
		}

	if (status) {
		fprintf(stderr, usage, sysgen, sysgen, sysgen, sysgen);
		exit(2);
	}

	for (c = 0; slist[c] != NULL; c++)
		init_struct(slist[c]);

	/* read sysgen library information */
	if ((status = isdir(libname)) <= 0) {
		if (!status)
			errno = ENOTDIR;
		efile(libname);
		exit(1);
	}

	dtbl_fdtp  = read_fdtbl(descriptions);
	cftbl_fdtp = read_fdtbl(configs);

	uconfig = create_string("sysV68");

	p = libpath(datafile);
	file = fopen(p, "r");
	free_string(p);

	if (file != NULL) {
		p = mkbuf(BUFSIZE);
		if ((fgets(p, BUFSIZE, file) != NULL) &&
		    ((q = strrchr(p, ' ')) != NULL)) {
			if (c = strlen(p))
				p[c - 1] = '\0';
			free_string(uconfig);
			uconfig = create_string(q + 1);
		}
		free(p);
		fclose(file);
	}

	(*function)(argc, argv);

	write_counts();
	return(exit_status);
}


/* DATA FILE CREATION
 *
 */

/* Output data block
 *
 */
void write_format(file, fp, dp, sdatap)
FILE *file;
register FORMAT *fp;
register char *dp, *sdatap;
{
	static char *wrtnumformat[] = { "%d", "%#x", "%#o" };

	register char *p;

	while (fp->f_format != FMT_END) {
		fprintf(file, "%s", keyword[fp->f_token - T_DESCFILE]);
		switch (fp->f_format) {
		case FMT_PROMPT:
			break;
		case FMT_STRING:
			fputc('\t', file);
			fputc('"', file);
			p = *LOCCHARPTR(dp, fp->f_sdata, sdatap);
			while (*p) {
				if (*p == '"')
					fputc('\\', file);
				fputc(*p, file);
				p++;
			}
			fputc('"', file);
			break;
		case FMT_DECIMAL:
		case FMT_HEXADEC:
		case FMT_OCTAL:
			fputc('\t', file);
			fprintf(file, wrtnumformat[fp->f_format - FMT_DECIMAL],
				*LOCINTEGER(dp, fp->f_sdata, sdatap));
			break;
		}
		fputc('\n', file);
		fp++;
	}
}

/* Output Selected Lines
 *
 */
void write_bit_lines(file, fbp, value)
FILE *file;
register BFORMAT *fbp;
register int value;
{
	while (fbp->fb_token) {
		if (value & fbp->fb_mask)
			fprintf(file, "%s\n", 
				keyword[fbp->fb_token - T_DESCFILE]);
		fbp++;
	}
}


/* STRUCTURE MANAGEMENT
 *
 */

/* Initialize Structure List
 *
 */
void init_struct(slp)
register STRUCT_LIST *slp;
{
	register STRUCT *sp, *sp2;
	register int i;

	sp = sp2 = slp->s_list_free;
	for (i = 0; i < slp->s_list_init - 1; i++) {
		sp2->s_nextfree = (STRUCT *)(&sp2->s_structstart +
					slp->s_list_size);
		sp2 = sp2->s_nextfree;
	}
	sp2->s_nextfree = NULL;
}

/* Allocate Structure
 *
 */
char *alloc_struct(slp)
register STRUCT_LIST *slp;
{
	register STRUCT *sp, *sp2;
	register int i;

	if ((sp = slp->s_list_free) == NULL) {
		sp = sp2 = (STRUCT *)mkbuf(slp->s_list_size * STRUCTCOUNT);
		for (i = 0; i < STRUCTCOUNT - 1; i++) {
			sp2->s_nextfree = (STRUCT *)(&sp2->s_structstart +
						slp->s_list_size);
			sp2 = sp2->s_nextfree;
		}
		sp2->s_nextfree = NULL;
	}
	slp->s_list_free = sp->s_nextfree;

	if (++slp->s_list_count <= slp->s_list_max)
		slp->s_list_max++;

	memset(&sp->s_structstart, '\0', slp->s_list_size);
	return(&sp->s_structstart);
}

/* De-Allocate Structure
 *
 */
void free_struct(slp, sp)
register STRUCT_LIST *slp;
register STRUCT *sp;
{
	sp->s_nextfree = slp->s_list_free;
	slp->s_list_free = sp;
	slp->s_list_count--;
}


/* TABLE MANAGEMENT
 *
 */
static TABLE tablebuf[TABLE_COUNT];

STRUCT_LIST table_slist = {
	sizeof(TABLE),
	0,
	0,
	TABLE_COUNT,
	STRUCTP(tablebuf)
};

/* Create Table				Create a new table structure and
 *					initialize it, by saving the new
 * table's de-allocate entry routine pointer (typically free(2)).  Return
 * the new table pointer.
 *
 */
TABLE *create_table(free, dup, cmp)
void (*free)();
char *(*dup)();
int (*cmp)();
{
	register TABLE *tp;

	tp = alloc_table();
	tp->tbl_free = free;
	tp->tbl_dup = dup;
	tp->tbl_cmp = cmp;
	return(tp);
}

/* Duplicate Table
 *
 */
char *duplicate_table(tp)
register TABLE *tp;
{
	register TABLE *ntp;
	register int i;

	ntp = create_table(tp->tbl_free, tp->tbl_dup);
	for (i = 0; i < tp->tbl_count; i++)
		put_entry_in_table(ntp, (*tp->tbl_dup)(tp->tbl_ptr[i]));
	return(CP(ntp));
}

/* Grow Table			This routine is used to grow a table's pointer
 *				array if it is found that the current array has
 * no empty slots.  No actual growth takes place if an empty slot is found.
 * In NO cases, is the table entry count incremented.
 *
 */
void grtbl(tp)
register TABLE *tp;
{
	if (tp->tbl_count < tp->tbl_size)
		return;

	tp->tbl_size += TABLE_GROW_INCREMENT;

	tp->tbl_ptr = (char **)((tp->tbl_count) ?
		realloc((char *)(tp->tbl_ptr), sizeof(char *) * tp->tbl_size) :
		malloc(sizeof(char *) * tp->tbl_size));

	if (tp->tbl_ptr == NULL) {
		perror(sysgen);
		exit(1);
	}
}

/* Append Entry To Table		Given table and new entry pointers,
 *					locate an empty slot in the table's
 * pointer array and save the entry pointer in it.   If there are no available
 * slots in the pointer array, re-allocate a new one with more space.
 *
 */
void put_entry_in_table(tp, p)
register TABLE *tp;
char *p;
{
	grtbl(tp);
	tp->tbl_ptr[tp->tbl_count++] = p;
}

/* Insert Entry In Table	When an entry is to be inserted at a specific
 *				location (not the end as in the Append Entry...
 * routine above, then this routine is used.
 *
 */
void insert_table_entry(tp, i, p)
register TABLE *tp;
register int i;
char *p;
{
	register int j;

	grtbl(tp);
	for (j = tp->tbl_count; j > i; --j)
		tp->tbl_ptr[j] = tp->tbl_ptr[j - 1];
	tp->tbl_count++;
	tp->tbl_ptr[j] = p;
}

/* Sort Table
 *
 */
void sorttbl(tp)
register TABLE *tp;
{
	if (tp->tbl_cmp == NULL)
		return;
	qsort(tp->tbl_ptr, tp->tbl_count, sizeof(char *), tp->tbl_cmp);
}

/* Remove And De-Allocate Table Entry	Deallocate the indicated entry then,
 *					move all entries occurring after this
 * entry down one slot, in order to assure there are no "holes" in the table.
 *
 */
void remove_table_entry(tp, i)
register TABLE *tp;
int i;
{
	(*tp->tbl_free)(tp->tbl_ptr[i]);
	tp->tbl_count--;
	while (i < tp->tbl_count) {
		tp->tbl_ptr[i] = tp->tbl_ptr[i + 1];
		i++;
	}
}

/* De-Allocate Table		De-allocate all entries found in the table,
 *				then de-allocate the table's pointer table,
 * and, finally deallocate the table structure itself.
 *
 */
void free_table(tp)
TABLE *tp;
{
	while (tp->tbl_count)
		remove_table_entry(tp, tp->tbl_count - 1);

	if (tp->tbl_size)
		free(tp->tbl_ptr);

	free_struct(&table_slist, tp);
}

/* Duplicate String
 *
 */
char *dpstring(p)
char *p;
{
	return(create_string(p));
}


/* PATHNAME MANAGEMENT
 *
 */

/* Check Directory Accessibility
 *
 */
int isdir(path)
char *path;
{
	struct stat	buf;

	if (stat(path, &buf))
		return(-1);
	return((buf.st_mode & S_IFMT) == S_IFDIR);
}

/* Access(2) Library File
 *
 */
int libaccess(name, amode)
char *name;
int amode;
{
	register char *path;
	register int status;

	path = libpath(name);
	status = access(path, amode);
	free_string(path);
	return(status);
}

/* Return Name Portion Of Path
 *
 */
char *justname(pathname)
char *pathname;
{
	register char *name;

	name = ((name = strrchr(pathname, '/')) == NULL) ? pathname : name + 1;
	return(create_string(name));
}

/* Return Path Portion Of Pathname
 *
 */
char *justpath(pathname)
char *pathname;
{
	register char *slash;
	register int length;

	if ((slash = strrchr(pathname, '/')) == NULL)
		return(create_string(null));

	if (!(length = slash - pathname))
		length = 1;

	return(strncpy(mkbuf(length + 1), pathname, length));
}

/* Generate Library Sub-Directory Pathname
 *
 */
char *libdir(dir, name)
char *dir, *name;
{
	register char *p, *path;

	path = pcat(p = libpath(dir), name);
	free_string(p);
	return(path);
}


/* LEXICAL ANALYSIS
 *
 */
char yytext[BUFSIZE];
FILE *yyin;

#define STROVR "%s: %s(%s): line #%d: string input buffer overflow\n"
#define TOKOVR "%s: %s(%s): line #%d: token input buffer overflow\n"
#define ILLNUM "%s: %s(%s): line #%d: '%s': invalid number\n"
#define BASE 0				/* STRTOL(3C) */

#define BEGIN	0
#define COMMENT	1
#define SKIPTOK	2
#define TOKEN	3
#define ARG	4
#define STRING	5
#define TOKNUM	6
#define NUMBER	7
#define END	8
#define ERROR	9

int yylex()
{
	extern char *keyword[];
	register int c, state, yyleng;
	register char **k, *p, *q;
	char *cp;

	static int yystate = BEGIN;

	state = yystate;

	for (;;) {
		if ((c = fgetc(yyin)) == EOF) {
			c = ((state == BEGIN) && (feof(yyin))) ? 0 : T_ERROR;
			yystate = BEGIN;
			return(c);
		}

		if (c == '\n')
			yyline++;

		switch (state) {
		case BEGIN:
			switch (c) {
			case '*':
				state = COMMENT;
				continue;
			case '\n':
				continue;
			case '#':
				yyleng = 0;
				state = TOKNUM;
				continue;
			default:
				yyleng = 0;
				state = TOKEN;
				ungetc(c, yyin);
				continue;
			}
		case COMMENT:
			if (c == '\n')
				state = BEGIN;
			continue;
		case SKIPTOK:
			switch (c) {
			case '\n':
				ungetc('\n', yyin);
				--yyline;
				yystate = BEGIN;
				return(yylval);
			case '\t':
				yystate = ARG;
				return(yylval);
			default:
				continue;
			}
		case TOKEN:
			switch (c) {
			case '\n':
				ungetc('\n', yyin);
				--yyline;
			case '\t':
				yystate = (c == '\t') ? ARG : BEGIN;
				yytext[yyleng] = '\0';

				k = keyword;
				for (c = 0; c <= (T_CNFGVALU-T_DESCFILE); c++) {
					p = *k++;
					q = yytext;
					while (*p++ == *q)
						if (!*q++)
							return(c + T_DESCFILE);
				}
				return(T_ERROR);
			default:
				if (yyleng >= BUFSIZE-1) {
					fprintf(stderr, TOKOVR,
						sysgen, yyfdp->fd_name,
						yyfdp->fd_uvers, yyline);
					state = ERROR;
					continue;
				}
				yytext[yyleng++] = c;
				continue;
			}
		case ARG:
			switch (c) {
			case '\t':
			case ' ':
				continue;
			case '*':
				state = COMMENT;
				continue;
			case '\n':
				state = BEGIN;
				continue;
			case '"':
				yyleng = 0;
				state = STRING;
				continue;
			default:
				ungetc(c, yyin);
				yyleng = 0;
				state = NUMBER;
				continue;
			}
		case STRING:
			switch (c) {
			case '"':
				if ((!yyleng) || (yytext[yyleng-1] != '\\')) {
					yystate = END;
					yytext[yyleng] = '\0';
					yylval = (int)create_string(yytext);
					return(T_STRING);
				}
				yyleng--;
			default:
				if (yyleng >= BUFSIZE-1) {
					fprintf(stderr, STROVR,
						sysgen, yyfdp->fd_name,
						yyfdp->fd_uvers, yyline);
					state = ERROR;
					continue;
				}
				yytext[yyleng++] = c;
				continue;
			}
		case TOKNUM:
		case NUMBER:
			switch (c) {
			case '\n':
				--yyline;
			case '*':
				ungetc(c, yyin);
			case ' ':
			case '\t':
				yytext[yyleng] = '\0';
				yylval = (int)strtol(yytext, &cp, BASE);

				if (cp != yytext + yyleng) {
					fprintf(stderr, ILLNUM, sysgen,
						yyfdp->fd_name,
						yyfdp->fd_uvers,
						yyline, yytext);
					state = ERROR;
					continue;
				}

				switch (state) {
				case TOKNUM:
					state = SKIPTOK;
					continue;
				case NUMBER:
					yystate = END;
					return(T_NUM);
				}
			default:
				if (yyleng >= BUFSIZE-1) {
					fprintf(stderr, STROVR, sysgen,
						yyfdp->fd_name,
						yyfdp->fd_uvers,
						yyline, yytext);
					state = ERROR;
					continue;
				}
				yytext[yyleng++] = c;
				continue;
			}
		case END:
			switch (c) {
			case ' ':
			case '\t':
				continue;
			case '\n':
				state = BEGIN;
				continue;
			case '*':
				state = COMMENT;
				continue;
			}
		case ERROR:
		default:
			if (c == '\n') {
				ungetc('\n', yyin);
				--yyline;
				yystate = BEGIN;
				return(T_ERROR);
			}
			continue;
		}
	}
}


/* SCCS FILE MANAGEMENT
 *
 */

/* Is SCCS Archive?
 *
 */
int issccs(path)
char *path;
{
	register char *name, *cmd;
	register int status;

	if (!(status = ((*(name = justname(path)) != 's') ||
	   (*(name+1) != '.')))) {
		cmd = mkbuf(BUFSIZE);
		sprintf(cmd, "val -s %s/%s", libname, path);
		status = system(cmd);
		free(cmd);
	}
	free_string(name);
	return((status) ? 0 : -1);
}

char *sccsitem(path, vers, item)
char *path, *vers;
int item;
{
	register char *buf, *svers;
	register int i, status;
	register FILE *file;

	svers = NULL;
	buf = mkbuf(BUFSIZE);
	sprintf(buf, "prs -r%s -d:%c: %s/%s", vers, item, libname, path);

	if ((file = popen(buf, "r")) != NULL) {
		if (fgets(buf, BUFSIZE, file) != NULL) {
			if (i = strlen(buf))
				buf[i - 1] = '\0';
			svers = create_string(buf);
		}
		while (fgets(buf, BUFSIZE, file) != NULL)
			;
		status = ferror(file);
		if ((pclose(file)) || (status)) {
			if (svers != NULL) {
				free_string(svers);
				svers = NULL;
			}
			fprintf(stderr, "%s: %s(%s): ", sysgen, path, vers);
		}
	}
	free(buf);
	return(svers);
}

char *sccscomm(path, vers)
char *path, *vers;
{
	return(sccsitem(path, vers, 'C'));
}

char *sccsvers(path, vers)
char *path, *vers;
{
	return(sccsitem(path, vers, 'I'));
}

char *nxtsccs(path)
char *path;
{
	register char *buf, *svers;
	register FILE *file;
	register int i;

	svers = NULL;

	buf = mkbuf(BUFSIZE);
	sprintf(buf, "get -e %s/%s >/dev/null 2>&1", libname, path);

	if (!system(buf)) {
		sprintf(buf, "unget %s/%s", libname, path);
		if ((file = popen(buf, "r")) != NULL) {
			if (fgets(buf, BUFSIZE, file) != NULL) {
				if (i = strlen(buf))
					buf[i - 1] = '\0';
				svers = create_string(buf);
			}
			while (fgets(buf, BUFSIZE, file) != NULL)
				;
			i = ferror(file);
			if (((pclose(file)) || (i)) && (svers != NULL)) {
				free_string(svers);
				svers = NULL;
			}
		}
	}
	free(buf);
	return(svers);
}

void sccscreat(path)
char *path;
{
	register char *buf;

	buf = mkbuf(BUFSIZE);
	sprintf(buf, "admin -n -fb -fn %s", path);
	system(buf);
	free_string(buf);
}

void sccsunget(sfile)
SFILE *sfile;
{
	register char *buf;

	buf = mkbuf(BUFSIZE);
	sprintf(buf, "unget -s -r%s %s >/dev/null 2>&1",
			sfile->s_vers, sfile->s_path);
	system(buf);
	free(buf);
	free_string(sfile->s_vers);
	free_string(sfile->s_path);
	free_string(sfile->s_type);
}

/* Open SCCS File
 *
 */
FILE *sccsopen(name, vers, type)
char *name, *vers, *type;
{
	register char *path, *onlyname;
	register int i;

	for (i = 0; i < _NFILE; i++)
		if (sccsfile[i].s_file == NULL)
			break;

	if (i >= _NFILE) {
		errno = EMFILE;
		return(NULL);
	}

	onlyname = justname(name);
	path = libpath(name);

	if (strncmp("s.", onlyname, 2)) {
		sccsfile[i].s_arflag = 0;
		sccsfile[i].s_file = fopen(path, type);
		free_string(path);
	} else {
		sccsfile[i].s_arflag = -1;

		for (;;) {
			if (access(path, A_EXIS))
				if (!strcmp(type, "r")) {
					errno = ENOENT;
					break;
				} else {
					sccscreat(path);
					if (access(path, A_EXIS))
						break;
				}

			sccsfile[i].s_vers = sccsget(path, vers, type);

			if (sccsfile[i].s_vers == NULL)
				break;

			sccsfile[i].s_path = path;
			sccsfile[i].s_type = create_string(type);
			sccsfile[i].s_file = fopen(onlyname + 2, type);

			if (sccsfile[i].s_file == NULL)
				sccsunget(&sccsfile[i]);
			break;
		}
	}
	free_string(onlyname);
	return(sccsfile[i].s_file);
}

/* Get SCCS G-File
 *
 */
char *sccsget(path, vers, type)
register char *path;
char *vers, *type;
{
	register char *buf, *svers;
	register FILE *file;
	register int i;

	svers = NULL;
	buf = mkbuf(BUFSIZE);
	sprintf(buf, "get -r%s %s %s 2>/dev/null", vers,
		(strcmp(type, "r")) ? "-e" : null, path);

	if ((file = popen(buf, "r")) != NULL) {
		if (fgets(buf, BUFSIZE, file) != NULL) {
			if (i = strlen(buf))
				buf[i - 1] = '\0';
			svers = create_string(buf);
			while (fgets(buf, BUFSIZE, file) != NULL)
				;
		}
		i = ferror(file);
		if (((pclose(file)) || (i)) && (svers != NULL)) {
			free_string(svers);
			svers = NULL;
		}
	}
	free(buf);
	return(svers);
}

/* Close SCCS File
 *
 */
int sccsclose(file, comments)
FILE *file;
char *comments;
{
	register char *buf, *name;
	register int i, status;

	for (i = 0; i < _NFILE; i++)
		if (sccsfile[i].s_file == file)
			break;

	if (i >= _NFILE)
		return(-1);

	if (fclose(file))
		return(-1);

	sccsfile[i].s_file = NULL;

	if (!sccsfile[i].s_arflag)
		return(0);

	buf = mkbuf(BUFSIZE);
	if (!strcmp(sccsfile[i].s_type, "r")) {
		name = justname(sccsfile[i].s_path);
		sprintf(buf, "rm -f %s", name + 2);
		free_string(name);
	} else
		sprintf(buf, "delta -s -r%s \"-y%s\" %s 2>/dev/null",
			sccsfile[i].s_vers, comments, sccsfile[i].s_path);

	status = system(buf);

	free(buf);
	free_string(sccsfile[i].s_path);
	free_string(sccsfile[i].s_vers);
	free_string(sccsfile[i].s_type);

	return(status);
}
