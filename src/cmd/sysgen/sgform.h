/* sgform.h
 *
 */

/* GENERAL FIELD DEFINTIONS
 *
 */
#define ISFIELD(f) ((FIELD *)&f)
#define LENGTH(s) (sizeof(s) - 1)			/* lenght of text */
#define CENTER(s) ((79 - LENGTH(s)) / 2)		/* center text */
#define RIGHT(s) (40 - 4 - LENGTH(s))			/* prompt text */


/* FIELDS
 *
 */

/* Simple Field		The simple field data structure specifies the minimum
 *			information present for a given form field.  These data
 * items are:  the field's starting column number (0 <= col <= COLS), its size
 * (maximum number of characters), its display, change, and open routines.
 *
 */
typedef struct {			/* SIMPLE FIELD DEFINITION	*/
	int	fld_s_col;		/* starting column		*/
	int	fld_s_size;		/* size				*/
	void	(*fld_s_dsp)();		/* display routine		*/
	void	(*fld_s_chg)();		/* change routine		*/
	void	(*fld_s_open)();	/* open routine			*/
} SIMPLE_FIELD;


/* Prompt Field		A prompt field is a field with a static text string
 *			associated to it; the user may not change this text
 * value (i.e., the prompt field change routine is always errcond()).  An
 * additional field is added to the prompt field data structure which is the
 * the pointer to the static text string associated to the field.
 *
 */
typedef struct {			/* PROMPT FIELD	DEFINITION	*/
	int	fld_p_col;		/* starting column		*/
	int	fld_p_size;		/* size				*/
	void	(*fld_p_dsp)();		/* display routine		*/
	void	(*fld_p_chg)();		/* change routine		*/
	void	(*fld_p_open)();	/* open routine			*/
	char	*fld_p_prompt;		/* static data pointer		*/
} PROMPT_FIELD;


/* Keyword Field	A keyword prompt field locates its text value in
 *			in the keyword[] array.  Like the prompt field, the
 * user may not modify this field type's value.
 *
 */
typedef struct {			/* KEYWORD FIELD DEFINITION	*/
	int	fld_k_col;		/* starting column		*/
	int	fld_k_size;		/* size				*/
	void	(*fld_k_dsp)();		/* display routine		*/
	void	(*fld_k_chg)();		/* change routine		*/
	void	(*fld_k_open)();	/* open routine			*/
	int	fld_k_token;		/* keyword number		*/
} TOKEN_FIELD;


/* Text Field		A text field represents text data found in a data
 *			structure associated to the form at create context
 * time.  Consequently, when it is display time, an offset to the text pointer
 * is calculated and added to the start of the form's data structure.
 *
 */
typedef struct {			/* TEXT FIELD DEFINITION	*/
	int	fld_t_col;		/* starting column		*/
	int	fld_t_size;		/* size				*/
	void	(*fld_t_dsp)();		/* display routine		*/
	void	(*fld_t_chg)();		/* change routine		*/
	void	(*fld_t_open)();	/* open routine			*/
	char	**fld_t_sname;		/* sample name pointer		*/
} TEXT_FIELD;


/* Numeric Field	
 *
 */
typedef struct {
	int	fld_n_col;		/* starting column		*/
	int	fld_n_size;		/* size				*/
	void	(*fld_n_dsp)();		/* display routine		*/
	void	(*fld_n_chg)();		/* change routine		*/
	void	(*fld_n_open)();	/* open routine			*/
	int	*fld_n_snum;		/* sample name pointer		*/
	int	fld_n_numtype;		/* number display type		*/
} NUMERIC_FIELD;

/* Number Field Type Field Values:
 *
 */
#define DECIMAL_FIELD 		0	/* Decimal display type		*/
#define HEXADECIMAL_FIELD	1	/* Hexadecimal display type	*/
#define OCTAL_FIELD		2	/* Octal display type		*/


/* BIT FIELD		This field type represents on the screen a text field
 *			which is associated to a single of bit of data, which
 * causes the enable/disable form line symbol '*' to be displayed.
 *
 */
typedef struct {			/* Bit Selection Field		*/
	int	fld_b_col;		/* starting column		*/
	int	fld_b_size;		/* size				*/
	void	(*fld_b_dsp)();		/* display routine		*/
	void	(*fld_b_chg)();		/* change routine		*/
	void	(*fld_b_open)();	/* open routine			*/
	int	fld_b_keyno;		/* keyword token value		*/
	int	fld_b_mask;		/* mask field for bit		*/
} BIT_FIELD;


/* FORM FIELD
 *
 */
typedef union {				/* FORM FIELD DEFINITION	*/
	SIMPLE_FIELD	fld_simple;	/* simple field			*/
	PROMPT_FIELD	fld_prt;	/* prompt field			*/
	TOKEN_FIELD	fld_key;	/* keyword prompt field		*/
	TEXT_FIELD	fld_text;	/* text field			*/
	NUMERIC_FIELD	fld_num;	/* number field			*/
	BIT_FIELD	fld_bit;	/* bit field			*/
} FIELD;

#define fld_col		fld_simple.fld_s_col
#define fld_size	fld_simple.fld_s_size
#define fld_dsp		fld_simple.fld_s_dsp
#define fld_chg		fld_simple.fld_s_chg
#define fld_open	fld_simple.fld_s_open

#define fld_prompt	fld_prt.fld_p_prompt
#define fld_token	fld_key.fld_k_token
#define fld_sname	fld_text.fld_t_sname
#define fld_snum	fld_num.fld_n_snum
#define fld_numtype	fld_num.fld_n_numtype
#define fld_mask	fld_bit.fld_b_mask


/* FORM LINE
 *
 */
