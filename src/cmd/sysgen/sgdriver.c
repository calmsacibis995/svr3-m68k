/* sgdriver.c	%W%
 *
 */
#include "sysgen.h"
#include "sgyacc.h"
#include "sgkey.h"
#include "sgform.h"


/* CONFIGURATION NAME STRUCTURE
 *
 */
typedef struct {
	char	*cf_name;		/* data name			*/
} CFNAME;

static CFNAME cfnamebuf[CFNAME_COUNT];

STRUCT_LIST cfname_slist = {
	sizeof(CFNAME),
	0,
	0,
	CFNAME_COUNT,
	STRUCTP(cfnamebuf)
};

#define alloc_cfname() ((CFNAME *)alloc_struct(&cfname_slist))

CFNAME scfname;				/* sample configuration name struct */

/* Create Configuration Name Declaration
 *
 */
char *new_cfname(name)
char *name;
{
	register CFNAME *cfp;

	cfp = alloc_cfname();
	cfp->cf_name = name;
	return(CP(cfp));
}

/* Create Configuration Name
 *
 */
CFNAME *create_cfname(name)
char *name;
{
	register CFNAME *cfnp;

	cfnp = alloc_cfname();
	cfnp->cf_name = create_string(name);
	return(cfnp);
}

/* Duplicate Configuration Name
 *
 */
char *dup_cfname(cfnp)
register CFNAME *cfnp;
{
	register CFNAME *ncfnp;

	ncfnp = alloc_cfname();
	ncfnp->cf_name = create_string(cfnp->cf_name);
	return(CP(ncfnp));
}

/* De-Allocate Configuration Name
 *
 */
void free_cfname(cfnp)
CFNAME *cfnp;
{
	free_string(cfnp->cf_name);
	free_struct(&cfname_slist, cfnp);
}

/* Output Configuration Name Declaration
 *
 */
void write_cfname(file, cfnp)
FILE *file;
CFNAME *cfnp;
{
	fprintf(file, "%s\t\"%s\"\n", keyword[T_DRVCTSD - T_DESCFILE],
		cfnp->cf_name);
}

/* Output Master(4), Part 1, Field 11 or 12 or 13
 *
 */
void out_cfname(file, cfnp)
FILE *file;
CFNAME *cfnp;
{
	fprintf(file, "%-12s", cfnp->cf_name);
}


/* CONFIGURATION NAME TABLE
 *
 */

/* Output Configuration Name Table
 *
 */
void write_cfntbl(file, cfntp)
FILE *file;
register TABLE *cfntp;
{
	register int i;

	for (i = 0; i < cfntp->tbl_count; i++)
		write_cfname(file, get_configname_table_entry(cfntp, i));
}

/* Output Master(4), Part 1, Fields 11 through 13
 *
 */
void out_cfntbl(file, cfntp)
FILE *file;
register TABLE *cfntp;
{
	register int i;

	for (i = 0; i < cfntp->tbl_count; i++)
		out_cfname(file, get_configname_table_entry(cfntp, i));

	fputc('\n', file);
}


/* CONFIGURATION NAME TABLE FORM
 *
 */

/* Name Field
 *
 */
#define display_cfname_name_field display_text_field
#define change_cfname_name_field change_text_field
#define open_cfname_name_field flash

static TEXT_FIELD cfname_name_field = {
	40,
	10,
	display_cfname_name_field,
	change_cfname_name_field,
	open_cfname_name_field,
	&scfname.cf_name
};

/* Define Form
 *
 */
#define quit_cfname_form quit_sub_context

void add_cfname_form_line()
{
	register char *newname;

	insert_entry_in_configname_table(acp->c_cfntbl, acp->c_curline,
					create_cfname(null));

	display_form_body();
	update_context();

	newname = change_field(acp->c_field, ((CFNAME *)acp->c_tbl)->cf_name,
			acp->c_scrline, &textacs);

	remove_configname_table_entry(acp->c_cfntbl, acp->c_curline);

	if (newname != NULL) {
		insert_entry_in_configname_table(acp->c_cfntbl, acp->c_curline,
				create_cfname(newname));
		free_string(newname);
		--acp->c_modified;
	}

	display_form_body();
	update_context();
}

void del_cfname_form_line()
{
	remove_configname_table_entry(acp->c_cfntbl, acp->c_curline);
	display_form_body();
	--acp->c_modified;
	update_context();
}

#define dup_cfname_form_line flash

static FIELD *cfname_name_fields[] = {
	ISFIELD(cfname_name_field)
};

static LINE cfname_form_lines[] = {
	{ 0, 1, cfname_name_fields }
};

FORM cfname_form = {
	CP(&scfname),
	NULL,
	quit_cfname_form,
	0,
	add_cfname_form_line,
	del_cfname_form_line,
	dup_cfname_form_line,
	cfname_form_lines
};


/* CONFIGURATION FILE TABLE CONFIGURATION NAME TABLE FORM
 *
 */

/* Name Field
 *
 */
#define display_cftbl_cfname_name_field display_text_field
#define change_cftbl_cfname_name_field flash
#define open_cftbl_cfname_name_field flash

static TEXT_FIELD cftbl_cfname_name_field = {
	40,
	10,
	display_cftbl_cfname_name_field,
	change_cftbl_cfname_name_field,
	open_cftbl_cfname_name_field,
	&scfname.cf_name

};

/* Define Form
 *
 */
#define quit_cftbl_cfname_form quit_sub_context

#define add_cftbl_cfname_form_line flash
#define del_cftbl_cfname_form_line flash
#define dup_cftbl_cfname_form_line flash

static FIELD *cftbl_cfname_name_fields[] = {
	ISFIELD(cftbl_cfname_name_field)
};

static LINE cftbl_cfname_form_lines[] = {
	{ 0, 1, cftbl_cfname_name_fields }
};

FORM cftbl_cfname_form = {
	CP(&scfname),
	NULL,
	quit_cftbl_cfname_form,
	0,
	add_cftbl_cfname_form_line,
	del_cftbl_cfname_form_line,
	dup_cftbl_cfname_form_line,
	cftbl_cfname_form_lines
};


/* BLOCK/CHARACTER DEVICE MASK STRUCTURE
 *
 */
typedef struct {
	int	dd_na	: 22,	/* not applicable			*/
		dd_sh	: 1,	/* stream handler			*/
		dd_sbch : 1,	/* separate block/char handlers		*/
		dd_ts	: 1,	/* device has a tty structure		*/
		dd_ih	: 1,	/* has an initialization handler	*/
		dd_pfh	: 1,	/* has a power-failure handler		*/
		dd_oh	: 1,	/* has an open handler			*/
		dd_ch	: 1,	/* has a close handler			*/
		dd_rh	: 1,	/* has a read handler			*/
		dd_wh	: 1,	/* has a write handler			*/
		dd_ioh	: 1;	/* has an ioctl handler			*/
} DRVBCDM;


/* BLOCK/CHARACTER DEVICE MASK FORM
 *
 */
#define LENGTH_OF_MASK_FIELD LENGTH(S_DRVSBCH)

/* Stream Handler Field
 *
 */
#define display_mask_stream_handler_field display_bit_field
#define change_mask_stream_handler_field change_bit_field
#define open_mask_stream_handler_field flash

static BIT_FIELD mask_stream_handler_field = {
	5,
	LENGTH_OF_MASK_FIELD,
	display_mask_stream_handler_field,
	change_mask_stream_handler_field,
	open_mask_stream_handler_field,
	T_DRVSTR,
	001000
};

/* Separate Open And Close Routines Field
 *
 */
#define display_mask_open_close_field display_bit_field
#define change_mask_open_close_field change_bit_field
#define open_mask_open_close_field flash

static BIT_FIELD mask_open_close_field = {
	5,
	LENGTH_OF_MASK_FIELD,
	display_mask_open_close_field,
	change_mask_open_close_field,
	open_mask_open_close_field,
	T_DRVSBCH,
	000400
};

/* TTY Structure Field
 *
 */
#define display_mask_tty_field display_bit_field
#define change_mask_tty_field change_bit_field
#define open_mask_tty_field flash

static BIT_FIELD mask_tty_field = {
	5,
	LENGTH_OF_MASK_FIELD,
	display_mask_tty_field,
	change_mask_tty_field,
	open_mask_tty_field,
	T_DRVTTY,
	000200
};

/* Initialization Handler Field
 *
 */
#define display_mask_init_field display_bit_field
#define change_mask_init_field change_bit_field
#define open_mask_init_field flash

static BIT_FIELD mask_init_field = {
	5,
	LENGTH_OF_MASK_FIELD,
	display_mask_init_field,
	change_mask_init_field,
	open_mask_init_field,
	T_DRVINIT,
	000100
};

/* Power-Failure Handler Field
 *
 */
#define display_mask_power_fail_field display_bit_field
#define change_mask_power_fail_field change_bit_field
#define open_mask_power_fail_field flash

static BIT_FIELD mask_power_fail_field = {
	5,
	LENGTH_OF_MASK_FIELD,
	display_mask_power_fail_field,
	change_mask_power_fail_field,
	open_mask_power_fail_field,
	T_DRVPFAIL,
	000040
};

/* Open Handler Field
 *
 */
#define display_mask_open_field display_bit_field
#define change_mask_open_field change_bit_field
#define open_mask_open_field flash

static BIT_FIELD mask_open_field = {
	5,
	LENGTH_OF_MASK_FIELD,
	display_mask_open_field,
	change_mask_open_field,
	open_mask_open_field,
	T_DRVOPEN,
	000020
};

/* Close Handler Field
 *
 */
#define display_mask_close_field display_bit_field
#define change_mask_close_field change_bit_field
#define open_mask_close_field flash

static BIT_FIELD mask_close_field = {
	5,
	LENGTH_OF_MASK_FIELD,
	display_mask_close_field,
	change_mask_close_field,
	open_mask_close_field,
	T_DRVCLOSE,
	000010
};

/* Read Handler Field
 *
 */
#define display_mask_read_field display_bit_field
#define change_mask_read_field change_bit_field
#define open_mask_read_field flash

static BIT_FIELD mask_read_field = {
	5,
	LENGTH_OF_MASK_FIELD,
	display_mask_read_field,
	change_mask_read_field,
	open_mask_read_field,
	T_DRVREAD,
	000004
};

/* Write Handler Field
 *
 */
#define display_mask_write_field display_bit_field
#define change_mask_write_field change_bit_field
#define open_mask_write_field flash

static BIT_FIELD mask_write_field = {
	5,
	LENGTH_OF_MASK_FIELD,
	display_mask_write_field,
	change_mask_write_field,
	open_mask_write_field,
	T_DRVWRITE,
	000002
};

/* IOCTL Handler Field
 *
 */
#define display_mask_ioctl_field display_bit_field
#define change_mask_ioctl_field change_bit_field
#define open_mask_ioctl_field flash

static BIT_FIELD mask_ioctl_field = {
	5,
	LENGTH_OF_MASK_FIELD,
	display_mask_ioctl_field,
	change_mask_ioctl_field,
	open_mask_ioctl_field,
	T_DRVIOCTL,
	000001
};

/* Define Form
 *
 */
#define quit_mask_form quit_sub_context

#define add_mask_form_line flash
#define del_mask_form_line flash
#define dup_mask_form_line flash

static FIELD *mask_stream_handler_fields[] = {
	ISFIELD(mask_stream_handler_field)
};

static FIELD *mask_open_close_fields[] = {
	ISFIELD(mask_open_close_field)
};

static FIELD *mask_tty_fields[]  = {
	ISFIELD(mask_tty_field)
};

static FIELD *mask_init_fields[]  = {
	ISFIELD(mask_init_field)
};

static FIELD *mask_power_fail_fields[]  = {
	ISFIELD(mask_power_fail_field)
};

static FIELD *mask_open_fields[]  = {
	ISFIELD(mask_open_field)
};

static FIELD *mask_close_fields[]  = {
	ISFIELD(mask_close_field)
};

static FIELD *mask_read_fields[]  = {
	ISFIELD(mask_read_field)
};

static FIELD *mask_write_fields[]  = {
	ISFIELD(mask_write_field)
};

static FIELD *mask_ioctl_fields[]  = {
	ISFIELD(mask_ioctl_field)
};

static LINE mask_form_lines[] = {
	{ 0, 1, mask_stream_handler_fields },
	{ 1, 1, mask_open_close_fields },
	{ 2, 1, mask_tty_fields },
	{ 3, 1, mask_init_fields },
	{ 4, 1, mask_power_fail_fields },
	{ 5, 1, mask_open_fields },
	{ 6, 1, mask_close_fields },
	{ 7, 1, mask_read_fields },
	{ 8, 1, mask_write_fields },
	{ 9, 1, mask_ioctl_fields }
};

static FORM mask_form = {
	NULL,
	NULL,
	quit_mask_form,
	10,
	add_mask_form_line,
	del_mask_form_line,
	dup_mask_form_line,
	mask_form_lines
};


/* CONFIGURATION FILE TABLE BLOCK/CHARACTER DEVICE FORM
 *
 */

/* Stream Handler Field
 *
 */
#define display_cftbl_mask_stream_handler_field display_bit_field
#define change_cftbl_mask_stream_handler_field flash
#define open_cftbl_mask_stream_handler_field flash

static BIT_FIELD cftbl_mask_stream_handler_field = {
	5,
	LENGTH_OF_MASK_FIELD,
	display_cftbl_mask_stream_handler_field,
	change_cftbl_mask_stream_handler_field,
	open_cftbl_mask_stream_handler_field,
	T_DRVSTR,
	001000
};

/* Separate Open And Close Routines Field
 *
 */
#define display_cftbl_mask_open_close_field display_bit_field
#define change_cftbl_mask_open_close_field flash
#define open_cftbl_mask_open_close_field flash

static BIT_FIELD cftbl_mask_open_close_field = {
	5,
	LENGTH_OF_MASK_FIELD,
	display_cftbl_mask_open_close_field,
	change_cftbl_mask_open_close_field,
	open_cftbl_mask_open_close_field,
	T_DRVSBCH,
	000400
};

/* TTY Structure Field
 *
 */
#define display_cftbl_mask_tty_field display_bit_field
#define change_cftbl_mask_tty_field flash
#define open_cftbl_mask_tty_field flash

static BIT_FIELD cftbl_mask_tty_field = {
	5,
	LENGTH_OF_MASK_FIELD,
	display_cftbl_mask_tty_field,
	change_cftbl_mask_tty_field,
	open_cftbl_mask_tty_field,
	T_DRVTTY,
	000200
};

/* Initialization Handler Field
 *
 */
#define display_cftbl_mask_init_field display_bit_field
#define change_cftbl_mask_init_field flash
#define open_cftbl_mask_init_field flash

static BIT_FIELD cftbl_mask_init_field = {
	5,
	LENGTH_OF_MASK_FIELD,
	display_cftbl_mask_init_field,
	change_cftbl_mask_init_field,
	open_cftbl_mask_init_field,
	T_DRVINIT,
	000100
};

/* Power-Failure Handler Field
 *
 */
#define display_cftbl_mask_power_fail_field display_bit_field
#define change_cftbl_mask_power_fail_field flash
#define open_cftbl_mask_power_fail_field flash

static BIT_FIELD cftbl_mask_power_fail_field = {
	5,
	LENGTH_OF_MASK_FIELD,
	display_cftbl_mask_power_fail_field,
	change_cftbl_mask_power_fail_field,
	open_cftbl_mask_power_fail_field,
	T_DRVPFAIL,
	000040
};

/* Open Handler Field
 *
 */
#define display_cftbl_mask_open_field display_bit_field
#define change_cftbl_mask_open_field flash
#define open_cftbl_mask_open_field flash

