/* sgfile.c	%W%
 *
 */
#include "sysgen.h"
#include "sgyacc.h"
#include "sgform.h"


/* FILE DATA
 *
 */
static FILEDATA filedatabuf[FILEDATA_COUNT];

STRUCT_LIST filedata_slist = {
	sizeof(FILEDATA),
	0,
	0,
	FILEDATA_COUNT,
	STRUCTP(filedatabuf)
};

/* Create File Name Data
 *
 */
FILEDATA *create_filedata()
{
	register FILEDATA *fdp;

	fdp = alloc_filedata();

	fdp->fd_name  = create_string(null);
	fdp->fd_desc  = create_string(null);
	fdp->fd_data  = create_table(free, dpstring);
	fdp->fd_uvers = create_string(null);
	fdp->fd_svers = create_string(null);
	fdp->fd_comm  = create_string(null);

	return(fdp);
}

/* Duplicate File Data
 *
 */
char *dup_filedata(fdp)
FILEDATA *fdp;
{
	register FILEDATA *nfdp;

	nfdp = alloc_filedata();

	nfdp->fd_name   = create_string(fdp->fd_name);
	nfdp->fd_type   = fdp->fd_type;
	nfdp->fd_desc   = create_string(fdp->fd_desc);
	nfdp->fd_uvers  = create_string(fdp->fd_uvers);
	nfdp->fd_svers  = create_string(fdp->fd_svers);
	nfdp->fd_comm   = create_string(fdp->fd_comm);
	nfdp->fd_data   = (TABLE *)duplicate_table(fdp->fd_data);
	nfdp->fd_select = fdp->fd_select;

	return(CP(nfdp));
}

/* Create Missing File Data
 *
 */
FILEDATA *missing_filedata(name, vers)
char *name, *vers;
{
	register FILEDATA *fdp;

	fdp = alloc_filedata();

	fdp->fd_name  = create_string(name);
	fdp->fd_desc  = create_string(null);
	fdp->fd_data  = create_table(free, dpstring);
	fdp->fd_uvers = create_string(vers);
	fdp->fd_svers = create_string(null);
	fdp->fd_comm  = create_string(null);

	fdp->fd_select = SELECTED_ERROR;
	fdp->fd_error  = ENOENT;

	return(fdp);
}

/* Get File Data
 *
 */
FILEDATA *read_filedata(name, vers, directory)
char *name, *vers, *directory;
{
	register FILEDATA *fdp;
	register FILE *file;
	register char *path;

	fdp = alloc_filedata();

	fdp->fd_name  = create_string(name);
	fdp->fd_uvers = create_string(vers);

	path = pcat(directory, name);
	if (fdp->fd_issccs = issccs(path)) {
		fdp->fd_svers = sccsvers(path, vers);
		fdp->fd_comm  = sccscomm(path, fdp->fd_svers);
	} else {
		fdp->fd_svers = create_string(null);
		fdp->fd_comm = create_string(null);
	}

	file = sccsopen(path, fdp->fd_svers, "r");
	free_string(path);

	if (file == NULL) {
		fdp->fd_error = errno;
		fdp->fd_desc  = create_string(null);
		fdp->fd_data  = create_table(free, dpstring, NULL);
		return(fdp);
	}

	yyin   = file;
	yyline = 1;
	yyfdp  = fdp;
	yydir  = directory;

	if (yyparse()) {
		fdp->fd_error = EINVAL;
		fdp->fd_desc  = create_string(null);
		fdp->fd_data  = create_table(free, dpstring, NULL);
	}
		
	sccsclose(file, null);
	sorttbl(fdp->fd_data);
	return(fdp);
}

/* Update File Data 
 *
 */
