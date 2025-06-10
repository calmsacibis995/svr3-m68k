/* sgmiscdesc.c	%W%
 *
 */
#include "sysgen.h"
#include "sgyacc.h"
#include "sgkey.h"
#include "sgform.h"


/* PARAMETER DATA STRUCTURE
 *
 */
typedef struct {
	char	*p_name;		/* parameter name		*/
	char	*p_conf;		/* conf.c name			*/
	char	*p_dval;		/* default value		*/
} PARAM;

static PARAM parambuf[PARAM_COUNT];

STRUCT_LIST param_slist = {
	sizeof(PARAM),
	0,
	0,
	PARAM_COUNT,
	STRUCTP(parambuf)
};

#define alloc_param() ((PARAM *)alloc_struct(&param_slist))

PARAM sparam;				/* sample parameter structure	*/

/* Create New Parameter
 *
 */
char *new_param(name, conf, dval)
char *name, *conf, *dval;
{
	register PARAM *paramp;

	paramp = alloc_param();

	paramp->p_name = name;
	paramp->p_conf = conf;
	paramp->p_dval = dval;

	return(CP(paramp));
}

/* Create New Parameter Description
 *
 */
PARAM *create_param(name, cname, value)
char *name, *cname, *value;
{
	register PARAM *paramp;

	paramp = alloc_param();

	paramp->p_name = create_string(name);
	paramp->p_conf = create_string(cname);
	paramp->p_dval = create_string(value);

	return(paramp);
}

/* Duplicate Parameter Description
 *
 */
PARAM *dup_param(paramp)
register PARAM *paramp;
{
	register PARAM *nparamp;

	nparamp = alloc_param();

	nparamp->p_name = create_string(paramp->p_name);
	nparamp->p_conf = create_string(paramp->p_conf);
	nparamp->p_dval = create_string(paramp->p_dval);

	return(nparamp);
}

/* De-Allocate Parameter Description
 *
 */
void free_param(paramp)
register PARAM *paramp;
{
	free_string(paramp->p_name);
	free_string(paramp->p_conf);
	free_string(paramp->p_dval);
	free_struct(&param_slist, paramp);
}

/* Output Parameter Description
 *
 */
void write_param(file, paramp)
FILE *file;
PARAM *paramp;
{
	static FORMAT param_format[] = {
		{ FMT_STRING, T_PARNAME, CP(&sparam.p_name) },
		{ FMT_STRING, T_PARCONF, CP(&sparam.p_conf) },
		{ FMT_STRING, T_PARDVAL, CP(&sparam.p_dval) },
		{ FMT_END,    0,	 NULL		    }
	};

	extern void write_format();

	write_format(file, param_format, paramp, &sparam);
}

/* Output Dfile(4), Second Part, Parameter Specification
 *
 */
void out_dfile_param(file, paramp, cip)
FILE *file;
PARAM *paramp;
CONFIG_ITEM *cip;
{
	if (cip->ci_assn)
		fprintf(file, "%-18s%s\n", paramp->p_name, cip->ci_string);
}

/* Output Master(4), Part 3
 *
 */
void out_master_param(file, paramp)
FILE *file;
PARAM *paramp;
{
	fprintf(file, (*paramp->p_dval) ? "%-18s%-18s%s\n" : "%-18s%-18s\n",
		paramp->p_name, paramp->p_conf, paramp->p_dval);
}


/* PARAMETER FORM
 *
 */

/* Name Prompt
 *
 */
#define display_param_name_prompt display_token_field
#define change_param_name_prompt flash
#define open_param_name_prompt flash

static TOKEN_FIELD param_name_prompt = {
	RIGHT(S_PARNAME),
	LENGTH(S_PARNAME),
	display_param_name_prompt,
	change_param_name_prompt,
	open_param_name_prompt,
	T_PARNAME
};

/* Name Field
 *
 */
#define display_param_name_field display_text_field
#define change_param_name_field change_text_field
#define open_param_name_field flash

static TEXT_FIELD param_name_field = {
	40,
	16,
	display_param_name_field,
	change_param_name_field,
	open_param_name_field,
	&sparam.p_name
};

/* Conf.c Name Prompt
 *
 */
#define display_param_conf_prompt display_token_field
#define change_param_conf_prompt flash
#define open_param_conf_prompt flash

static TOKEN_FIELD param_conf_prompt = {
	RIGHT(S_PARCONF),
	LENGTH(S_PARCONF),
	display_param_conf_prompt,
	change_param_conf_prompt,
	open_param_conf_prompt,
	T_PARCONF
};

/* Conf.c Name Field
 *
 */
#define display_param_conf_field display_text_field
#define change_param_conf_field change_text_field
#define open_param_conf_field flash

static TEXT_FIELD param_conf_field = {
	40,
	16,
	display_param_conf_field,
	change_param_conf_field,
	open_param_conf_field,
	&sparam.p_conf
};

/* Default Value Prompt
 *
 */
#define display_param_dvalue_prompt display_token_field
#define change_param_dvalue_prompt flash
#define open_param_dvalue_prompt flash

static TOKEN_FIELD param_dvalue_prompt = {
	RIGHT(S_PARDVAL),
	LENGTH(S_PARDVAL),
	display_param_dvalue_prompt,
	change_param_dvalue_prompt,
	open_param_dvalue_prompt,
	T_PARDVAL
};

/* Default Value Field
 *
 */
#define display_param_dvalue_field display_text_field
#define change_param_dvalue_field change_text_field
#define open_param_dvalue_field flash

static TEXT_FIELD param_dvalue_field = {
	40,
	30,
	display_param_dvalue_field,
	change_param_dvalue_field,
	open_param_dvalue_field,
	&sparam.p_dval
};

/* Define Form
 *
 */
#define quit_param_form quit_sub_context

#define add_param_form_line flash
#define del_param_form_line flash
#define dup_param_form_line flash

static FIELD *param_name_fields[] = {
	ISFIELD(param_name_prompt),
	ISFIELD(param_name_field)
};
static FIELD *param_conf_fields[] = {
	ISFIELD(param_conf_prompt),
	ISFIELD(param_conf_field)
};

static FIELD *param_dvalue_fields[] = {
	ISFIELD(param_dvalue_prompt),
	ISFIELD(param_dvalue_field)
};

static LINE param_form_lines[] = {
	{ 0, 2, param_name_fields },
	{ 2, 2, param_conf_fields },
	{ 3, 2, param_dvalue_fields }
};

FORM param_form = {
	CP(&sparam),
	NULL,
	quit_param_form,
	4,
	add_param_form_line,
	del_param_form_line,
	dup_param_form_line,
	param_form_lines
};


/* CONFIGURATION FILE TABLE PARAMETER FORM
 *
 */

/* Name Prompt
 *
 */
#define cftbl_param_name_prompt param_name_prompt

/* Name Field
 *
 */
#define display_cftbl_param_name_field display_text_field
#define change_cftbl_param_name_field flash
#define open_cftbl_param_name_field flash

static TEXT_FIELD cftbl_param_name_field = {
	40,
	16,
	display_cftbl_param_name_field,
	change_cftbl_param_name_field,
	open_cftbl_param_name_field,
	&sparam.p_name
};

/* Conf.c Name Prompt
 *
 */
#define cftbl_param_conf_prompt param_conf_prompt

/* Conf.c Name Field
 *
 */
#define display_cftbl_param_conf_field display_text_field
#define change_cftbl_param_conf_field flash
#define open_cftbl_param_conf_field flash

static TEXT_FIELD cftbl_param_conf_field = {
	40,
	16,
	display_cftbl_param_conf_field,
	change_cftbl_param_conf_field,
	open_cftbl_param_conf_field,
	&sparam.p_conf
};

/* Default Value Prompt
 *
 */
#define cftbl_param_dvalue_prompt param_dvalue_prompt

/* Default Value Field
 *
 */
#define display_cftbl_param_dvalue_field display_text_field
#define change_cftbl_param_dvalue_field flash
#define open_cftbl_param_dvalue_field flash

static TEXT_FIELD cftbl_param_dvalue_field = {
	40,
	30,
	display_cftbl_param_dvalue_field,
	change_cftbl_param_dvalue_field,
	open_cftbl_param_dvalue_field,
	&sparam.p_dval
};

/* Value Prompt
 *
 */