static BIT_FIELD cftbl_mask_open_field = {
	5,
	LENGTH_OF_MASK_FIELD,
	display_cftbl_mask_open_field,
	change_cftbl_mask_open_field,
	open_cftbl_mask_open_field,
	T_DRVOPEN,
	000020
};

/* Close Handler Field
 *
 */
#define display_cftbl_mask_close_field display_bit_field
#define change_cftbl_mask_close_field flash
#define open_cftbl_mask_close_field flash

static BIT_FIELD cftbl_mask_close_field = {
	5,
	LENGTH_OF_MASK_FIELD,
	display_cftbl_mask_close_field,
	change_cftbl_mask_close_field,
	open_cftbl_mask_close_field,
	T_DRVCLOSE,
	000010
};

/* Read Handler Field
 *
 */
#define display_cftbl_mask_read_field display_bit_field
#define change_cftbl_mask_read_field flash
#define open_cftbl_mask_read_field flash

static BIT_FIELD cftbl_mask_read_field = {
	5,
	LENGTH_OF_MASK_FIELD,
	display_cftbl_mask_read_field,
	change_cftbl_mask_read_field,
	open_cftbl_mask_read_field,
	T_DRVREAD,
	000004
};

/* Write Handler Field
 *
 */
#define display_cftbl_mask_write_field display_bit_field
#define change_cftbl_mask_write_field flash
#define open_cftbl_mask_write_field flash

static BIT_FIELD cftbl_mask_write_field = {
	5,
	LENGTH_OF_MASK_FIELD,
	display_cftbl_mask_write_field,
	change_cftbl_mask_write_field,
	open_cftbl_mask_write_field,
	T_DRVWRITE,
	000002
};

/* IOCTL Handler Field
 *
 */
#define display_cftbl_mask_ioctl_field display_bit_field
#define change_cftbl_mask_ioctl_field flash
#define open_cftbl_mask_ioctl_field flash

static BIT_FIELD cftbl_mask_ioctl_field = {
	5,
	LENGTH_OF_MASK_FIELD,
	display_cftbl_mask_ioctl_field,
	change_cftbl_mask_ioctl_field,
	open_cftbl_mask_ioctl_field,
	T_DRVIOCTL,
	000001
};

/* Define Form
 *
 */
#define quit_cftbl_mask_form quit_sub_context

#define add_cftbl_mask_form_line flash
#define del_cftbl_mask_form_line flash
#define dup_cftbl_mask_form_line flash

static FIELD *cftbl_mask_stream_handler_fields[] = {
	ISFIELD(cftbl_mask_stream_handler_field)
};

static FIELD *cftbl_mask_open_close_fields[] = {
	ISFIELD(cftbl_mask_open_close_field)
};

static FIELD *cftbl_mask_tty_fields[]  = {
	ISFIELD(cftbl_mask_tty_field)
};

static FIELD *cftbl_mask_init_fields[]  = {
	ISFIELD(cftbl_mask_init_field)
};

static FIELD *cftbl_mask_power_fail_fields[]  = {
	ISFIELD(cftbl_mask_power_fail_field)
};

static FIELD *cftbl_mask_open_fields[]  = {
	ISFIELD(cftbl_mask_open_field)
};

static FIELD *cftbl_mask_close_fields[]  = {
	ISFIELD(cftbl_mask_close_field)
};

static FIELD *cftbl_mask_read_fields[]  = {
	ISFIELD(cftbl_mask_read_field)
};

static FIELD *cftbl_mask_write_fields[]  = {
	ISFIELD(cftbl_mask_write_field)
};

static FIELD *cftbl_mask_ioctl_fields[]  = {
	ISFIELD(cftbl_mask_ioctl_field)
};

static LINE cftbl_mask_form_lines[] = {
	{ 0, 1, cftbl_mask_stream_handler_fields },
	{ 1, 1, cftbl_mask_open_close_fields },
	{ 2, 1, cftbl_mask_tty_fields },
	{ 3, 1, cftbl_mask_init_fields },
	{ 4, 1, cftbl_mask_power_fail_fields },
	{ 5, 1, cftbl_mask_open_fields },
	{ 6, 1, cftbl_mask_close_fields },
	{ 7, 1, cftbl_mask_read_fields },
	{ 8, 1, cftbl_mask_write_fields },
	{ 9, 1, cftbl_mask_ioctl_fields }
};

static FORM cftbl_mask_form = {
	NULL,
	NULL,
	quit_cftbl_mask_form,
	10,
	add_cftbl_mask_form_line,
	del_cftbl_mask_form_line,
	dup_cftbl_mask_form_line,
	cftbl_mask_form_lines
};


/* FILESYSTEM HANDLER MASK STRUCTURE
 *
 */
typedef struct {
	int	dh_na	: 5,	/* not used				*/
		dh_ioc	: 1,	/* has a ioctl handler			*/
		dh_info	: 1,	/* has an info handler			*/
		dh_ctrl	: 1,	/* has a control handler		*/
		dh_ntfy	: 1,	/* has a notify handler			*/
		dh_satr	: 1,	/* has a set attributes handler		*/
		dh_rmap	: 1,	/* has a read map handler		*/
		dh_fmap	: 1,	/* has a free map handler		*/
		dh_amap	: 1,	/* has an allocate map handler		*/
		dh_gdnt	: 1,	/* has a get dents handler		*/
		dh_acce	: 1,	/* has an access handler		*/
		dh_stat	: 1,	/* has a status filesystem handler	*/
		dh_upda	: 1,	/* has an update handler		*/
		dh_cino	: 1,	/* has a close inode handler		*/
		dh_oino	: 1,	/* has an open inode handler		*/
		dh_gino	: 1,	/* has a get indoe handler		*/
		dh_umnt	: 1,	/* has an umount handler		*/
		dh_mnt	: 1,	/* has a mount handler			*/
		dh_nami	: 1,	/* has a namei handler			*/
		dh_sfil	: 1,	/* has a status file handler		*/
		dh_itrn	: 1,	/* has an inode truncate handler	*/
		dh_init	: 1,	/* has an initialization handler	*/
		dh_wino	: 1,	/* has a write inode handler		*/
		dh_rino	: 1,	/* has a read inode handler		*/
		dh_uino	: 1,	/* has an update inod handler		*/
		dh_nuse	: 1,	/* not used				*/
		dh_ird	: 1,	/* has an inode read handler		*/
		dh_iput	: 1;	/* has an inode put handler		*/
} DRVFSHM;


/* FILESYSTEM HANDLER MASK FORM
 *
 */
#define LENGTH_OF_FSH_MASK_FIELD LENGTH(S_FSHUPINOD)

/* IOCTL Field
 *
 */
#define display_fsh_mask_ioctl_field display_bit_field
#define change_fsh_mask_ioctl_field change_bit_field
#define open_fsh_mask_ioctl_field flash

static BIT_FIELD fsh_mask_ioctl_field = {
	5,
	LENGTH_OF_FSH_MASK_FIELD,
	display_fsh_mask_ioctl_field,
	change_fsh_mask_ioctl_field,
	open_fsh_mask_ioctl_field,
	T_FSHIOCTL,
	0400000000
};

/* Information Field
 *
 */
#define display_fsh_mask_info_field display_bit_field
#define change_fsh_mask_info_field change_bit_field
#define open_fsh_mask_info_field flash

static BIT_FIELD fsh_mask_info_field = {
	5,
	LENGTH_OF_FSH_MASK_FIELD,
	display_fsh_mask_info_field,
	change_fsh_mask_info_field,
	open_fsh_mask_info_field,
	T_FSHINFO,
	0200000000
};

/* Control Field
 *
 */
#define display_fsh_mask_control_field display_bit_field
#define change_fsh_mask_control_field change_bit_field
#define open_fsh_mask_control_field flash

static BIT_FIELD fsh_mask_control_field = {
	5,
	LENGTH_OF_FSH_MASK_FIELD,
	display_fsh_mask_control_field,
	change_fsh_mask_control_field,
	open_fsh_mask_control_field,
	T_FSHCTRL,
	0100000000
};

/* Notify Field
 *
 */
#define display_fsh_mask_notify_field display_bit_field
#define change_fsh_mask_notify_field change_bit_field
#define open_fsh_mask_notify_field flash

static BIT_FIELD fsh_mask_notify_field = {
	5,
	LENGTH_OF_FSH_MASK_FIELD,
	display_fsh_mask_notify_field,
	change_fsh_mask_notify_field,
	open_fsh_mask_notify_field,
	T_FSHNOTIFY,
	0040000000
};

/* Set Attributes Field
 *
 */
#define display_fsh_mask_set_attr_field display_bit_field
#define change_fsh_mask_set_attr_field change_bit_field
#define open_fsh_mask_set_attr_field flash

static BIT_FIELD fsh_mask_set_attr_field = {
	5,
	LENGTH_OF_FSH_MASK_FIELD,
	display_fsh_mask_set_attr_field,
	change_fsh_mask_set_attr_field,
	open_fsh_mask_set_attr_field,
	T_FSHSATTR,
	0020000000
};

/* Read Map Field
 *
 */
#define display_fsh_mask_read_map_field display_bit_field
#define change_fsh_mask_read_map_field change_bit_field
#define open_fsh_mask_read_map_field flash

static BIT_FIELD fsh_mask_read_map_field = {
	5,
	LENGTH_OF_FSH_MASK_FIELD,
	display_fsh_mask_read_map_field,
	change_fsh_mask_read_map_field,
	open_fsh_mask_read_map_field,
	T_FSHRDMAP,
	0010000000
};

/* Free Map Field
 *
 */
#define display_fsh_mask_free_map_field display_bit_field
#define change_fsh_mask_free_map_field change_bit_field
#define open_fsh_mask_free_map_field flash

static BIT_FIELD fsh_mask_free_map_field = {
	5,
	LENGTH_OF_FSH_MASK_FIELD,
	display_fsh_mask_free_map_field,
	change_fsh_mask_free_map_field,
	open_fsh_mask_free_map_field,
	T_FSHFRMAP,
	0004000000
};

/* Allocate Map Field
 *
 */
#define display_fsh_mask_alloc_map_field display_bit_field
#define change_fsh_mask_alloc_map_field change_bit_field
#define open_fsh_mask_alloc_map_field flash

static BIT_FIELD fsh_mask_alloc_map_field = {
	5,
	LENGTH_OF_FSH_MASK_FIELD,
	display_fsh_mask_alloc_map_field,
	change_fsh_mask_alloc_map_field,
	open_fsh_mask_alloc_map_field,
	T_FSHALMAP,
	0002000000
};

/* Get Dents Field
 *
 */
#define display_fsh_mask_get_dents_field display_bit_field
#define change_fsh_mask_get_dents_field change_bit_field
#define open_fsh_mask_get_dents_field flash

static BIT_FIELD fsh_mask_get_dents_field = {
	5,
	LENGTH_OF_FSH_MASK_FIELD,
	display_fsh_mask_get_dents_field,
	change_fsh_mask_get_dents_field,
	open_fsh_mask_get_dents_field,
	T_FSHGDENTS,
	0001000000
};

/* Access Field
 *
 */
#define display_fsh_mask_access_field display_bit_field
#define change_fsh_mask_access_field change_bit_field
#define open_fsh_mask_access_field flash

static BIT_FIELD fsh_mask_access_field = {
	5,
	LENGTH_OF_FSH_MASK_FIELD,
	display_fsh_mask_access_field,
	change_fsh_mask_access_field,
	open_fsh_mask_access_field,
	T_FSHACCESS,
	0000400000
};

/* Status Field
 *
 */
#define display_fsh_mask_status_field display_bit_field
#define change_fsh_mask_status_field change_bit_field
#define open_fsh_mask_status_field flash

static BIT_FIELD fsh_mask_status_field = {
	5,
	LENGTH_OF_FSH_MASK_FIELD,
	display_fsh_mask_status_field,
	change_fsh_mask_status_field,
	open_fsh_mask_status_field,
	T_FSHSTATUS,
	0000200000
};

/* Update Field
 *
 */
#define display_fsh_mask_update_field display_bit_field
#define change_fsh_mask_update_field change_bit_field
#define open_fsh_mask_update_field flash

static BIT_FIELD fsh_mask_update_field = {
	5,
	LENGTH_OF_FSH_MASK_FIELD,
	display_fsh_mask_update_field,
	change_fsh_mask_update_field,
	open_fsh_mask_update_field,
	T_FSHUPDATE,
	0000100000
};

/* Close Inode Field
 *
 */
#define display_fsh_mask_close_inode_field display_bit_field
#define change_fsh_mask_close_inode_field change_bit_field
#define open_fsh_mask_close_inode_field flash

static BIT_FIELD fsh_mask_close_inode_field = {
	5,
	LENGTH_OF_FSH_MASK_FIELD,
	display_fsh_mask_close_inode_field,
	change_fsh_mask_close_inode_field,
	open_fsh_mask_close_inode_field,
	T_FSHCLINOD,
	0000040000
};

/* Open Inode Field
 *
 */
#define display_fsh_mask_open_inode_field display_bit_field
#define change_fsh_mask_open_inode_field change_bit_field
#define open_fsh_mask_open_inode_field flash

static BIT_FIELD fsh_mask_open_inode_field = {
	5,
	LENGTH_OF_FSH_MASK_FIELD,
	display_fsh_mask_open_inode_field,
	change_fsh_mask_open_inode_field,
	open_fsh_mask_open_inode_field,
	T_FSHOPINOD,
	0000020000
};

/* Get Inode Field
 *
 */
#define display_fsh_mask_get_inode_field display_bit_field
#define change_fsh_mask_get_inode_field change_bit_field
#define open_fsh_mask_get_inode_field flash

static BIT_FIELD fsh_mask_get_inode_field = {
	5,
	LENGTH_OF_FSH_MASK_FIELD,
	display_fsh_mask_get_inode_field,
	change_fsh_mask_get_inode_field,
	open_fsh_mask_get_inode_field,
	T_FSHGTINOD,
	0000010000
};

/* Unmount Field
 *
 */
#define display_fsh_mask_umount_field display_bit_field
#define change_fsh_mask_umount_field change_bit_field
#define open_fsh_mask_umount_field flash

static BIT_FIELD fsh_mask_umount_field = {
	5,
	LENGTH_OF_FSH_MASK_FIELD,
	display_fsh_mask_umount_field,
	change_fsh_mask_umount_field,
	open_fsh_mask_umount_field,
	T_FSHUMOUNT,
	0000004000
};

/* Mount Field
 *
 */
#define display_fsh_mask_mount_field display_bit_field
#define change_fsh_mask_mount_field change_bit_field
#define open_fsh_mask_mount_field flash

static BIT_FIELD fsh_mask_mount_field = {
	5,
	LENGTH_OF_FSH_MASK_FIELD,
	display_fsh_mask_mount_field,
	change_fsh_mask_mount_field,
	open_fsh_mask_mount_field,
	T_FSHMOUNT,
	0000002000
};

/* Namei Field
 *
 */
#define display_fsh_mask_namei_field display_bit_field
#define change_fsh_mask_namei_field change_bit_field
#define open_fsh_mask_namei_field flash

static BIT_FIELD fsh_mask_namei_field = {
	5,
	LENGTH_OF_FSH_MASK_FIELD,
	display_fsh_mask_namei_field,
	change_fsh_mask_namei_field,
	open_fsh_mask_namei_field,
	T_FSHNAMEI,
	0000001000
};

/* Status File Field
 *
 */
#define display_fsh_mask_status_file_field display_bit_field
#define change_fsh_mask_status_file_field change_bit_field
#define open_fsh_mask_status_file_field flash