void update_filedata(fdp, uvers, directory)
FILEDATA *fdp;
char *uvers;
{
	register FILEDATA *nfdp;
	register TABLE *tp;
	register char *p;

	if (!strcmp(fdp->fd_svers, uvers))
		return;

	nfdp = read_filedata(fdp->fd_name, uvers, directory);

	p = fdp->fd_uvers;
	fdp->fd_uvers = nfdp->fd_uvers;
	nfdp->fd_uvers = p;

	p = fdp->fd_svers;
	fdp->fd_svers = nfdp->fd_svers;
	nfdp->fd_svers = p;

	p = fdp->fd_comm;
	fdp->fd_comm = nfdp->fd_comm;
	nfdp->fd_comm = p;

	tp = fdp->fd_data;
	fdp->fd_data = nfdp->fd_data;
	nfdp->fd_data = tp;

	free_filedata(nfdp);
}

/* Compare File Data
 *
 */
int compare_filedata(afdp, bfdp)
FILEDATA **afdp, **bfdp;
{
	register int result;

	return((result = strcmp((*afdp)->fd_desc, (*bfdp)->fd_desc)) ?
		result : strcmp((*afdp)->fd_name, (*bfdp)->fd_name));
}

/* De-Allocate File Name Data
 *
 */
void free_filedata(fdp)
register FILEDATA *fdp;
{
	free_string(fdp->fd_name);
	free_string(fdp->fd_desc);
	free_string(fdp->fd_uvers);
	free_string(fdp->fd_svers);
	free_string(fdp->fd_comm);
	free_table(fdp->fd_data);
	free_struct(&filedata_slist, fdp);
}

/* Output File Data
 *
 */
void write_filedata(fdp, directory, fileformat, save_data)
register FILEDATA *fdp;
char *directory;
FORMAT *fileformat;
void (*save_data)();
{
	register char *path;
	register FILE *file;

	path = pcat(directory, fdp->fd_name);
	file = sccsopen(path, fdp->fd_svers, "w");
	free_string(path);

	if (file != NULL) {
		fprintf(file, "* %s\n*\n", fdp->fd_name);
		write_format(file, fileformat, fdp, &sfiledata);
		(*save_data)(file, fdp->fd_data);
		sccsclose(file, fdp->fd_comm);
	} else {
		sysmsg();
		flash();
	}
}

/* Save File Data
 *
 */
void save_filedata(fdp, directory, fileformat, save_data)
register FILEDATA *fdp;
char *directory;
FORMAT *fileformat;
void (*save_data)();
{
	if ((fdp->fd_modified) && (confirm("Save changes to %s", fdp->fd_name)))
		write_filedata(fdp, directory, fileformat, save_data);
}


/* FILE DATA TABLE
 *
 */

/* Input File Data Table
 *
 */
TABLE *read_fdtbl(directory)
char *directory;
{
	register TABLE *fdtp;
	register struct dirent *direntp;
	register DIR *dirp;
	register char *path;

	path = libpath(directory);
	dirp = opendir(path);

	free_string(path);
	if (dirp == NULL)
		return(NULL);

	fdtp = alloc_filedata_table();
	while ((direntp = readdir(dirp)) != NULL)
		if (direntp->d_name[0] != '.')
			put_entry_in_filedata_table(fdtp,
				read_filedata(&direntp->d_name[0],
						null, directory));

	closedir(dirp);
	sorttbl(fdtp);
	return(fdtp);
}

/* Locate File Data
 *
 */
int search_fdtbl(fdtp, name)
register TABLE *fdtp;
register char *name;
{
	register int i;

	for (i = 0; i < fdtp->tbl_count; i++)
		if (!strcmp(name, get_filedata_table_entry(fdtp, i)->fd_name))
			return(i);
	return(-1);
}

/* Save File Data Table
 *
 */
static void save_fdtbl(fdtp, directory, file_format, save_data)
register TABLE *fdtp;
char *directory;
FORMAT *file_format;
void (*save_data)();
{
	register int i;

	for (i = 0; i < fdtp->tbl_count; i++)
		save_filedata(get_filedata_table_entry(fdtp, i), directory,
				file_format, save_data);
}

/* Add Missing File Data To Table
 *
 */