#define display_cftbl_param_value_prompt display_token_field
#define change_cftbl_param_value_prompt flash
#define open_cftbl_param_value_prompt flash

static TOKEN_FIELD cftbl_param_value_prompt = {
	RIGHT(S_CNFGVALU),
	LENGTH(S_CNFGVALU),
	display_cftbl_param_value_prompt,
	change_cftbl_param_value_prompt,
	open_cftbl_param_value_prompt,
	T_CNFGVALU
};

/* Value Field
 *
 */
void display_cftbl_param_value_field(fieldp, paramp, scrlin)
FIELD *fieldp;
PARAM *paramp;
int scrlin;
{
	register CONFIG_ITEM *cip;

	display_field(fieldp, (((cip = (CONFIG_ITEM *)acp->c_other) != NULL) &&
		(cip->ci_assn)) ? cip->ci_string : paramp->p_dval, scrlin);
}

void change_cftbl_param_value_field()
{
	register CONFIG_ITEM *cip;
	register char *newstring;
	register CONTEXT *cp;
	register int type, newnum;

	if ((cip = (CONFIG_ITEM *)acp->c_other) == NULL) {
		flash();
botmsg("This description must be selected before a value may be assigned.");
		return;
	}

	if (!cip->ci_assn)
		cip->ci_string = create_string(null);

	newstring = change_field(acp->c_field, cip->ci_string,
				acp->c_scrline, &textacs);

	if (newstring != NULL) {
		free_string(cip->ci_string);
		cip->ci_assn = CIT_STRING;
		cip->ci_string = newstring;
		--acp->c_modified;
	} else if (!cip->ci_assn)
		free_string(cip->ci_string);

	update_context();
}

#define open_cftbl_param_value_field flash

static SIMPLE_FIELD cftbl_param_value_field = {
	40,
	30,
	display_cftbl_param_value_field,
	change_cftbl_param_value_field,
	open_cftbl_param_value_field,
};

/* Define Form
 *
 */
#define quit_cftbl_param_form quit_sub_context

#define add_cftbl_param_form_line flash
#define del_cftbl_param_form_line flash
#define dup_cftbl_param_form_line flash

static FIELD *cftbl_param_name_fields[] = {
	ISFIELD(cftbl_param_name_prompt),
	ISFIELD(cftbl_param_name_field)
};

static FIELD *cftbl_param_conf_fields[] = {
	ISFIELD(cftbl_param_conf_prompt),
	ISFIELD(cftbl_param_conf_field)
};

static FIELD *cftbl_param_dvalue_fields[] = {
	ISFIELD(cftbl_param_dvalue_prompt),
	ISFIELD(cftbl_param_dvalue_field)
};

static FIELD *cftbl_param_value_fields[] = {
	ISFIELD(cftbl_param_value_prompt),
	ISFIELD(cftbl_param_value_field)
};

static LINE cftbl_param_form_lines[] = {
	{ 0, 2, cftbl_param_name_fields },
	{ 2, 2, cftbl_param_conf_fields },
	{ 3, 2, cftbl_param_dvalue_fields },
	{ 4, 2, cftbl_param_value_fields }
};

FORM cftbl_param_form = {
	CP(&sparam),
	NULL,
	quit_cftbl_param_form,
	5,
	add_cftbl_param_form_line,
	del_cftbl_param_form_line,
	dup_cftbl_param_form_line,
	cftbl_param_form_lines
};


/* MICROPROCESSOR SPECIFICATION DATA STRUCTURE
 *
 */
typedef struct {
	int	mpu_num;		/* mpu number			*/
} MPU;

static MPU mpubuf[MPU_COUNT];

STRUCT_LIST mpu_slist = {
	sizeof(MPU),
	0,
	0,
	MPU_COUNT,
	STRUCTP(mpubuf)
};

#define alloc_mpu() ((MPU *)alloc_struct(&mpu_slist))

MPU smpu;				/* sample mpu structure		*/

/* Create New MPU
 *
 */
char *new_mpu(num)
int num;
{
	register MPU *mpup;

	mpup = alloc_mpu();
	mpup->mpu_num = num;
	return(CP(mpup));
}

/* Create MPU Specification
 *
 */
MPU *create_mpu()
{
	return(alloc_mpu());
}

/* Duplicate MPU Specification
 *
 */
MPU *dup_mpu(mpup)
MPU *mpup;
{
	register MPU *nmpu;

	nmpu = alloc_mpu();
	nmpu->mpu_num = mpup->mpu_num;
	return(nmpu);
}

/* De-Allocate MPU Description
 *
 */
void free_mpu(mpup)
MPU *mpup;
{
	free_struct(&mpu_slist, mpup);
}

/* Output MPU Description
 *
 */
void write_mpu(file, mpup)
FILE *file;
MPU *mpup;
{
	static FORMAT mpu_format[] = {
		{ FMT_DECIMAL,	T_MPUNUM, CP(&smpu.mpu_num) },
		{ FMT_END,	0,	  NULL		    }
	};

	extern void write_format();

	write_format(file, mpu_format, mpup, &smpu);
}

/* Output Master(4), Part 4
 *
 */
void out_mpu(file, mpup)
FILE *file;
MPU *mpup;
{
	fprintf(file, "mpu %d\n", mpup->mpu_num);
}


/* MPU FORM
 *
 */

/* Processor Number Prompt
 *
 */
#define display_mpu_num_prompt display_token_field
#define change_mpu_num_prompt flash
#define open_mpu_num_prompt flash

static TOKEN_FIELD mpu_num_prompt = {
	RIGHT(S_MPUNUM),
	LENGTH(S_MPUNUM),
	display_mpu_num_prompt,
	change_mpu_num_prompt,
	open_mpu_num_prompt,
	T_MPUNUM
};

/* Processor Number Field
 *
 */
#define display_mpu_num_field display_numeric_field
#define change_mpu_num_field change_numeric_field
#define open_mpu_num_field flash

static NUMERIC_FIELD mpu_num_field = {
	40,
	5,
	display_mpu_num_field,
	change_mpu_num_field,
	open_mpu_num_field,
	&smpu.mpu_num,
	DECIMAL_FIELD
};

/* Define Form
 *
 */
#define quit_mpu_form quit_sub_context

#define add_mpu_form_line flash
#define del_mpu_form_line flash
#define dup_mpu_form_line flash

static FIELD *mpu_num_fields[] = {
	ISFIELD(mpu_num_prompt),
	ISFIELD(mpu_num_field)
};

static LINE mpu_form_lines[] = {
	{ 0, 2, mpu_num_fields }
};

FORM mpu_form = {
	CP(&smpu),
	NULL,
	quit_mpu_form,
	1,
	add_mpu_form_line,
	del_mpu_form_line,
	dup_mpu_form_line,
	mpu_form_lines
};


/* CONFIGURATION FILE TABLE MPU FORM
 *
 */

/* Processor Number
 *
 */
#define cftbl_mpu_num_prompt mpu_num_prompt

/* Processor Number Field
 *
 */
#define display_cftbl_mpu_num_field display_numeric_field
#define change_cftbl_mpu_num_field flash
#define open_cftbl_mpu_num_field flash

static NUMERIC_FIELD cftbl_mpu_num_field = {
	40,
	5,
	display_cftbl_mpu_num_field,
	change_cftbl_mpu_num_field,
	open_cftbl_mpu_num_field,
	&smpu.mpu_num
};

/* Define Form
 *
 */
#define quit_cftbl_mpu_form quit_sub_context

#define add_cftbl_mpu_form_line flash
#define del_cftbl_mpu_form_line flash
#define dup_cftbl_mpu_form_line flash

static FIELD *cftbl_mpu_num_fields[] = {
	ISFIELD(cftbl_mpu_num_prompt),
	ISFIELD(cftbl_mpu_num_field)
};

static LINE cftbl_mpu_form_lines[] = {
	{ 0, 2, cftbl_mpu_num_fields }
};

FORM cftbl_mpu_form = {
	CP(&smpu),
	NULL,
	quit_cftbl_mpu_form,
	1,
	add_cftbl_mpu_form_line,
	del_cftbl_mpu_form_line,
	dup_cftbl_mpu_form_line,
	cftbl_mpu_form_lines
};


/* NON-UNIQUE DRIVER SPECIFICATION DATA STRUCTURE
 *
 */