static BIT_FIELD fsh_mask_status_file_field = {
	5,
	LENGTH_OF_FSH_MASK_FIELD,
	display_fsh_mask_status_file_field,
	change_fsh_mask_status_file_field,
	open_fsh_mask_status_file_field,
	T_FSHFSTAT,
	0000000400
};

/* Inode Truncate Field
 *
 */
#define display_fsh_mask_inode_trunc_field display_bit_field
#define change_fsh_mask_inode_trunc_field change_bit_field
#define open_fsh_mask_inode_trunc_field flash

static BIT_FIELD fsh_mask_inode_trunc_field = {
	5,
	LENGTH_OF_FSH_MASK_FIELD,
	display_fsh_mask_inode_trunc_field,
	change_fsh_mask_inode_trunc_field,
	open_fsh_mask_inode_trunc_field,
	T_FSHITRUNC,
	0000000200
};

/* Initialization Field
 *
 */
#define display_fsh_mask_init_field display_bit_field
#define change_fsh_mask_init_field change_bit_field
#define open_fsh_mask_init_field flash

static BIT_FIELD fsh_mask_init_field = {
	5,
	LENGTH_OF_FSH_MASK_FIELD,
	display_fsh_mask_init_field,
	change_fsh_mask_init_field,
	open_fsh_mask_init_field,
	T_FSHINIT,
	0000000100
};

/* Write Inode Field
 *
 */
#define display_fsh_mask_write_inode_field display_bit_field
#define change_fsh_mask_write_inode_field change_bit_field
#define open_fsh_mask_write_inode_field flash

static BIT_FIELD fsh_mask_write_inode_field = {
	5,
	LENGTH_OF_FSH_MASK_FIELD,
	display_fsh_mask_write_inode_field,
	change_fsh_mask_write_inode_field,
	open_fsh_mask_write_inode_field,
	T_FSHWRINOD,
	0000000040
};

/* Read Inode Field
 *
 */
#define display_fsh_mask_read_inode_field display_bit_field
#define change_fsh_mask_read_inode_field change_bit_field
#define open_fsh_mask_read_inode_field flash

static BIT_FIELD fsh_mask_read_inode_field = {
	5,
	LENGTH_OF_FSH_MASK_FIELD,
	display_fsh_mask_read_inode_field,
	change_fsh_mask_read_inode_field,
	open_fsh_mask_read_inode_field,
	T_FSHRDINOD,
	0000000020
};

/* Update Inode Field
 *
 */
#define display_fsh_mask_update_inode_field display_bit_field
#define change_fsh_mask_update_inode_field change_bit_field
#define open_fsh_mask_update_inode_field flash

static BIT_FIELD fsh_mask_update_inode_field = {
	5,
	LENGTH_OF_FSH_MASK_FIELD,
	display_fsh_mask_update_inode_field,
	change_fsh_mask_update_inode_field,
	open_fsh_mask_update_inode_field,
	T_FSHUPINOD,
	0000000010
};

/* Inode Read Field
 *
 */
#define display_fsh_mask_inode_read_field display_bit_field
#define change_fsh_mask_inode_read_field change_bit_field
#define open_fsh_mask_inode_read_field flash

static BIT_FIELD fsh_mask_inode_read_field = {
	5,
	LENGTH_OF_FSH_MASK_FIELD,
	display_fsh_mask_inode_read_field,
	change_fsh_mask_inode_read_field,
	open_fsh_mask_inode_read_field,
	T_FSHIREAD,
	0000000002
};

/* Inode Put Field
 *
 */
#define display_fsh_mask_inode_put_field display_bit_field
#define change_fsh_mask_inode_put_field change_bit_field
#define open_fsh_mask_inode_put_field flash

static BIT_FIELD fsh_mask_inode_put_field = {
	5,
	LENGTH_OF_FSH_MASK_FIELD,
	display_fsh_mask_inode_put_field,
	change_fsh_mask_inode_put_field,
	open_fsh_mask_inode_put_field,
	T_FSHIPUT,
	0000000001
};

/* Define Form
 *
 */
#define quit_fsh_mask_form quit_sub_context

#define add_fsh_mask_form_line flash
#define del_fsh_mask_form_line flash
#define dup_fsh_mask_form_line flash

static FIELD *fsh_mask_ioctl_fields[] = {
	ISFIELD(fsh_mask_ioctl_field)
};

static FIELD *fsh_mask_info_fields[] = {
	ISFIELD(fsh_mask_info_field)
};

static FIELD *fsh_mask_control_fields[] = {
	ISFIELD(fsh_mask_control_field)
};

static FIELD *fsh_mask_notify_fields[] = {
	ISFIELD(fsh_mask_notify_field)
};

static FIELD *fsh_mask_set_attr_fields[] = {
	ISFIELD(fsh_mask_set_attr_field)
};

static FIELD *fsh_mask_read_map_fields[] = {
	ISFIELD(fsh_mask_read_map_field)
};

static FIELD *fsh_mask_free_map_fields[] = {
	ISFIELD(fsh_mask_free_map_field)
};

static FIELD *fsh_mask_alloc_map_fields[] = {
	ISFIELD(fsh_mask_alloc_map_field)
};

static FIELD *fsh_mask_get_dents_fields[] = {
	ISFIELD(fsh_mask_get_dents_field)
};

static FIELD *fsh_mask_access_fields[] = {
	ISFIELD(fsh_mask_access_field)
};

static FIELD *fsh_mask_status_fields[] = {
	ISFIELD(fsh_mask_status_field)
};

static FIELD *fsh_mask_update_fields[] = {
	ISFIELD(fsh_mask_update_field)
};

static FIELD *fsh_mask_close_inode_fields[] = {
	ISFIELD(fsh_mask_close_inode_field)
};

static FIELD *fsh_mask_open_inode_fields[] = {
	ISFIELD(fsh_mask_open_inode_field)
};

static FIELD *fsh_mask_get_inode_fields[] = {
	ISFIELD(fsh_mask_get_inode_field)
};

static FIELD *fsh_mask_umount_fields[] = {
	ISFIELD(fsh_mask_umount_field)
};

static FIELD *fsh_mask_mount_fields[] = {
	ISFIELD(fsh_mask_mount_field)
};

static FIELD *fsh_mask_namei_fields[] = {
	ISFIELD(fsh_mask_namei_field)
};

static FIELD *fsh_mask_status_file_fields[] = {
	ISFIELD(fsh_mask_status_file_field)
};

static FIELD *fsh_mask_inode_trunc_fields[] = {
	ISFIELD(fsh_mask_inode_trunc_field)
};

static FIELD *fsh_mask_init_fields[] = {
	ISFIELD(fsh_mask_init_field)
};

static FIELD *fsh_mask_write_inode_fields[] = {
	ISFIELD(fsh_mask_write_inode_field)
};

static FIELD *fsh_mask_read_inode_fields[] = {
	ISFIELD(fsh_mask_read_inode_field)
};

static FIELD *fsh_mask_update_inode_fields[] = {
	ISFIELD(fsh_mask_update_inode_field)
};

static FIELD *fsh_mask_inode_read_fields[] = {
	ISFIELD(fsh_mask_inode_read_field)
};

static FIELD *fsh_mask_inode_put_fields[] = {
	ISFIELD(fsh_mask_inode_put_field)
};

static LINE fsh_mask_form_lines[] = {
	{  0, 1, fsh_mask_ioctl_fields },
	{  1, 1, fsh_mask_info_fields },
	{  2, 1, fsh_mask_control_fields },
	{  3, 1, fsh_mask_notify_fields },
	{  4, 1, fsh_mask_set_attr_fields },
	{  5, 1, fsh_mask_read_map_fields },
	{  6, 1, fsh_mask_free_map_fields },
	{  7, 1, fsh_mask_alloc_map_fields },
	{  8, 1, fsh_mask_get_dents_fields },
	{  9, 1, fsh_mask_access_fields },
	{ 10, 1, fsh_mask_status_fields },
	{ 11, 1, fsh_mask_update_fields },
	{ 12, 1, fsh_mask_close_inode_fields },
	{ 13, 1, fsh_mask_open_inode_fields },
	{ 14, 1, fsh_mask_get_inode_fields },
	{ 15, 1, fsh_mask_umount_fields },
	{ 16, 1, fsh_mask_mount_fields },
	{ 17, 1, fsh_mask_namei_fields },
	{ 18, 1, fsh_mask_status_file_fields },
	{ 19, 1, fsh_mask_inode_trunc_fields },
	{ 20, 1, fsh_mask_init_fields },
	{ 21, 1, fsh_mask_write_inode_fields },
	{ 22, 1, fsh_mask_read_inode_fields },
	{ 23, 1, fsh_mask_update_inode_fields },
	{ 24, 1, fsh_mask_inode_read_fields },
	{ 25, 1, fsh_mask_inode_put_fields }
};

static FORM fsh_mask_form = {
	NULL,
	NULL,
	quit_fsh_mask_form,
	26,
	add_fsh_mask_form_line,
	del_fsh_mask_form_line,
	dup_fsh_mask_form_line,
	fsh_mask_form_lines
};


/* CONFIGURATION FILE TABLE FILESYSTEM HANDLER MASK FORM
 *
 */

/* IOCTL Field
 *
 */
#define display_cftbl_fsh_mask_ioctl_field display_bit_field
#define change_cftbl_fsh_mask_ioctl_field flash
#define open_cftbl_fsh_mask_ioctl_field flash

static BIT_FIELD cftbl_fsh_mask_ioctl_field = {
	5,
	LENGTH_OF_FSH_MASK_FIELD,
	display_cftbl_fsh_mask_ioctl_field,
	change_cftbl_fsh_mask_ioctl_field,
	open_cftbl_fsh_mask_ioctl_field,
	T_FSHIOCTL,
	0400000000
};

/* Information Field
 *
 */
#define display_cftbl_fsh_mask_info_field display_bit_field
#define change_cftbl_fsh_mask_info_field flash
#define open_cftbl_fsh_mask_info_field flash

static BIT_FIELD cftbl_fsh_mask_info_field = {
	5,
	LENGTH_OF_FSH_MASK_FIELD,
	display_cftbl_fsh_mask_info_field,
	change_cftbl_fsh_mask_info_field,
	open_cftbl_fsh_mask_info_field,
	T_FSHINFO,
	0200000000
};

/* Control Field
 *
 */
#define display_cftbl_fsh_mask_control_field display_bit_field
#define change_cftbl_fsh_mask_control_field flash
#define open_cftbl_fsh_mask_control_field flash

static BIT_FIELD cftbl_fsh_mask_control_field = {
	5,
	LENGTH_OF_FSH_MASK_FIELD,
	display_cftbl_fsh_mask_control_field,
	change_cftbl_fsh_mask_control_field,
	open_cftbl_fsh_mask_control_field,
	T_FSHCTRL,
	0100000000
};

/* Notify Field
 *
 */
#define display_cftbl_fsh_mask_notify_field display_bit_field
#define change_cftbl_fsh_mask_notify_field flash
#define open_cftbl_fsh_mask_notify_field flash

static BIT_FIELD cftbl_fsh_mask_notify_field = {
	5,
	LENGTH_OF_FSH_MASK_FIELD,
	display_cftbl_fsh_mask_notify_field,
	change_cftbl_fsh_mask_notify_field,
	open_cftbl_fsh_mask_notify_field,
	T_FSHNOTIFY,
	0040000000
};

/* Set Attributes Field
 *
 */
#define display_cftbl_fsh_mask_set_attr_field display_bit_field
#define change_cftbl_fsh_mask_set_attr_field flash
#define open_cftbl_fsh_mask_set_attr_field flash

static BIT_FIELD cftbl_fsh_mask_set_attr_field = {
	5,
	LENGTH_OF_FSH_MASK_FIELD,
	display_cftbl_fsh_mask_set_attr_field,
	change_cftbl_fsh_mask_set_attr_field,
	open_cftbl_fsh_mask_set_attr_field,
	T_FSHSATTR,
	0020000000
};

/* Read Map Field
 *
 */
#define display_cftbl_fsh_mask_read_map_field display_bit_field
#define change_cftbl_fsh_mask_read_map_field flash
#define open_cftbl_fsh_mask_read_map_field flash

static BIT_FIELD cftbl_fsh_mask_read_map_field = {
	5,
	LENGTH_OF_FSH_MASK_FIELD,
	display_cftbl_fsh_mask_read_map_field,
	change_cftbl_fsh_mask_read_map_field,
	open_cftbl_fsh_mask_read_map_field,
	T_FSHRDMAP,
	0010000000
};

/* Free Map Field
 *
 */
#define display_cftbl_fsh_mask_free_map_field display_bit_field
#define change_cftbl_fsh_mask_free_map_field flash
#define open_cftbl_fsh_mask_free_map_field flash

static BIT_FIELD cftbl_fsh_mask_free_map_field = {
	5,
	LENGTH_OF_FSH_MASK_FIELD,
	display_cftbl_fsh_mask_free_map_field,
	change_cftbl_fsh_mask_free_map_field,
	open_cftbl_fsh_mask_free_map_field,
	T_FSHFRMAP,
	0004000000
};

/* Allocate Map Field
 *
 */
#define display_cftbl_fsh_mask_alloc_map_field display_bit_field
#define change_cftbl_fsh_mask_alloc_map_field flash
#define open_cftbl_fsh_mask_alloc_map_field flash

static BIT_FIELD cftbl_fsh_mask_alloc_map_field = {
	5,
	LENGTH_OF_FSH_MASK_FIELD,
	display_cftbl_fsh_mask_alloc_map_field,
	change_cftbl_fsh_mask_alloc_map_field,
	open_cftbl_fsh_mask_alloc_map_field,
	T_FSHALMAP,
	0002000000
};

/* Get Dents Field
 *
 */
#define display_cftbl_fsh_mask_get_dents_field display_bit_field
#define change_cftbl_fsh_mask_get_dents_field flash
#define open_cftbl_fsh_mask_get_dents_field flash

static BIT_FIELD cftbl_fsh_mask_get_dents_field = {
	5,
	LENGTH_OF_FSH_MASK_FIELD,
	display_cftbl_fsh_mask_get_dents_field,
	change_cftbl_fsh_mask_get_dents_field,
	open_cftbl_fsh_mask_get_dents_field,
	T_FSHGDENTS,
	0001000000
};

/* Access Field
 *
 */
#define display_cftbl_fsh_mask_access_field display_bit_field
#define change_cftbl_fsh_mask_access_field flash
#define open_cftbl_fsh_mask_access_field flash

static BIT_FIELD cftbl_fsh_mask_access_field = {
	5,
	LENGTH_OF_FSH_MASK_FIELD,
	display_cftbl_fsh_mask_access_field,
	change_cftbl_fsh_mask_access_field,
	open_cftbl_fsh_mask_access_field,
	T_FSHACCESS,
	0000400000
};

/* Status Field
 *
 */
#define display_cftbl_fsh_mask_status_field display_bit_field
#define change_cftbl_fsh_mask_status_field flash
#define open_cftbl_fsh_mask_status_field flash

static BIT_FIELD cftbl_fsh_mask_status_field = {
	5,
	LENGTH_OF_FSH_MASK_FIELD,
	display_cftbl_fsh_mask_status_field,
	change_cftbl_fsh_mask_status_field,
	open_cftbl_fsh_mask_status_field,
	T_FSHSTATUS,
	0000200000
};

/* Update Field
 *
 */
#define display_cftbl_fsh_mask_update_field display_bit_field
#define change_cftbl_fsh_mask_update_field flash
#define open_cftbl_fsh_mask_update_field flash

static BIT_FIELD cftbl_fsh_mask_update_field = {
	5,
	LENGTH_OF_FSH_MASK_FIELD,
	display_cftbl_fsh_mask_update_field,
	change_cftbl_fsh_mask_update_field,
	open_cftbl_fsh_mask_update_field,
	T_FSHUPDATE,
	0000100000
};

