/* sgdesc.c	%W%
 *
 */
#include "sysgen.h"
#include "sgyacc.h"
#include "sgform.h"


/* DESCRIPTION STRUCTURE
 *
 */
typedef struct {
	int	d_type;			/* type of description		*/
	int	d_select;		/* description is selected flag	*/
	char	*d_keyword;		/* description keyword		*/
	char	*d_desc;		/* description name		*/
	char	*d_ptr;			/* data structure pointer	*/
} DESC;

static DESC descbuf[DESC_COUNT];

STRUCT_LIST desc_slist = {
	sizeof(DESC),
	0,
	0,
	DESC_COUNT,
	STRUCTP(descbuf)
};

#define alloc_desc() ((DESC *)alloc_struct(&desc_slist))

DESC sdesc;	/* Sample Sescription */

/* Description Types
 *
 */
static char *desc_name[] = {
	NULL, "Driver", "Filesystem", "Device", "System Devices",
	"Parameter", "Processor", "Non-Unique Driver", "Probe",
	"Alien Handler", "Multiple Handler", "Memory Config"
};

/* Compare Descriptions		Used with the quick-sort routine, this
 *				function compares two descriptions' to
 * find which occurs first in the sorted description table.
 *
 */
int compare_desc(adp, bdp)
DESC **adp, **bdp;
{
	register int result;

	return((result = strcmp((*adp)->d_desc, (*bdp)->d_desc)) ?
		result : strcmp((*adp)->d_keyword, (*bdp)->d_keyword));
}

/* Enable/Disable Description
 *
 */
#define enable_desc(dp) ((dp)->d_select = SELECTED_TRUE)
#define disable_desc(dp) ((dp)->d_select = SELECTED_FALSE)

/* Create A New Description	Create a new description and initialize
 *				its fields with values supplied by the
 * caller.
 *
 */
char *new_desc(key, desc, type, ptr)
char *key, *desc, *ptr;
int type;
{
	register DESC *dp;

	dp = alloc_desc();

	dp->d_keyword = key;
	dp->d_desc    = desc;
	dp->d_type    = type;
	dp->d_ptr     = ptr;
	dp->d_select  = SELECTED_FALSE;

	return(CP(dp));
}

/* Create New Description	Given a keyword, description, and type
 *				create a new description item.  If the type
 * supplied to the routine is not DT_NONE (temporary description used when
 * creating a new description item), then call the appropriate routine
 * to allocate and initialize the data associated to the new description item.
 *
 */
DESC *create_desc(key, name, type)
char *key, *name;
int type;
{
	extern char *create_driver(), *create_filesystem_handler(),
		*create_device(), *create_sdtbl(), *create_param(),
		*create_mpu(), *create_force(), *create_probe(),
		*create_alien(), *create_dup(), *create_ram();

	static char *(*create_desc_jump_table[])() = {
		NULL, create_driver, create_filesystem_handler, create_device,
		create_sdtbl, create_param, create_mpu,
		create_force, create_probe, create_alien,
		create_dup, create_ram
	};

	register DESC *dp;

	dp  = alloc_desc();
	dp->d_keyword  = create_string(key);
	dp->d_desc = create_string(name);

	if ((dp->d_type = type) != DT_NONE)
		dp->d_ptr = (*create_desc_jump_table[dp->d_type])(null, null,
						null, null);

	dp->d_select = SELECTED_FALSE;
	return(dp);
}

/* Duplicate Description
 *
 */
char *dup_desc(dp)
register DESC *dp;
{
	extern char *dup_driver(), *dup_device(), *dup_param(),
		*dup_mpu(), *dup_force(), *dup_probe(),
		*dup_alien(), *dup_dup(), *dup_ram();

	static char *(*dup_desc_name[])() = {
		NULL, dup_driver, dup_driver, dup_device,
		duplicate_table, dup_param, dup_mpu, dup_force,
		dup_probe, dup_alien, dup_dup, dup_ram
	};

	register DESC *ndp;

	ndp = alloc_desc();

	ndp->d_keyword  = create_string(dp->d_keyword);
	ndp->d_desc     = create_string(dp->d_desc);

	if ((ndp->d_type = dp->d_type) != DT_NONE)
		ndp->d_ptr = (*dup_desc_name[dp->d_type])(dp->d_ptr);

	return(CP(ndp));
}