typedef struct {
	char	*force_id;		/* unique identifier		*/
} FORCE;

static FORCE forcebuf[FORCE_COUNT];

STRUCT_LIST force_slist = {
	sizeof(FORCE),
	0,
	0,
	FORCE_COUNT,
	STRUCTP(forcebuf)
};

#define alloc_force() ((FORCE *)alloc_struct(&force_slist))

FORCE sforce;				/* sample force structure	*/


/* New Non-Unique Driver Specification
 *
 */
char *new_force(id)
char *id;
{
	register FORCE *forcep;

	forcep = alloc_force();
	forcep->force_id = id;
	return(CP(forcep));
}

/* Create Non-uniqe Driver Specification
 *
 */
FORCE *create_force(id)
char *id;
{
	register FORCE *fp;

	fp = alloc_force();
	fp->force_id = create_string(id);
	return(fp);
}

/* Duplicate Non-uniqe Driver Specification
 *
 */
FORCE *dup_force(fp)
register FORCE *fp;
{
	register FORCE *nfp;

	nfp = alloc_force();
	nfp->force_id = create_string(fp->force_id);
	return(nfp);
}

/* De-Allocate Non-Unique Driver Specification
 *
 */
void free_force(fp)
FORCE *fp;
{
	free_string(fp->force_id);
	free_struct(&force_slist, fp);
}

/* Output Non-uniqe Driver Specification
 *
 */
void write_force(file, fp)
FILE *file;
FORCE *fp;
{
	static FORMAT forceformat[] = {
		{ FMT_STRING, T_FRCID,	CP(&sforce.force_id) },
		{ FMT_END,    0,	NULL		     }
	};

	extern void write_format();

	write_format(file, forceformat, fp, &sforce);
}

/* Output Dfile(4), Third Part, Item 1
 *
 */
void out_force(file, forcep)
FILE *file;
FORCE *forcep;
{
	fprintf(file, "force %s\n", forcep->force_id);
}


/* NON-UNIQUE DRIVER FORM
 *
 */

/* ID Prompt
 *
 */
#define display_force_id_prompt display_token_field
#define change_force_id_prompt flash
#define open_force_id_prompt flash

static TOKEN_FIELD force_id_prompt = {
	RIGHT(S_FRID),
	LENGTH(S_FRID),
	display_force_id_prompt,
	change_force_id_prompt,
	open_force_id_prompt,
	T_FRCID
};

/* ID Field
 *
 */
#define display_force_id_field display_text_field
#define change_force_id_field change_text_field
#define open_force_id_field flash

static TEXT_FIELD force_id_field = {
	40,
	10,
	display_force_id_field,
	change_force_id_field,
	open_force_id_field,
	&sforce.force_id
};

/* Define Form
 *
 */
#define quit_force_form quit_sub_context

#define add_force_form_line flash
#define del_force_form_line flash
#define dup_force_form_line flash

static FIELD *force_id_fields[] = {
	ISFIELD(force_id_prompt),
	ISFIELD(force_id_field)
};

static LINE force_form_lines[] = {
	{ 0, 2, force_id_fields }
};

FORM force_form = {
	CP(&sforce),
	NULL,
	quit_force_form,
	1,
	add_force_form_line,
	del_force_form_line,
	dup_force_form_line,
	force_form_lines
};


/* CONFIGURATION FILE TABLE NON-UNIQUE DRIVER FORM
 *
 */

/* ID Prompt
 *
 */
#define cftbl_force_id_prompt force_id_prompt

/* ID Field
 *
 */
#define display_cftbl_force_id_field display_text_field
#define change_cftbl_force_id_field flash
#define open_cftbl_force_id_field flash

static TEXT_FIELD cftbl_force_id_field = {
	40,
	10,
	display_cftbl_force_id_field,
	change_cftbl_force_id_field,
	open_cftbl_force_id_field,
	&sforce.force_id
};

/* Define Form
 *
 */
#define quit_cftbl_force_form quit_sub_context

#define add_cftbl_force_form_line flash
#define del_cftbl_force_form_line flash
#define dup_cftbl_force_form_line flash

static FIELD *cftbl_force_id_fields[] = {
	ISFIELD(cftbl_force_id_prompt),
	ISFIELD(cftbl_force_id_field)
};

static LINE cftbl_force_form_lines[] = {
	{ 0, 2, cftbl_force_id_fields }
};

FORM cftbl_force_form = {
	CP(&sforce),
	NULL,
	quit_cftbl_force_form,
	1,
	add_cftbl_force_form_line,
	del_cftbl_force_form_line,
	dup_cftbl_force_form_line,
	cftbl_force_form_lines
};


/* MEMORY PROBE SPECIFICATION DATA STRUCTURE
 *
 */
typedef struct {
	int	probe_addr;		/* probe address		*/
	int	probe_value;		/* probe value			*/
} PROBE;

static PROBE probebuf[PROBE_COUNT];

STRUCT_LIST probe_slist = {
	sizeof(PROBE),
	0,
	0,
	PROBE_COUNT,
	STRUCTP(probebuf)
};

#define alloc_probe() ((PROBE *)alloc_struct(&probe_slist))

PROBE sprobe;				/* sample probe structure	*/

/* Create Memory Probe Specification
 *
 */
char *new_probe(addr, value)
int addr, value;
{
	register PROBE *probep;

	probep = alloc_probe();

	probep->probe_addr = addr;
	probep->probe_value = value;

	return(CP(probep));
}

/* Create Memory Probe Specification
 *
 */
PROBE *create_probe()
{
	return(alloc_probe());
}

/* Duplicate Memory Probe Specification
 *
 */
PROBE *dup_probe(probep)
register PROBE *probep;
{
	register PROBE *nprobep;

	nprobep = alloc_probe();

	nprobep->probe_addr  = probep->probe_addr;
	nprobep->probe_value = probep->probe_value;

	return(nprobep);
}

/* De-Allocate Memory Probe Specification
 *
 */
void free_probe(probep)
PROBE *probep;
{
	free_struct(&probe_slist, probep);
}

/* Output Memory Probe Specification
 *
 */
void write_probe(file, probep)
FILE *file;
PROBE *probep;
{
	static FORMAT probeformat[] = {
		{ FMT_HEXADEC, T_PRBADDR, CP(&sprobe.probe_addr)  },
		{ FMT_HEXADEC, T_PRBVAL,  CP(&sprobe.probe_value) },
		{ FMT_END,     0,	  NULL			  }
	};

	extern void write_format();

	write_format(file, probeformat, probep, &sprobe);
}

/* Output Dfile(4), Third Part, Item 2
 *
 */
void out_probe(file, probep)
FILE *file;
PROBE *probep;
{
	fprintf(file, "probe %-10x%x\n",
		probep->probe_addr, probep->probe_value);
}


/* MEMORY PROBE FORM
 *
 */

/* Address Prompt
 *
 */
#define display_probe_address_prompt display_token_field
#define change_probe_address_prompt flash
#define open_probe_address_prompt flash

static TOKEN_FIELD probe_address_prompt = {
	RIGHT(S_PRBADDR),
	LENGTH(S_PRBADDR),
	display_probe_address_prompt,
	change_probe_address_prompt,
	open_probe_address_prompt,
	T_PRBADDR
};

/* Address Field
 *
 */
#define display_probe_address_field display_numeric_field
#define change_probe_address_field change_numeric_field
#define open_probe_address_field flash

static NUMERIC_FIELD probe_address_field = {
	40,
	8,
	display_probe_address_field,
	change_probe_address_field,
	open_probe_address_field,
	&sprobe.probe_addr,
	HEXADECIMAL_FIELD
};

/* Value Prompt
 *
 */
#define display_probe_value_prompt display_token_field
#define change_probe_value_prompt flash
#define open_probe_value_prompt flash

static TOKEN_FIELD probe_value_prompt = {
	RIGHT(S_PRBVAL),
	LENGTH(S_PRBVAL),
	display_probe_value_prompt,
	change_probe_value_prompt,
	open_probe_value_prompt,
	T_PRBVAL
};

/* Value Field
 *
 */
#define display_probe_value_field display_numeric_field
#define change_probe_value_field change_numeric_field
#define open_probe_value_field flash