/* Close Inode Field
 *
 */
#define display_cftbl_fsh_mask_close_inode_field display_bit_field
#define change_cftbl_fsh_mask_close_inode_field flash
#define open_cftbl_fsh_mask_close_inode_field flash

static BIT_FIELD cftbl_fsh_mask_close_inode_field = {
	5,
	LENGTH_OF_FSH_MASK_FIELD,
	display_cftbl_fsh_mask_close_inode_field,
	change_cftbl_fsh_mask_close_inode_field,
	open_cftbl_fsh_mask_close_inode_field,
	T_FSHCLINOD,
	0000040000
};

/* Open Inode Field
 *
 */
#define display_cftbl_fsh_mask_open_inode_field display_bit_field
#define change_cftbl_fsh_mask_open_inode_field flash
#define open_cftbl_fsh_mask_open_inode_field flash

static BIT_FIELD cftbl_fsh_mask_open_inode_field = {
	5,
	LENGTH_OF_FSH_MASK_FIELD,
	display_cftbl_fsh_mask_open_inode_field,
	change_cftbl_fsh_mask_open_inode_field,
	open_cftbl_fsh_mask_open_inode_field,
	T_FSHOPINOD,
	0000020000
};

/* Get Inode Field
 *
 */
#define display_cftbl_fsh_mask_get_inode_field display_bit_field
#define change_cftbl_fsh_mask_get_inode_field flash
#define open_cftbl_fsh_mask_get_inode_field flash

static BIT_FIELD cftbl_fsh_mask_get_inode_field = {
	5,
	LENGTH_OF_FSH_MASK_FIELD,
	display_cftbl_fsh_mask_get_inode_field,
	change_cftbl_fsh_mask_get_inode_field,
	open_cftbl_fsh_mask_get_inode_field,
	T_FSHGTINOD,
	0000010000
};

/* Unmount Field
 *
 */
#define display_cftbl_fsh_mask_umount_field display_bit_field
#define change_cftbl_fsh_mask_umount_field flash
#define open_cftbl_fsh_mask_umount_field flash

static BIT_FIELD cftbl_fsh_mask_umount_field = {
	5,
	LENGTH_OF_FSH_MASK_FIELD,
	display_cftbl_fsh_mask_umount_field,
	change_cftbl_fsh_mask_umount_field,
	open_cftbl_fsh_mask_umount_field,
	T_FSHUMOUNT,
	0000004000
};

/* Mount Field
 *
 */
#define display_cftbl_fsh_mask_mount_field display_bit_field
#define change_cftbl_fsh_mask_mount_field flash
#define open_cftbl_fsh_mask_mount_field flash

static BIT_FIELD cftbl_fsh_mask_mount_field = {
	5,
	LENGTH_OF_FSH_MASK_FIELD,
	display_cftbl_fsh_mask_mount_field,
	change_cftbl_fsh_mask_mount_field,
	open_cftbl_fsh_mask_mount_field,
	T_FSHMOUNT,
	0000002000
};

/* Namei Field
 *
 */
#define display_cftbl_fsh_mask_namei_field display_bit_field
#define change_cftbl_fsh_mask_namei_field flash
#define open_cftbl_fsh_mask_namei_field flash

static BIT_FIELD cftbl_fsh_mask_namei_field = {
	5,
	LENGTH_OF_FSH_MASK_FIELD,
	display_cftbl_fsh_mask_namei_field,
	change_cftbl_fsh_mask_namei_field,
	open_cftbl_fsh_mask_namei_field,
	T_FSHNAMEI,
	0000001000
};

/* Status File Field
 *
 */
#define display_cftbl_fsh_mask_status_file_field display_bit_field
#define change_cftbl_fsh_mask_status_file_field flash
#define open_cftbl_fsh_mask_status_file_field flash

static BIT_FIELD cftbl_fsh_mask_status_file_field = {
	5,
	LENGTH_OF_FSH_MASK_FIELD,
	display_cftbl_fsh_mask_status_file_field,
	change_cftbl_fsh_mask_status_file_field,
	open_cftbl_fsh_mask_status_file_field,
	T_FSHFSTAT,
	0000000400
};

/* Inode Truncate Field
 *
 */
#define display_cftbl_fsh_mask_inode_trunc_field display_bit_field
#define change_cftbl_fsh_mask_inode_trunc_field flash
#define open_cftbl_fsh_mask_inode_trunc_field flash

static BIT_FIELD cftbl_fsh_mask_inode_trunc_field = {
	5,
	LENGTH_OF_FSH_MASK_FIELD,
	display_cftbl_fsh_mask_inode_trunc_field,
	change_cftbl_fsh_mask_inode_trunc_field,
	open_cftbl_fsh_mask_inode_trunc_field,
	T_FSHITRUNC,
	0000000200
};

/* Initialization Field
 *
 */
#define display_cftbl_fsh_mask_init_field display_bit_field
#define change_cftbl_fsh_mask_init_field flash
#define open_cftbl_fsh_mask_init_field flash

static BIT_FIELD cftbl_fsh_mask_init_field = {
	5,
	LENGTH_OF_FSH_MASK_FIELD,
	display_cftbl_fsh_mask_init_field,
	change_cftbl_fsh_mask_init_field,
	open_cftbl_fsh_mask_init_field,
	T_FSHINIT,
	0000000100
};

/* Write Inode Field
 *
 */
#define display_cftbl_fsh_mask_write_inode_field display_bit_field
#define change_cftbl_fsh_mask_write_inode_field flash
#define open_cftbl_fsh_mask_write_inode_field flash

static BIT_FIELD cftbl_fsh_mask_write_inode_field = {
	5,
	LENGTH_OF_FSH_MASK_FIELD,
	display_cftbl_fsh_mask_write_inode_field,
	change_cftbl_fsh_mask_write_inode_field,
	open_cftbl_fsh_mask_write_inode_field,
	T_FSHWRINOD,
	0000000040
};

/* Read Inode Field
 *
 */
#define display_cftbl_fsh_mask_read_inode_field display_bit_field
#define change_cftbl_fsh_mask_read_inode_field flash
#define open_cftbl_fsh_mask_read_inode_field flash

static BIT_FIELD cftbl_fsh_mask_read_inode_field = {
	5,
	LENGTH_OF_FSH_MASK_FIELD,
	display_cftbl_fsh_mask_read_inode_field,
	change_cftbl_fsh_mask_read_inode_field,
	open_cftbl_fsh_mask_read_inode_field,
	T_FSHRDINOD,
	0000000020
};

/* Update Inode Field
 *
 */
#define display_cftbl_fsh_mask_update_inode_field display_bit_field
#define change_cftbl_fsh_mask_update_inode_field flash
#define open_cftbl_fsh_mask_update_inode_field flash

static BIT_FIELD cftbl_fsh_mask_update_inode_field = {
	5,
	LENGTH_OF_FSH_MASK_FIELD,
	display_cftbl_fsh_mask_update_inode_field,
	change_cftbl_fsh_mask_update_inode_field,
	open_cftbl_fsh_mask_update_inode_field,
	T_FSHUPINOD,
	0000000010
};

/* Inode Read Field
 *
 */
#define display_cftbl_fsh_mask_inode_read_field display_bit_field
#define change_cftbl_fsh_mask_inode_read_field flash
#define open_cftbl_fsh_mask_inode_read_field flash

static BIT_FIELD cftbl_fsh_mask_inode_read_field = {
	5,
	LENGTH_OF_FSH_MASK_FIELD,
	display_cftbl_fsh_mask_inode_read_field,
	change_cftbl_fsh_mask_inode_read_field,
	open_cftbl_fsh_mask_inode_read_field,
	T_FSHIREAD,
	0000000002
};

/* Inode Put Field
 *
 */
#define display_cftbl_fsh_mask_inode_put_field display_bit_field
#define change_cftbl_fsh_mask_inode_put_field flash
#define open_cftbl_fsh_mask_inode_put_field flash

static BIT_FIELD cftbl_fsh_mask_inode_put_field = {
	5,
	LENGTH_OF_FSH_MASK_FIELD,
	display_cftbl_fsh_mask_inode_put_field,
	change_cftbl_fsh_mask_inode_put_field,
	open_cftbl_fsh_mask_inode_put_field,
	T_FSHIPUT,
	0000000001
};

/* Define Form
 *
 */
#define quit_cftbl_fsh_mask_form quit_sub_context

#define add_cftbl_fsh_mask_form_line flash
#define del_cftbl_fsh_mask_form_line flash
#define dup_cftbl_fsh_mask_form_line flash

static FIELD *cftbl_fsh_mask_ioctl_fields[] = {
	ISFIELD(cftbl_fsh_mask_ioctl_field)
};

static FIELD *cftbl_fsh_mask_info_fields[] = {
	ISFIELD(cftbl_fsh_mask_info_field)
};

static FIELD *cftbl_fsh_mask_control_fields[] = {
	ISFIELD(cftbl_fsh_mask_control_field)
};

static FIELD *cftbl_fsh_mask_notify_fields[] = {
	ISFIELD(cftbl_fsh_mask_notify_field)
};

static FIELD *cftbl_fsh_mask_set_attr_fields[] = {
	ISFIELD(cftbl_fsh_mask_set_attr_field)
};

static FIELD *cftbl_fsh_mask_read_map_fields[] = {
	ISFIELD(cftbl_fsh_mask_read_map_field)
};

static FIELD *cftbl_fsh_mask_free_map_fields[] = {
	ISFIELD(cftbl_fsh_mask_free_map_field)
};

static FIELD *cftbl_fsh_mask_alloc_map_fields[] = {
	ISFIELD(cftbl_fsh_mask_alloc_map_field)
};

static FIELD *cftbl_fsh_mask_get_dents_fields[] = {
	ISFIELD(cftbl_fsh_mask_get_dents_field)
};

static FIELD *cftbl_fsh_mask_access_fields[] = {
	ISFIELD(cftbl_fsh_mask_access_field)
};

static FIELD *cftbl_fsh_mask_status_fields[] = {
	ISFIELD(cftbl_fsh_mask_status_field)
};

static FIELD *cftbl_fsh_mask_update_fields[] = {
	ISFIELD(cftbl_fsh_mask_update_field)
};

static FIELD *cftbl_fsh_mask_close_inode_fields[] = {
	ISFIELD(cftbl_fsh_mask_close_inode_field)
};

static FIELD *cftbl_fsh_mask_open_inode_fields[] = {
	ISFIELD(cftbl_fsh_mask_open_inode_field)
};

static FIELD *cftbl_fsh_mask_get_inode_fields[] = {
	ISFIELD(cftbl_fsh_mask_get_inode_field)
};

static FIELD *cftbl_fsh_mask_umount_fields[] = {
	ISFIELD(cftbl_fsh_mask_umount_field)
};

static FIELD *cftbl_fsh_mask_mount_fields[] = {
	ISFIELD(cftbl_fsh_mask_mount_field)
};

static FIELD *cftbl_fsh_mask_namei_fields[] = {
	ISFIELD(cftbl_fsh_mask_namei_field)
};

static FIELD *cftbl_fsh_mask_status_file_fields[] = {
	ISFIELD(cftbl_fsh_mask_status_file_field)
};

static FIELD *cftbl_fsh_mask_inode_trunc_fields[] = {
	ISFIELD(cftbl_fsh_mask_inode_trunc_field)
};

static FIELD *cftbl_fsh_mask_init_fields[] = {
	ISFIELD(cftbl_fsh_mask_init_field)
};

static FIELD *cftbl_fsh_mask_write_inode_fields[] = {
	ISFIELD(cftbl_fsh_mask_write_inode_field)
};

static FIELD *cftbl_fsh_mask_read_inode_fields[] = {
	ISFIELD(cftbl_fsh_mask_read_inode_field)
};

static FIELD *cftbl_fsh_mask_update_inode_fields[] = {
	ISFIELD(cftbl_fsh_mask_update_inode_field)
};

static FIELD *cftbl_fsh_mask_inode_read_fields[] = {
	ISFIELD(cftbl_fsh_mask_inode_read_field)
};

static FIELD *cftbl_fsh_mask_inode_put_fields[] = {
	ISFIELD(cftbl_fsh_mask_inode_put_field)
};

static LINE cftbl_fsh_mask_form_lines[] = {
	{  0, 1, cftbl_fsh_mask_ioctl_fields },
	{  1, 1, cftbl_fsh_mask_info_fields },
	{  2, 1, cftbl_fsh_mask_control_fields },
	{  3, 1, cftbl_fsh_mask_notify_fields },
	{  4, 1, cftbl_fsh_mask_set_attr_fields },
	{  5, 1, cftbl_fsh_mask_read_map_fields },
	{  6, 1, cftbl_fsh_mask_free_map_fields },
	{  7, 1, cftbl_fsh_mask_alloc_map_fields },
	{  8, 1, cftbl_fsh_mask_get_dents_fields },
	{  9, 1, cftbl_fsh_mask_access_fields },
	{ 10, 1, cftbl_fsh_mask_status_fields },
	{ 11, 1, cftbl_fsh_mask_update_fields },
	{ 12, 1, cftbl_fsh_mask_close_inode_fields },
	{ 13, 1, cftbl_fsh_mask_open_inode_fields },
	{ 14, 1, cftbl_fsh_mask_get_inode_fields },
	{ 15, 1, cftbl_fsh_mask_umount_fields },
	{ 16, 1, cftbl_fsh_mask_mount_fields },
	{ 17, 1, cftbl_fsh_mask_namei_fields },
	{ 18, 1, cftbl_fsh_mask_status_file_fields },
	{ 19, 1, cftbl_fsh_mask_inode_trunc_fields },
	{ 20, 1, cftbl_fsh_mask_init_fields },
	{ 21, 1, cftbl_fsh_mask_write_inode_fields },
	{ 22, 1, cftbl_fsh_mask_read_inode_fields },
	{ 23, 1, cftbl_fsh_mask_update_inode_fields },
	{ 24, 1, cftbl_fsh_mask_inode_read_fields },
	{ 25, 1, cftbl_fsh_mask_inode_put_fields }
};

static FORM cftbl_fsh_mask_form = {
	NULL,
	NULL,
	quit_cftbl_fsh_mask_form,
	26,
	add_cftbl_fsh_mask_form_line,
	del_cftbl_fsh_mask_form_line,
	dup_cftbl_fsh_mask_form_line,
	cftbl_fsh_mask_form_lines
};


/* DEVICE TYPE STRUCTURE
 *
 */
typedef struct {
	int	db_na	: 19,	/* not applicable			*/
		db_fsh	: 1,	/* file system handler			*/
		db_civa	: 1,	/* create interrupt vector array	*/
		db_cbcmn: 1,	/* create block/char major number	*/
		db_cila	: 1,	/* create interrupt levle array		*/
		db_nu	: 1,	/* not used				*/
		db_aoootd:1,	/* allow only one of these devices	*/
		db_scf	: 1,	/* suppress count field			*/
		db_siv	: 1,	/* suppress interrupt vector		*/
		db_rd	: 1,	/* required device			*/
		db_bd	: 1,	/* block device				*/
		db_cd	: 1,	/* character device			*/
		db_idd	: 1,	/* interrupt driven device		*/
		db_svma	: 1;	/* single vector/multiple addresses	*/
} DRVTBIT;

/* Output Driver Type and Configuration Declarations
 *
 */