/* De-Allocate Description
 *
 */
void free_desc(dp)
register DESC *dp;
{
	extern void free_driver(), free_device(), free_table(),
		free_param(), free_mpu(), free_force(),
		free_probe(), free_alien(), free_dup(),
		free_ram();

	static void (*free_desc_name[])() = {
		NULL, free_driver, free_driver, free_device, free_table,
		free_param, free_mpu, free_force, free_probe,
		free_alien, free_dup, free_ram
	};

	free_string(dp->d_keyword);
	free_string(dp->d_desc);
	if (dp->d_type != DT_NONE)
		(*free_desc_name[dp->d_type])(dp->d_ptr);
	free_struct(&desc_slist, dp);
}

/* Output Description
 *
 */
void write_desc(file, dp)
FILE *file;
register DESC *dp;
{
	void write_driver(), write_filesystem_handler(), write_device(),
		write_sdtbl(), write_param(), write_mpu(), write_force(),
		write_probe(), write_alien(), write_dup(), write_ram();

	static struct {
		void	(*d_wrtfnc)();
		int	d_token;
	} write_desc_name[] = {
		{ NULL,		0	  },
		{ write_driver,	T_DRIVER  },
		{ write_filesystem_handler,	T_FILSYS  },
		{ write_device,	T_DEVDESC },
		{ write_sdtbl,	T_SYSDEV  },
		{ write_param,	T_PARDESC },
		{ write_mpu,	T_MPU	  },
		{ write_force,	T_FORCE	  },
		{ write_probe,	T_PROBE	  },
		{ write_alien,	T_ALIEN	  },
		{ write_dup,	T_DUP	  },
		{ write_ram,	T_RAM	  }
	};

	static FORMAT descformat[] = {
		{ FMT_STRING,	0,		CP(&sdesc.d_desc)	},
		{ FMT_STRING,	T_DESCKEY,	CP(&sdesc.d_keyword)	},
		{ FMT_END,	NULL,		NULL			}
	};

	descformat[0].f_token = write_desc_name[dp->d_type].d_token;
	fprintf(file, "*\n* %s\n*\n", dp->d_keyword);
	write_format(file, descformat, dp, &sdesc);
	(*write_desc_name[dp->d_type].d_wrtfnc)(file, dp->d_ptr);
}


/* DESCRIPTION TABLE
 *
 */

/* Locate Description		Given a description table and a keyword then
 *				locate the description whose keyword matches
 * the given keyword.  If found return the zero-based index of the matching
 * entry.  If not found, then return negative one.
 *
 */
int locate_desc(dtp, keyword)
register TABLE *dtp;
register char *keyword;
{
	register int i;

	for (i = 0; i < dtp->tbl_count; i++)
		if (!strcmp(keyword, get_desc_table_entry(dtp, i)->d_keyword))
			return(i);
	return(-1);
}

/* Output Description Table	Given a output stream a description table,
 *				write the description table information
 * to the output stream.
 *
 */
void write_dtbl(file, dtp)
FILE *file;
register TABLE *dtp;
{
	void write_desc();
	register int i;

	for (i = 0; i < dtp->tbl_count; i++)
		write_desc(file, get_desc_table_entry(dtp, i));
}

/* Disable Description Table
 *
 */
void disable_dtbl(dtp)
register TABLE *dtp;
{
	register int i;

	for (i = 0; i < dtp->tbl_count; i++)
		disable_desc(get_desc_table_entry(dtp, i));
}