static NUMERIC_FIELD probe_value_field = {
	40,
	2,
	display_probe_value_field,
	change_probe_value_field,
	open_probe_value_field,
	&sprobe.probe_value
};

/* Define Form
 *
 */
#define quit_probe_form quit_sub_context

#define add_probe_form_line flash
#define del_probe_form_line flash
#define dup_probe_form_line flash

static FIELD *probe_address_fields[] = {
	ISFIELD(probe_address_prompt),
	ISFIELD(probe_address_field)
};

static FIELD *probe_value_fields[] = {
	ISFIELD(probe_value_prompt),
	ISFIELD(probe_value_field)
};

static LINE probe_form_lines[] = {
	{ 0, 2, probe_address_fields },
	{ 1, 2, probe_value_fields }
};

FORM probe_form = {
	CP(&sprobe),
	NULL,
	quit_probe_form,
	2,
	add_probe_form_line,
	del_probe_form_line,
	dup_probe_form_line,
	probe_form_lines
};


/* CONFIGURATION FILE TABLE MEMORY PROBE FORM
 *
 */

/* Address Prompt
 *
 */
#define cftbl_probe_address_prompt probe_address_prompt

/* Address Field
 *
 */
#define display_cftbl_probe_address_field display_numeric_field
#define change_cftbl_probe_address_field flash
#define open_cftbl_probe_address_field flash

static NUMERIC_FIELD cftbl_probe_address_field = {
	40,
	8,
	display_cftbl_probe_address_field,
	change_cftbl_probe_address_field,
	open_cftbl_probe_address_field,
	&sprobe.probe_addr,
	HEXADECIMAL_FIELD
};

/* Value Prompt
 *
 */
#define cftbl_probe_value_prompt probe_value_prompt

/* Value Field
 *
 */
#define display_cftbl_probe_value_field display_numeric_field
#define change_cftbl_probe_value_field flash
#define open_cftbl_probe_value_field flash

static NUMERIC_FIELD cftbl_probe_value_field = {
	40,
	2,
	display_cftbl_probe_value_field,
	change_cftbl_probe_value_field,
	open_cftbl_probe_value_field,
	&sprobe.probe_value,
	HEXADECIMAL_FIELD
};

/* Define Form
 *
 */
#define quit_cftbl_probe_form quit_sub_context

#define add_cftbl_probe_form_line flash
#define del_cftbl_probe_form_line flash
#define dup_cftbl_probe_form_line flash

static FIELD *cftbl_probe_address_fields[] = {
	ISFIELD(cftbl_probe_address_prompt),
	ISFIELD(cftbl_probe_address_field)
};

static FIELD *cftbl_probe_value_fields[] = {
	ISFIELD(cftbl_probe_value_prompt),
	ISFIELD(cftbl_probe_value_field)
};

static LINE cftbl_probe_form_lines[] = {
	{ 0, 2, cftbl_probe_address_fields },
	{ 1, 2, cftbl_probe_value_fields }
};

FORM cftbl_probe_form = {
	CP(&sprobe),
	NULL,
	quit_cftbl_probe_form,
	2,
	add_cftbl_probe_form_line,
	del_cftbl_probe_form_line,
	dup_cftbl_probe_form_line,
	cftbl_probe_form_lines
};


/* ALIEN HANDLER ENTRY SPECIFICATION DATA STRUCTURE
 *
 */
typedef struct {
	int	alien_vec;		/* normal exception vector	*/
	int	alien_addr;		/* alien entry point		*/
	char	*alien_name;		/* alien handler name		*/
} ALIEN;

static ALIEN alienbuf[ALIEN_COUNT];

STRUCT_LIST alien_slist = {
	sizeof(ALIEN),
	0,
	0,
	ALIEN_COUNT,
	STRUCTP(alienbuf)
};

#define alloc_alien() ((ALIEN *)alloc_struct(&alien_slist))

ALIEN salien;				/* sample alien structure	*/

/* Create Alien Handler Entry Specification
 *
 */
char *new_alien(vec, addr, name)
int vec, addr;
char *name;
{
	register ALIEN *alienp;

	alienp = alloc_alien();

	alienp->alien_vec = vec;
	alienp->alien_addr = addr;
	alienp->alien_name = name;

	return(CP(alienp));
}

/* Create Alien Handler Entry Specification
 *
 */
ALIEN *create_alien(name)
char *name;
{
	register ALIEN *alienp;

	alienp = alloc_alien();
	alienp->alien_name = create_string(name);
	return(alienp);
}

/* Duplicate Alien Handler Entry Specification
 *
 */
ALIEN *dup_alien(alienp)
register ALIEN *alienp;
{
	register ALIEN *nalienp;

	nalienp = alloc_alien();

	nalienp->alien_vec  = alienp->alien_vec;
	nalienp->alien_addr = alienp->alien_addr;
	nalienp->alien_name = create_string(alienp->alien_name);

	return(nalienp);
}

/* De-Allocate Alien Handler Entry Specification
 *
 */
void free_alien(alienp)
register ALIEN *alienp;
{
	free_string(alienp->alien_name);
	free_struct(alien_slist, alienp);
}

/* Output Alien Handler Entry Specification
 *
 */
void write_alien(file, alienp)
FILE *file;
ALIEN *alienp;
{
	static FORMAT alienformat[] = {
		{ FMT_HEXADEC, T_ALVEC,  CP(&salien.alien_vec)	},
		{ FMT_HEXADEC, T_ALADDR, CP(&salien.alien_addr)	},
		{ FMT_STRING,  T_ALNAME, CP(&salien.alien_name)	},
		{ FMT_END,     0,	 NULL			}
	};

	extern void write_format();

	write_format(file, alienformat, alienp, &salien);
}

/* Output Dfile(4), Third Part, Item 3
 *
 */
void out_alien(file, alienp)
FILE *file;
ALIEN *alienp;
{
	fprintf(file, "alien %-10x%-10x%s\n",
		alienp->alien_vec, alienp->alien_addr, alienp->alien_name);
}


/* ALIEN HANDLER FORM
 *
 */

/* Vector Prompt
 *
 */
#define display_alien_vector_prompt display_token_field
#define change_alien_vector_prompt flash
#define open_alien_vector_prompt flash

static TOKEN_FIELD alien_vector_prompt = {
	RIGHT(S_ALVEC),
	LENGTH(S_ALVEC),
	display_alien_vector_prompt,
	change_alien_vector_prompt,
	open_alien_vector_prompt,
	T_ALVEC
};

/* Vector Field
 *
 */
#define display_alien_vector_field display_numeric_field
#define change_alien_vector_field change_numeric_field
#define open_alien_vector_field flash

static NUMERIC_FIELD alien_vector_field = {
	40,
	8,
	display_alien_vector_field,
	change_alien_vector_field,
	open_alien_vector_field,
	&salien.alien_vec,
	HEXADECIMAL_FIELD
};

/* Address Prompt
 *
 */
#define display_alien_address_prompt display_token_field
#define change_alien_address_prompt flash
#define open_alien_address_prompt flash

static TOKEN_FIELD alien_address_prompt = {
	RIGHT(S_ALADDR),
	LENGTH(S_ALADDR),
	display_alien_address_prompt,
	change_alien_address_prompt,
	open_alien_address_prompt,
	T_ALADDR
};

/* Address Field
 *
 */
#define display_alien_address_field display_numeric_field
#define change_alien_address_field change_numeric_field
#define open_alien_address_field flash

static NUMERIC_FIELD alien_address_field = {
	40,
	8,
	display_alien_address_field,
	change_alien_address_field,
	open_alien_address_field,
	&salien.alien_addr,
	HEXADECIMAL_FIELD
};

/* Name Prompt
 *
 */
#define display_alien_name_prompt display_token_field
#define change_alien_name_prompt flash
#define open_alien_name_prompt flash

static TOKEN_FIELD alien_name_prompt = {
	RIGHT(S_ALNAME),
	LENGTH(S_ALNAME),
	display_alien_name_prompt,
	change_alien_name_prompt,
	open_alien_name_prompt,
	T_ALNAME
};

/* Name Field
 *
 */
#define display_alien_name_field display_text_field
#define change_alien_name_field change_text_field
#define open_alien_name_field flash

static TEXT_FIELD alien_name_field = {
	40,
	10,
	display_alien_name_field,
	change_alien_name_field,
	open_alien_name_field,
	&salien.alien_name
};

