/* sgdevice.c	%W%
 *
 */
#include "sysgen.h"
#include "sgyacc.h"
#include "sgform.h"
#include "sgkey.h"


/* DEVICE DESCRIPTION DATA STRUCTURE
 *
 */
typedef struct {			/* device structure		*/
	char	*dev_name;		/* driver name			*/
	int	dev_ivl;		/* interrupt vector location	*/
	int	dev_addr;		/* address			*/
	int	dev_brl;		/* bus request level		*/
	int	dev_dnod;		/* default number of devices	*/
} DEVICE;

static DEVICE devbuf[DEVICE_COUNT];

STRUCT_LIST device_slist = {
	sizeof(DEVICE),
	0,
	0,
	DEVICE_COUNT,
	STRUCTP(devbuf)
};
#define alloc_device() ((DEVICE *)alloc_struct(&device_slist))

DEVICE sdevice;				/* sample device structure	*/

/* Create New Device
 *
 */
char *new_device(name, brl, addr, ivl, dnod)
char *name;
int brl, addr, ivl, dnod;
{
	register DEVICE *devp;

	devp = alloc_device();

	devp->dev_name = name;
	devp->dev_brl  = brl;
	devp->dev_addr = addr;
	devp->dev_ivl  = ivl;
	devp->dev_dnod = dnod;

	return(CP(devp));
}

/* Create Device
 *
 */
DEVICE *create_device(name)
char *name;
{
	register DEVICE *devp;

	devp = alloc_device();
	devp->dev_name = create_string(name);
	return(devp);
}

/* Duplicate Device
 *
 */
DEVICE *dup_device(devp)
register DEVICE *devp;
{
	register DEVICE *ndevp;

	ndevp = alloc_device();

	ndevp->dev_name = create_string(devp->dev_name);
	ndevp->dev_ivl  = devp->dev_ivl;
	ndevp->dev_addr = devp->dev_addr;
	ndevp->dev_brl  = devp->dev_brl;
	ndevp->dev_dnod = devp->dev_dnod;

	return(ndevp);
}

/* De-Allocate Device
 *
 */
void free_device(devp)
DEVICE *devp;
{
	free_string(devp->dev_name);
	free_struct(&device_slist, devp);
}

/* Output Device
 *
 */
void write_device(file, devicep)
FILE *file;
DEVICE *devicep;
{
	static FORMAT device_format[] = {
		{ FMT_STRING,	T_DEVTYPE, CP(&sdevice.dev_name) },
		{ FMT_DECIMAL,	T_DEVBRL,  CP(&sdevice.dev_brl)	 },
		{ FMT_HEXADEC,	T_DEVADDR, CP(&sdevice.dev_addr) },
		{ FMT_HEXADEC,	T_DEVIVL,  CP(&sdevice.dev_ivl)	 },
		{ FMT_DECIMAL,	T_DEVDNOD, CP(&sdevice.dev_dnod) },
		{ FMT_END,	0,	   NULL			 }
	};

	write_format(file, device_format, devicep, &sdevice);
}

/* Output Dfile(4), First Part
 *
 */
void out_device(file, devp, cip)
FILE *file;
register DEVICE *devp;
CONFIG_ITEM *cip;
{
	register int num;

	fprintf(file, "%-12s%-10x%-10x%-4d",
		devp->dev_name, devp->dev_ivl, devp->dev_addr, devp->dev_brl);

	if (num = ((cip->ci_assn) && (cip->ci_num)) ?
	    cip->ci_num : devp->dev_dnod)
		fprintf(file, "%d", num);

	fputc('\n', file);
}


/* DEVICE FORM
 *
 */

/* Name Prompt
 *
 */
#define display_device_name_prompt display_token_field
#define change_device_name_prompt flash
#define open_device_name_prompt flash

static TOKEN_FIELD device_name_prompt = {
	RIGHT(S_DEVTYPE),
	LENGTH(S_DEVTYPE),
	display_device_name_prompt,
	change_device_name_prompt,
	open_device_name_prompt,
	T_DEVTYPE
};

/* Name Field
 *
 */
#define display_device_name_field display_text_field
#define change_device_name_field change_text_field
#define open_device_name_field flash

static TEXT_FIELD device_name_field  = {
	40,
	10,
	display_device_name_field,
	change_device_name_field,
	open_device_name_field,
	&sdevice.dev_name
};

/* Default Number Of Devices Prompt
 *
 */
#define display_device_default_no_of_devices_prompt display_token_field
#define change_device_default_no_of_devices_prompt flash
#define open_device_default_no_of_devices_prompt flash

static TOKEN_FIELD device_default_no_of_devices_prompt = {
	RIGHT(S_DEVDNOD),
	LENGTH(S_DEVDNOD),
	display_device_default_no_of_devices_prompt,
	change_device_default_no_of_devices_prompt,
	open_device_default_no_of_devices_prompt,
	T_DEVDNOD
};

/* Default Number Of Devices Field
 *
 */
#define display_device_defaule_no_of_devices_field display_numeric_field
#define change_device_defaule_no_of_devices_field change_numeric_field
#define open_device_defaule_no_of_devices_field flash

static NUMERIC_FIELD device_default_no_of_devices_field = {
	40,
	3,
	display_device_defaule_no_of_devices_field,
	change_device_defaule_no_of_devices_field,
	open_device_defaule_no_of_devices_field,
	&sdevice.dev_dnod,
	DECIMAL_FIELD
};

/* Interrupt Vector Level Prompt
 *
 */
#define display_device_vector_level_prompt display_token_field
#define change_device_vector_level_prompt flash
#define open_device_vector_level_prompt flash

static TOKEN_FIELD device_vector_level_prompt = {
	RIGHT(S_DEVIVL),
	LENGTH(S_DEVIVL),
	display_device_vector_level_prompt,
	change_device_vector_level_prompt,
	open_device_vector_level_prompt,
	T_DEVIVL
};

/* Interrupt Vector Level Field
 *
 */
#define display_device_vector_level_field display_numeric_field
#define change_device_vector_level_field change_numeric_field
#define open_device_vector_level_field flash

static NUMERIC_FIELD device_vector_level_field = {
	40,
	8,
	display_device_vector_level_field,
	change_device_vector_level_field,
	open_device_vector_level_field,
	&sdevice.dev_ivl,
	HEXADECIMAL_FIELD
};

/* Address Prompt
 *
 */
#define display_device_address_prompt display_token_field
#define change_device_address_prompt flash
#define open_device_address_prompt flash

static TOKEN_FIELD device_address_prompt = {
	RIGHT(S_DEVADDR),
	LENGTH(S_DEVADDR),
	display_device_address_prompt,
	change_device_address_prompt,
	open_device_address_prompt,
	T_DEVADDR
};

/* Address Field
 *
 */
#define display_device_address_field display_numeric_field
#define change_device_address_field change_numeric_field
#define open_device_address_field flash

static NUMERIC_FIELD device_address_field = {
	40,
	8,
	display_device_address_field,
	change_device_address_field,
	open_device_address_field,
	&sdevice.dev_addr,
	HEXADECIMAL_FIELD
};

/* Bus Request Level Prompt
 *
 */
