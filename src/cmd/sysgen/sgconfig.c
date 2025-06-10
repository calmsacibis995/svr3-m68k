/* sgconfig.c	%W%
 *
 */
#include "sysgen.h"
#include "sgyacc.h"


/* CONFIGURATION ITEM STRUCTURE
 *
 * The following routines manipulate the configuration item data structure.
 *
 */
static CONFIG_ITEM cibuf[CONFIG_ITEM_COUNT];

STRUCT_LIST configitem_slist = {
	sizeof(CONFIG_ITEM),
	0,
	0,
	CONFIG_ITEM_COUNT,
	STRUCTP(cibuf)
};

/* Create New Configuration Item
 *
 */
char *new_configitem(keyword, assn, value)
char *keyword;
int assn, value;
{
	register CONFIG_ITEM *cip;

	cip = alloc_configitem();

	cip->ci_keyword = keyword;
	switch (cip->ci_assn = assn) {
	case CIT_NONE:
		break;
	case CIT_NUM:
	case CIT_STRING:
		cip->ci_num = value;
	}

	return(CP(cip));
}

/* Create Select Name Structure		Create and initialize a new
 *					configuration item data structure.
 * The assigned argument specifies whether a value argument was passed,
 * and if so, what type.
 *
 */
CONFIG_ITEM *create_configitem(name, assigned, value)
char *name;
int assigned, value;
{
	register CONFIG_ITEM *cip;

	cip = alloc_configitem();
	cip->ci_keyword = create_string(name);

	switch (cip->ci_assn = assigned) {
	case CIT_NONE:
		break;
	case CIT_NUM:
		cip->ci_num = value;
		break;
	case CIT_STRING:
		cip->ci_string = create_string((char *)value);
		break;
	}
	return(cip);
}

/* Duplicate Configuration Item Data
 *
 */
char *dup_configitem(cip)
register CONFIG_ITEM *cip;
{
	register CONFIG_ITEM *ncip;

	ncip = alloc_configitem();

	ncip->ci_keyword = create_string(cip->ci_keyword);
	switch (ncip->ci_assn = cip->ci_assn) {
	case CIT_NONE:
		break;
	case CIT_NUM:
		ncip->ci_num = cip->ci_num;
		break;
	case CIT_STRING:
		ncip->ci_string = create_string(cip->ci_string);
		break;
	}
	return(CP(ncip));
}

/* Select Configuration Item
 *
 */
void select_configitem(cip, dtp)
register CONFIG_ITEM *cip;
register TABLE *dtp;
{
	selected_desc(dtp, cip->ci_keyword);
}

/* De-Allocate Configuration Item Data
 *
 */
void free_configitem(cip)
CONFIG_ITEM *cip;
{
	free_string(cip->ci_keyword);

	if (cip->ci_assn == CIT_STRING)
		free_string(cip->ci_string);
	
	free_struct(&configitem_slist, cip);
}

/* Output Configuration Item Data
 *
 */
void write_configitem(file, cip)
FILE *file;
CONFIG_ITEM *cip;
{
	static FORMAT configitemnone[] = {
		{ FMT_STRING, T_CNFGITEM, CP(&sconfigitem.ci_keyword) },
		{ FMT_END,    0,         NULL			     }
	};

	static FORMAT configitemnumber[] = {
		{ FMT_STRING,  T_CNFGITEM, CP(&sconfigitem.ci_keyword) },
		{ FMT_DECIMAL, T_CNFGVALU, CP(&sconfigitem.ci_num)     },
		{ FMT_END,     0,		NULL		      }
	};

	static FORMAT configitemstring[] = {
		{ FMT_STRING, T_CNFGITEM, CP(&sconfigitem.ci_keyword) },
		{ FMT_STRING, T_CNFGVALU, CP(&sconfigitem.ci_string)  },
		{ FMT_END,    0,         NULL			     }
	};

	static FORMAT *configitemformat[] = {
		configitemnone,
		configitemnumber,
		configitemstring
	};

	write_format(file, configitemformat[cip->ci_assn], cip, &sconfigitem);
}


/* CONFIGURATION ITEM TABLE
 *
 * The following routines manipulate TABLEs of configuration item data
 * (CONFIG_ITEM) structures.
 *
 */