void write_type(file, type, cfntp)
FILE *file;
int type;
TABLE *cfntp;
{
	static BFORMAT type_format[] = {
		{ T_DRVCIVA,	004000	},
		{ T_DRVCCBMN,	002000	},
		{ T_DRVCILA,	001000	},
		{ T_DRVONE,	000200	},
		{ T_DRVSCF,	000100	},
		{ T_DRVSIV,	000040	},
		{ T_DRVREQ,	000020	},
		{ T_DRVBLOCK,	000010	},
		{ T_DRVCHAR,	000004	},
		{ T_DRVIDD,	000002	},
		{ T_DRVSVMA,	000001	},
		{ 0,		0	}
	};

	fprintf(file, "%s\n", keyword[T_DRVTYPE - T_DESCFILE]);
	write_bit_lines(file, type_format, type);

	fprintf(file, "%s\n", keyword[T_DRVDATA - T_DESCFILE]);
	write_cfntbl(file, cfntp);
}


/* DEVICE TYPE FORM
 *
 */
#define LENGTH_OF_DEVICE_TYPE_FIELD LENGTH(S_DRVCCBMN)

/* Create Vector Array Field
 *
 */
#define display_type_vector_array_field display_bit_field
#define change_type_vector_array_field change_bit_field
#define open_type_vector_array_field flash

static BIT_FIELD type_vector_array_field = {
	5,
	LENGTH_OF_DEVICE_TYPE_FIELD,
	display_type_vector_array_field,
	change_type_vector_array_field,
	open_type_vector_array_field,
	T_DRVCIVA,
	004000
};

/* Create Character Or Block Major Numbers Field
 *
 */
#define display_type_major_nos_field display_bit_field
#define change_type_major_nos_field change_bit_field
#define open_type_major_nos_field flash

static BIT_FIELD type_major_nos_field = {
	5,
	LENGTH_OF_DEVICE_TYPE_FIELD,
	display_type_major_nos_field,
	change_type_major_nos_field,
	open_type_major_nos_field,
	T_DRVCCBMN,
	002000
};

/* Create Interrupt Level Array Field
 *
 */
#define display_type_level_array_field display_bit_field
#define change_type_level_array_field change_bit_field
#define open_type_level_array_field flash

static BIT_FIELD type_level_array_field = {
	5,
	LENGTH_OF_DEVICE_TYPE_FIELD,
	display_type_level_array_field,
	change_type_level_array_field,
	open_type_level_array_field,
	T_DRVCILA,
	001000
};

/* Allow Only One Of These Devices Field
 *
 */
#define display_type_only_one_field display_bit_field
#define change_type_only_one_field change_bit_field
#define open_type_only_one_field flash

static BIT_FIELD type_only_one_field = {
	5,
	LENGTH_OF_DEVICE_TYPE_FIELD,
	display_type_only_one_field,
	change_type_only_one_field,
	open_type_only_one_field,
	T_DRVONE,
	000200
};

/* Do Not Create A Count Field
 *
 */
#define display_type_no_count_field display_bit_field
#define change_type_no_count_field change_bit_field
#define open_type_no_count_field flash

static BIT_FIELD type_no_count_field = {
	5,
	LENGTH_OF_DEVICE_TYPE_FIELD,
	display_type_no_count_field,
	change_type_no_count_field,
	open_type_no_count_field,
	T_DRVSCF,
	000100
};

/* Do Not Create An Interrupt Vector Field
 *
 */
#define display_type_no_vector_field display_bit_field
#define change_type_no_vector_field change_bit_field
#define open_type_no_vector_field flash

static BIT_FIELD type_no_vector_field = {
	5,
	LENGTH_OF_DEVICE_TYPE_FIELD,
	display_type_no_vector_field,
	change_type_no_vector_field,
	open_type_no_vector_field,
	T_DRVSIV,
	000040
};

/* Required Device Field
 *
 */
#define display_type_required_field display_bit_field
#define change_type_required_field change_bit_field
#define open_type_required_field flash

static BIT_FIELD type_required_field = {
	5,
	LENGTH_OF_DEVICE_TYPE_FIELD,
	display_type_required_field,
	change_type_required_field,
	open_type_required_field,
	T_DRVREQ,
	000020
};

/* Block Device Field
 *
 */
#define display_type_block_field display_bit_field
#define change_type_block_field change_bit_field
#define open_type_block_field flash

static BIT_FIELD type_block_field = {
	5,
	LENGTH_OF_DEVICE_TYPE_FIELD,
	display_type_block_field,
	change_type_block_field,
	open_type_block_field,
	T_DRVBLOCK,
	000010
};

/* Character Device Field
 *
 */
#define display_type_char_field display_bit_field
#define change_type_char_field change_bit_field
#define open_type_char_field flash

static BIT_FIELD type_char_field = {
	5,
	LENGTH_OF_DEVICE_TYPE_FIELD,
	display_type_char_field,
	change_type_char_field,
	open_type_char_field,
	T_DRVCHAR,
	000004
};

/* Interrupt Driven Field
 *
 */
#define display_type_interrupt_field display_bit_field
#define change_type_interrupt_field change_bit_field
#define open_type_interrupt_field flash

static BIT_FIELD type_interrupt_field = {
	5,
	LENGTH_OF_DEVICE_TYPE_FIELD,
	display_type_interrupt_field,
	change_type_interrupt_field,
	open_type_interrupt_field,
	T_DRVIDD,
	000002
};

/* Allow For A Single Vector Definition With Multiple Addresses Field
 *
 */
#define display_type_multiple_addresses_field display_bit_field
#define change_type_multiple_addresses_field change_bit_field
#define open_type_multiple_addresses_field flash

static BIT_FIELD type_multiple_addresses_field = {
	5,
	LENGTH_OF_DEVICE_TYPE_FIELD,
	display_type_multiple_addresses_field,
	change_type_multiple_addresses_field,
	open_type_multiple_addresses_field,
	T_DRVSVMA,
	000001
};

/* Define Form
 *
 */
#define quit_type_form quit_sub_context

#define add_type_form_line flash
#define del_type_form_line flash
#define dup_type_form_line flash

static FIELD *type_vector_array_fields[] = {
	ISFIELD(type_vector_array_field)
};

static FIELD *type_major_nos_fields[] = {
	ISFIELD(type_major_nos_field)
};

static FIELD *type_level_array_fields[] = {
	ISFIELD(type_level_array_field)
};

static FIELD *type_only_one_fields[] = {
	ISFIELD(type_only_one_field)
};

static FIELD *type_no_count_fields[] = {
	ISFIELD(type_no_count_field)
};

static FIELD *type_no_vector_fields[] = {
	ISFIELD(type_no_vector_field)
};

static FIELD *type_required_fields[] = {
	ISFIELD(type_required_field)
};

static FIELD *type_block_fields[] = {
	ISFIELD(type_block_field)
};

static FIELD *type_char_fields[] = {
	ISFIELD(type_char_field)
};

static FIELD *type_interrupt_fields[] = {
	ISFIELD(type_interrupt_field)
};

static FIELD *type_multiple_addresses_fields[] = {
	ISFIELD(type_multiple_addresses_field)
};

static LINE type_form_lines[] = {
	{  0, 1, type_vector_array_fields },
	{  1, 1, type_major_nos_fields },
	{  2, 1, type_level_array_fields },
	{  3, 1, type_only_one_fields },
	{  4, 1, type_no_count_fields },
	{  5, 1, type_no_vector_fields },
	{  6, 1, type_required_fields },
	{  7, 1, type_block_fields },
	{  8, 1, type_char_fields },
	{  9, 1, type_interrupt_fields },
	{ 10, 1, type_multiple_addresses_fields }
};

static FORM type_form = {
	NULL,
	NULL,
	quit_type_form,
	11,
	add_type_form_line,
	del_type_form_line,
	dup_type_form_line,
	type_form_lines
};


/* CONFIGURATION FILE TABLE DEVICE TYPE FORM
 *
 */

/* Create Vector Array Field
 *
 */
#define display_cftbl_type_vector_array_field display_bit_field
#define change_cftbl_type_vector_array_field flash
#define open_cftbl_type_vector_array_field flash

static BIT_FIELD cftbl_type_vector_array_field = {
	5,
	LENGTH_OF_DEVICE_TYPE_FIELD,
	display_cftbl_type_vector_array_field,
	change_cftbl_type_vector_array_field,
	open_cftbl_type_vector_array_field,
	T_DRVCIVA,
	004000
};

/* Create Character Or Block Major Numbers Field
 *
 */
#define display_cftbl_type_major_nos_field display_bit_field
#define change_cftbl_type_major_nos_field flash
#define open_cftbl_type_major_nos_field flash

static BIT_FIELD cftbl_type_major_nos_field = {
	5,
	LENGTH_OF_DEVICE_TYPE_FIELD,
	display_cftbl_type_major_nos_field,
	change_cftbl_type_major_nos_field,
	open_cftbl_type_major_nos_field,
	T_DRVCCBMN,
	002000
};

/* Create Interrupt Level Array Field
 *
 */
#define display_cftbl_type_level_array_field display_bit_field
#define change_cftbl_type_level_array_field flash
#define open_cftbl_type_level_array_field flash

static BIT_FIELD cftbl_type_level_array_field = {
	5,
	LENGTH_OF_DEVICE_TYPE_FIELD,
	display_cftbl_type_level_array_field,
	change_cftbl_type_level_array_field,
	open_cftbl_type_level_array_field,
	T_DRVCILA,
	001000
};

/* Allow Only One Of These Devices Field
 *
 */
#define display_cftbl_type_only_one_field display_bit_field
#define change_cftbl_type_only_one_field flash
#define open_cftbl_type_only_one_field flash

static BIT_FIELD cftbl_type_only_one_field = {
	5,
	LENGTH_OF_DEVICE_TYPE_FIELD,
	display_cftbl_type_only_one_field,
	change_cftbl_type_only_one_field,
	open_cftbl_type_only_one_field,
	T_DRVONE,
	000200
};

/* Do Not Create A Count Field
 *
 */
#define display_cftbl_type_no_count_field display_bit_field
#define change_cftbl_type_no_count_field flash
#define open_cftbl_type_no_count_field flash

static BIT_FIELD cftbl_type_no_count_field = {
	5,
	LENGTH_OF_DEVICE_TYPE_FIELD,
	display_cftbl_type_no_count_field,
	change_cftbl_type_no_count_field,
	open_cftbl_type_no_count_field,
	T_DRVSCF,
	000100
};

/* Do Not Create An Interrupt Vector Field
 *
 */
#define display_cftbl_type_no_vector_field display_bit_field
#define change_cftbl_type_no_vector_field flash
#define open_cftbl_type_no_vector_field flash

static BIT_FIELD cftbl_type_no_vector_field = {
	5,
	LENGTH_OF_DEVICE_TYPE_FIELD,
	display_cftbl_type_no_vector_field,
	change_cftbl_type_no_vector_field,
	open_cftbl_type_no_vector_field,
	T_DRVSIV,
	000040
};

/* Required Device Field
 *
 */
#define display_cftbl_type_required_field display_bit_field
#define change_cftbl_type_required_field flash
#define open_cftbl_type_required_field flash

static BIT_FIELD cftbl_type_required_field = {
	5,
	LENGTH_OF_DEVICE_TYPE_FIELD,
	display_cftbl_type_required_field,
	change_cftbl_type_required_field,
	open_cftbl_type_required_field,
	T_DRVREQ,
	000020
};

/* Block Device Field
 *
 */
#define display_cftbl_type_block_field display_bit_field
#define change_cftbl_type_block_field flash
#define open_cftbl_type_block_field flash

static BIT_FIELD cftbl_type_block_field = {
	5,
	LENGTH_OF_DEVICE_TYPE_FIELD,
	display_cftbl_type_block_field,
	change_cftbl_type_block_field,
	open_cftbl_type_block_field,
	T_DRVBLOCK,
	000010
};

/* Character Device Field
 *
 */
#define display_cftbl_type_char_field display_bit_field
#define change_cftbl_type_char_field flash
#define open_cftbl_type_char_field flash

static BIT_FIELD cftbl_type_char_field = {
	5,
	LENGTH_OF_DEVICE_TYPE_FIELD,
	display_cftbl_type_char_field,
	change_cftbl_type_char_field,
	open_cftbl_type_char_field,
	T_DRVCHAR,
	000004
};

/* Interrupt Driven Field
 *
 */
#define display_cftbl_type_interrupt_field display_bit_field
#define change_cftbl_type_interrupt_field flash
#define open_cftbl_type_interrupt_field flash

static BIT_FIELD cftbl_type_interrupt_field = {
	5,
	LENGTH_OF_DEVICE_TYPE_FIELD,
	display_cftbl_type_interrupt_field,
	change_cftbl_type_interrupt_field,
	open_cftbl_type_interrupt_field,
	T_DRVIDD,
	000002
};

/* Allow For A Single Vector Definition With Multiple Addresses Field
 *
 */
#define display_cftbl_type_multiple_addresses_field display_bit_field
#define change_cftbl_type_multiple_addresses_field flash
#define open_cftbl_type_multiple_addresses_field flash

static BIT_FIELD cftbl_type_multiple_addresses_field = {
	5,
	LENGTH_OF_DEVICE_TYPE_FIELD,
	display_cftbl_type_multiple_addresses_field,
	change_cftbl_type_multiple_addresses_field,
	open_cftbl_type_multiple_addresses_field,
	T_DRVSVMA,
	000001
};

/* Define Form
 *
 */
#define quit_cftbl_type_form quit_sub_context

#define add_cftbl_type_form_line flash
#define del_cftbl_type_form_line flash
#define dup_cftbl_type_form_line flash

static FIELD *cftbl_type_vector_array_fields[] = {
	ISFIELD(cftbl_type_vector_array_field)
};

static FIELD *cftbl_type_major_nos_fields[] = {
	ISFIELD(cftbl_type_major_nos_field)
};

static FIELD *cftbl_type_level_array_fields[] = {
	ISFIELD(cftbl_type_level_array_field)
};

static FIELD *cftbl_type_only_one_fields[] = {
	ISFIELD(cftbl_type_only_one_field)
};

static FIELD *cftbl_type_no_count_fields[] = {
	ISFIELD(cftbl_type_no_count_field)
};

static FIELD *cftbl_type_no_vector_fields[] = {
	ISFIELD(cftbl_type_no_vector_field)
};

static FIELD *cftbl_type_required_fields[] = {
	ISFIELD(cftbl_type_required_field)
};

static FIELD *cftbl_type_block_fields[] = {
	ISFIELD(cftbl_type_block_field)
};

static FIELD *cftbl_type_char_fields[] = {
	ISFIELD(cftbl_type_char_field)
};

static FIELD *cftbl_type_interrupt_fields[] = {
	ISFIELD(cftbl_type_interrupt_field)
};

static FIELD *cftbl_type_multiple_addresses_fields[] = {
	ISFIELD(cftbl_type_multiple_addresses_field)
};

static LINE cftbl_type_form_lines[] = {
	{  0, 1, cftbl_type_vector_array_fields },
	{  1, 1, cftbl_type_major_nos_fields },
	{  2, 1, cftbl_type_level_array_fields },
	{  3, 1, cftbl_type_only_one_fields },
	{  4, 1, cftbl_type_no_count_fields },
	{  5, 1, cftbl_type_no_vector_fields },
	{  6, 1, cftbl_type_required_fields },
	{  7, 1, cftbl_type_block_fields },
	{  8, 1, cftbl_type_char_fields },
	{  9, 1, cftbl_type_interrupt_fields },
	{ 10, 1, cftbl_type_multiple_addresses_fields }
};

static FORM cftbl_type_form = {
	NULL,
	NULL,
	quit_cftbl_type_form,
	11,
	add_cftbl_type_form_line,
	del_cftbl_type_form_line,
	dup_cftbl_type_form_line,
	cftbl_type_form_lines
};


/* DRIVER STRUCTURE
 *
 */