typedef struct {			/* FORM LINE DEFINITION		*/
	int	line_line;		/* line				*/
	int	line_count;		/* line field count		*/
	FIELD	**line_field;		/* form field defn table ptr	*/
} LINE;


/* FORM
 *
 */
typedef struct fdef {			/* FORM DEFINITION		*/
	char	*form_sample;		/* data block sample		*/
	int	*form_sselect;		/* sample select field location	*/
	void	(*form_quit)();		/* quit routine			*/
	int	form_count;		/* line count			*/
	void	(*form_add)();		/* line add routine		*/
	void	(*form_del)();		/* line delete routine		*/
	void	(*form_dup)();		/* line duplicate routine	*/
	LINE	*form_line;		/* line table pointer		*/
} FORM;


/* COMMAND TYPES
 *
 */					/*	Constant Commands	*/
#define CMD_ERROR	0		/* error			*/
#define CMD_IGNORE	1		/* ignore this command		*/
#define CMD_ESCAPE	2		/* escape to shell		*/
#define CMD_INFO	3		/* info				*/
#define CMD_REDRAW	4		/* redraw			*/
#define CMD_RIGHT	5		/* right			*/
#define CMD_LEFT	6		/* left				*/

					/*   Form Dependent Commands	*/
#define CMD_ADD		7		/* add				*/
#define CMD_DUP		8		/* duplicate			*/
#define CMD_QUIT	9		/* quit				*/

#define CMD_UNDO	10		/* undo				*/

					/*   Line Dependent Commands	*/
#define CMD_DOWN	11		/* down				*/
#define CMD_NEXT	12		/* next line			*/
#define CMD_TOP		13		/* to top			*/
#define CMD_BOT		14		/* to bottom			*/
#define CMD_MID		15		/* to middle			*/
#define CMD_SDOWN	16		/* scroll down			*/
#define CMD_SUP		17		/* scroll up			*/
#define CMD_FWD		18		/* forward			*/
#define CMD_BACK	19		/* backward			*/
					/*				*/
#define CMD_DEL		20		/* delete			*/
#define CMD_UP		21		/* up				*/

					/*   Field Dependent Commands	*/
#define CMD_CHG		22		/* change			*/
#define CMD_OPEN	23		/* open				*/

#define CMD_CNT		24		/* Number Of Commands		*/


#define CMD_SEL		CMD_NEXT	/* selection key for desc type	*/


/* FORM CONTEXTS
 *
 * Each context in this table is associated with a form which has been
 * activated; only the last entry in the context table is active at a
 * given time.
 *
 */
typedef struct {			/* Context Structure		*/
					/*				*/
					/* constant w/i context		*/
	FORM	*c_form;		/* context form pointer		*/
	int	c_listform;		/* list form flag		*/
	int	c_lines;		/* form line count		*/
	int	c_lastline;		/* last form line		*/
	char	*c_other;		/* other form information	*/
					/*				*/
					/* form-related fields		*/
	TABLE	*c_tbl;			/* form data block pointer	*/
	int	c_modified;		/* data block modified flag	*/
	int	c_error;		/* error flag			*/
					/*				*/
					/* line-related fields		*/
	int	c_curline;		/* current form line		*/
	int	c_lindex;		/* current form line index	*/
	LINE	*c_line;		/* current line defintion	*/
	int	c_topline;		/* top form line		*/
	int	c_botline;		/* bottom form line		*/
					/*				*/
					/* field-related fields	*/
	int	c_fieldno;		/* field number on current line	*/
	FIELD	*c_field;		/* current field definition	*/
					/*				*/
					/* screen-related fields	*/
	int	c_scrtop;		/* top form body screen line	*/
	int	c_scrline;		/* current screen line		*/
					/*				*/
					/* command-related fields	*/
	int	c_cmd;			/* current command type		*/
	int	c_lastcmd;		/* last command type		*/
	void	(*c_cmdtbl[CMD_CNT])();	/* command dispatch table	*/
					/*				*/
} CONTEXT;

extern STRUCT_LIST context_slist;
#define alloc_context() ((CONTEXT *)alloc_struct(&context_slist))

#define c_fdtbl  c_tbl
#define c_dtbl   c_tbl
#define c_cfntbl c_tbl
#define c_sdtbl  c_tbl
#define c_mintbl c_tbl

CONTEXT *acp;

#define get_context_table_entry(i) get_table_entry(&contbl, i, CONTEXT)
#define put_entry_in_context_table(cp) put_entry_in_table(&contbl, cp)
#define remove_context_table_entry(i) remove_table_entry(&contbl, i)


/* ALLOWABLE CHARACTERS SPECIFICATION
 *
 */
typedef struct {			/* allowable characters		*/
	char	*acs_first;		/* first characters		*/
	char	*acs_rest;		/* all other characters		*/
} ACSPEC;

extern ACSPEC textacs, fileacs, *numacs[], versacs;


/* GLOBAL FORM ROUTINES
 *
 */
extern void	display_bit_field();	/* Display Enable/Disable keyword */
extern void	display_token_field(); /* Display keyword prompt field */
extern void	display_numeric_field(); /* Display numeric field	*/
extern void	display_prompt_field();	/* Display static text field	*/
extern void	display_text_field();	/* Display data field		*/
extern void	change_bit_field();	/* Enable/Disable keyword field	*/
extern char	*change_field();	/* Change Field Data		*/
extern void	change_numeric_field();	/* Change numeric field		*/
extern void	change_text_field();	/* Change data field		*/
extern char	*convert_numeric_field(); /* Convert number to ASCII	*/
extern void	quit_read_form();	/* Return to read_form() caller	*/
extern void	quit_sub_context();	/* Return to previous context	*/
extern void	quit_sub_fdtbl_form();	/* Return to cftbl_fdtbl_form	*/