#define display_device_request_level_prompt display_token_field
#define change_device_request_level_prompt flash
#define open_device_request_level_prompt flash

static TOKEN_FIELD device_request_level_prompt = {
	RIGHT(S_DEVBRL),
	LENGTH(S_DEVBRL),
	display_device_request_level_prompt,
	change_device_request_level_prompt,
	open_device_request_level_prompt,
	T_DEVBRL
};

/* Bus Request Level Field
 *
 */
#define display_device_request_level_field display_numeric_field
#define change_device_request_level_field change_numeric_field
#define open_device_request_level_field flash

static NUMERIC_FIELD device_request_level_field = {
	40,
	1,
	display_device_request_level_field,
	change_device_request_level_field,
	open_device_request_level_field,
	&sdevice.dev_brl,
	DECIMAL_FIELD
};

/* Define Form
 *
 */
#define quit_device_form quit_sub_context

#define add_device_form_line flash
#define del_device_form_line flash
#define dup_device_form_line flash

static FIELD *device_name_fields[] = {
	ISFIELD(device_name_prompt),
	ISFIELD(device_name_field)
};

static FIELD *device_default_no_of_devices_fields[] = {
	ISFIELD(device_default_no_of_devices_prompt),
	ISFIELD(device_default_no_of_devices_field)
};

static FIELD *device_vector_level_fields[] = {
	ISFIELD(device_vector_level_prompt),
	ISFIELD(device_vector_level_field)
};

static FIELD *device_address_fields[] = {
	ISFIELD(device_address_prompt),
	ISFIELD(device_address_field)
};

static FIELD *device_request_level_fields[] = {
	ISFIELD(device_request_level_prompt),
	ISFIELD(device_request_level_field)
};

static LINE device_form_lines[] = {
	{ 0, 2, device_name_fields },
	{ 2, 2, device_default_no_of_devices_fields },
	{ 4, 2, device_vector_level_fields },
	{ 5, 2, device_address_fields },
	{ 6, 2, device_request_level_fields }
};

FORM device_form = {
	CP(&sdevice),
	NULL,
	quit_device_form,
	7,
	add_device_form_line,
	del_device_form_line,
	dup_device_form_line,
	device_form_lines
};


/* CONFIGURATION FILE TABLE DEVICE FORM
 *
 */

/* Name Field
 *
 */
#define display_cftbl_device_name_field display_text_field
#define change_cftbl_device_name_field flash
#define open_cftbl_device_name_field flash

static TEXT_FIELD cftbl_device_name_field  = {
	40,
	10,
	display_cftbl_device_name_field,
	change_cftbl_device_name_field,
	open_cftbl_device_name_field,
	&sdevice.dev_name
};

/* Number Of Devices Prompt
 *
 */
#define display_cftbl_device_no_of_devices_prompt display_token_field
#define change_cftbl_device_no_of_devices_prompt flash
#define open_cftbl_device_no_of_devices_prompt flash

static TOKEN_FIELD cftbl_device_no_of_devices_prompt = {
	RIGHT(S_DEVNOD),
	LENGTH(S_DEVNOD),
	display_cftbl_device_no_of_devices_prompt,
	change_cftbl_device_no_of_devices_prompt,
	open_cftbl_device_no_of_devices_prompt,
	T_DEVNOD
};

/* Number Of Devices Field
 *
 */
static void display_cftbl_device_no_of_devices_field(fieldp, devp, scrlin)
FIELD *fieldp;
char *devp;
int scrlin;
{
	register CONFIG_ITEM *cip;
	register char *ascii_number_string;

	cip = (CONFIG_ITEM *)acp->c_other;

	ascii_number_string = ((cip != NULL) && (cip->ci_assn)) ?
		convert_numeric_field(fieldp, cip,
			&sconfigitem, &sconfigitem.ci_num) :
		convert_numeric_field(fieldp, devp,
			&sdevice, &sdevice.dev_dnod);

	display_field(fieldp, ascii_number_string, scrlin);
	free_string(ascii_number_string);
}

static void change_cftbl_device_no_of_devices_field()
{
	register CONFIG_ITEM *cip;

	if ((cip = (CONFIG_ITEM *)acp->c_other) == NULL) {
		flash();
botmsg("This description must be selected before a value may be assigned.");
		return;
	}

	if (!cip->ci_assn)
		cip->ci_num = 0;

	if (change_numeric_data(cip, &sconfigitem))
		cip->ci_assn = CIT_NUM;

	update_context();
}

#define open_cftbl_device_no_of_devices_field flash

static NUMERIC_FIELD cftbl_device_no_of_devices_field = {
	40,
	3,
	display_cftbl_device_no_of_devices_field,
	change_cftbl_device_no_of_devices_field,
	open_cftbl_device_no_of_devices_field,
	&sconfigitem.ci_num,
	DECIMAL_FIELD
};

/* Default Number Of Devices Field
 *
 */
#define display_cftbl_device_defaule_no_of_devices_field display_numeric_field
#define change_cftbl_device_defaule_no_of_devices_field flash
#define open_cftbl_device_defaule_no_of_devices_field flash

static NUMERIC_FIELD cftbl_device_default_no_of_devices_field = {
	40,
	3,
	display_cftbl_device_defaule_no_of_devices_field,
	change_cftbl_device_defaule_no_of_devices_field,
	open_cftbl_device_defaule_no_of_devices_field,
	&sdevice.dev_dnod,
	DECIMAL_FIELD
};

/* Interrupt Vector Level Field
 *
 */
#define display_cftbl_device_vector_level_field display_numeric_field
#define change_cftbl_device_vector_level_field flash
#define open_cftbl_device_vector_level_field flash

static NUMERIC_FIELD cftbl_device_vector_level_field = {
	40,
	8,
	display_cftbl_device_vector_level_field,
	change_cftbl_device_vector_level_field,
	open_cftbl_device_vector_level_field,
	&sdevice.dev_ivl,
	HEXADECIMAL_FIELD
};

/* Address Field
 *
 */
#define display_cftbl_device_address_field display_numeric_field
#define change_cftbl_device_address_field flash
#define open_cftbl_device_address_field flash

static NUMERIC_FIELD cftbl_device_address_field = {
	40,
	8,
	display_cftbl_device_address_field,
	change_cftbl_device_address_field,
	open_cftbl_device_address_field,
	&sdevice.dev_addr,
	HEXADECIMAL_FIELD
};

/* Bus Request Level Field
 *
 */
#define display_cftbl_device_request_level_field display_numeric_field
#define change_cftbl_device_request_level_field flash
#define open_cftbl_device_request_level_field flash

static NUMERIC_FIELD cftbl_device_request_level_field = {
	40,
	1,
	display_cftbl_device_request_level_field,
	change_cftbl_device_request_level_field,
	open_cftbl_device_request_level_field,
	&sdevice.dev_brl,
	DECIMAL_FIELD
};

/* Other Fields
 *
 */
#define cftbl_device_name_prompt device_name_prompt 
#define cftbl_device_default_no_of_devices_prompt \
		device_default_no_of_devices_prompt 
#define cftbl_device_vector_level_prompt device_vector_level_prompt 
#define cftbl_device_address_prompt device_address_prompt 
#define cftbl_device_request_level_prompt device_request_level_prompt 

/* Define Form
 *
 */