/* Device Mask Union
 *
 */
typedef union {
	int	dm_int;		/* the whole nine yards			*/
	DRVBCDM	dm_bcdm;	/* block/char device mask field		*/
	DRVFSHM	dm_fshm;	/* filesystem handler mask field	*/
} DRVMASK;

/* Driver Type Union
 *
 */
typedef union {
	int	dt_int;		/* all bits treated together		*/
	DRVTBIT	dt_bit;		/* bit fields				*/
} DRVTYPE;

/* Driver Structure
 *
 */
typedef struct {
	char	*drv_name;		/* device name			*/
	char	*drv_pref;		/* handler prefix		*/
	int	drv_mbrl;		/* maximum bus request level	*/
	int	drv_mdpc;		/* maximum devs per controller	*/
	int	drv_ivs;		/* interrupt vector size	*/
	int	drv_mbdn;		/* major block device number	*/
	int	drv_mcdn;		/* major char device number	*/
	int	drv_prs;		/* page register size		*/
	DRVMASK	drv_mask;		/* device mask bits		*/
	DRVTYPE	drv_type;		/* device type bits		*/
	TABLE	*drv_data;		/* structure name table		*/
} DRIVER;

#define drv_mint	drv_mask.dm_int

#define drv_sh		drv_mask.dm_bcdm.dd_sh
#define drv_sbch	drv_mask.dm_bcdm.dd_sbch
#define drv_ts		drv_mask.dm_bcdm.dd_ts
#define drv_ih		drv_mask.dm_bcdm.dd_ih
#define drv_pfh		drv_mask.dm_bcdm.dd_pfh
#define drv_oh		drv_mask.dm_bcdm.dd_oh
#define drv_ch		drv_mask.dm_bcdm.dd_ch
#define drv_rh		drv_mask.dm_bcdm.dd_rh
#define drv_wh		drv_mask.dm_bcdm.dd_wh
#define drv_ioh		drv_mask.dm_bcdm.dd_ioh

#define drv_ioc		drv_mask.dm_fshm.dh_ioc
#define drv_info	drv_mask.dm_fshm.dh_info
#define drv_ctrl	drv_mask.dm_fshm.dh_ctrl
#define drv_ntfy	drv_mask.dm_fshm.dh_ntfy
#define drv_satr	drv_mask.dm_fshm.dh_satr
#define drv_rmap	drv_mask.dm_fshm.dh_rmap
#define drv_fmap	drv_mask.dm_fshm.dh_fmap
#define drv_amap	drv_mask.dm_fshm.dh_amap
#define drv_gdnt	drv_mask.dm_fshm.dh_gdnt
#define drv_acce	drv_mask.dm_fshm.dh_acce
#define drv_stat	drv_mask.dm_fshm.dh_stat
#define drv_upda	drv_mask.dm_fshm.dh_upda
#define drv_cino	drv_mask.dm_fshm.dh_cino
#define drv_oino	drv_mask.dm_fshm.dh_oino
#define drv_gino	drv_mask.dm_fshm.dh_gino
#define drv_umnt	drv_mask.dm_fshm.dh_umnt
#define drv_mnt		drv_mask.dm_fshm.dh_mnt
#define drv_nami	drv_mask.dm_fshm.dh_nami
#define drv_sfil	drv_mask.dm_fshm.dh_sfil
#define drv_itrn	drv_mask.dm_fshm.dh_itrn
#define drv_init	drv_mask.dm_fshm.dh_init
#define drv_wino	drv_mask.dm_fshm.dh_wino
#define drv_rino	drv_mask.dm_fshm.dh_rino
#define drv_uino	drv_mask.dm_fshm.dh_uino
#define drv_ird		drv_mask.dm_fshm.dh_ird
#define drv_iput	drv_mask.dm_fshm.dh_iput

#define drv_tint	drv_type.dt_int

#define drv_fsh		drv_type.dt_bit.db_fsh
#define drv_civa	drv_type.dt_bit.db_civa
#define drv_cbcmn	drv_type.dt_bit.db_cbcmn
#define drv_cila	drv_type.dt_bit.db_cila
#define drv_aoootd	drv_type.dt_bit.db_aoootd
#define drv_scf		drv_type.dt_bit.db_scf
#define drv_siv		drv_type.dt_bit.db_siv
#define drv_rd		drv_type.dt_bit.db_rd
#define drv_bd		drv_type.dt_bit.db_bd
#define drv_cd		drv_type.dt_bit.db_cd
#define drv_idd		drv_type.dt_bit.db_idd
#define drv_svma	drv_type.dt_bit.db_svma

static DRIVER driverbuf[DRIVER_COUNT];

STRUCT_LIST driver_slist = {
	sizeof(DRIVER),
	0,
	0,
	DRIVER_COUNT,
	STRUCTP(driverbuf)
};

#define alloc_driver() ((DRIVER *)alloc_struct(&driver_slist))

DRIVER sdriver;				/* sample driver structure	*/


/* Create New Driver
 *
 */
char *new_driver(name, pref, ivs, mbdn, mcdn, mbrl, mdpc, prs, mint, tint, data)
char *name, *pref;
int ivs, mbdn, mcdn, mbrl, mdpc, prs, mint, tint;
TABLE *data;
{
	register DRIVER *drvp;

	drvp = alloc_driver();

	drvp->drv_name = name;
	drvp->drv_pref = pref;
	drvp->drv_ivs  = ivs;
	drvp->drv_mbdn = mbdn;
	drvp->drv_mcdn = mcdn;
	drvp->drv_mbrl = mbrl;
	drvp->drv_mdpc = mdpc;
	drvp->drv_prs  = prs;
	drvp->drv_mint = mint;
	drvp->drv_tint = tint;
	drvp->drv_data = data;

	return(CP(drvp));
}

/* Create New Device Description
 *
 */
DRIVER *create_driver(name, pref)
char *name, *pref;
{
	register DRIVER *drvp;

	drvp = alloc_driver();

	drvp->drv_name = create_string(name);
	drvp->drv_pref = create_string(pref);
	drvp->drv_data = alloc_configname_table();

	return(drvp);
}

/* Create New File System Handler Description
 *
 */
DRIVER *create_filesystem_handler(name, pref)
char *name, *pref;
{
	register DRIVER *drvp;

	drvp = create_driver(name, pref);
	drvp->drv_fsh = 1;
	return(drvp);
}

/* Duplicate Driver
 *
 */
DRIVER *dup_driver(drvp)
register DRIVER *drvp;
{
	register DRIVER *ndrvp;

	ndrvp = alloc_driver();

	ndrvp->drv_name = create_string(drvp->drv_name);
	ndrvp->drv_pref = create_string(drvp->drv_pref);
	ndrvp->drv_mbrl = drvp->drv_mbrl;
	ndrvp->drv_mdpc = drvp->drv_mdpc;
	ndrvp->drv_ivs  = drvp->drv_ivs;
	ndrvp->drv_mbdn = drvp->drv_mbdn;
	ndrvp->drv_mcdn = drvp->drv_mcdn;
	ndrvp->drv_prs  = drvp->drv_prs;
	ndrvp->drv_mint = drvp->drv_mint;
	ndrvp->drv_tint = drvp->drv_tint;
	ndrvp->drv_data = (TABLE *)dup_configname_table(drvp->drv_data);

	return(ndrvp);
}

/* De-Allocate Device Description
 *
 */
void free_driver(drvp)
register DRIVER *drvp;
{
	free_string(drvp->drv_name);
	free_string(drvp->drv_pref);
	free_configname_table(drvp->drv_data);
	free_struct(&driver_slist, drvp);
}

/* Output Block/Character Device Driver Information
 *
 */
void write_driver(file, drvp)
FILE *file;
register DRIVER *drvp;
{
	static FORMAT driver_format[] = {
		{ FMT_STRING,	T_DRVNAME, CP(&sdriver.drv_name) },
		{ FMT_STRING,	T_DRVHDLR, CP(&sdriver.drv_pref) },
		{ FMT_DECIMAL,	T_DRVIVS,  CP(&sdriver.drv_ivs)	 },
		{ FMT_DECIMAL,	T_DRVMBDN, CP(&sdriver.drv_mbdn) },
		{ FMT_DECIMAL,	T_DRVMCDN, CP(&sdriver.drv_mcdn) },
		{ FMT_DECIMAL,	T_DRVMBRL, CP(&sdriver.drv_mbrl) },
		{ FMT_DECIMAL,	T_DRVMDPC, CP(&sdriver.drv_mdpc) },
		{ FMT_DECIMAL,	T_DRVPRS,  CP(&sdriver.drv_prs)	 },
		{ FMT_PROMPT,	T_BCMASK,  NULL			 },
		{ FMT_END,	0,	   NULL			 }
	};

	static BFORMAT driverbformat[] = {
		{ T_DRVSTR,	001000	},
		{ T_DRVSBCH,	000400	},
		{ T_DRVTTY,	000200	},
		{ T_DRVINIT,	000100	},
		{ T_DRVPFAIL,	000040	},
		{ T_DRVOPEN,	000020	},
		{ T_DRVCLOSE,	000010	},
		{ T_DRVREAD,	000004	},
		{ T_DRVWRITE,	000002	},
		{ T_DRVIOCTL,	000001	},
		{ 0,		0	}
	};

	write_format(file, driver_format, drvp, &sdriver);
	write_bit_lines(file, driverbformat, drvp->drv_mint);
	write_type(file, drvp->drv_tint, drvp->drv_data);
}

/* Output Filesystem Handler Driver
 *
 */
void write_filesystem_handler(file, drvp)
FILE *file;
register DRIVER *drvp;
{
	static FORMAT fshformat[] = {
		{ FMT_STRING,	T_FSHNM,   CP(&sdriver.drv_name) },
		{ FMT_STRING,	T_DRVHDLR, CP(&sdriver.drv_pref) },
		{ FMT_DECIMAL,	T_DRVIVS,  CP(&sdriver.drv_ivs)	 },
		{ FMT_DECIMAL,	T_DRVMBDN, CP(&sdriver.drv_mbdn) },
		{ FMT_DECIMAL,	T_DRVMCDN, CP(&sdriver.drv_mcdn) },
		{ FMT_DECIMAL,	T_DRVMBRL, CP(&sdriver.drv_mbrl) },
		{ FMT_DECIMAL,	T_DRVMDPC, CP(&sdriver.drv_mdpc) },
		{ FMT_DECIMAL,	T_DRVPRS,  CP(&sdriver.drv_prs)	 },
		{ FMT_PROMPT,	T_FSHMASK, NULL			 },
		{ FMT_END,	0,	   NULL			 }
	};

	static BFORMAT fshbformat[] = {
		{ T_FSHIOCTL,	0400000000 },
		{ T_FSHINFO,	0200000000 },
		{ T_FSHCTRL,	0100000000 },
		{ T_FSHNOTIFY,	0040000000 },
		{ T_FSHSATTR,	0020000000 },
		{ T_FSHRDMAP,	0010000000 },
		{ T_FSHFRMAP,	0004000000 },
		{ T_FSHALMAP,	0002000000 },
		{ T_FSHGDENTS,	0001000000 },
		{ T_FSHACCESS,	0000400000 },
		{ T_FSHSTATUS,	0000200000 },
		{ T_FSHUPDATE,	0000100000 },
		{ T_FSHCLINOD,	0000040000 },
		{ T_FSHOPINOD,	0000020000 },
		{ T_FSHGTINOD,	0000010000 },
		{ T_FSHUMOUNT,	0000004000 },
		{ T_FSHMOUNT,	0000002000 },
		{ T_FSHNAMEI,	0000001000 },
		{ T_FSHFSTAT,	0000000400 },
		{ T_FSHITRUNC,	0000000200 },
		{ T_FSHINIT,	0000000100 },
		{ T_FSHWRINOD,	0000000040 },
		{ T_FSHRDINOD,	0000000020 },
		{ T_FSHUPINOD,	0000000010 },
		{ T_FSHIREAD,	0000000002 },
		{ T_FSHIPUT,	0000000001 },
		{ 0,		0	   }
	};

	extern void write_format(), write_bit_lines();
	register int i;

	write_format(file, fshformat, drvp, &sdriver);
	write_bit_lines(file, fshbformat, drvp->drv_mint);
	write_type(file, drvp->drv_tint, drvp->drv_data);
}

/* Output Master(4), Part 1
 *
 */
void out_driver(file, drvp)
FILE *file;
register DRIVER *drvp;
{
	void out_cfntbl();

	fprintf(file, "%-12s%-5d%-12.10o%-8.6o%-6s%-5d%-5d%-5d%-5d%-5d",
		drvp->drv_name, drvp->drv_ivs,  drvp->drv_mint,
		drvp->drv_tint, drvp->drv_pref, drvp->drv_prs,
		drvp->drv_mbdn, drvp->drv_mcdn, drvp->drv_mdpc,
		drvp->drv_mbrl);

	out_cfntbl(file, drvp->drv_data);
}


/* DRIVER FORM
 *
 */

/* Name Prompt
 *
 */
#define display_driver_name_prompt display_token_field
#define change_driver_name_prompt flash
#define open_driver_name_prompt flash

static TOKEN_FIELD driver_name_prompt = {
	RIGHT(S_DRVNAME),
	LENGTH(S_DRVNAME),
	display_driver_name_prompt,
	change_driver_name_prompt,
	open_driver_name_prompt,
	T_DRVNAME
};

/* Name Field
 *
 */
#define display_driver_name_field display_text_field
#define change_driver_name_field change_text_field
#define open_driver_name_field flash

static TEXT_FIELD driver_name_field = {
	40,
	10,
	display_driver_name_field,
	change_driver_name_field,
	open_driver_name_field,
	&sdriver.drv_name
};

/* Prefix Prompt
 *
 */
#define display_driver_prefix_prompt display_token_field
#define change_driver_prefix_prompt flash
#define open_driver_prefix_prompt flash

static TOKEN_FIELD driver_prefix_prompt = {
	RIGHT(S_DRVHDLR),
	LENGTH(S_DRVHDLR),
	display_driver_prefix_prompt,
	change_driver_prefix_prompt,
	open_driver_prefix_prompt,
	T_DRVHDLR
};

/* Prefix Field
 *
 */
#define display_driver_prefix_field display_text_field
#define change_driver_prefix_field change_text_field
#define open_driver_prefix_field flash

static TEXT_FIELD driver_prefix_field = {
	40,
	4,
	display_driver_prefix_field,
	change_driver_prefix_field,
	open_driver_prefix_field,
	&sdriver.drv_pref
};

/* Interrupt Vectors Size Prompt
 *
 */
#define display_driver_vector_size_prompt display_token_field
#define change_driver_vector_size_prompt flash
#define open_driver_vector_size_prompt flash

static TOKEN_FIELD driver_vector_size_prompt = {
	RIGHT(S_DRVIVS),
	LENGTH(S_DRVIVS),
	display_driver_vector_size_prompt,
	change_driver_vector_size_prompt,
	open_driver_vector_size_prompt,
	T_DRVIVS
};

/* Interrupt Vectors Size Field
 *
 */
#define display_driver_vector_size_field display_numeric_field
#define change_driver_vector_size_field change_numeric_field
#define open_driver_vector_size_field flash

static NUMERIC_FIELD driver_vector_size_field = {
	40,
	3,
	display_driver_vector_size_field,
	change_driver_vector_size_field,
	open_driver_vector_size_field,
	&sdriver.drv_ivs,
	DECIMAL_FIELD
};

/* Maximum Bus Request Level Prompt
 *
 */
#define display_driver_max_bus_request_prompt display_token_field
#define change_driver_max_bus_request_prompt flash
#define open_driver_max_bus_request_prompt flash