/* Define Form
 *
 */
#define quit_alien_form quit_sub_context

#define add_alien_form_line flash
#define del_alien_form_line flash
#define dup_alien_form_line flash

static FIELD *alien_vector_fields[] = {
	ISFIELD(alien_vector_prompt),
	ISFIELD(alien_vector_field)
};

static FIELD *alien_address_fields[] = {
	ISFIELD(alien_address_prompt),
	ISFIELD(alien_address_field)
};

static FIELD *alien_name_fields[] = {
	ISFIELD(alien_name_prompt),
	ISFIELD(alien_name_field)
};

static LINE alien_form_lines[] = {
	{ 0, 2, alien_vector_fields  },
	{ 1, 2, alien_address_fields },
	{ 2, 2, alien_name_fields }
};

FORM alien_form = {
	CP(&salien),
	NULL,
	quit_alien_form,
	3,
	add_alien_form_line,
	del_alien_form_line,
	dup_alien_form_line,
	alien_form_lines
};


/* CONFIGURATION FILE TABLE ALIEN HANDLER FORM
 *
 */

/* Vector Prompt
 *
 */
#define cftbl_alien_vector_prompt alien_vector_prompt

/* Vector Field
 *
 */
#define display_cftbl_alien_vector_field display_numeric_field
#define change_cftbl_alien_vector_field flash
#define open_cftbl_alien_vector_field flash

static NUMERIC_FIELD cftbl_alien_vector_field = {
	40,
	8,
	display_cftbl_alien_vector_field,
	change_cftbl_alien_vector_field,
	open_cftbl_alien_vector_field,
	&salien.alien_vec,
	HEXADECIMAL_FIELD
};

/* Address Prompt
 *
 */
#define cftbl_alien_address_prompt alien_address_prompt

/* Address Field
 *
 */
#define display_cftbl_alien_address_field display_numeric_field
#define change_cftbl_alien_address_field flash
#define open_cftbl_alien_address_field flash

static NUMERIC_FIELD cftbl_alien_address_field = {
	40,
	8,
	display_cftbl_alien_address_field,
	change_cftbl_alien_address_field,
	open_cftbl_alien_address_field,
	&salien.alien_addr,
	HEXADECIMAL_FIELD
};

/* Name Prompt
 *
 */
#define cftbl_alien_name_prompt alien_name_prompt

/* Name Field
 *
 */
#define display_cftbl_alien_name_field display_text_field
#define change_cftbl_alien_name_field flash
#define open_cftbl_alien_name_field flash

static TEXT_FIELD cftbl_alien_name_field = {
	40,
	10,
	display_cftbl_alien_name_field,
	change_cftbl_alien_name_field,
	open_cftbl_alien_name_field,
	&salien.alien_name
};

/* Define Form
 *
 */
#define quit_cftbl_alien_form quit_sub_context

#define add_cftbl_alien_form_line flash
#define del_cftbl_alien_form_line flash
#define dup_cftbl_alien_form_line flash

static FIELD *cftbl_alien_vector_fields[] = {
	ISFIELD(cftbl_alien_vector_prompt),
	ISFIELD(cftbl_alien_vector_field)
};

static FIELD *cftbl_alien_address_fields[] = {
	ISFIELD(cftbl_alien_address_prompt),
	ISFIELD(cftbl_alien_address_field)
};

static FIELD *cftbl_alien_name_fields[] = {
	ISFIELD(cftbl_alien_name_prompt),
	ISFIELD(cftbl_alien_name_field)
};

static LINE cftbl_alien_form_lines[] = {
	{ 0, 2, cftbl_alien_vector_fields },
	{ 1, 2, cftbl_alien_address_fields },
	{ 2, 2, cftbl_alien_name_fields }
};

FORM cftbl_alien_form = {
	CP(&salien),
	NULL,
	quit_cftbl_alien_form,
	3,
	add_cftbl_alien_form_line,
	del_cftbl_alien_form_line,
	dup_cftbl_alien_form_line,
	cftbl_alien_form_lines
};


/* MULTIPLE HANDLER SPECIFICATION DATA STRUCTURE
 *
 */
typedef struct {
	int	dup_flag;		/* duplicate flag bit mask	*/
	int	dup_vec;		/* exception vector address	*/
	char	*dup_hdlr;		/* exception handler name	*/
	int	dup_arg;		/* option handler argument	*/
} DUP;

static DUP dupbuf[DUP_COUNT];

STRUCT_LIST dup_slist = {
	sizeof(DUP),
	0,
	0,
	DUP_COUNT,
	STRUCTP(dupbuf)
};

#define alloc_dup() ((DUP *)alloc_struct(&dup_slist))

DUP sdup;				/* sample dup structure		*/

/* Create Multiple Handler Specification
 *
 */
char *new_dup(flag, vec, hdlr, arg)
int flag, vec, arg;
char *hdlr;
{
	register DUP *dupp;

	dupp = alloc_dup();

	dupp->dup_flag = flag;
	dupp->dup_vec  = vec;
	dupp->dup_hdlr = hdlr;
	dupp->dup_arg  = arg;

	return(CP(dupp));
}

/* Create Multiple Handler Specification
 *
 */
DUP *create_dup(handler)
char *handler;
{
	register DUP *dupp;

	dupp = alloc_dup();
	dupp->dup_hdlr = create_string(handler);
	return(dupp);
}

/* Duplicate Multiple Handler Specification
 *
 */
DUP *dup_dup(dupp)
register DUP *dupp;
{
	register DUP *ndupp;

	ndupp = alloc_dup();

	ndupp->dup_flag = dupp->dup_flag;
	ndupp->dup_vec  = dupp->dup_vec;
	ndupp->dup_hdlr = create_string(dupp->dup_hdlr);
	ndupp->dup_arg  = dupp->dup_arg;

	return(ndupp);
}

/* De-Allocate Multiple Handler Specification
 *
 */
void free_dup(dupp)
DUP *dupp;
{
	free_string(dupp->dup_hdlr);
	free_struct(&dup_slist, dupp);
}

/* Output Multiple Handler Specification
 *
 */
void write_dup(file, dupp)
FILE *file;
DUP *dupp;
{
	static BFORMAT dupbformat[] = {
		{ T_DUPINTRET,	 DF_INTRET	},
		{ T_DUPALLTRAPS, DF_ALLTRAPS	},
		{ T_DUPBEQRTS,	 DF_BEQRTS	},
		{ T_DUPARG,	 DF_ARG		},
		{ 0,		 0		}
	};

	static FORMAT dupformat[] = {
		{ FMT_HEXADEC,	T_DUPVEC,  CP(&sdup.dup_vec)	},
		{ FMT_STRING,	T_DUPHDLR, CP(&sdup.dup_hdlr)	},
		{ FMT_END,	0,	   NULL			}
	};

	static FORMAT dupaformat[] = {
		{ FMT_HEXADEC,	T_DUPARGVAL,	CP(&sdup.dup_arg) },
		{ FMT_END,	0,		NULL		  }
	};

	extern void write_format(), write_bit_lines();

	write_bit_lines(file, dupbformat, dupp->dup_flag);
	write_format(file, dupformat, dupp, &sdup);

	if (dupp->dup_flag & DF_ARG)
		write_format(file, dupaformat, dupp, &sdup);
}

/* Output Dfile(4), Third Part, Item 4
 *
 */
void out_dup(file, dupp)
FILE *file;
DUP *dupp;
{
	fprintf(file, "dup %-4o%-10x", dupp->dup_flag, dupp->dup_vec);

	if (dupp->dup_flag & DF_ARG)
		fprintf(file, "%-12s%d\n", dupp->dup_hdlr, dupp->dup_arg);
	else
		fprintf(file, "%s\n", dupp->dup_hdlr);
}


/* MULTIPLE HANDLER FORM
 *
 */

/* Interrupt Return Field
 *
 */
#define display_dup_intret_field display_bit_field
#define change_dup_intret_field change_bit_field
#define open_dup_intret_field flash

static BIT_FIELD dup_intret_field = {
	5,
	LENGTH(S_DUPINTRET),
	display_dup_intret_field,
	change_dup_intret_field,
	open_dup_intret_field,
	T_DUPINTRET,
	DF_INTRET
};

/* All Traps Field
 *
 */