#define quit_cftbl_device_form quit_sub_context

#define add_cftbl_device_form_line flash
#define del_cftbl_device_form_line flash
#define dup_cftbl_device_form_line flash

static FIELD *cftbl_device_name_fields[] = {
	ISFIELD(cftbl_device_name_prompt),
	ISFIELD(cftbl_device_name_field)
};

static FIELD *cftbl_device_no_of_devices_fields[] = {
	ISFIELD(cftbl_device_no_of_devices_prompt),
	ISFIELD(cftbl_device_no_of_devices_field)
};

static FIELD *cftbl_device_default_no_of_devices_fields[] = {
	ISFIELD(cftbl_device_default_no_of_devices_prompt),
	ISFIELD(cftbl_device_default_no_of_devices_field)
};

static FIELD *cftbl_device_vector_level_fields[] = {
	ISFIELD(cftbl_device_vector_level_prompt),
	ISFIELD(cftbl_device_vector_level_field)
};

static FIELD *cftbl_device_address_fields[] = {
	ISFIELD(cftbl_device_address_prompt),
	ISFIELD(cftbl_device_address_field)
};

static FIELD *cftbl_device_request_level_fields[] = {
	ISFIELD(cftbl_device_request_level_prompt),
	ISFIELD(cftbl_device_request_level_field)
};

static LINE cftbl_device_form_lines[] = {
	{ 0, 2, cftbl_device_name_fields },
	{ 2, 2, cftbl_device_no_of_devices_fields },
	{ 3, 2, cftbl_device_default_no_of_devices_fields },
	{ 5, 2, cftbl_device_vector_level_fields },
	{ 6, 2, cftbl_device_address_fields },
	{ 7, 2, cftbl_device_request_level_fields }
};

FORM cftbl_device_form = {
	CP(&sdevice),
	NULL,
	quit_cftbl_device_form,
	8,
	add_cftbl_device_form_line,
	del_cftbl_device_form_line,
	dup_cftbl_device_form_line,
	cftbl_device_form_lines
};


/* MINOR DEVICE SPECIFICATION
 *
 */
typedef struct {
	int	m_root;			/* root minor device number	*/
	int	m_pipe;			/* pipe minor device number	*/
	int	m_dump;			/* dump minor device number	*/
	int	m_swap;			/* swap minor device number	*/
	int	m_swplo;		/* lowest disk block (decimal)	*/
	int	m_nswap;		/* number of disk blocks (dec)	*/
} MINOR;

static MINOR minorbuf[MINOR_COUNT];

STRUCT_LIST minor_slist = {
	sizeof(MINOR),
	0,
	0,
	MINOR_COUNT,
	STRUCTP(minorbuf)
};

#define alloc_minor() ((MINOR *)alloc_struct(&minor_slist))

MINOR sminor;				/* sample minor device specification */

/* Create Minor Devices Specification
 *
 */
char *new_minor(root, pipe, dump, swap, swplo, nswap)
int root, pipe, dump, swap, swplo, nswap;
{
	register MINOR *minorp;

	minorp = alloc_minor();

	minorp->m_root  = root;
	minorp->m_pipe  = pipe;
	minorp->m_dump  = dump;
	minorp->m_swap  = swap;
	minorp->m_swplo = swplo;
	minorp->m_nswap = nswap;

	return(CP(minorp));
}

/* Duplicate Minor Devices Specification
 *
 */
char *dup_minor(minp)
register MINOR *minp;
{
	register MINOR *nminp;

	nminp = alloc_minor();

	nminp->m_root  = minp->m_root;
	nminp->m_pipe  = minp->m_pipe;
	nminp->m_dump  = minp->m_dump;
	nminp->m_swap  = minp->m_swap;
	nminp->m_swplo = minp->m_swplo;
	nminp->m_nswap = minp->m_nswap;

	return(CP(nminp));
}

/* De-Allocate Minor Device Specification
 *
 */
void free_minor(mp)
MINOR *mp;
{
	free_struct(&minor_slist, mp);
}

/* Output Minor Device Specification
 *
 */
static void write_minor(file, mp)
FILE *file;
MINOR *mp;
{
	static FORMAT minorformat[] = {
		{ FMT_PROMPT,	T_MINOR,  NULL		      },
		{ FMT_OCTAL,	T_MROOT,  CP(&sminor.m_root)  },
		{ FMT_OCTAL,	T_MPIPE,  CP(&sminor.m_pipe)  },
		{ FMT_OCTAL,	T_MDUMP,  CP(&sminor.m_dump)  },
		{ FMT_OCTAL,	T_MSWAP,  CP(&sminor.m_swap)  },
		{ FMT_DECIMAL,	T_MSWPLO, CP(&sminor.m_swplo) },
		{ FMT_DECIMAL,	T_MNSWAP, CP(&sminor.m_nswap) },
		{ FMT_END,	0,	  NULL		      }
	};

	extern void write_format();

	write_format(file, minorformat, mp, &sminor);
}

/* Output Dfile(4), Second Part, Driver Followed By Minor Device Number
 *
 */
static void out_minor(file, mp, driver, minptr, swapdevice)
FILE *file;
register MINOR *mp;
char *driver;
int *minptr, swapdevice;
{
	register int i;

	fprintf(file, "%-8s", driver);

	i = *LOCINTEGER(mp,  minptr,  &sminor);
	fprintf(file, (i < 0) ? "%d" : "%o", i);

	if (swapdevice)
		fprintf(file, " %d %d", mp->m_swplo, mp->m_nswap);
}


/* MINOR DEVICE SPECIFICATION FORM
 *
 */

/* Root Device Minor Device Number Prompt
 *
 */
#define display_minor_root_prompt display_token_field
#define change_minor_root_prompt flash
#define open_minor_root_prompt flash

static TOKEN_FIELD minor_root_prompt = {
	RIGHT(S_MROOT),
	LENGTH(S_MROOT),
	display_minor_root_prompt,
	change_minor_root_prompt,
	open_minor_root_prompt,
	T_MROOT
};

/* Root Device Minor Device Number Field
 *
 */
#define display_minor_root_field display_numeric_field
#define change_minor_root_field change_numeric_field
#define open_minor_root_field flash

static NUMERIC_FIELD minor_root_field = {
	40,
	3,
	display_minor_root_field,
	change_minor_root_field,
	open_minor_root_field,
	&sminor.m_root,
	OCTAL_FIELD
};

/* Pipe Device Minor Device Number Prompt
 *
 */
#define display_minor_pipe_prompt display_token_field
#define change_minor_pipe_prompt flash
#define open_minor_pipe_prompt flash

static TOKEN_FIELD minor_pipe_prompt = {
	RIGHT(S_MPIPE),
	LENGTH(S_MPIPE),
	display_minor_pipe_prompt,
	change_minor_pipe_prompt,
	open_minor_pipe_prompt,
	T_MPIPE
};

/* Pipe Device Minor Device Number Field
 *
 */
#define display_minor_pipe_field display_numeric_field
#define change_minor_pipe_field change_numeric_field
#define open_minor_pipe_field flash

static NUMERIC_FIELD minor_pipe_field = {
	40,
	3,
	display_minor_pipe_field,
	change_minor_pipe_field,
	open_minor_pipe_field,
	&sminor.m_pipe,
	OCTAL_FIELD
};