/* Locate Configuration Item Data	Given a TABLE of configuration item
 *					data structures, and a keyword, this
 * routine searches the table for a configuration item data structure
 * associated with the keyword.
 *
 */
int search_citbl(citp, keyword)
register TABLE *citp;
char *keyword;
{
	register CONFIG_ITEM *cip;
	register int i;

	for (i = 0; i < citp->tbl_count; i++) {
		cip = get_configitem_table_entry(citp, i);
		if (!strcmp(keyword, cip->ci_keyword))
			return(i);
	}
	return(-1);
}

/* Select Configuration Item Table
 *
 */
void select_citbl(citp, dtp)
register TABLE *citp, *dtp;
{
	register int i;

	for (i = 0; i < citp->tbl_count; i++)
		select_configitem(get_configitem_table_entry(citp, i), dtp);
}

/* Create And Add Configuration Item To Table
 *
 */
void add_citbl(citp, keyword)
TABLE *citp;
char *keyword;
{
	put_entry_in_configitem_table(citp,
		create_configitem(keyword, CIT_NONE, 0));
}

/* Remove Configuration Item From Table
 *
 */
int remove_citbl(citp, keyword)
register TABLE *citp;
char *keyword;
{
	remove_configitem_table_entry(citp, search_citbl(citp, keyword));
	return(citp->tbl_count);
}

/* Output Configuration Item Data Table
 *
 */
static void write_citbl(file, citp)
FILE *file;
register TABLE *citp;
{
	void write_configitem();
	register int i;

	for (i = 0; i < citp->tbl_count; i++)
		write_configitem(file, get_configitem_table_entry(citp, i));
}


/* CONFIGURATION FILE
 *
 * The following routines support the CONFIG_FILE data structure.
 *
 */
static CONFIG_FILE cfbuf[CONFIG_FILE_COUNT];

STRUCT_LIST configfile_slist = {
	sizeof(CONFIG_FILE),
	0,
	0,
	CONFIG_FILE_COUNT,
	STRUCTP(cfbuf)
};

/* Create New Configuration File
 *
 */
char *new_configfile(name, vers, citp)
char *name, *vers;
TABLE *citp;
{
	register CONFIG_FILE *cfp;

	cfp = alloc_configfile();

	cfp->cf_name = name;
	cfp->cf_vers = vers;
	cfp->cf_citbl = citp;

	return(CP(cfp));
}

/* Create Configuraiton File Data	Allocate and initialize a configuration
 *					file data structure using the name and
 * version supplied by the caller; allocate a new configuration item table
 * for the new data structure.
 *
 */
CONFIG_FILE *create_configfile(name, vers)
char *name, *vers;
{
	register CONFIG_FILE *cfp;

	cfp = alloc_configfile();

	cfp->cf_name  = create_string(name);
	cfp->cf_vers  = create_string(vers);
	cfp->cf_citbl = alloc_configitem_table();

	return(cfp);
}

/* Duplicate Configuration File Data
 *
 */
char *dup_configfile(cfp)
register CONFIG_FILE *cfp;
{
	register CONFIG_FILE *ncfp;

	ncfp = alloc_configfile();

	ncfp->cf_name  = create_string(cfp->cf_name);
	ncfp->cf_vers  = create_string(cfp->cf_vers);
	ncfp->cf_citbl = (TABLE *)dup_configitem_table(cfp->cf_citbl);

	return(CP(ncfp));
}

/* Get Configuration File Data		Given a file data (FILEDATA) structure
 *					pointer, this routine allocates and
 * initializes a new configuration file data (CONFIG_FILE) structure with the
 * name and user version found in the file data structure.  In addition, each
 * item of information found in the file data structure, is included in the
 * new configuration file data structure by creating and initializing a new
 * configuration item structure and including it in the the configuration file
 * data structure's item table.
 *
 */
CONFIG_FILE *get_configfile(fdp)
FILEDATA *fdp;
{
	register CONFIG_FILE *cfp;
	register TABLE *dtp;
	register int i;

	cfp = create_configfile(fdp->fd_name, fdp->fd_uvers);
	config_dtbl(fdp->fd_data, cfp->cf_citbl);
	return(cfp);
}

/* Select Configuration File
 *
 */