#define display_dup_alltraps_field display_bit_field
#define change_dup_alltraps_field change_bit_field
#define open_dup_alltraps_field flash

static BIT_FIELD dup_alltraps_field = {
	5,
	LENGTH(S_DUPALLTRAPS),
	display_dup_alltraps_field,
	change_dup_alltraps_field,
	open_dup_alltraps_field,
	T_DUPALLTRAPS,
	DF_ALLTRAPS
};

/* Branch Equal Field
 *
 */
#define display_dup_beq_field display_bit_field
#define change_dup_beq_field change_bit_field
#define open_dup_beq_field flash

static BIT_FIELD dup_beq_field = {
	5,
	LENGTH(S_DUPBEQRTS),
	display_dup_beq_field,
	change_dup_beq_field,
	open_dup_beq_field,
	T_DUPBEQRTS,
	DF_BEQRTS
};

/* Has Argument Field
 *
 */
#define display_dup_has_arg_field display_bit_field
#define change_dup_has_arg_field change_bit_field
#define open_dup_has_arg_field flash

static BIT_FIELD dup_has_arg_field = {
	5,
	LENGTH(S_DUPARG),
	display_dup_has_arg_field,
	change_dup_has_arg_field,
	open_dup_has_arg_field,
	T_DUPARG,
	DF_ARG
};

/* Vector Prompt
 *
 */
#define display_dup_vector_prompt display_token_field
#define change_dup_vector_prompt flash
#define open_dup_vector_prompt flash

static TOKEN_FIELD dup_vector_prompt = {
	RIGHT(S_DUPVEC),
	LENGTH(S_DUPVEC),
	display_dup_vector_prompt,
	change_dup_vector_prompt,
	open_dup_vector_prompt,
	T_DUPVEC
};

/* Vector Field
 *
 */
#define display_dup_vector_field display_numeric_field
#define change_dup_vector_field change_numeric_field
#define open_dup_vector_field flash

static NUMERIC_FIELD dup_vector_field = {
	40,
	8,
	display_dup_vector_field,
	change_dup_vector_field,
	open_dup_vector_field,
	&sdup.dup_vec,
	HEXADECIMAL_FIELD
};

/* Handler Prompt
 *
 */
#define display_dup_handler_prompt display_token_field
#define change_dup_handler_prompt flash
#define open_dup_handler_prompt flash

static TOKEN_FIELD dup_handler_prompt = {
	RIGHT(S_DUPHDLR),
	LENGTH(S_DUPHDLR),
	display_dup_handler_prompt,
	change_dup_handler_prompt,
	open_dup_handler_prompt,
	T_DUPHDLR
};

/* Handler Field
 *
 */
#define display_dup_handler_field display_text_field
#define change_dup_handler_field change_text_field
#define open_dup_handler_field flash

static TEXT_FIELD dup_handler_field = {
	40,
	10,
	display_dup_handler_field,
	change_dup_handler_field,
	open_dup_handler_field,
	&sdup.dup_hdlr
};

/* Value Prompt
 *
 */
#define display_dup_value_prompt display_token_field
#define change_dup_value_prompt flash
#define open_dup_value_prompt flash

static TOKEN_FIELD dup_value_prompt = {
	RIGHT(S_DUPARGVAL),
	LENGTH(S_DUPARGVAL),
	display_dup_value_prompt,
	change_dup_value_prompt,
	open_dup_value_prompt,
	T_DUPARGVAL
};

/* Value Field
 *
 */
#define display_dup_value_field display_numeric_field
#define change_dup_value_field change_numeric_field
#define open_dup_value_field flash

static NUMERIC_FIELD dup_value_field = {
	40,
	8,
	display_dup_value_field,
	change_dup_value_field,
	open_dup_value_field,
	&sdup.dup_arg,
	HEXADECIMAL_FIELD
};

/* Define Form
 *
 */
#define quit_dup_form quit_sub_context

#define add_dup_form_line flash
#define del_dup_form_line flash
#define dup_dup_form_line flash

static FIELD *dup_intret_fields[] = {
	ISFIELD(dup_intret_field)
};

static FIELD *dup_alltraps_fields[] = {
	ISFIELD(dup_alltraps_field)
};

static FIELD *dup_beq_fields[] = {
	ISFIELD(dup_beq_field)
};

static FIELD *dup_has_arg_fields[] = {
	ISFIELD(dup_has_arg_field)
};

static FIELD *dup_vector_fields[] = {
	ISFIELD(dup_vector_prompt),
	ISFIELD(dup_vector_field)
};

static FIELD *dup_handler_fields[] = {
	ISFIELD(dup_handler_prompt),
	ISFIELD(dup_handler_field)
};

static FIELD *dup_value_fields[] = {
	ISFIELD(dup_value_prompt),
	ISFIELD(dup_value_field)
};

static LINE dup_form_lines[] = {
	{ 0, 1, dup_intret_fields  },
	{ 1, 1, dup_alltraps_fields  },
	{ 2, 1, dup_beq_fields  },
	{ 3, 1, dup_has_arg_fields  },
	{ 5, 2, dup_vector_fields  },
	{ 6, 2, dup_handler_fields },
	{ 8, 2, dup_value_fields  }
};

FORM dup_form = {
	CP(&sdup),
	NULL,
	quit_dup_form,
	9,
	add_dup_form_line,
	del_dup_form_line,
	dup_dup_form_line,
	dup_form_lines
};


/* CONFIGURATION FILE TABLE MULTIPLE HANDLER FORM
 *
 */

/* Interrupt Return Field
 *
 */
#define display_cftbl_dup_intret_field display_bit_field
#define change_cftbl_dup_intret_field flash
#define open_cftbl_dup_intret_field flash

static BIT_FIELD cftbl_dup_intret_field = {
	5,
	LENGTH(S_DUPINTRET),
	display_cftbl_dup_intret_field,
	change_cftbl_dup_intret_field,
	open_cftbl_dup_intret_field,
	T_DUPINTRET,
	DF_INTRET
};

/* All Traps Field
 *
 */
#define display_cftbl_dup_alltraps_field display_bit_field
#define change_cftbl_dup_alltraps_field flash
#define open_cftbl_dup_alltraps_field flash

static BIT_FIELD cftbl_dup_alltraps_field = {
	5,
	LENGTH(S_DUPALLTRAPS),
	display_cftbl_dup_alltraps_field,
	change_cftbl_dup_alltraps_field,
	open_cftbl_dup_alltraps_field,
	T_DUPALLTRAPS,
	DF_ALLTRAPS
};

/* Branch Equal Field
 *
 */
#define display_cftbl_dup_beq_field display_bit_field
#define change_cftbl_dup_beq_field flash
#define open_cftbl_dup_beq_field flash

static BIT_FIELD cftbl_dup_beq_field = {
	5,
	LENGTH(S_DUPBEQRTS),
	display_cftbl_dup_beq_field,
	change_cftbl_dup_beq_field,
	open_cftbl_dup_beq_field,
	T_DUPBEQRTS,
	DF_BEQRTS
};

/* Has Argument Field
 *
 */
#define display_cftbl_dup_has_arg_field display_bit_field
#define change_cftbl_dup_has_arg_field flash
#define open_cftbl_dup_has_arg_field flash

static BIT_FIELD cftbl_dup_has_arg_field = {
	5,
	LENGTH(S_DUPARG),
	display_cftbl_dup_has_arg_field,
	change_cftbl_dup_has_arg_field,
	open_cftbl_dup_has_arg_field,
	T_DUPARG,
	DF_ARG
};

/* Vector Prompt
 *
 */
#define cftbl_dup_vector_prompt dup_vector_prompt

/* Vector Field
 *
 */
#define display_cftbl_dup_vector_field display_numeric_field
#define change_cftbl_dup_vector_field flash
#define open_cftbl_dup_vector_field flash

static NUMERIC_FIELD cftbl_dup_vector_field = {
	40,
	8,
	display_cftbl_dup_vector_field,
	change_cftbl_dup_vector_field,
	open_cftbl_dup_vector_field,
	&sdup.dup_vec,
	HEXADECIMAL_FIELD
};

/* Handler Prompt
 *
 */
#define cftbl_dup_handler_prompt dup_handler_prompt

/* Handler Field
 *
 */
#define display_cftbl_dup_handler_field display_text_field
#define change_cftbl_dup_handler_field change_text_field
#define open_cftbl_dup_handler_field flash