/* Dump Device Minor Device Number Prompt
 *
 */
#define display_minor_dump_prompt display_token_field
#define change_minor_dump_prompt flash
#define open_minor_dump_prompt flash

static TOKEN_FIELD minor_dump_prompt = {
	RIGHT(S_MDUMP),
	LENGTH(S_MDUMP),
	display_minor_dump_prompt,
	change_minor_dump_prompt,
	open_minor_dump_prompt,
	T_MDUMP
};

/* Dump Device Minor Device Number Field
 *
 */
#define display_minor_dump_field display_numeric_field
#define change_minor_dump_field change_numeric_field
#define open_minor_dump_field flash

static NUMERIC_FIELD minor_dump_field = {
	40,
	3,
	display_minor_dump_field,
	change_minor_dump_field,
	open_minor_dump_field,
	&sminor.m_dump,
	OCTAL_FIELD
};

/* Swap Device Minor Device Number Prompt
 *
 */
#define display_minor_swap_prompt display_token_field
#define change_minor_swap_prompt flash
#define open_minor_swap_prompt flash

static TOKEN_FIELD minor_swap_prompt = {
	RIGHT(S_MSWAP),
	LENGTH(S_MSWAP),
	display_minor_swap_prompt,
	change_minor_swap_prompt,
	open_minor_swap_prompt,
	T_MSWAP
};

/* Swap Device Minor Device Number Field
 *
 */
#define display_minor_swap_field display_numeric_field
#define change_minor_swap_field change_numeric_field
#define open_minor_swap_field flash

static NUMERIC_FIELD minor_swap_field = {
	40,
	3,
	display_minor_swap_field,
	change_minor_swap_field,
	open_minor_swap_field,
	&sminor.m_swap,
	OCTAL_FIELD
};

/* Swap Low Block Number Prompt
 *
 */
#define display_minor_swap_low_prompt display_token_field
#define change_minor_swap_low_prompt flash
#define open_minor_swap_low_prompt flash

static TOKEN_FIELD minor_swap_low_prompt = {
	RIGHT(S_MSWPLO),
	LENGTH(S_MSWPLO),
	display_minor_swap_low_prompt,
	change_minor_swap_low_prompt,
	open_minor_swap_low_prompt,
	T_MSWPLO
};

/* Swap Low Block Number Field
 *
 */
#define display_minor_swap_low_field display_numeric_field
#define change_minor_swap_low_field change_numeric_field
#define open_minor_swap_low_field flash

static NUMERIC_FIELD minor_swap_low_field = {
	40,
	10,
	display_minor_swap_low_field,
	change_minor_swap_low_field,
	open_minor_swap_low_field,
	&sminor.m_swplo,
	OCTAL_FIELD
};

/* Number Of Blocks In Swap Area Prompt
 *
 */
#define display_minor_no_swap_prompt display_token_field
#define change_minor_no_swap_prompt flash
#define open_minor_no_swap_prompt flash

static TOKEN_FIELD minor_no_swap_prompt = {
	RIGHT(S_MNSWAP),
	LENGTH(S_MNSWAP),
	display_minor_no_swap_prompt,
	change_minor_no_swap_prompt,
	open_minor_no_swap_prompt,
	T_MNSWAP
};

/* Number Of Blocks In Swap Area Field
 *
 */
#define display_minor_no_swap_field display_numeric_field
#define change_minor_no_swap_field change_numeric_field
#define open_minor_no_swap_field flash

static NUMERIC_FIELD minor_no_swap_field = {
	40,
	10,
	display_minor_no_swap_field,
	change_minor_no_swap_field,
	open_minor_no_swap_field,
	&sminor.m_nswap,
	OCTAL_FIELD
};

/* Define Form
 *
 */
#define quit_minor_form quit_sub_context

#define add_minor_form_line flash
#define del_minor_form_line flash
#define dup_minor_form_line flash

static FIELD *minor_root_fields[] = {
	ISFIELD(minor_root_prompt),
	ISFIELD(minor_root_field)
};

static FIELD *minor_pipe_fields[] = {
	ISFIELD(minor_pipe_prompt),
	ISFIELD(minor_pipe_field)
};

static FIELD *minor_dump_fields[] = {
	ISFIELD(minor_dump_prompt),
	ISFIELD(minor_dump_field)
};

static FIELD *minor_swap_fields[] = {
	ISFIELD(minor_swap_prompt),
	ISFIELD(minor_swap_field)
};

static FIELD *minor_swap_low_fields[] = {
	ISFIELD(minor_swap_low_prompt),
	ISFIELD(minor_swap_low_field)
};

static FIELD *minor_no_swap_fields[] = {
	ISFIELD(minor_no_swap_prompt),
	ISFIELD(minor_no_swap_field)
};

static LINE minor_form_lines[] = {
	{ 0, 2, minor_root_fields },
	{ 1, 2, minor_pipe_fields },
	{ 2, 2, minor_dump_fields },
	{ 3, 2, minor_swap_fields },
	{ 5, 2, minor_swap_low_fields },
	{ 6, 2, minor_no_swap_fields }
};

static FORM minor_form = {
	CP(&sminor),
	NULL,
	quit_minor_form,
	7,
	add_minor_form_line,
	del_minor_form_line,
	dup_minor_form_line,
	minor_form_lines
};


/* CONFIGURATION FILE TABLE MINOR DEVICE SPECICATION FORM
 *
 */

/* Root Device Minor Device Number Field
 *
 */
#define display_cftbl_minor_root_field display_numeric_field
#define change_cftbl_minor_root_field flash
#define open_cftbl_minor_root_field flash

static NUMERIC_FIELD cftbl_minor_root_field = {
	40,
	3,
	display_cftbl_minor_root_field,
	change_cftbl_minor_root_field,
	open_cftbl_minor_root_field,
	&sminor.m_root,
	OCTAL_FIELD
};

/* Pipe Device Minor Device Number Field
 *
 */
#define display_cftbl_minor_pipe_field display_numeric_field
#define change_cftbl_minor_pipe_field flash
#define open_cftbl_minor_pipe_field flash

static NUMERIC_FIELD cftbl_minor_pipe_field = {
	40,
	3,
	display_cftbl_minor_pipe_field,
	change_cftbl_minor_pipe_field,
	open_cftbl_minor_pipe_field,
	&sminor.m_pipe,
	OCTAL_FIELD
};

/* Dump Device Minor Device Number Field
 *
 */
#define display_cftbl_minor_dump_field display_numeric_field
#define change_cftbl_minor_dump_field flash
#define open_cftbl_minor_dump_field flash

static NUMERIC_FIELD cftbl_minor_dump_field = {
	40,
	3,
	display_cftbl_minor_dump_field,
	change_cftbl_minor_dump_field,
	open_cftbl_minor_dump_field,
	&sminor.m_dump,
	OCTAL_FIELD
};

/* Swap Device Minor Device Number Field
 *
 */
#define display_cftbl_minor_swap_field display_numeric_field
#define change_cftbl_minor_swap_field flash
#define open_cftbl_minor_swap_field flash