void add_fdtbl(fdtp, name, vers)
TABLE *fdtp;
char *name, *vers;
{
	put_entry_in_filedata_table(fdtp, missing_filedata(name, vers));
}


/* FILE DATA TABLE FORM
 *
 */

/* Make Active A File Data Table Form
 *
 */
static void open_fdtbl_form(fdtp, formp, curline)
TABLE *fdtp;
FORM *formp;
int curline;
{
	create_context(fdtp, formp);

	if (curline >= 0) {
		acp->c_curline = curline;
		update_context();
		(*acp->c_field->fld_open)();
	}
}

/* Description Field
 *
 */
#define display_fdtbl_desc_field display_text_field

static void change_fdtbl_desc_field()
{
	register FILEDATA *fdp;
	register char *olddesc;

	fdp = get_filedata_table_entry(acp->c_fdtbl, acp->c_curline);
	olddesc = create_string(fdp->fd_desc);

	change_text_field();

	if (strcmp(olddesc, fdp->fd_desc))
		--fdp->fd_modified;

	free_string(olddesc);
}

static int open_fdtbl_desc_field(formp)
FORM *formp;
{
	register FILEDATA *fdp;

	fdp = get_filedata_table_entry(acp->c_fdtbl, acp->c_curline);
	if (fdp->fd_error) {
		errno = fdp->fd_error;
		sysmsg();
		flash();
		return(-1);
	}
	create_context(fdp->fd_data, formp);
	return(0);
}

/* Name Field
 *
 */
#define display_fdtbl_name_field display_text_field

static void change_fdtbl_name_field(directory)
char *directory;
{
	register char *cmd, *newname;
	register FILEDATA *fdp;
	register int i;

	fdp = get_filedata_table_entry(acp->c_fdtbl, acp->c_curline);
	newname = change_field(acp->c_field, fdp->fd_name,
				acp->c_scrline, &fileacs);

	if (newname == NULL)
		return;

	for (i = 0; i < acp->c_fdtbl->tbl_count; i++)
		if (!strcmp(newname,
		    get_filedata_table_entry(acp->c_fdtbl, i)->fd_name)) {
			botmsg("That name is already in use. No action taken.");
			flash();
			display_current_form_line();
			free_string(newname);
			return;
		}

	if (confirm("Confirm file name change")) {
		cmd = mkbuf(BUFSIZE);
		sprintf(cmd, "mv %s/%s/%s %s/%s/%s",
			libname, directory, fdp->fd_name,
			libname, directory, newname);

		if (!system(cmd)) {
			free_string(fdp->fd_name);
			fdp->fd_name = create_string(newname);
		}
		free(cmd);
	}
	free_string(newname);
	update_context();
}

/* Define Form
 *
 */
#define quit_fdtbl_form quit_read_form

static void add_fdtbl_form_line(directory, type, datafree, datadup, datacmp)
char *directory, *(*datadup)();
int type, (*datacmp)();
void (*datafree)();
{
	extern ACSPEC fileacs;
	extern char *change_field();

	register char *name, *path;
	register FILEDATA *fdp;
	register FILE *file;
	register int i, curline;

	curline = acp->c_curline;
	insert_entry_in_filedata_table(acp->c_fdtbl, curline,
					create_filedata());
	acp->c_fieldno = 1;	/* move to key field */

	display_form_body();
	update_context();

	fdp = get_filedata_table_entry(acp->c_fdtbl, curline);
	name = change_field(acp->c_field, fdp->fd_name,
				acp->c_scrline, &fileacs);

	if (name != NULL) {
		if (search_fdtbl(acp->c_fdtbl, name) >= 0) {
			remove_filedata_table_entry(acp->c_fdtbl, curline);
			botmsg("%s: already exists - no action taken.", name);
			free_string(name);
		} else {
			free_string(fdp->fd_name);
			fdp->fd_name = name;

			free_table(fdp->fd_data);
			fdp->fd_type = type;
			fdp->fd_data = create_table(datafree, datadup, datacmp);

			path = pcat(directory, name);
			fdp->fd_issccs = issccs(path);
			free_string(path);

			--fdp->fd_modified;
		}
	} else {
		remove_filedata_table_entry(acp->c_fdtbl, acp->c_curline);
		botmsg("No change.");
	}

	display_form_body();
	acp->c_fieldno = 0;
	update_context();
}