static TOKEN_FIELD driver_max_bus_request_prompt = {
	RIGHT(S_DRVMBRL),
	LENGTH(S_DRVMBRL),
	display_driver_max_bus_request_prompt,
	change_driver_max_bus_request_prompt,
	open_driver_max_bus_request_prompt,
	T_DRVMBRL
};

/* Maximum Bus Request Level Field
 *
 */
#define display_driver_max_bus_request_field display_numeric_field
#define change_driver_max_bus_request_field change_numeric_field
#define open_driver_max_bus_request_field flash

static NUMERIC_FIELD driver_max_bus_request_field = {
	40,
	3,
	display_driver_max_bus_request_field,
	change_driver_max_bus_request_field,
	open_driver_max_bus_request_field,
	&sdriver.drv_mbrl,
	DECIMAL_FIELD
};

/* Page Registers Size Prompt
 *
 */
#define display_driver_page_reg_size_prompt display_token_field
#define change_driver_page_reg_size_prompt flash
#define open_driver_page_reg_size_prompt flash

static TOKEN_FIELD driver_page_reg_size_prompt = {
	RIGHT(S_DRVPRS),
	LENGTH(S_DRVPRS),
	display_driver_page_reg_size_prompt,
	change_driver_page_reg_size_prompt,
	open_driver_page_reg_size_prompt,
	T_DRVPRS
};

/* Page Registers Size Field
 *
 */
#define display_driver_page_reg_size_field display_numeric_field
#define change_driver_page_reg_size_field change_numeric_field
#define open_driver_page_reg_size_field flash

static NUMERIC_FIELD driver_page_reg_size_field = {
	40,
	3,
	display_driver_page_reg_size_field,
	change_driver_page_reg_size_field,
	open_driver_page_reg_size_field,
	&sdriver.drv_prs,
	DECIMAL_FIELD
};

/* Maximum Devices Per Controller Prompt
 *
 */
#define display_driver_max_devices_prompt display_token_field
#define change_driver_max_devices_prompt flash
#define open_driver_max_devices_prompt flash

static TOKEN_FIELD driver_max_devices_prompt = {
	RIGHT(S_DRVMDPC),
	LENGTH(S_DRVMDPC),
	display_driver_max_devices_prompt,
	change_driver_max_devices_prompt,
	open_driver_max_devices_prompt,
	T_DRVMDPC
};

/* Maximum Devices Per Controller Field
 *
 */
#define display_driver_max_devices_field display_numeric_field
#define change_driver_max_devices_field change_numeric_field
#define open_driver_max_devices_field flash

static NUMERIC_FIELD driver_max_devices_field = {
	40,
	3,
	display_driver_max_devices_field,
	change_driver_max_devices_field,
	open_driver_max_devices_field,
	&sdriver.drv_mdpc,
	DECIMAL_FIELD
};

/* Major Block Device Number Prompt
 *
 */
#define display_driver_maj_block_no_prompt display_token_field
#define change_driver_maj_block_no_prompt flash
#define open_driver_maj_block_no_prompt flash

static TOKEN_FIELD driver_maj_block_no_prompt = {
	RIGHT(S_DRVMBDN),
	LENGTH(S_DRVMBDN),
	display_driver_maj_block_no_prompt,
	change_driver_maj_block_no_prompt,
	open_driver_maj_block_no_prompt,
	T_DRVMBDN
};

/* Major Block Device Number Field
 *
 */
#define display_driver_maj_block_no_field display_numeric_field
#define change_driver_maj_block_no_field change_numeric_field
#define open_driver_maj_block_no_field flash

static NUMERIC_FIELD driver_maj_block_no_field = {
	40,
	3,
	display_driver_maj_block_no_field,
	change_driver_maj_block_no_field,
	open_driver_maj_block_no_field,
	&sdriver.drv_mbdn,
	DECIMAL_FIELD
};

/* Major Character Device Number Prompt
 *
 */
#define display_driver_maj_char_no_prompt display_token_field
#define change_driver_maj_char_no_prompt flash
#define open_driver_maj_char_no_prompt flash

static TOKEN_FIELD driver_maj_char_no_prompt = {
	RIGHT(S_DRVMCDN),
	LENGTH(S_DRVMCDN),
	display_driver_maj_char_no_prompt,
	change_driver_maj_char_no_prompt,
	open_driver_maj_char_no_prompt,
	T_DRVMCDN
};

/* Major Character Device Number Field
 *
 */
#define display_driver_maj_char_no_field display_numeric_field
#define change_driver_maj_char_no_field change_numeric_field
#define open_driver_maj_char_no_field flash

static NUMERIC_FIELD driver_maj_char_no_field = {
	40,
	3,
	display_driver_maj_char_no_field,
	change_driver_maj_char_no_field,
	open_driver_maj_char_no_field,
	&sdriver.drv_mcdn,
	DECIMAL_FIELD
};

/* Handler Form Prompt
 *
 */
#define HANDLER_FORM_PROMPT "Block And/Or Character Device Handlers List"

#define display_driver_handler_form_prompt display_prompt_field
#define change_driver_handler_form_prompt flash

void open_driver_handler_form_prompt()
{
	create_context(&((DRIVER *)acp->c_tbl)->drv_mint, &mask_form);
}

static PROMPT_FIELD driver_handler_form_prompt = {
	CENTER(HANDLER_FORM_PROMPT),
	LENGTH(HANDLER_FORM_PROMPT),
	display_driver_handler_form_prompt,
	change_driver_handler_form_prompt,
	open_driver_handler_form_prompt,
	HANDLER_FORM_PROMPT
};

/* Device Type Form Prompt
 *
 */
#define TYPE_FORM_PROMPT "Driver Miscellaneous Information Selection List"

#define display_driver_type_form_prompt display_prompt_field
#define change_driver_type_form_prompt flash

void open_driver_type_form_prompt()
{
	create_context(&((DRIVER *)acp->c_tbl)->drv_tint, &type_form);
}

static PROMPT_FIELD driver_type_form_prompt = {
	CENTER(TYPE_FORM_PROMPT),
	LENGTH(TYPE_FORM_PROMPT),
	display_driver_type_form_prompt,
	change_driver_type_form_prompt,
	open_driver_type_form_prompt,
	TYPE_FORM_PROMPT
};

/* Configuration Names Form Prompt
 *
 */
#define CONFIG_NAMES_FORM_PROMPT "Configuration Table Declarations List"

#define display_driver_config_names_form_prompt display_prompt_field
#define change_driver_config_names_form_prompt flash

void open_driver_config_names_form_prompt()
{
	create_context(((DRIVER *)acp->c_tbl)->drv_data, &cfname_form);
}

static PROMPT_FIELD driver_config_names_form_prompt = {
	CENTER(CONFIG_NAMES_FORM_PROMPT),
	LENGTH(CONFIG_NAMES_FORM_PROMPT),
	display_driver_config_names_form_prompt,
	change_driver_config_names_form_prompt,
	open_driver_config_names_form_prompt,
	CONFIG_NAMES_FORM_PROMPT
};

/* Define Form
 *
 */
#define quit_driver_form quit_sub_context

#define add_driver_form_line flash
#define del_driver_form_line flash
#define dup_driver_form_line flash

static FIELD *driver_name_fields[] = {
	ISFIELD(driver_name_prompt),
	ISFIELD(driver_name_field)
};

static FIELD *driver_prefix_fields[] = {
	ISFIELD(driver_prefix_prompt),
	ISFIELD(driver_prefix_field)
};

static FIELD *driver_vector_size_fields[] = {
	ISFIELD(driver_vector_size_prompt),
	ISFIELD(driver_vector_size_field)
};

static FIELD *driver_page_reg_size_fields[] = {
	ISFIELD(driver_page_reg_size_prompt),
	ISFIELD(driver_page_reg_size_field)
};

static FIELD *driver_max_bus_request_fields[] = {
	ISFIELD(driver_max_bus_request_prompt),
	ISFIELD(driver_max_bus_request_field)
};

static FIELD *driver_max_devices_fields[] = {
	ISFIELD(driver_max_devices_prompt),
	ISFIELD(driver_max_devices_field)
};

static FIELD *driver_maj_block_no_fields[] = {
	ISFIELD(driver_maj_block_no_prompt),
	ISFIELD(driver_maj_block_no_field)
};

static FIELD *driver_maj_char_no_fields[] = {
	ISFIELD(driver_maj_char_no_prompt),
	ISFIELD(driver_maj_char_no_field)
};

static FIELD *driver_handler_form_prompts[] = {
	ISFIELD(driver_handler_form_prompt)
};

static FIELD *driver_type_form_prompts[] = {
	ISFIELD(driver_type_form_prompt)
};

static FIELD *driver_config_names_form_prompts[] = {
	ISFIELD(driver_config_names_form_prompt)
};

static LINE driver_form_lines[] = {
	{  0, 2, driver_name_fields },
	{  1, 2, driver_prefix_fields },

	{  4, 2, driver_vector_size_fields },
	{  5, 2, driver_page_reg_size_fields },
	{  6, 2, driver_max_bus_request_fields },
	{  7, 2, driver_max_devices_fields },

	{ 10, 2, driver_maj_block_no_fields },
	{ 11, 2, driver_maj_char_no_fields },

	{ 14, 1, driver_handler_form_prompts },
	{ 15, 1, driver_type_form_prompts },
	{ 16, 1, driver_config_names_form_prompts },
};

FORM driver_form = {
	CP(&sdriver),
	NULL,
	quit_driver_form,
	17,
	add_driver_form_line,
	del_driver_form_line,
	dup_driver_form_line,
	driver_form_lines
};


/* CONFIGURATION FILE TABLE DRIVER FORM
 *
 */

/* Name Prompt
 *
 */
#define cftbl_driver_name_prompt driver_name_prompt

/* Name Field
 *
 */
#define display_cftbl_driver_name_field display_text_field
#define change_cftbl_driver_name_field flash
#define open_cftbl_driver_name_field flash

static TEXT_FIELD cftbl_driver_name_field = {
	40,
	10,
	display_cftbl_driver_name_field,
	change_cftbl_driver_name_field,
	open_cftbl_driver_name_field,
	&sdriver.drv_name
};

/* Prefix Prompt
 *
 */
#define cftbl_driver_prefix_prompt driver_prefix_prompt

/* Prefix Field
 *
 */
#define display_cftbl_driver_prefix_field display_text_field
#define change_cftbl_driver_prefix_field flash
#define open_cftbl_driver_prefix_field flash

static TEXT_FIELD cftbl_driver_prefix_field = {
	40,
	4,
	display_cftbl_driver_prefix_field,
	change_cftbl_driver_prefix_field,
	open_cftbl_driver_prefix_field,
	&sdriver.drv_pref
};

/* Interrupt Vectors Size Prompt
 *
 */
#define cftbl_driver_vector_size_prompt driver_vector_size_prompt

/* Interrupt Vectors Size Field
 *
 */
#define display_cftbl_driver_vector_size_field display_numeric_field
#define change_cftbl_driver_vector_size_field flash
#define open_cftbl_driver_vector_size_field flash

static NUMERIC_FIELD cftbl_driver_vector_size_field = {
	40,
	3,
	display_cftbl_driver_vector_size_field,
	change_cftbl_driver_vector_size_field,
	open_cftbl_driver_vector_size_field,
	&sdriver.drv_ivs,
	DECIMAL_FIELD
};

/* Maximum Bus Request Level Prompt
 *
 */
#define cftbl_driver_max_bus_request_prompt driver_max_bus_request_prompt 

/* Maximum Bus Request Level Field
 *
 */
#define display_cftbl_driver_max_bus_request_field display_numeric_field
#define change_cftbl_driver_max_bus_request_field flash
#define open_cftbl_driver_max_bus_request_field flash

static NUMERIC_FIELD cftbl_driver_max_bus_request_field = {
	40,
	3,
	display_cftbl_driver_max_bus_request_field,
	change_cftbl_driver_max_bus_request_field,
	open_cftbl_driver_max_bus_request_field,
	&sdriver.drv_mbrl,
	DECIMAL_FIELD
};

/* Page Registers Size Prompt
 *
 */
#define cftbl_driver_page_reg_size_prompt driver_page_reg_size_prompt 

/* Page Registers Size Field
 *
 */
#define display_cftbl_driver_page_reg_size_field display_numeric_field
#define change_cftbl_driver_page_reg_size_field flash
#define open_cftbl_driver_page_reg_size_field flash

static NUMERIC_FIELD cftbl_driver_page_reg_size_field = {
	40,
	3,
	display_cftbl_driver_page_reg_size_field,
	change_cftbl_driver_page_reg_size_field,
	open_cftbl_driver_page_reg_size_field,
	&sdriver.drv_prs,
	DECIMAL_FIELD
};

/* Maximum Devices Per Controller Prompt
 *
 */
#define cftbl_driver_max_devices_prompt driver_max_devices_prompt 

/* Maximum Devices Per Controller Field
 *
 */
#define display_cftbl_driver_max_devices_field display_numeric_field
#define change_cftbl_driver_max_devices_field flash
#define open_cftbl_driver_max_devices_field flash

static NUMERIC_FIELD cftbl_driver_max_devices_field = {
	40,
	3,
	display_cftbl_driver_max_devices_field,
	change_cftbl_driver_max_devices_field,
	open_cftbl_driver_max_devices_field,
	&sdriver.drv_mdpc,
	DECIMAL_FIELD
};

/* Major Block Device Number Prompt
 *
 */
#define cftbl_driver_maj_block_no_prompt driver_maj_block_no_prompt 

/* Major Block Device Number Field
 *
 */
#define display_cftbl_driver_maj_block_no_field display_numeric_field
#define change_cftbl_driver_maj_block_no_field flash
#define open_cftbl_driver_maj_block_no_field flash

static NUMERIC_FIELD cftbl_driver_maj_block_no_field = {
	40,
	3,
	display_cftbl_driver_maj_block_no_field,
	change_cftbl_driver_maj_block_no_field,
	open_cftbl_driver_maj_block_no_field,
	&sdriver.drv_mbdn,
	DECIMAL_FIELD
};

/* Major Character Device Number Prompt
 *
 */
#define cftbl_driver_maj_char_no_prompt driver_maj_char_no_prompt 

/* Major Character Device Number Field
 *
 */
#define display_cftbl_driver_maj_char_no_field display_numeric_field
#define change_cftbl_driver_maj_char_no_field flash
#define open_cftbl_driver_maj_char_no_field flash

static NUMERIC_FIELD cftbl_driver_maj_char_no_field = {
	40,
	3,
	display_cftbl_driver_maj_char_no_field,
	change_cftbl_driver_maj_char_no_field,
	open_cftbl_driver_maj_char_no_field,
	&sdriver.drv_mcdn,
	DECIMAL_FIELD
};

/* Handler Form Prompt
 *
 */
#define display_cftbl_driver_handler_form_prompt display_prompt_field
#define change_cftbl_driver_handler_form_prompt flash

void open_cftbl_driver_handler_form_prompt()
{
	create_context(&((DRIVER *)acp->c_tbl)->drv_mint, &cftbl_mask_form);
}

static PROMPT_FIELD cftbl_driver_handler_form_prompt = {
	CENTER(HANDLER_FORM_PROMPT),
	LENGTH(HANDLER_FORM_PROMPT),
	display_cftbl_driver_handler_form_prompt,
	change_cftbl_driver_handler_form_prompt,
	open_cftbl_driver_handler_form_prompt,
	HANDLER_FORM_PROMPT
};

/* Device Type Form Prompt
 *
 */
#define display_cftbl_driver_type_form_prompt display_prompt_field
#define change_cftbl_driver_type_form_prompt flash