static NUMERIC_FIELD cftbl_minor_swap_field = {
	40,
	3,
	display_cftbl_minor_swap_field,
	change_cftbl_minor_swap_field,
	open_cftbl_minor_swap_field,
	&sminor.m_swap,
	OCTAL_FIELD
};

/* Swap Low Block Number Field
 *
 */
#define display_cftbl_minor_swap_low_field display_numeric_field
#define change_cftbl_minor_swap_low_field flash
#define open_cftbl_minor_swap_low_field flash

static NUMERIC_FIELD cftbl_minor_swap_low_field = {
	40,
	10,
	display_cftbl_minor_swap_low_field,
	change_cftbl_minor_swap_low_field,
	open_cftbl_minor_swap_low_field,
	&sminor.m_swplo,
	OCTAL_FIELD
};

/* Number Of Blocks In Swap Area Field
 *
 */
#define display_cftbl_minor_no_swap_field display_numeric_field
#define change_cftbl_minor_no_swap_field flash
#define open_cftbl_minor_no_swap_field flash

static NUMERIC_FIELD cftbl_minor_no_swap_field = {
	40,
	10,
	display_cftbl_minor_no_swap_field,
	change_cftbl_minor_no_swap_field,
	open_cftbl_minor_no_swap_field,
	&sminor.m_nswap,
	OCTAL_FIELD
};

/* Other Fields
 *
 */
#define cftbl_minor_root_prompt minor_root_prompt 
#define cftbl_minor_pipe_prompt minor_pipe_prompt 
#define cftbl_minor_dump_prompt minor_dump_prompt 
#define cftbl_minor_swap_prompt minor_swap_prompt 
#define cftbl_minor_swap_low_prompt minor_swap_low_prompt 
#define cftbl_minor_no_swap_prompt minor_no_swap_prompt 

/* Define Form
 *
 */
#define quit_cftbl_minor_form quit_sub_context

#define add_cftbl_minor_form_line flash
#define del_cftbl_minor_form_line flash
#define dup_cftbl_minor_form_line flash

static FIELD *cftbl_minor_root_fields[] = {
	ISFIELD(cftbl_minor_root_prompt),
	ISFIELD(cftbl_minor_root_field)
};

static FIELD *cftbl_minor_pipe_fields[] = {
	ISFIELD(cftbl_minor_pipe_prompt),
	ISFIELD(cftbl_minor_pipe_field)
};

static FIELD *cftbl_minor_dump_fields[] = {
	ISFIELD(cftbl_minor_dump_prompt),
	ISFIELD(cftbl_minor_dump_field)
};

static FIELD *cftbl_minor_swap_fields[] = {
	ISFIELD(cftbl_minor_swap_prompt),
	ISFIELD(cftbl_minor_swap_field)
};

static FIELD *cftbl_minor_swap_low_fields[] = {
	ISFIELD(cftbl_minor_swap_low_prompt),
	ISFIELD(cftbl_minor_swap_low_field)
};

static FIELD *cftbl_minor_no_swap_fields[] = {
	ISFIELD(cftbl_minor_no_swap_prompt),
	ISFIELD(cftbl_minor_no_swap_field)
};

static LINE cftbl_minor_form_lines[] = {
	{ 0, 2, cftbl_minor_root_fields },
	{ 1, 2, cftbl_minor_pipe_fields },
	{ 2, 2, cftbl_minor_dump_fields },
	{ 3, 2, cftbl_minor_swap_fields },
	{ 5, 2, cftbl_minor_swap_low_fields },
	{ 6, 2, cftbl_minor_no_swap_fields }
};

static FORM cftbl_minor_form = {
	CP(&sminor),
	NULL,
	quit_cftbl_minor_form,
	7,
	add_cftbl_minor_form_line,
	del_cftbl_minor_form_line,
	dup_cftbl_minor_form_line,
	cftbl_minor_form_lines
};


/* MINOR DEVICE SPECIFICATION TABLE
 *
 */

/* Output Minor Device Specification Table
 *
 */
static void write_mintbl(file, mtp)
FILE *file;
register TABLE *mtp;
{
	void write_minor();
	register int i;

	for (i = 0; i < mtp->tbl_count; ++i)
		write_minor(file, get_minor_table_entry(mtp, i));
}

/* Output Dfile(4), Second Part, Minor Device Number Table
 *
 */
static void out_mintbl(file, mtp, driver, minptr, swapdevice)
FILE *file;
register TABLE *mtp;
char *driver;
int *minptr, swapdevice;
{
	void out_minor();
	register int i;

	for (i = 0; i < mtp->tbl_count; i++) {
		if (i) fprintf(file, ", ");
		out_minor(file, get_minor_table_entry(mtp, i),
				driver, minptr, swapdevice);
	}
}


/* MINOR DEVICE SPECIFICATION TABLE FORM
 *
 */

/* Minor Device Specifiaction Table Field
 *
 */
#define MINOR_FORM_PROMPT "Minor Device Numbers Set #"

static void display_mintbl_field(fieldp, data, scrlin, cp)
FIELD *fieldp;
char *data;
int scrlin;
CONTEXT *cp;
{
	register char *buf, **dataptr;

	dataptr = cp->c_mintbl->tbl_ptr;
	while (*dataptr != data)
		++dataptr;

	buf = mkbuf(BUFSIZE);
	sprintf(buf, "%s%d", MINOR_FORM_PROMPT,
		dataptr - cp->c_mintbl->tbl_ptr);
	display_field(fieldp, buf, scrlin);
	free(buf);
}

#define change_mintbl_field flash

static void open_mintbl_field()
{
	create_context(get_minor_table_entry(acp->c_mintbl, acp->c_curline),
			&minor_form);
}

static SIMPLE_FIELD mintbl_field = {
	CENTER(MINOR_FORM_PROMPT),
	LENGTH(MINOR_FORM_PROMPT) + 3,
	display_mintbl_field,
	change_mintbl_field,
	open_mintbl_field
};

/* Define Form
 *
 */
#define quit_mintbl_form quit_sub_context

static void add_mintbl_form_line()
{
	insert_entry_in_minor_table(acp->c_mintbl,
		acp->c_curline, alloc_minor());
	display_form_body();
	update_context();
	--acp->c_modified;
}

static void del_mintbl_form_line()
{
	remove_minor_table_entry(acp->c_mintbl, acp->c_curline);
	display_form_body();
	update_context();
	--acp->c_modified;
}

#define dup_mintbl_form_line flash

static FIELD *mintbl_fields[] = {
	ISFIELD(mintbl_field)
};

static LINE mintbl_form_lines[] = {
	{ 0, 1, mintbl_fields }
};

static FORM mintbl_form = {
	NULL,
	NULL,
	quit_mintbl_form,
	0,
	add_mintbl_form_line,
	del_mintbl_form_line,
	dup_mintbl_form_line,
	mintbl_form_lines
};


/* CONFIGURATION FILE TABLE MINOR DEVICE SPECIFICATION TABLE FORM
 *
 */

/* Minor Device Specifiaction Table Field
 *
 */
#define display_cftbl_mintbl_field display_mintbl_field
#define change_cftbl_mintbl_field flash

static void open_cftbl_mintbl_field()
{
	create_context(get_minor_table_entry(acp->c_mintbl, acp->c_curline),
			&cftbl_minor_form);
}