static TEXT_FIELD cftbl_dup_handler_field = {
	40,
	10,
	display_cftbl_dup_handler_field,
	change_cftbl_dup_handler_field,
	open_cftbl_dup_handler_field,
	&sdup.dup_hdlr
};

/* Value Prompt
 *
 */
#define cftbl_dup_value_prompt dup_value_prompt

/* Value Field
 *
 */
#define display_cftbl_dup_value_field display_numeric_field
#define change_cftbl_dup_value_field flash
#define open_cftbl_dup_value_field flash

static NUMERIC_FIELD cftbl_dup_value_field = {
	40,
	8,
	display_cftbl_dup_value_field,
	change_cftbl_dup_value_field,
	open_cftbl_dup_value_field,
	&sdup.dup_arg,
	HEXADECIMAL_FIELD
};

/* Define Form
 *
 */
#define quit_cftbl_dup_form quit_sub_context

#define add_cftbl_dup_form_line flash
#define del_cftbl_dup_form_line flash
#define dup_cftbl_dup_form_line flash

static FIELD *cftbl_dup_intret_fields[] = {
	ISFIELD(cftbl_dup_intret_field)
};

static FIELD *cftbl_dup_alltraps_fields[] = {
	ISFIELD(cftbl_dup_alltraps_field)
};

static FIELD *cftbl_dup_beq_fields[] = {
	ISFIELD(cftbl_dup_beq_field)
};

static FIELD *cftbl_dup_has_arg_fields[] = {
	ISFIELD(cftbl_dup_has_arg_field)
};

static FIELD *cftbl_dup_vector_fields[] = {
	ISFIELD(cftbl_dup_vector_prompt),
	ISFIELD(cftbl_dup_vector_field)
};

static FIELD *cftbl_dup_handler_fields[] = {
	ISFIELD(cftbl_dup_handler_prompt),
	ISFIELD(cftbl_dup_handler_field)
};

static FIELD *cftbl_dup_value_fields[] = {
	ISFIELD(cftbl_dup_value_prompt),
	ISFIELD(cftbl_dup_value_field)
};

static LINE cftbl_dup_form_lines[] = {
	{ 0, 1, cftbl_dup_intret_fields  },
	{ 1, 1, cftbl_dup_alltraps_fields  },
	{ 2, 1, cftbl_dup_beq_fields  },
	{ 3, 1, cftbl_dup_has_arg_fields  },
	{ 5, 2, cftbl_dup_vector_fields  },
	{ 6, 2, cftbl_dup_handler_fields },
	{ 8, 2, cftbl_dup_value_fields  }
};

FORM cftbl_dup_form = {
	CP(&sdup),
	NULL,
	quit_cftbl_dup_form,
	9,
	add_cftbl_dup_form_line,
	del_cftbl_dup_form_line,
	dup_cftbl_dup_form_line,
	cftbl_dup_form_lines
};


/* MEMORY CONFIGURATION SPECIFICATION DATA STRUCTURE
 *
 */
typedef struct {
	int	ram_flag;		/* flag value			*/
	int	ram_low;		/* low address			*/
	int	ram_high;		/* high address			*/
	int	ram_size;		/* optional size argument	*/
} RAM;

static RAM rambuf[RAM_COUNT];

STRUCT_LIST ram_slist = {
	sizeof(RAM),
	0,
	0,
	RAM_COUNT,
	STRUCTP(rambuf)
};

#define alloc_ram() ((RAM *)alloc_struct(&ram_slist))

RAM sram;				/* sample ram structure		*/

/* New Memory Configuration Specification
 *
 */
char *new_ram(flag, low, high, size)
int flag, low, high, size;
{
	register RAM *ramp;

	ramp = alloc_ram();

	ramp->ram_flag = flag;
	ramp->ram_low  = low;
	ramp->ram_high = high;
	ramp->ram_size = size;

	return(CP(ramp));
}

/* Create Memory Configuration Specification
 *
 */
RAM *create_ram()
{
	return(alloc_ram());
}

/* Duplicate Memory Configuration Specification
 *
 */
RAM *dup_ram(ramp)
register RAM *ramp;
{
	register RAM *nramp;

	nramp = alloc_ram();

	nramp->ram_flag = ramp->ram_flag;
	nramp->ram_low  = ramp->ram_low;
	nramp->ram_high = ramp->ram_high;
	nramp->ram_size = ramp->ram_size;

	return(nramp);
}

/* De-Allocate Memory Configuration Specification
 *
 */
void free_ram(ramp)
RAM *ramp;
{
	free_struct(&ram_slist, ramp);
}

/* Output Memory Configuration Specification
 *
 */
void write_ram(file, ramp)
FILE *file;
RAM *ramp;
{
	static BFORMAT rambformat[] = {
		{ T_RAMNOPARTY,	 RF_NOPARTY  },
		{ T_RAMSINGLE,	 RF_SINGLE   },
		{ T_RAMMULTIPLE, RF_MULTIPLE },
		{ 0,		 0	     }
	};

	static FORMAT ramformat[] = {
		{ FMT_HEXADEC,	T_RAMLOW,  CP(&sram.ram_low)  },
		{ FMT_HEXADEC,	T_RAMHIGH, CP(&sram.ram_high) },
		{ FMT_END,	0,	   0		      }
	};

	static FORMAT ramsformat[] = {
		{ FMT_HEXADEC,	T_RAMSIZE, CP(&sram.ram_size) },
		{ FMT_END,	0,	   0		      }
	};

	extern void write_format(), write_bit_lines();

	write_bit_lines(file, rambformat, ramp->ram_flag);
	write_format(file, ramformat, ramp, &sram);

	if (ramp->ram_flag & RF_MULTIPLE)
		write_format(file, ramsformat, ramp, &sram);
}

/* Output Dfile(4), Third Part, Item 5
 *
 */
void out_ram(file, ramp)
FILE *file;
RAM *ramp;
{
	fprintf(file, "ram %-2o%-10x", ramp->ram_flag, ramp->ram_low);

	if (ramp->ram_flag & RF_MULTIPLE)
		fprintf(file, "%-10x%x\n", ramp->ram_high, ramp->ram_size);
	else
		fprintf(file, "%x\n", ramp->ram_high);
}

/* MEMORY CONFIGURATION FORM
 *
 */

/* No Parity Field
 *
 */
#define display_ram_no_parity_field display_bit_field
#define change_ram_no_parity_field change_bit_field
#define open_ram_no_parity_field flash

static BIT_FIELD ram_no_parity_field = {
	5,
	LENGTH(S_RAMNOPARTY),
	display_ram_no_parity_field,
	change_ram_no_parity_field,
	open_ram_no_parity_field,
	T_RAMNOPARTY,
	RF_NOPARTY
};

/* Single Memory Block Field
 *
 */
#define display_ram_single_field display_bit_field
#define change_ram_single_field change_bit_field
#define open_ram_single_field flash

static BIT_FIELD ram_single_field = {
	5,
	LENGTH(S_RAMSINGLE),
	display_ram_single_field,
	change_ram_single_field,
	open_ram_single_field,
	T_RAMSINGLE,
	RF_SINGLE
};

/* Multiple Memory Block Field
 *
 */
#define display_ram_multiple_field display_bit_field
#define change_ram_multiple_field change_bit_field
#define open_ram_multiple_field flash

static BIT_FIELD ram_multiple_field = {
	5,
	LENGTH(S_RAMMULTIPLE),
	display_ram_multiple_field,
	change_ram_multiple_field,
	open_ram_multiple_field,
	T_RAMMULTIPLE,
	RF_MULTIPLE
};

/* Low Address Prompt
 *
 */
#define display_ram_low_prompt display_token_field
#define change_ram_low_prompt flash
#define open_ram_low_prompt flash

static TOKEN_FIELD ram_low_prompt = {
	RIGHT(S_RAMLOW),
	LENGTH(S_RAMLOW),
	display_ram_low_prompt,
	change_ram_low_prompt,
	open_ram_low_prompt,
	T_RAMLOW
};

/* Low Address Field
 *
 */
#define display_ram_low_field display_numeric_field
#define change_ram_low_field change_numeric_field
#define open_ram_low_field flash