/* Enable Description In Table
 *
 */
void enable_dtbl(dtp)
register TABLE *dtp;
{
	register int i;

	for (i = 0; i < dtp->tbl_count; i++)
		enable_desc(get_desc_table_entry(dtp, i));
}

/* Indicate Enabled Description In Table
 *
 */
void selected_desc(dtp, keyword)
register TABLE *dtp;
char *keyword;
{
	register DESC *dp;
	register int i;

	if ((i = locate_desc(dtp, keyword)) < 0) {
		dp = create_desc(keyword, null, DT_NONE);
		dp->d_select = SELECTED_ERROR;
		put_entry_in_desc_table(dtp, dp);
	} else
		 get_desc_table_entry(dtp, i)->d_select = SELECTED_TRUE;
}

/* Remove Erroneous Descriptions From Table
 *
 */
void remove_error_desc(dtp)
register TABLE *dtp;
{
	register int i;

	for (i = dtp->tbl_count - 1; i >= 0; i--)
		if (get_desc_table_entry(dtp, i)->d_select == SELECTED_ERROR)
			remove_desc_table_entry(dtp, i);
}

/* Configure Description Table
 *
 */
void config_dtbl(dtp, citp)
register TABLE *dtp, *citp;
{
	register int i;

	for (i = 0; i < dtp->tbl_count; i++)
		add_citbl(citp, get_desc_table_entry(dtp, i)->d_keyword);
}


/* DESCRIPTION TABLE FORM
 *
 */

/* Description Field
 *
 */
#define display_dtbl_desc_field display_text_field
#define change_dtbl_desc_field change_text_field

void open_dtbl_desc_field()
{
	extern FORM driver_form, fsh_form, device_form, sdtbl_form,
		param_form, mpu_form, force_form, probe_form,
		alien_form, dup_form, ram_form;

	static FORM *desc_subform[] = {
		NULL, &driver_form, &fsh_form, &device_form, &sdtbl_form,
		&param_form, &mpu_form, &force_form, &probe_form,
		&alien_form, &dup_form, &ram_form };

	register DESC *dp;

	dp = get_desc_table_entry(acp->c_dtbl, acp->c_curline);
	create_context(dp->d_ptr, desc_subform[dp->d_type]);
}

static TEXT_FIELD dtbl_desc_field = {
	5,
	39,
	display_dtbl_desc_field,
	change_dtbl_desc_field,
	open_dtbl_desc_field,
	&sdesc.d_desc
};

/* Keyword Field
 *
 */
#define display_dtbl_keyword_field display_text_field
#define change_dtbl_keyword_field change_text_field
#define open_dtbl_keyword_field open_dtbl_desc_field

static TEXT_FIELD dtbl_keyword_field = {
	48,
	10,
	display_dtbl_keyword_field,
	change_dtbl_keyword_field,
	open_dtbl_keyword_field,
	&sdesc.d_keyword
};

/* Type Field
 *
 */
void display_dtbl_type_field(fieldp, dp, scrlin)
SIMPLE_FIELD *fieldp;
DESC *dp;
int scrlin;
{
	display_field(fieldp, desc_name[dp->d_type], scrlin);
}

#define change_dtbl_type_field flash
#define open_dtbl_type_field flash

static SIMPLE_FIELD dtbl_type_field = {
	62,
	17,
	display_dtbl_type_field,
	change_dtbl_type_field,
	open_dtbl_type_field
};

/* Define Form
 *
 */
#define quit_dtbl_form quit_sub_fdtbl_form