static SIMPLE_FIELD cftbl_mintbl_field = {
	CENTER(MINOR_FORM_PROMPT),
	LENGTH(MINOR_FORM_PROMPT) + 3,
	display_cftbl_mintbl_field,
	change_cftbl_mintbl_field,
	open_cftbl_mintbl_field
};

/* Define Form
 *
 */
#define quit_cftbl_mintbl_form quit_sub_context

#define add_cftbl_mintbl_form_line flash
#define del_cftbl_mintbl_form_line flash
#define dup_cftbl_mintbl_form_line flash

static FIELD *cftbl_mintbl_fields[] = {
	ISFIELD(cftbl_mintbl_field)
};

static LINE cftbl_mintbl_form_lines[] = {
	{ 0, 1, cftbl_mintbl_fields }
};

static FORM cftbl_mintbl_form = {
	NULL,
	NULL,
	quit_cftbl_mintbl_form,
	0,
	add_cftbl_mintbl_form_line,
	del_cftbl_mintbl_form_line,
	dup_cftbl_mintbl_form_line,
	cftbl_mintbl_form_lines
};


/* SYSTEM DEVICES STRUCTURE
 *
 */
typedef struct {
	char	*s_root;		/* root device name		*/
	char	*s_pipe;		/* pipe device name		*/
	char	*s_dump;		/* dump device name		*/
	char	*s_swap;		/* swap device name		*/
	TABLE	*s_mintbl;		/* minor device number decs	*/
} SYSDEV;

static SYSDEV sysdevbuf[SYSDEV_COUNT];

STRUCT_LIST sysdev_slist = {
	sizeof(SYSDEV),
	0,
	0,
	SYSDEV_COUNT,
	STRUCTP(sysdevbuf)
};

#define alloc_sysdev() ((SYSDEV *)alloc_struct(&sysdev_slist))

SYSDEV ssysdev;				/* sample system devices structure */

/* Create System Devices Specification
 *
 */
char *new_sysdev(root, pipe, dump, swap, mintp)
char *root, *pipe, *dump, *swap;
TABLE *mintp;
{
	register SYSDEV *sysdevp;

	sysdevp = alloc_sysdev();

	sysdevp->s_root   = root;
	sysdevp->s_pipe   = pipe;
	sysdevp->s_dump   = dump;
	sysdevp->s_swap   = swap;
	sysdevp->s_mintbl = mintp;

	return(CP(sysdevp));
}

/* Allocate System Devices Specification
 *
 */
SYSDEV *create_sysdev(rootnm, pipenm, dumpnm, swapnm)
char *rootnm, *pipenm, *dumpnm, *swapnm;
{
	register SYSDEV *sdp;

	sdp = alloc_sysdev();

	sdp->s_root   = create_string(rootnm);
	sdp->s_pipe   = create_string(pipenm);
	sdp->s_dump   = create_string(dumpnm);
	sdp->s_swap   = create_string(swapnm);
	sdp->s_mintbl = alloc_minor_table();

	return(sdp);
}

/* Duplicate System Devices Specification
 *
 */
char *dup_sysdev(sdp)
register SYSDEV *sdp;
{
	register SYSDEV *nsdp;

	nsdp = alloc_sysdev();

	nsdp->s_root   = create_string(sdp->s_root);
	nsdp->s_pipe   = create_string(sdp->s_pipe);
	nsdp->s_dump   = create_string(sdp->s_dump);
	nsdp->s_swap   = create_string(sdp->s_swap);
	nsdp->s_mintbl = (TABLE *)dup_minor_table(sdp->s_mintbl);

	return(CP(nsdp));
}

/* De-allocate System Devices Specification
 *
 */
void free_sysdev(sdp)
register SYSDEV *sdp;
{
	free_string(sdp->s_root);
	free_string(sdp->s_pipe);
	free_string(sdp->s_dump);
	free_string(sdp->s_swap);
	free_minor_table(sdp->s_mintbl);
	free_struct(&sysdev_slist, sdp);
}

/* Output System Devices Specification
 *
 */
static void write_sysdev(file, sdp)
FILE *file;
register SYSDEV *sdp;
{
	static FORMAT sysdevformat[] = {
		{ FMT_STRING, T_SDROOT,	CP(&ssysdev.s_root) },
		{ FMT_STRING, T_SDPIPE,	CP(&ssysdev.s_pipe) },
		{ FMT_STRING, T_SDDUMP,	CP(&ssysdev.s_dump) },
		{ FMT_STRING, T_SDSWAP,	CP(&ssysdev.s_swap) },
		{ FMT_END,    0,	NULL		    }
	};

	write_format(file, sysdevformat, sdp, &ssysdev);
	write_mintbl(file, sdp->s_mintbl);
}

/* Output Dfile, Second Part, Root/Pipe/Dump/Swap Device Specification
 *
 */
static void out_sysdev(file, sdtp, type, devptr, minptr, swapdevice)
FILE *file;
TABLE *sdtp;
char *type, **devptr;
int *minptr, swapdevice;
{
	register SYSDEV *sdp;
	register int i;

	fprintf(file, "%s", type);

	for (i = 0; i < sdtp->tbl_count; i++) {
		if (i) fprintf(file, ",\n");
		fputc('\t', file);

		sdp = get_sysdev_table_entry(sdtp, i);
		out_mintbl(file, sdp->s_mintbl,
			*LOCCHARPTR(sdp, devptr, &ssysdev),
			minptr, swapdevice);
	}
	fputc('\n', file);
}


/* SYSTEM DEVICES FORM
 *
 */

/* Root Device Name Prompt
 *
 */
#define display_sysdev_root_prompt display_token_field
#define change_sysdev_root_prompt flash
#define open_sysdev_root_prompt flash

static TOKEN_FIELD sysdev_root_prompt = {
	RIGHT(S_SDROOT),
	LENGTH(S_SDROOT),
	display_sysdev_root_prompt,
	change_sysdev_root_prompt,
	open_sysdev_root_prompt,
	T_SDROOT
};

/* Root Device Name Field
 *
 */
#define display_sysdev_root_field display_text_field
#define change_sysdev_root_field change_text_field
#define open_sysdev_root_field flash

static TEXT_FIELD sysdev_root_field = {
	40,
	10,
	display_sysdev_root_field,
	change_sysdev_root_field,
	open_sysdev_root_field,
	&ssysdev.s_root
};

/* Pipe Device Name Prompt
 *
 */
#define display_sysdev_pipe_prompt display_token_field
#define change_sysdev_pipe_prompt flash
#define open_sysdev_pipe_prompt flash

static TOKEN_FIELD sysdev_pipe_prompt = {
	RIGHT(S_SDPIPE),
	LENGTH(S_SDPIPE),
	display_sysdev_pipe_prompt,
	change_sysdev_pipe_prompt,
	open_sysdev_pipe_prompt,
	T_SDPIPE
};

/* Pipe Device Name Field
 *
 */
#define display_sysdev_pipe_field display_text_field
#define change_sysdev_pipe_field change_text_field
#define open_sysdev_pipe_field flash

static TEXT_FIELD sysdev_pipe_field = {
	40,
	10,
	display_sysdev_pipe_field,
	change_sysdev_pipe_field,
	open_sysdev_pipe_field,
	&ssysdev.s_pipe
};