static void del_fdtbl_form_line(directory)
char *directory;
{
	register FILEDATA *fdp;
	register char *cmd;

	if (!confirm("Delete this file"))
		return;

	fdp = get_filedata_table_entry(acp->c_fdtbl, acp->c_curline);
	cmd  = mkbuf(BUFSIZE);
	sprintf(cmd, "rm -f %s/%s/%s 2>&1", libname, directory, fdp->fd_name);

	if (!system(cmd)) {
		remove_filedata_table_entry(acp->c_fdtbl, acp->c_curline);
		display_form_body();
	}
	free(cmd);
	update_context();
}

#define dup_fdtbl_form_line flash


/* DESCRIPTION TABLE FILE DATA TABLE
 *
 */

/* Save Description Table File Data Table
 *
 */
void save_dtbl_fdtbl()
{
	extern void write_dtbl();

	static FORMAT dtbl_fdtbl_format[] = {
		{ FMT_STRING, T_DESCFILE, CP(&sfiledata.fd_desc) },
		{ FMT_END,    NULL,	  NULL			 }
	};

	save_fdtbl(dtbl_fdtp, descriptions, dtbl_fdtbl_format, write_dtbl);
}


/* DESCRIPTION TABLE FILE DATA TABLE FORM
 *
 */

/* Description Field
 *
 */
#define display_dtbl_fdtbl_desc_field display_fdtbl_desc_field
#define change_dtbl_fdtbl_desc_field change_fdtbl_desc_field

static void open_dtbl_fdtbl_desc_field()
{
	extern FORM dtbl_form;

	open_fdtbl_desc_field(&dtbl_form);
}

static TEXT_FIELD dtbl_fdtbl_desc_field = {
	5,
	56,
	display_dtbl_fdtbl_desc_field,
	change_dtbl_fdtbl_desc_field,
	open_dtbl_fdtbl_desc_field,
	&sfiledata.fd_desc
};

/* Name Field
 *
 */
#define display_dtbl_fdtbl_name_field display_fdtbl_name_field

static void change_dtbl_fdtbl_name_field()
{
	change_fdtbl_name_field(descriptions);
}

#define open_dtbl_fdtbl_name_field open_dtbl_fdtbl_desc_field

static TEXT_FIELD dtbl_fdtbl_name_field = {
	65,
	14,
	display_dtbl_fdtbl_name_field,
	change_dtbl_fdtbl_name_field,
	open_dtbl_fdtbl_name_field,
	&sfiledata.fd_name
};

/* Define Form
 *
 */
#define quit_dtbl_fdtbl_form quit_read_form

static void add_dtbl_fdtbl_form_line()
{
	extern void free_desc();
	extern char *dup_desc();
	extern int compare_desc();

	add_fdtbl_form_line(descriptions, FT_DESC,
				free_desc, dup_desc, compare_desc);
}

static void del_dtbl_fdtbl_form_line()
{
	del_fdtbl_form_line(descriptions);
}

#define dup_dtbl_fdtbl_form_line dup_fdtbl_form_line

static FIELD *dtbl_fdtbl_fields[] = {
	ISFIELD(dtbl_fdtbl_desc_field),
	ISFIELD(dtbl_fdtbl_name_field)
};

static LINE dtbl_fdtbl_form_lines[] = {
	{ 0, 2, dtbl_fdtbl_fields }
};