int select_type()
{
	register int type;

	type = DT_DRIVER;

	attron(A_BOLD);
	move(acp->c_scrline, acp->c_field->fld_col - 3);
	addstr("->");
	attroff(A_BOLD);

	attron(A_REVERSE);
	for (;;) {
		display_field(acp->c_field, desc_name[type], acp->c_scrline);
		move(acp->c_scrline, acp->c_field->fld_col);
		refresh();

		switch(get_command()) {
		case CMD_UP:
			if (!--type)
				type = DT_COUNT - 1;
			break;
		case CMD_DOWN:
			if (++type == DT_COUNT)
				type = DT_DRIVER;
			break;
		case CMD_SEL:
			attroff(A_REVERSE);
			return(type);
		default:
			flash();
		}
	}
	/*NOTREACHED*/
}

void add_dtbl_form_line()
{
	extern char *change_field();
	register char *key;

	insert_entry_in_desc_table(acp->c_dtbl, acp->c_curline,
		create_desc(null, null, DT_NONE));
	acp->c_fieldno = 1;

	display_form_body();
	update_context();

	key = change_field(acp->c_field,
		get_desc_table_entry(acp->c_dtbl, acp->c_curline)->d_keyword,
		acp->c_scrline, &textacs);
	remove_desc_table_entry(acp->c_dtbl, acp->c_curline);

	if (key != NULL) {
		insert_entry_in_desc_table(acp->c_dtbl, acp->c_curline,
			create_desc(key, null, DT_NONE));
		acp->c_fieldno = 2;
		display_form_body();
		update_context();
		remove_desc_table_entry(acp->c_dtbl, acp->c_curline);
		insert_entry_in_desc_table(acp->c_dtbl, acp->c_curline,
			create_desc(key, null, select_type()));
		--acp->c_modified;
		acp->c_fieldno = 0;
		free_string(key);
	}
	display_form_body();
	update_context();
}

void del_dtbl_form_line()
{
	if (!confirm("Delete this description"))
		return;

	remove_desc_table_entry(acp->c_dtbl, acp->c_curline);
	--acp->c_modified;
	display_form_body();
	update_context();
}

#define dup_dtbl_form_line flash

static FIELD *dtbl_fields[] = {
	ISFIELD(dtbl_desc_field),
	ISFIELD(dtbl_keyword_field),
	ISFIELD(dtbl_type_field)
};

static LINE dtbl_form_lines[] = {
	{ 0, 3, dtbl_fields }
};

FORM dtbl_form = {
	CP(&sdesc),
	NULL,
	quit_dtbl_form,
	0,
	add_dtbl_form_line,
	del_dtbl_form_line,
	dup_dtbl_form_line,
	dtbl_form_lines
};


/* CONFIGURATION FILE TABLE - DESCRIPTION TABLE FORM
 *
 */

/* Description Field
 *
 */
#define display_cftbl_dtbl_desc_field display_text_field

void change_cftbl_dtbl_desc_field()
{
	extern char *add_cftbl();
	extern int remove_cftbl_item();
	extern void add_configfile();

	register DESC *dp;
	register CONTEXT *cp;
	register FILEDATA *fdp;
	register TABLE *cftp;

	dp  = get_desc_table_entry(acp->c_dtbl, acp->c_curline);

	cp   = get_context_table_entry(contbl.tbl_count - 1);
	fdp  = get_filedata_table_entry(cp->c_fdtbl, cp->c_curline);
	cftp = (TABLE *)cp->c_other;

	switch (dp->d_select) {
	case SELECTED_FALSE:
		dp->d_select = SELECTED_TRUE;
		if (acp->c_other == NULL) {
			fdp->fd_select = SELECTED_TRUE;
			acp->c_other =
				add_cftbl(cftp, fdp->fd_name, fdp->fd_uvers);
		}
		add_configfile(acp->c_other, dp->d_keyword);
		break;
	case SELECTED_TRUE:
		if (!confirm("De-select this item")) {
			botmsg("No change.");
			return;
		}
		dp->d_select = SELECTED_FALSE;
		if (!remove_cftbl_item(cftp, fdp->fd_name, dp->d_keyword)) {
			fdp->fd_select = SELECTED_FALSE;
			acp->c_other   = NULL;
		}
		break;
	case SELECTED_ERROR:
		botmsg("Item not found, can't select.");
		flash();
		return;
	}
	--acp->c_modified;
	update_context();
}