/* Dump Device Name Prompt
 *
 */
#define display_sysdev_dump_prompt display_token_field
#define change_sysdev_dump_prompt flash
#define open_sysdev_dump_prompt flash

static TOKEN_FIELD sysdev_dump_prompt = {
	RIGHT(S_SDDUMP),
	LENGTH(S_SDDUMP),
	display_sysdev_dump_prompt,
	change_sysdev_dump_prompt,
	open_sysdev_dump_prompt,
	T_SDDUMP
};

/* Dump Device Name Field
 *
 */
#define display_sysdev_dump_field display_text_field
#define change_sysdev_dump_field change_text_field
#define open_sysdev_dump_field flash

static TEXT_FIELD sysdev_dump_field = {
	40,
	10,
	display_sysdev_dump_field,
	change_sysdev_dump_field,
	open_sysdev_dump_field,
	&ssysdev.s_dump
};

/* Swap Device Name Prompt
 *
 */
#define display_sysdev_swap_prompt display_token_field
#define change_sysdev_swap_prompt flash
#define open_sysdev_swap_prompt flash

static TOKEN_FIELD sysdev_swap_prompt = {
	RIGHT(S_SDSWAP),
	LENGTH(S_SDSWAP),
	display_sysdev_swap_prompt,
	change_sysdev_swap_prompt,
	open_sysdev_swap_prompt,
	T_SDSWAP
};

/* Swap Device Name Field
 *
 */
#define display_sysdev_swap_field display_text_field
#define change_sysdev_swap_field change_text_field
#define open_sysdev_swap_field flash

static TEXT_FIELD sysdev_swap_field = {
	40,
	10,
	display_sysdev_swap_field,
	change_sysdev_swap_field,
	open_sysdev_swap_field,
	&ssysdev.s_swap
};

/* Minor Device Number Sets Form
 *
 */
#define MINTBL_FORM_PROMPT "Minor Device Number Sets Sub-Form"

#define display_sysdev_mintbl_prompt display_prompt_field
#define change_sysdev_mintbl_prompt flash

static void open_sysdev_mintbl_prompt()
{
	create_context(((SYSDEV *)acp->c_tbl)->s_mintbl, &mintbl_form);
}

static PROMPT_FIELD sysdev_minor_prompt = {
	CENTER(MINTBL_FORM_PROMPT),
	LENGTH(MINTBL_FORM_PROMPT),
	display_sysdev_mintbl_prompt,
	change_sysdev_mintbl_prompt,
	open_sysdev_mintbl_prompt,
	MINTBL_FORM_PROMPT
};

/* Define Form
 *
 */
#define quit_sysdev_form quit_sub_context

#define add_sysdev_form_line flash
#define del_sysdev_form_line flash
#define dup_sysdev_form_line flash

static FIELD *sysdev_root_fields[] = {
	ISFIELD(sysdev_root_prompt),
	ISFIELD(sysdev_root_field)
};

static FIELD *sysdev_pipe_fields[] = {
	ISFIELD(sysdev_pipe_prompt),
	ISFIELD(sysdev_pipe_field)
};

static FIELD *sysdev_dump_fields[] = {
	ISFIELD(sysdev_dump_prompt),
	ISFIELD(sysdev_dump_field)
};

static FIELD *sysdev_swap_fields[] = {
	ISFIELD(sysdev_swap_prompt),
	ISFIELD(sysdev_swap_field)
};

static FIELD *sysdev_minor_prompts[] = {
	ISFIELD(sysdev_minor_prompt)
};

static LINE sysdev_form_lines[] = {
	{ 0, 2, sysdev_root_fields },
	{ 1, 2, sysdev_pipe_fields },
	{ 2, 2, sysdev_dump_fields },
	{ 3, 2, sysdev_swap_fields },
	{ 5, 1, sysdev_minor_prompts }
};

static FORM sysdev_form = {
	CP(&ssysdev),
	NULL,
	quit_sysdev_form,
	6,
	add_sysdev_form_line,
	del_sysdev_form_line,
	dup_sysdev_form_line,
	sysdev_form_lines
};


/* CONFIGURATION FILE TABLE SYSTEM DEVICES FORM
 *
 */

/* Root Device Name Field
 *
 */
#define display_cftbl_sysdev_root_field display_text_field
#define change_cftbl_sysdev_root_field flash
#define open_cftbl_sysdev_root_field flash

static TEXT_FIELD cftbl_sysdev_root_field = {
	40,
	10,
	display_cftbl_sysdev_root_field,
	change_cftbl_sysdev_root_field,
	open_cftbl_sysdev_root_field,
	&ssysdev.s_root
};

/* Pipe Device Name Field
 *
 */
#define display_cftbl_sysdev_pipe_field display_text_field
#define change_cftbl_sysdev_pipe_field flash
#define open_cftbl_sysdev_pipe_field flash

static TEXT_FIELD cftbl_sysdev_pipe_field = {
	40,
	10,
	display_cftbl_sysdev_pipe_field,
	change_cftbl_sysdev_pipe_field,
	open_cftbl_sysdev_pipe_field,
	&ssysdev.s_pipe
};

/* Dump Device Name Field
 *
 */
#define display_cftbl_sysdev_dump_field display_text_field
#define change_cftbl_sysdev_dump_field flash
#define open_cftbl_sysdev_dump_field flash

static TEXT_FIELD cftbl_sysdev_dump_field = {
	40,
	10,
	display_cftbl_sysdev_dump_field,
	change_cftbl_sysdev_dump_field,
	open_cftbl_sysdev_dump_field,
	&ssysdev.s_dump
};

/* Swap Device Name Field
 *
 */
#define display_cftbl_sysdev_swap_field display_text_field
#define change_cftbl_sysdev_swap_field flash
#define open_cftbl_sysdev_swap_field flash

static TEXT_FIELD cftbl_sysdev_swap_field = {
	40,
	10,
	display_cftbl_sysdev_swap_field,
	change_cftbl_sysdev_swap_field,
	open_cftbl_sysdev_swap_field,
	&ssysdev.s_swap
};

/* Minor Device Number Sets Form
 *
 */
#define display_cftbl_sysdev_mintbl_prompt display_prompt_field
#define change_cftbl_sysdev_mintbl_prompt flash

static void open_cftbl_sysdev_mintbl_prompt()
{
	create_context(((SYSDEV *)acp->c_tbl)->s_mintbl, &cftbl_mintbl_form);
}

static PROMPT_FIELD cftbl_sysdev_mintbl_prompt = {
	CENTER(MINTBL_FORM_PROMPT),
	LENGTH(MINTBL_FORM_PROMPT),
	display_cftbl_sysdev_mintbl_prompt,
	change_cftbl_sysdev_mintbl_prompt,
	open_cftbl_sysdev_mintbl_prompt,
	MINTBL_FORM_PROMPT
};

/* Other Fields
 *
 */
#define cftbl_sysdev_root_prompt sysdev_root_prompt 
#define cftbl_sysdev_pipe_prompt sysdev_pipe_prompt 
#define cftbl_sysdev_dump_prompt sysdev_dump_prompt 
#define cftbl_sysdev_swap_prompt sysdev_swap_prompt 