static FORM dtbl_fdtbl_form = {
	CP(&sfiledata),
	NULL,
	quit_dtbl_fdtbl_form,
	0,
	add_dtbl_fdtbl_form_line,
	del_dtbl_fdtbl_form_line,
	dup_dtbl_fdtbl_form_line,
	dtbl_fdtbl_form_lines
};

/* Make Description Table File Data Table Form Active
 *
 */
void open_dtbl_fdtbl_form(line)
int line;
{
	open_fdtbl_form(dtbl_fdtp, &dtbl_fdtbl_form, line);
}


/* CONFIGURATION FILE TABLE FILE DATA TABLE
 *
 */
FORMAT cftbl_format[] = {
	{ FMT_STRING, T_MACHFILE, CP(&sfiledata.fd_desc) },
	{ FMT_END,    NULL,	  NULL			 }
};

/* Save Configuration File Table File Data Table
 *
 */
void save_cftbl_fdtbl()
{
	extern void write_cftbl();

	save_fdtbl(cftbl_fdtp, configs, cftbl_format, write_cftbl);
}


/* FILE DATA SUB FORMS
 *
 */

/* Quit Form
 *
 */
void quit_sub_fdtbl_form()
{
	register CONTEXT *cp;
	register int i;

	if (acp->c_modified) {
		i = 0;
		do {
			cp = get_context_table_entry(i++);
		} while (cp->c_form->form_sample != CP(&sfiledata));
		--get_filedata_table_entry(cp->c_fdtbl,
				cp->c_curline)->fd_modified;
	}
	quit_context();
}


/* CONFIGURATION FILE TABLE DESCRIPTION TABLE FILE DATA TABLE FORM
 *
 */

/* Description Field
 *
 */
#define display_cftbl_dtbl_fdtbl_desc_field display_fdtbl_desc_field

static void change_cftbl_dtbl_fdtbl_desc_field()
{
	register FILEDATA *fdp;
	register int i;

	fdp = get_filedata_table_entry(acp->c_fdtbl, acp->c_curline);
	switch(fdp->fd_select) {
	case SELECTED_FALSE:
		put_entry_in_configfile_table(acp->c_other,
						get_configfile(fdp));
		fdp->fd_select = SELECTED_TRUE;
		enable_dtbl(get_filedata_table_entry(acp->c_fdtbl,
						acp->c_curline)->fd_data);
		--acp->c_modified;
		break;
	case SELECTED_TRUE:
		if (!confirm("De-select this item and its sub-items"))
			return;

		fdp->fd_select = SELECTED_FALSE;
		remove_configfile_table_entry(acp->c_other,
				search_cftbl(acp->c_other, fdp->fd_name));
		disable_dtbl(fdp->fd_data);
		--acp->c_modified;
		break;
	case SELECTED_ERROR:
		botmsg("%s: item not found, can't select.", fdp->fd_name);
		flash();
	}
	update_context();
}

static void open_cftbl_dtbl_fdtbl_desc_field()
{
	extern FORM cftbl_dtbl_form;

	register CONFIG_FILE *cfp;
	register FILEDATA *fdp;
	register TABLE *cftp;
	register int i;

	fdp = get_filedata_table_entry(acp->c_fdtbl, acp->c_curline);

	cftp = (TABLE *)acp->c_other;
	i = search_cftbl(cftp, fdp->fd_name);
	cfp = (i >= 0) ? get_configfile_table_entry(acp->c_other, i) : NULL;

	if (!open_fdtbl_desc_field(&cftbl_dtbl_form))
		acp->c_other = (char *)cfp;
}

static TEXT_FIELD cftbl_dtbl_fdtbl_desc_field = {
	5,
	56,
	display_cftbl_dtbl_fdtbl_desc_field,
	change_cftbl_dtbl_fdtbl_desc_field,
	open_cftbl_dtbl_fdtbl_desc_field,
	&sfiledata.fd_desc
};

/* Name Field
 *
 */
#define display_cftbl_dtbl_fdtbl_name_field display_fdtbl_name_field

static void change_cftbl_dtbl_fdtbl_name_field()
{
	flash();
}