void open_cftbl_dtbl_desc_field()
{
	extern char *find_configitem();

	extern FORM cftbl_driver_form,
		cftbl_fsh_form,
		cftbl_device_form,
		cftbl_sdtbl_form,
		cftbl_param_form,
		cftbl_mpu_form,
		cftbl_force_form,
		cftbl_probe_form,
		cftbl_alien_form,
		cftbl_dup_form,
		cftbl_ram_form;

	static FORM *cftbl_desc_subform[] = {
		NULL,
		&cftbl_driver_form,
		&cftbl_fsh_form,
		&cftbl_device_form,
		&cftbl_sdtbl_form,
		&cftbl_param_form,
		&cftbl_mpu_form,
		&cftbl_force_form,
		&cftbl_probe_form,
		&cftbl_alien_form,
		&cftbl_dup_form,
		&cftbl_ram_form
	};

	register CONFIG_FILE *cfp;
	register DESC *dp;

	if (!(dp = get_desc_table_entry(acp->c_dtbl, acp->c_curline))->d_type) {
		botmsg("This description item cannot be opened.");
		flash();
		return;
	}
	cfp = (CONFIG_FILE *)acp->c_other;

	create_context(dp->d_ptr, cftbl_desc_subform[dp->d_type]);

	acp->c_other = find_configitem(cfp, dp->d_keyword);
	display_form_body();
	update_context();
}

static TEXT_FIELD cftbl_dtbl_desc_field = {
	5,
	39,
	display_cftbl_dtbl_desc_field,
	change_cftbl_dtbl_desc_field,
	open_cftbl_dtbl_desc_field,
	&sdesc.d_desc
};

/* Keyword Field
 *
 */
#define display_cftbl_dtbl_keyword_field display_text_field
#define change_cftbl_dtbl_keyword_field change_text_field
#define open_cftbl_dtbl_keyword_field open_cftbl_dtbl_desc_field

static TEXT_FIELD cftbl_dtbl_keyword_field = {
	48,
	10,
	display_cftbl_dtbl_keyword_field,
	change_cftbl_dtbl_keyword_field,
	open_cftbl_dtbl_keyword_field,
	&sdesc.d_keyword
};

/* Type Field
 *
 */
#define cftbl_dtbl_type_field dtbl_type_field

/* Define Form
 *
 */
#define quit_cftbl_dtbl_form quit_sub_fdtbl_form

#define add_cftbl_dtbl_form_line flash

void del_cftbl_dtbl_form_line()
{
	register DESC *dp;
	register CONTEXT *cp;
	register FILEDATA *fdp;
	register TABLE *cftp;

	dp = get_desc_table_entry(acp->c_dtbl, acp->c_curline);
	switch (dp->d_select) {
	case SELECTED_FALSE:
	case SELECTED_TRUE:
		botmsg("Can't delete item.");
		flash();
		break;
	case SELECTED_ERROR:
		cp   = get_context_table_entry(contbl.tbl_count - 1);
		fdp  = get_filedata_table_entry(cp->c_fdtbl, cp->c_curline);
		cftp = (TABLE *)cp->c_other;

		if (!remove_cftbl_item(cftp, fdp->fd_name, dp->d_keyword)) {
			fdp->fd_select = SELECTED_FALSE;
			acp->c_other   = NULL;
		}
		remove_desc_table_entry(acp->c_dtbl, acp->c_curline);
		--acp->c_modified;
		display_form_body();
		update_context();
		break;
	}
}

#define dup_cftbl_dtbl_form_line flash

static FIELD *cftbl_dtbl_fields[] = {
	ISFIELD(cftbl_dtbl_desc_field),
	ISFIELD(cftbl_dtbl_keyword_field),
	ISFIELD(cftbl_dtbl_type_field)
};