static NUMERIC_FIELD ram_low_field = {
	40,
	8,
	display_ram_low_field,
	change_ram_low_field,
	open_ram_low_field,
	&sram.ram_low
};

/* High Address Prompt
 *
 */
#define display_ram_high_prompt display_token_field
#define change_ram_high_prompt flash
#define open_ram_high_prompt flash

static TOKEN_FIELD ram_high_prompt = {
	RIGHT(S_RAMHIGH),
	LENGTH(S_RAMHIGH),
	display_ram_high_prompt,
	change_ram_high_prompt,
	open_ram_high_prompt,
	T_RAMHIGH
};

/* High Address Field
 *
 */
#define display_ram_high_field display_numeric_field
#define change_ram_high_field change_numeric_field
#define open_ram_high_field flash

static NUMERIC_FIELD ram_high_field = {
	40,
	8,
	display_ram_high_field,
	change_ram_high_field,
	open_ram_high_field,
	&sram.ram_high,
	HEXADECIMAL_FIELD
};

/* Memory Block Size Prompt
 *
 */
#define display_ram_size_prompt display_token_field
#define change_ram_size_prompt flash
#define open_ram_size_prompt flash

static TOKEN_FIELD ram_size_prompt = {
	RIGHT(S_RAMSIZE),
	LENGTH(S_RAMSIZE),
	display_ram_size_prompt,
	change_ram_size_prompt,
	open_ram_size_prompt,
	T_RAMSIZE
};

/* Memory Block Size Field
 *
 */
#define display_ram_size_field display_numeric_field
#define change_ram_size_field change_numeric_field
#define open_ram_size_field flash

static NUMERIC_FIELD ram_size_field = {
	40,
	8,
	display_ram_size_field,
	change_ram_size_field,
	open_ram_size_field,
	&sram.ram_size,
	HEXADECIMAL_FIELD
};

/* Define Form
 *
 */
#define quit_ram_form quit_sub_context

#define add_ram_form_line flash
#define del_ram_form_line flash
#define dup_ram_form_line flash

static FIELD *ram_no_parity_fields[] = {
	ISFIELD(ram_no_parity_field)
};

static FIELD *ram_single_fields[] = {
	ISFIELD(ram_single_field)
};

static FIELD *ram_multiple_fields[] = {
	ISFIELD(ram_multiple_field)
};

static FIELD *ram_low_fields[] = {
	ISFIELD(ram_low_prompt),
	ISFIELD(ram_low_field)
};

static FIELD *ram_high_fields[] = {
	ISFIELD(ram_high_prompt),
	ISFIELD(ram_high_field)
};

static FIELD *ram_size_fields[] = {
	ISFIELD(ram_size_prompt),
	ISFIELD(ram_size_field)
};

static LINE ram_form_lines[] = {
	{ 0, 1, ram_no_parity_fields },
	{ 1, 1, ram_single_fields  },
	{ 2, 1, ram_multiple_fields },
	{ 4, 2, ram_low_fields  },
	{ 5, 2, ram_high_fields },
	{ 7, 2, ram_size_fields }
};

FORM ram_form = {
	CP(&sram),
	NULL,
	quit_ram_form,
	8,
	add_ram_form_line,
	del_ram_form_line,
	dup_ram_form_line,
	ram_form_lines
};


/* CONFIGURATION FILE TABLE MEMORY CONFIGURATION FORM
 *
 */

/* No Parity Field
 *
 */
#define display_cftbl_ram_no_parity_field display_bit_field
#define change_cftbl_ram_no_parity_field flash
#define open_cftbl_ram_no_parity_field flash

static BIT_FIELD cftbl_ram_no_parity_field = {
	5,
	LENGTH(S_RAMNOPARTY),
	display_cftbl_ram_no_parity_field,
	change_cftbl_ram_no_parity_field,
	open_cftbl_ram_no_parity_field,
	T_RAMNOPARTY,
	RF_NOPARTY
};

/* Single Memory Block Field
 *
 */
#define display_cftbl_ram_single_field display_bit_field
#define change_cftbl_ram_single_field flash
#define open_cftbl_ram_single_field flash

static BIT_FIELD cftbl_ram_single_field = {
	5,
	LENGTH(S_RAMSINGLE),
	display_cftbl_ram_single_field,
	change_cftbl_ram_single_field,
	open_cftbl_ram_single_field,
	T_RAMSINGLE,
	RF_SINGLE
};

/* Multiple Memory Block Field
 *
 */
#define display_cftbl_ram_multiple_field display_bit_field
#define change_cftbl_ram_multiple_field flash
#define open_cftbl_ram_multiple_field flash

static BIT_FIELD cftbl_ram_multiple_field = {
	5,
	LENGTH(S_RAMMULTIPLE),
	display_cftbl_ram_multiple_field,
	change_cftbl_ram_multiple_field,
	open_cftbl_ram_multiple_field,
	T_RAMMULTIPLE,
	RF_MULTIPLE
};

/* Low Address Prompt
 *
 */
#define cftbl_ram_low_prompt ram_low_prompt

/* Low Address Field
 *
 */
#define display_cftbl_ram_low_field display_numeric_field
#define change_cftbl_ram_low_field flash
#define open_cftbl_ram_low_field flash

static NUMERIC_FIELD cftbl_ram_low_field = {
	40,
	8,
	display_cftbl_ram_low_field,
	change_cftbl_ram_low_field,
	open_cftbl_ram_low_field,
	&sram.ram_low
};

/* High Address Prompt
 *
 */
#define cftbl_ram_high_prompt ram_high_prompt

/* High Address Field
 *
 */
#define display_cftbl_ram_high_field display_numeric_field
#define change_cftbl_ram_high_field flash
#define open_cftbl_ram_high_field flash

static NUMERIC_FIELD cftbl_ram_high_field = {
	40,
	8,
	display_cftbl_ram_high_field,
	change_cftbl_ram_high_field,
	open_cftbl_ram_high_field,
	&sram.ram_high,
	HEXADECIMAL_FIELD
};

/* Memory Block Size Prompt
 *
 */
#define cftbl_ram_size_prompt ram_size_prompt

/* Memory Block Size Field
 *
 */
#define display_cftbl_ram_size_field display_numeric_field
#define change_cftbl_ram_size_field flash
#define open_cftbl_ram_size_field flash

static NUMERIC_FIELD cftbl_ram_size_field = {
	40,
	8,
	display_cftbl_ram_size_field,
	change_cftbl_ram_size_field,
	open_cftbl_ram_size_field,
	&sram.ram_size,
	HEXADECIMAL_FIELD
};

/* Define Form
 *
 */
#define quit_cftbl_ram_form quit_sub_context

#define add_cftbl_ram_form_line flash
#define del_cftbl_ram_form_line flash
#define dup_cftbl_ram_form_line flash

static FIELD *cftbl_ram_no_parity_fields[] = {
	ISFIELD(cftbl_ram_no_parity_field)
};

static FIELD *cftbl_ram_single_fields[] = {
	ISFIELD(cftbl_ram_single_field)
};

static FIELD *cftbl_ram_multiple_fields[] = {
	ISFIELD(cftbl_ram_multiple_field)
};

static FIELD *cftbl_ram_low_fields[] = {
	ISFIELD(cftbl_ram_low_prompt),
	ISFIELD(cftbl_ram_low_field)
};

static FIELD *cftbl_ram_high_fields[] = {
	ISFIELD(cftbl_ram_high_prompt),
	ISFIELD(cftbl_ram_high_field)
};

static FIELD *cftbl_ram_size_fields[] = {
	ISFIELD(cftbl_ram_size_prompt),
	ISFIELD(cftbl_ram_size_field)
};

static LINE cftbl_ram_form_lines[] = {
	{ 0, 1, cftbl_ram_no_parity_fields },
	{ 1, 1, cftbl_ram_single_fields  },
	{ 2, 1, cftbl_ram_multiple_fields },
	{ 4, 2, cftbl_ram_low_fields  },
	{ 5, 2, cftbl_ram_high_fields },
	{ 7, 2, cftbl_ram_size_fields }
};

FORM cftbl_ram_form = {
	CP(&sram),
	NULL,
	quit_cftbl_ram_form,
	8,
	add_cftbl_ram_form_line,
	del_cftbl_ram_form_line,
	dup_cftbl_ram_form_line,
	cftbl_ram_form_lines
};