#define open_cftbl_dtbl_fdtbl_name_field open_cftbl_dtbl_fdtbl_desc_field

static TEXT_FIELD cftbl_dtbl_fdtbl_name_field = {
	65,
	14,
	display_cftbl_dtbl_fdtbl_name_field,
	change_cftbl_dtbl_fdtbl_name_field,
	open_cftbl_dtbl_fdtbl_name_field,
	&sfiledata.fd_name
};

/* Define Form
 *
 */
static void quit_cftbl_dtbl_fdtbl_form()
{
	register FILEDATA *fdp;
	register CONTEXT *cp;
	register TABLE *fdtp, *dtp;
	register int i, j;

	fdtp = acp->c_fdtbl;
	for (i = fdtp->tbl_count - 1; i >= 0; i--) {
		fdp = get_filedata_table_entry(fdtp, i);
		if (fdp->fd_select == SELECTED_ERROR) {
			remove_filedata_table_entry(fdtp, i);
			continue;
		}
		remove_error_desc(fdp->fd_data);
	}
	fdtp->tbl_count = 0;
	quit_sub_fdtbl_form();
}

#define add_cftbl_dtbl_fdtbl_form_line flash

static void del_cftbl_dtbl_fdtbl_form_line()
{
	register TABLE *cftp;
	register FILEDATA *fdp;
	register int i;

	fdp = get_filedata_table_entry(acp->c_fdtbl, acp->c_curline);
	switch (fdp->fd_select) {
	case SELECTED_FALSE:
	case SELECTED_TRUE:
		botmsg("%s: cannot remove entry.", fdp->fd_name);
		flash();
		break;
	case SELECTED_ERROR:
		remove_configfile_table_entry(acp->c_other,
				search_cftbl(acp->c_other, fdp->fd_name));
		remove_filedata_table_entry(acp->c_fdtbl, acp->c_curline);
		--acp->c_modified;
		display_form_body();
		update_context();
		break;
	}
}

#define dup_cftbl_dtbl_fdtbl_form_line flash

static FIELD *cftbl_dtbl_fdtbl_fields[] = {
	ISFIELD(cftbl_dtbl_fdtbl_desc_field),
	ISFIELD(cftbl_dtbl_fdtbl_name_field)
};

static LINE cftbl_dtbl_fdtbl_form_lines[] = {
	{ 0, 2, cftbl_dtbl_fdtbl_fields }
};

static FORM cftbl_dtbl_fdtbl_form = {
	CP(&sfiledata),
	&sfiledata.fd_select,
	quit_cftbl_dtbl_fdtbl_form,
	0,
	add_cftbl_dtbl_fdtbl_form_line,
	del_cftbl_dtbl_fdtbl_form_line,
	dup_cftbl_dtbl_fdtbl_form_line,
	cftbl_dtbl_fdtbl_form_lines
};


/* CONFIGURATION FILE TABLE FILE DATA TABLE FORM
 *
 */

/* Description Field
 *
 */
#define display_cftbl_fdtbl_desc_field display_fdtbl_desc_field
#define change_cftbl_fdtbl_desc_field change_fdtbl_desc_field

static void open_cftbl_fdtbl_desc_field()
{
	static TABLE localfdtbl = {
		0,
		0,
		NULL,
		free_filedata,
		CPFUNC(dup_filedata),
		compare_filedata
	};

	register FILEDATA *fdp;
	register TABLE *fdtp, *cftp;
	register int i;

	fdtp = &localfdtbl;
	for (i = 0; i < dtbl_fdtp->tbl_count; i++) {
		fdp = get_filedata_table_entry(dtbl_fdtp, i);

		fdp->fd_select = SELECTED_FALSE;
		disable_dtbl(fdp->fd_data);

		put_entry_in_filedata_table(fdtp, fdp);
	}

	cftp = get_filedata_table_entry(acp->c_fdtbl, acp->c_curline)->fd_data;
	select_cftbl(cftp, fdtp);
	create_context(fdtp, &cftbl_dtbl_fdtbl_form);
	acp->c_other = (char *)cftp;
}