void open_cftbl_driver_type_form_prompt()
{
	create_context(&((DRIVER *)acp->c_tbl)->drv_tint, &cftbl_type_form);
}

static PROMPT_FIELD cftbl_driver_type_form_prompt = {
	CENTER(TYPE_FORM_PROMPT),
	LENGTH(TYPE_FORM_PROMPT),
	display_cftbl_driver_type_form_prompt,
	change_cftbl_driver_type_form_prompt,
	open_cftbl_driver_type_form_prompt,
	TYPE_FORM_PROMPT
};

/* Configuration Names Form Prompt
 *
 */
#define display_cftbl_driver_config_names_form_prompt display_prompt_field
#define change_cftbl_driver_config_names_form_prompt flash

void open_cftbl_driver_config_names_form_prompt()
{
	create_context(((DRIVER *)acp->c_tbl)->drv_data, &cftbl_cfname_form);
}

static PROMPT_FIELD cftbl_driver_config_names_form_prompt = {
	CENTER(CONFIG_NAMES_FORM_PROMPT),
	LENGTH(CONFIG_NAMES_FORM_PROMPT),
	display_cftbl_driver_config_names_form_prompt,
	change_cftbl_driver_config_names_form_prompt,
	open_cftbl_driver_config_names_form_prompt,
	CONFIG_NAMES_FORM_PROMPT
};

/* Define Form
 *
 */
#define quit_cftbl_driver_form quit_sub_context

#define add_cftbl_driver_form_line flash
#define del_cftbl_driver_form_line flash
#define dup_cftbl_driver_form_line flash

static FIELD *cftbl_driver_name_fields[] = {
	ISFIELD(cftbl_driver_name_prompt),
	ISFIELD(cftbl_driver_name_field)
};

static FIELD *cftbl_driver_prefix_fields[] = {
	ISFIELD(cftbl_driver_prefix_prompt),
	ISFIELD(cftbl_driver_prefix_field)
};

static FIELD *cftbl_driver_vector_size_fields[] = {
	ISFIELD(cftbl_driver_vector_size_prompt),
	ISFIELD(cftbl_driver_vector_size_field)
};

static FIELD *cftbl_driver_page_reg_size_fields[] = {
	ISFIELD(cftbl_driver_page_reg_size_prompt),
	ISFIELD(cftbl_driver_page_reg_size_field)
};

static FIELD *cftbl_driver_max_bus_request_fields[] = {
	ISFIELD(cftbl_driver_max_bus_request_prompt),
	ISFIELD(cftbl_driver_max_bus_request_field)
};

static FIELD *cftbl_driver_max_devices_fields[] = {
	ISFIELD(cftbl_driver_max_devices_prompt),
	ISFIELD(cftbl_driver_max_devices_field)
};

static FIELD *cftbl_driver_maj_block_no_fields[] = {
	ISFIELD(cftbl_driver_maj_block_no_prompt),
	ISFIELD(cftbl_driver_maj_block_no_field)
};

static FIELD *cftbl_driver_maj_char_no_fields[] = {
	ISFIELD(cftbl_driver_maj_char_no_prompt),
	ISFIELD(cftbl_driver_maj_char_no_field)
};

static FIELD *cftbl_driver_handler_form_prompts[] = {
	ISFIELD(cftbl_driver_handler_form_prompt)
};

static FIELD *cftbl_driver_type_form_prompts[] = {
	ISFIELD(cftbl_driver_type_form_prompt)
};

static FIELD *cftbl_driver_config_names_form_prompts[] = {
	ISFIELD(cftbl_driver_config_names_form_prompt)
};

static LINE cftbl_driver_form_lines[] = {
	{  0, 2, cftbl_driver_name_fields },
	{  1, 2, cftbl_driver_prefix_fields },

	{  4, 2, cftbl_driver_vector_size_fields },
	{  5, 2, cftbl_driver_page_reg_size_fields },
	{  6, 2, cftbl_driver_max_bus_request_fields },
	{  7, 2, cftbl_driver_max_devices_fields },

	{ 10, 2, cftbl_driver_maj_block_no_fields },
	{ 11, 2, cftbl_driver_maj_char_no_fields },

	{ 14, 1, cftbl_driver_handler_form_prompts },
	{ 15, 1, cftbl_driver_type_form_prompts },
	{ 16, 1, cftbl_driver_config_names_form_prompts },
};

FORM cftbl_driver_form = {
	CP(&sdriver),
	NULL,
	quit_cftbl_driver_form,
	17,
	add_cftbl_driver_form_line,
	del_cftbl_driver_form_line,
	dup_cftbl_driver_form_line,
	cftbl_driver_form_lines
};


/* FILESYSTEM HANDLER FORM
 *
 */

/* Name Prompt
 *
 */
#define display_fsh_name_prompt display_token_field
#define change_fsh_name_prompt flash
#define open_fsh_name_prompt flash

static TOKEN_FIELD fsh_name_prompt = {
	RIGHT(S_FSHNM),
	LENGTH(S_FSHNM),
	display_fsh_name_prompt,
	change_fsh_name_prompt,
	open_fsh_name_prompt,
	T_FSHNM
};

/* Handler Form Prompt
 *
 */
#define FSH_HANDLER_FORM_PROMPT "Filesystem Handler Routine List"

#define display_fsh_mask_form_prompt display_prompt_field
#define change_fsh_mask_form_prompt flash

void open_fsh_mask_form_prompt()
{
	create_context(&((DRIVER *)acp->c_tbl)->drv_mint, &fsh_mask_form);
}

static PROMPT_FIELD fsh_mask_form_prompt = {
	CENTER(FSH_HANDLER_FORM_PROMPT),
	LENGTH(FSH_HANDLER_FORM_PROMPT),
	display_fsh_mask_form_prompt,
	change_fsh_mask_form_prompt,
	open_fsh_mask_form_prompt,
	FSH_HANDLER_FORM_PROMPT
};

/* Other Fields
 *
 */
#define fsh_name_field driver_name_field
#define fsh_prefix_prompt driver_prefix_prompt
#define fsh_prefix_field driver_prefix_field
#define fsh_vector_size_prompt driver_vector_size_prompt
#define fsh_vector_size_field driver_vector_size_field
#define fsh_max_bus_request_prompt driver_max_bus_request_prompt
#define fsh_max_bus_request_field driver_max_bus_request_field
#define fsh_page_reg_size_prompt driver_page_reg_size_prompt
#define fsh_page_reg_size_field driver_page_reg_size_field
#define fsh_max_devices_prompt driver_max_devices_prompt
#define fsh_max_devices_field driver_max_devices_field
#define fsh_maj_block_no_prompt driver_maj_block_no_prompt
#define fsh_maj_block_no_field driver_maj_block_no_field
#define fsh_maj_char_no_prompt driver_maj_char_no_prompt
#define fsh_maj_char_no_field driver_maj_char_no_field
#define fsh_type_form_prompt driver_type_form_prompt 
#define fsh_config_names_form_prompt driver_config_names_form_prompt 

/* Define Form
 *
 */
#define quit_fsh_form quit_sub_context

#define add_fsh_form_line flash
#define del_fsh_form_line flash
#define dup_fsh_form_line flash

static FIELD *fsh_name_fields[] = {
	ISFIELD(fsh_name_prompt),
	ISFIELD(fsh_name_field)
};

static FIELD *fsh_prefix_fields[] = {
	ISFIELD(fsh_prefix_prompt),
	ISFIELD(fsh_prefix_field)
};

static FIELD *fsh_vector_size_fields[] = {
	ISFIELD(fsh_vector_size_prompt),
	ISFIELD(fsh_vector_size_field)
};

static FIELD *fsh_page_reg_size_fields[] = {
	ISFIELD(fsh_page_reg_size_prompt),
	ISFIELD(fsh_page_reg_size_field)
};

static FIELD *fsh_max_bus_request_fields[] = {
	ISFIELD(fsh_max_bus_request_prompt),
	ISFIELD(fsh_max_bus_request_field)
};

static FIELD *fsh_max_devices_fields[] = {
	ISFIELD(fsh_max_devices_prompt),
	ISFIELD(fsh_max_devices_field)
};

static FIELD *fsh_maj_block_no_fields[] = {
	ISFIELD(fsh_maj_block_no_prompt),
	ISFIELD(fsh_maj_block_no_field)
};

static FIELD *fsh_maj_char_no_fields[] = {
	ISFIELD(fsh_maj_char_no_prompt),
	ISFIELD(fsh_maj_char_no_field)
};

static FIELD *fsh_mask_form_prompts[] = {
	ISFIELD(fsh_mask_form_prompt)
};

static FIELD *fsh_type_form_prompts[] = {
	ISFIELD(fsh_type_form_prompt)
};

static FIELD *fsh_config_names_form_prompts[] = {
	ISFIELD(fsh_config_names_form_prompt)
};

static LINE fsh_form_lines[] = {
	{  0, 2, fsh_name_fields },
	{  1, 2, fsh_prefix_fields },

	{  4, 2, fsh_vector_size_fields },
	{  5, 2, fsh_page_reg_size_fields },
	{  6, 2, fsh_max_bus_request_fields },
	{  7, 2, fsh_max_devices_fields },

	{ 10, 2, fsh_maj_block_no_fields },
	{ 11, 2, fsh_maj_char_no_fields },

	{ 14, 1, fsh_mask_form_prompts },
	{ 15, 1, fsh_type_form_prompts },
	{ 16, 1, fsh_config_names_form_prompts },
};

FORM fsh_form = {
	CP(&sdriver),
	NULL,
	quit_fsh_form,
	17,
	add_fsh_form_line,
	del_fsh_form_line,
	dup_fsh_form_line,
	fsh_form_lines
};


/* CONFIGURATION FILE TABLE FILESYSTEM HANDLER FORM
 *
 */

/* Name Prompt
 *
 */
#define display_cftbl_fsh_name_prompt display_token_field
#define change_cftbl_fsh_name_prompt flash
#define open_cftbl_fsh_name_prompt flash

static TOKEN_FIELD cftbl_fsh_name_prompt = {
	RIGHT(S_FSHNM),
	LENGTH(S_FSHNM),
	display_cftbl_fsh_name_prompt,
	change_cftbl_fsh_name_prompt,
	open_cftbl_fsh_name_prompt,
	T_FSHNM
};

/* Handler Form Prompt
 *
 */
#define FSH_HANDLER_FORM_PROMPT "Filesystem Handler Routine List"

#define display_cftbl_fsh_mask_form_prompt display_prompt_field
#define change_cftbl_fsh_mask_form_prompt flash

void open_cftbl_fsh_mask_form_prompt()
{
	create_context(&((DRIVER *)acp->c_tbl)->drv_mint, &cftbl_fsh_mask_form);
}

static PROMPT_FIELD cftbl_fsh_mask_form_prompt = {
	CENTER(FSH_HANDLER_FORM_PROMPT),
	LENGTH(FSH_HANDLER_FORM_PROMPT),
	display_cftbl_fsh_mask_form_prompt,
	change_cftbl_fsh_mask_form_prompt,
	open_cftbl_fsh_mask_form_prompt,
	FSH_HANDLER_FORM_PROMPT
};

/* Other Fields
 *
 */
#define cftbl_fsh_name_field driver_name_field
#define cftbl_fsh_prefix_prompt driver_prefix_prompt
#define cftbl_fsh_prefix_field driver_prefix_field
#define cftbl_fsh_vector_size_prompt driver_vector_size_prompt
#define cftbl_fsh_vector_size_field driver_vector_size_field
#define cftbl_fsh_max_bus_request_prompt driver_max_bus_request_prompt
#define cftbl_fsh_max_bus_request_field driver_max_bus_request_field
#define cftbl_fsh_page_reg_size_prompt driver_page_reg_size_prompt
#define cftbl_fsh_page_reg_size_field driver_page_reg_size_field
#define cftbl_fsh_max_devices_prompt driver_max_devices_prompt
#define cftbl_fsh_max_devices_field driver_max_devices_field
#define cftbl_fsh_maj_block_no_prompt driver_maj_block_no_prompt
#define cftbl_fsh_maj_block_no_field driver_maj_block_no_field
#define cftbl_fsh_maj_char_no_prompt driver_maj_char_no_prompt
#define cftbl_fsh_maj_char_no_field driver_maj_char_no_field
#define cftbl_fsh_type_form_prompt driver_type_form_prompt 
#define cftbl_fsh_config_names_form_prompt driver_config_names_form_prompt 

/* Define Form
 *
 */
#define quit_cftbl_fsh_form quit_sub_context

#define add_cftbl_fsh_form_line flash
#define del_cftbl_fsh_form_line flash
#define dup_cftbl_fsh_form_line flash

static FIELD *cftbl_fsh_name_fields[] = {
	ISFIELD(cftbl_fsh_name_prompt),
	ISFIELD(cftbl_fsh_name_field)
};

static FIELD *cftbl_fsh_prefix_fields[] = {
	ISFIELD(cftbl_fsh_prefix_prompt),
	ISFIELD(cftbl_fsh_prefix_field)
};

static FIELD *cftbl_fsh_vector_size_fields[] = {
	ISFIELD(cftbl_fsh_vector_size_prompt),
	ISFIELD(cftbl_fsh_vector_size_field)
};

static FIELD *cftbl_fsh_page_reg_size_fields[] = {
	ISFIELD(cftbl_fsh_page_reg_size_prompt),
	ISFIELD(cftbl_fsh_page_reg_size_field)
};

static FIELD *cftbl_fsh_max_bus_request_fields[] = {
	ISFIELD(cftbl_fsh_max_bus_request_prompt),
	ISFIELD(cftbl_fsh_max_bus_request_field)
};

static FIELD *cftbl_fsh_max_devices_fields[] = {
	ISFIELD(cftbl_fsh_max_devices_prompt),
	ISFIELD(cftbl_fsh_max_devices_field)
};

static FIELD *cftbl_fsh_maj_block_no_fields[] = {
	ISFIELD(cftbl_fsh_maj_block_no_prompt),
	ISFIELD(cftbl_fsh_maj_block_no_field)
};

static FIELD *cftbl_fsh_maj_char_no_fields[] = {
	ISFIELD(cftbl_fsh_maj_char_no_prompt),
	ISFIELD(cftbl_fsh_maj_char_no_field)
};

static FIELD *cftbl_fsh_mask_form_prompts[] = {
	ISFIELD(cftbl_fsh_mask_form_prompt)
};

static FIELD *cftbl_fsh_type_form_prompts[] = {
	ISFIELD(cftbl_fsh_type_form_prompt)
};

static FIELD *cftbl_fsh_config_names_form_prompts[] = {
	ISFIELD(cftbl_fsh_config_names_form_prompt)
};

static LINE cftbl_fsh_form_lines[] = {
	{  0, 2, cftbl_fsh_name_fields },
	{  1, 2, cftbl_fsh_prefix_fields },

	{  4, 2, cftbl_fsh_vector_size_fields },
	{  5, 2, cftbl_fsh_page_reg_size_fields },
	{  6, 2, cftbl_fsh_max_bus_request_fields },
	{  7, 2, cftbl_fsh_max_devices_fields },

	{ 10, 2, cftbl_fsh_maj_block_no_fields },
	{ 11, 2, cftbl_fsh_maj_char_no_fields },

	{ 14, 1, cftbl_fsh_mask_form_prompts },
	{ 15, 1, cftbl_fsh_type_form_prompts },
	{ 16, 1, cftbl_fsh_config_names_form_prompts },
};

FORM cftbl_fsh_form = {
	CP(&sdriver),
	NULL,
	quit_cftbl_fsh_form,
	17,
	add_cftbl_fsh_form_line,
	del_cftbl_fsh_form_line,
	dup_cftbl_fsh_form_line,
	cftbl_fsh_form_lines
};