void select_configfile(cfp, fdtp)
register CONFIG_FILE *cfp;
register TABLE *fdtp;
{
	register FILEDATA *fdp;
	register int i;

	if ((i = search_fdtbl(fdtp, cfp->cf_name)) < 0)
		add_fdtbl(fdtp, cfp->cf_name, cfp->cf_vers);
	else {
		fdp = get_filedata_table_entry(fdtp, i);
		fdp->fd_select = SELECTED_TRUE;
		update_filedata(fdp, cfp->cf_vers);
		select_citbl(cfp->cf_citbl, fdp->fd_data);
	}
}

/* Locate If Present, Appropriate Configuration Item
 *
 */
char *find_configitem(cfp, keyword)
register CONFIG_FILE *cfp;
char *keyword;
{
	register int i;

	return(((cfp == NULL) ||
		((i = search_citbl(cfp->cf_citbl, keyword)) < 0)) ?
			NULL :
			CP(get_configitem_table_entry(cfp->cf_citbl, i)));
}

/* Add Configuration Item To Configuration File
 *
 */
void add_configfile(cfp, keyword)
CONFIG_FILE *cfp;
char *keyword;
{
	add_citbl(cfp->cf_citbl, keyword);
}

/* Remove Configuration Item From Configuration File
 *
 */
int remove_configfile(cfp, keyword)
register CONFIG_FILE *cfp;
char *keyword;
{
	return(remove_citbl(cfp->cf_citbl, keyword));
}

/* De-Allocate Configuration File Data
 *
 */
void free_configfile(cfp)
register CONFIG_FILE *cfp;
{
	free_string(cfp->cf_name);
	free_string(cfp->cf_vers);
	free_configitem_table(cfp->cf_citbl);
	free_struct(&configfile_slist, cfp);
}

/* Output Configuration File Data
 *
 */
void write_configfile(file, cfp)
FILE *file;
CONFIG_FILE *cfp;
{
	static FORMAT configfileformat[] = {
		{ FMT_STRING, T_CNFGFILE, CP(&sconfigfile.cf_name) },
		{ FMT_STRING, T_CNFGVERS, CP(&sconfigfile.cf_vers) },
		{ FMT_END,    0,	 NULL			  }
	};

	write_format(file, configfileformat, cfp, &sconfigfile);
	write_citbl(file, cfp->cf_citbl);
}


/* CONFIGURATION FILE TABLE
 *
 * These routines operate on tables of configuration file data (CONFIG_FILE)
 * structures.
 *
 */

/* Locate Configuration File Data	Given a TABLE of CONFIG_FILE structures
 *					and a file name, this routine searches
 * the table for the structure associated to the given name.  If a structure
 * is found then the zero-based index is returned; if no structure is found
 * which is associated to the given name, then negative one is returned.
 *
 */
int search_cftbl(cftp, name)
register TABLE *cftp;
register char *name;
{
	register int i;

	for (i = 0; i < cftp->tbl_count; i++)
		if (!strcmp(name, get_configfile_table_entry(cftp, i)->cf_name))
			return(i);
	return(-1);
}

/* Select Configuration Table
 *
 */
void select_cftbl(cftp, fdtp)
register TABLE *cftp, *fdtp;
{
	register int i;

	for (i = 0; i < cftp->tbl_count; i++)
		select_configfile(get_configfile_table_entry(cftp, i), fdtp);
}

/* Create And Add Configuration File To Table
 *
 */
char *add_cftbl(cftp, name, vers)
register TABLE *cftp;
char *name, *vers;
{
	register CONFIG_FILE *cfp;

	cfp = create_configfile(name, vers);
	put_entry_in_configfile_table(cftp, cfp);
	return(CP(cfp));
}

/* Remove Configuration Item From Configuration File Table
 *
 */
int remove_cftbl_item(cftp, name, keyword)
register TABLE *cftp;
char *name, *keyword;
{
	register int i, c;
	
	i = search_cftbl(cftp, name);
	c = remove_configfile(get_configfile_table_entry(cftp, i), keyword);
	if (!c) remove_configfile_table_entry(cftp, i);
	return(c);
}

/* Output Configuration File Data Table
 *
 */
void write_cftbl(file, cftp)
FILE *file;
register TABLE *cftp;
{
	void write_configfile();
	register int i;

	for (i = 0; i < cftp->tbl_count; i++)
		write_configfile(file, get_configfile_table_entry(cftp, i));
}