static TEXT_FIELD cftbl_fdtbl_desc_field = {
	5,
	56,
	display_cftbl_fdtbl_desc_field,
	change_cftbl_fdtbl_desc_field,
	open_cftbl_fdtbl_desc_field,
	&sfiledata.fd_desc
};

/* Name Field
 *
 */
#define display_cftbl_fdtbl_name_field display_fdtbl_name_field

static void change_cftbl_fdtbl_name_field()
{
	change_fdtbl_name_field(configs);
}

#define open_cftbl_fdtbl_name_field open_cftbl_fdtbl_desc_field

static TEXT_FIELD cftbl_fdtbl_name_field = {
	65,
	14,
	display_cftbl_fdtbl_name_field,
	change_cftbl_fdtbl_name_field,
	open_cftbl_fdtbl_name_field,
	&sfiledata.fd_name
};

/* Define Form
 *
 */
#define quit_cftbl_fdtbl_form quit_read_form

static void add_cftbl_fdtbl_form_line()
{
	extern void free_configfile();
	extern char *dup_configfile();

	add_fdtbl_form_line(configs, FT_MACH,
		free_configfile, dup_configfile, NULL);
}

static void del_cftbl_fdtbl_form_line()
{
	del_fdtbl_form_line(configs);
}

static void dup_cftbl_fdtbl_form_line()
{
	register char *name, *path;
	register FILEDATA *fdp;
	register FILE *file;
	register int i, curline;

	curline = acp->c_curline;
	fdp = (FILEDATA *)dup_filedata(get_filedata_table_entry(acp->c_fdtbl,
								curline));
	insert_entry_in_filedata_table(acp->c_fdtbl, curline, fdp);

	free_string(fdp->fd_name);
	fdp->fd_name = create_string(null);
	free_string(fdp->fd_uvers);
	fdp->fd_uvers = create_string(null);

	acp->c_fieldno = 1;

	display_form_body();
	update_context();

	name = change_field(acp->c_field, fdp->fd_name,
				acp->c_scrline, &fileacs);

	if (name != NULL) {
		if (search_fdtbl(acp->c_fdtbl, name) >= 0) {
			remove_filedata_table_entry(acp->c_fdtbl, curline);
			botmsg("%s: already exists - no action taken.", name);
			free_string(name);
		} else {
			free_string(fdp->fd_name);
			fdp->fd_name = name;

			path = pcat(configs, name);
			fdp->fd_issccs = issccs(path);
			free_string(path);
			--fdp->fd_modified;
		}
	} else {
		remove_filedata_table_entry(acp->c_fdtbl, curline);
		botmsg("No change.");
	}

	display_form_body();
	acp->c_fieldno = 0;
	update_context();
}

static FIELD *cftbl_fdtbl_fields[] = {
	ISFIELD(cftbl_fdtbl_desc_field),
	ISFIELD(cftbl_fdtbl_name_field)
};

static LINE cftbl_fdtbl_form_lines[] = {
	{ 0, 2, cftbl_fdtbl_fields }
};

static FORM cftbl_fdtbl_form = {
	CP(&sfiledata),
	NULL,
	quit_cftbl_fdtbl_form,
	0,
	add_cftbl_fdtbl_form_line,
	del_cftbl_fdtbl_form_line,
	dup_cftbl_fdtbl_form_line,
	cftbl_fdtbl_form_lines
};

/* Make Configuration File Table File Data Table Form Active
 *
 */
void open_cftbl_fdtbl_form(line)
int line;
{
	open_fdtbl_form(cftbl_fdtp, &cftbl_fdtbl_form, line);

	if ((line < 0) && ((line = search_fdtbl(cftbl_fdtp, uconfig)) > 0)) {
		display_current_form_line();
		acp->c_curline = line;
		update_context();
	}
}