static LINE cftbl_dtbl_form_lines[] = {
	{ 0, 3, cftbl_dtbl_fields }
};

FORM cftbl_dtbl_form = {
	CP(&sdesc),
	&sdesc.d_select,
	quit_cftbl_dtbl_form,
	0,
	add_cftbl_dtbl_form_line,
	del_cftbl_dtbl_form_line,
	dup_cftbl_dtbl_form_line,
	cftbl_dtbl_form_lines
};


/* CREATE SYSTEM CONFIGURATION FILES
 *
 */

/* Output Item
 *
 */
static void out_item(file, jmptbl, dtp, citp, itemtype)
FILE *file;
void (*jmptbl[])();
register TABLE *dtp, *citp;
int itemtype;
{
	register DESC *dp;
	register int i, j;

	for (i = 0; i < dtp->tbl_count; i++)
		if (((dp = get_desc_table_entry(dtp, i))->d_type == itemtype) &&
		    ((j = search_citbl(citp, dp->d_keyword)) >= 0))
			(*jmptbl[itemtype])(file, dp->d_ptr,
					get_configitem_table_entry(citp, j));
}

/* Output File Section
 *
 */
static void out_part(file, jmptbl, cftp, itemtype)
FILE *file;
void (*jmptbl[])();
register TABLE *cftp;
int itemtype;
{
	register int i, j;

	if (itemtype == '$') {
		fprintf(file, "$$$\n");
		return;
	}
	for (i = 0; i < dtbl_fdtp->tbl_count; i++) {
		j = search_cftbl(cftp,
			get_filedata_table_entry(dtbl_fdtp, i)->fd_name);
		if (j >= 0)
			out_item(file, jmptbl,
				get_filedata_table_entry(dtbl_fdtp, i)->fd_data,
				get_configfile_table_entry(cftp, j)->cf_citbl,
				itemtype);
	}
}

/* Output File
 *
 */
static void out_file(name, part, jmptbl, cftbl_fdp)
char *name;
int part[];
void (*jmptbl[])();
FILEDATA *cftbl_fdp;
{
	register FILE *file;
	register int *p;
	register char *path;

	path = libpath(name);
	file = fopen(path, "w");
	free_string(path);

	if (file == NULL) {
		fprintf(stderr, "%s:%s: ", sysgen, path);
		return;
	}

	fprintf(file, "*\t%s(4) based on configuration: %s.\n*\n",
		name, cftbl_fdp->fd_name);
	p = part;
	while (*p != DT_NONE)
		out_part(file, jmptbl, cftbl_fdp->fd_data, *p++);
	fclose(file);
}

/* Output Master & Dfile
 *
 */
void out_master_dfile(cftbl_fdp)
register FILEDATA *cftbl_fdp;
{
	extern void out_device(), out_sdtbl(), out_dfile_param(), out_force(),
		out_probe(), out_alien(), out_dup(), out_ram(),
		out_driver(), out_master_param(), out_mpu();

	static int dfileparts[] = {
		DT_DEVICE, DT_SYSDEV, DT_PARAM, DT_FORCE,
		DT_PROBE, DT_ALIEN, DT_DUP, DT_RAM, DT_NONE
	};

	static int masterparts[] = {
		DT_DRIVER, DT_FILSYS, '$', '$',
		DT_PARAM, '$', DT_MPU, DT_NONE
	};

	static void (*dfilejmp[])() = {
		NULL, NULL, NULL, out_device, out_sdtbl, out_dfile_param,
		NULL, out_force, out_probe, out_alien, out_dup, out_ram
	};

	static void (*masterjmp[])() = {
		NULL, out_driver, out_driver, NULL, NULL, out_master_param,
		out_mpu, NULL, NULL, NULL, NULL, NULL
	};

	out_file("dfile", dfileparts, dfilejmp, cftbl_fdp);
	out_file("master", masterparts, masterjmp, cftbl_fdp);
}