/* Define Form
 *
 */
#define quit_cftbl_sysdev_form quit_sub_context

#define add_cftbl_sysdev_form_line flash
#define del_cftbl_sysdev_form_line flash
#define dup_cftbl_sysdev_form_line flash

static FIELD *cftbl_sysdev_root_fields[] = {
	ISFIELD(cftbl_sysdev_root_prompt),
	ISFIELD(cftbl_sysdev_root_field)
};

static FIELD *cftbl_sysdev_pipe_fields[] = {
	ISFIELD(cftbl_sysdev_pipe_prompt),
	ISFIELD(cftbl_sysdev_pipe_field)
};

static FIELD *cftbl_sysdev_dump_fields[] = {
	ISFIELD(cftbl_sysdev_dump_prompt),
	ISFIELD(cftbl_sysdev_dump_field)
};

static FIELD *cftbl_sysdev_swap_fields[] = {
	ISFIELD(cftbl_sysdev_swap_prompt),
	ISFIELD(cftbl_sysdev_swap_field)
};

static FIELD *cftbl_sysdev_mintbl_prompts[] = {
	ISFIELD(cftbl_sysdev_mintbl_prompt)
};

static LINE cftbl_sysdev_form_lines[] = {
	{ 0, 2, cftbl_sysdev_root_fields },
	{ 1, 2, cftbl_sysdev_pipe_fields },
	{ 2, 2, cftbl_sysdev_dump_fields },
	{ 3, 2, cftbl_sysdev_swap_fields },
	{ 5, 1, cftbl_sysdev_mintbl_prompts }
};

static FORM cftbl_sysdev_form = {
	CP(&ssysdev),
	NULL,
	quit_cftbl_sysdev_form,
	6,
	add_cftbl_sysdev_form_line,
	del_cftbl_sysdev_form_line,
	dup_cftbl_sysdev_form_line,
	cftbl_sysdev_form_lines
};


/* SYSTEM DEVICES TABLE MANAGEMENT
 *
 */

/* Allocate System Devices Table
 *
 */
TABLE *create_sdtbl()
{
	return(alloc_sysdev_table());
}

/* Output System Devices Table
 *
 */
void write_sdtbl(file, sdtp)
FILE *file;
register TABLE *sdtp;
{
	void write_sysdev();
	register int i;

	for (i = 0; i < sdtp->tbl_count; i++)
		write_sysdev(file, get_sysdev_table_entry(sdtp, i));
}

/* Output Dfile(4), Second Part
 *
 */
void out_sdtbl(file, sdtp)
FILE *file;
TABLE *sdtp;
{
	out_sysdev(file, sdtp, "root", &ssysdev.s_root, &sminor.m_root, 0);
	out_sysdev(file, sdtp, "pipe", &ssysdev.s_pipe, &sminor.m_pipe, 0);
	out_sysdev(file, sdtp, "dump", &ssysdev.s_dump, &sminor.m_dump, 0);
	out_sysdev(file, sdtp, "swap", &ssysdev.s_swap, &sminor.m_swap, -1);
}


/* SYSTEM DEVICES TABLE FORM
 *
 */

/* System Devices Form Prompt
 *
 */
#define SYSDEV_FORM_PROMPT "System Devices Set #"

static void display_sdtbl_sysdev_prompt(fieldp, data, scrlin, cp)
FIELD *fieldp;
char *data;
int scrlin;
CONTEXT *cp;
{
	register char *buf, **dataptr;

	dataptr = cp->c_sdtbl->tbl_ptr;
	while (*dataptr != data)
		++dataptr;

	buf = mkbuf(BUFSIZE);
	sprintf(buf, "%s%d", SYSDEV_FORM_PROMPT,
			dataptr - cp->c_sdtbl->tbl_ptr);
	display_field(fieldp, buf, scrlin);
	free(buf);
}

#define change_sdtbl_sysdev_prompt flash

static void open_sdtbl_sysdev_prompt()
{
	create_context(get_sysdev_table_entry(acp->c_sdtbl, acp->c_curline),
			&sysdev_form);
}

static SIMPLE_FIELD sdtbl_sysdev_prompt = {
	CENTER(SYSDEV_FORM_PROMPT),
	LENGTH(SYSDEV_FORM_PROMPT) + 3,
	display_sdtbl_sysdev_prompt,
	change_sdtbl_sysdev_prompt,
	open_sdtbl_sysdev_prompt
};

/* Define Form
 *
 */
#define quit_sdtbl_form quit_sub_context

static void add_sdtbl_form_line()
{
	insert_entry_in_sysdev_table(acp->c_sdtbl, acp->c_curline,
		create_sysdev(null, null, null, null));
	display_form_body();
	update_context();
}

static void del_sdtbl_form_line()
{
	remove_sysdev_table_entry(acp->c_sdtbl, acp->c_curline);
	display_form_body();
	update_context();
}

#define dup_sdtbl_form_line flash

static FIELD *sdtbl_sysdev_prompts[] = {
	ISFIELD(sdtbl_sysdev_prompt)
};

static LINE sdtbl_form_lines[] = {
	{ 0, 1, sdtbl_sysdev_prompts }
};

FORM sdtbl_form = {
	CP(&ssysdev),
	NULL,
	quit_sdtbl_form,
	0,
	add_sdtbl_form_line,
	del_sdtbl_form_line,
	dup_sdtbl_form_line,
	sdtbl_form_lines
};


/* CONFIGURATION FILE TABLE SYSTEM DEVICES TABLE FORM
 *
 */

/* System Devices Form Prompt
 *
 */
#define display_cftbl_sdtbl_sysdev_prompt display_sdtbl_sysdev_prompt
#define change_cftbl_sdtbl_sysdev_prompt flash

static void open_cftbl_sdtbl_sysdev_prompt()
{
	create_context(get_sysdev_table_entry(acp->c_sdtbl, acp->c_curline),
			&cftbl_sysdev_form);
}

static SIMPLE_FIELD cftbl_sdtbl_sysdev_prompt = {
	CENTER(SYSDEV_FORM_PROMPT),
	LENGTH(SYSDEV_FORM_PROMPT) + 3,
	display_cftbl_sdtbl_sysdev_prompt,
	change_cftbl_sdtbl_sysdev_prompt,
	open_cftbl_sdtbl_sysdev_prompt
};

/* Define Form
 *
 */
#define quit_cftbl_sdtbl_form quit_sub_context

#define add_cftbl_sdtbl_form_line flash
#define del_cftbl_sdtbl_form_line flash
#define dup_cftbl_sdtbl_form_line flash

static FIELD *cftbl_sdtbl_sysdev_prompts[] = {
	ISFIELD(cftbl_sdtbl_sysdev_prompt)
};

static LINE cftbl_sdtbl_form_lines[] = {
	{ 0, 1, cftbl_sdtbl_sysdev_prompts }
};

FORM cftbl_sdtbl_form = {
	CP(&ssysdev),
	NULL,
	quit_cftbl_sdtbl_form,
	0,
	add_cftbl_sdtbl_form_line,
	del_cftbl_sdtbl_form_line,
	dup_cftbl_sdtbl_form_line,
	cftbl_sdtbl_form_lines
};
