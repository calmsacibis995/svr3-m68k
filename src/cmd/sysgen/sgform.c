/* sgform.c %W%
 *
 */
#include "sysgen.h"
#include "sgyacc.h"
#include "sgform.h"

#define DEL 0177


/* SCREEN LINES
 *
 */

/* Clear Screen Line
 *
 */
void blank_screen_line(lineno)
int lineno;
{
	register int i;

	move(lineno, 0);
	for (i = 0; i < COLS; i++)
		addch(' ');
}


/* BOTTOM SCREEN LINE
 *
 */

/* Display Bottom Line Message
 *
 */
void botmsg(format, arg0, arg1, arg2, arg3)
char *format, *arg0, *arg1, *arg2, *arg3;
{
	register int i;

	blank_screen_line(mylastline);
	move(mylastline, 0);
	printw(format, arg0, arg1, arg2, arg3);
	refresh();
}

/* Display System Error Message
 *
 */
void sysmsg()
{
	if (errno < sys_nerr)
		botmsg("Command failed: %s", sys_errlist[errno]);
	else
		botmsg("Command failed: errno = %d", errno);
	blank_screen_line(mylastline);
}

/* Request User Confirmation
 *
 */
int confirm(format, arg1, arg2, arg3)
char *format, *arg1, *arg2, *arg3;
{
#	define A_CONFIRM A_BOLD

	register int answer, i;
	register char *buf;

	if (alwaysyes)
		return(-1);

	buf = mkbuf(BUFSIZE);
	sprintf(buf, format, arg1, arg2, arg3);

	attron(A_CONFIRM);
	botmsg("%s? [y/n] ", buf);
	free(buf);

	do {
		switch(answer = getch()) {
		case 'n':
		case 'y':
			break;
		default:
			flash();
			answer = 0;
			break;
		}
	} while (!answer);

	addch(answer);
	refresh();

	attroff(A_CONFIRM);
	blank_screen_line(mylastline);
	return(answer == 'y');
}


/* FORM LINE HANDLING
 *
 */

/* Display Form Line		This routines displays the given form line
 *				by calling the display field routine for
 * each of the fields present on the line.  If the screen line is out of
 * range then the form line is not displayed.
 *
 */
void display_form_line(cp, linep, data, scrlin, attrs)
CONTEXT *cp;
register LINE *linep;
char *data;
int scrlin, attrs;
{
	static int lineattr[] = { A_NORMAL, A_NORMAL, A_BLINK };

	register FORM *formp;
	register FIELD *fieldp;
	register int i, select;

	formp = cp->c_form;
	select = (formp->form_sselect == NULL) ? SELECTED_FALSE :
		*LOCINTEGER(data, formp->form_sselect, formp->form_sample);

	attron(lineattr[select] | attrs);

	blank_screen_line(scrlin);

	if (select) {
		move(scrlin, 0);
		i = (select == SELECTED_TRUE) ? '*' : '?';
		addch(i);
	}

	for (i = 0; i < linep->line_count; i++) {
		fieldp = linep->line_field[i];
		(*fieldp->fld_dsp)(fieldp, data, scrlin, cp);
	}

	attroff(lineattr[select] | attrs);
}

/* Display Current Form Line
 *
 */
void display_current_form_line()
{
	display_form_line(acp, acp->c_line, (acp->c_listform) ?
		acp->c_tbl->tbl_ptr[acp->c_curline] : CP(acp->c_tbl),
		acp->c_scrline, 0);
}


/* COMMAND CHARACTERS
 *
 */
typedef struct {
	chtype	cmd_ch;			/* character			*/
	int	cmd_type;		/* command type			*/
} CMDCH;

/* Command Character Definitions
 *
 */
CMDCH cmdch[] = {
	{ 'k', CMD_UP },
		{ KEY_UP, CMD_UP },
	{ 'j', CMD_DOWN },
		{ KEY_DOWN, CMD_DOWN },
		{ '\n', CMD_DOWN },
	{ 'l', CMD_RIGHT },
		{ KEY_RIGHT, CMD_RIGHT },
	{ 'h', CMD_LEFT },
		{ KEY_LEFT, CMD_LEFT },
		{ 'H' - '@', CMD_LEFT },
	{ '\r', CMD_NEXT },
		{ KEY_ENTER, CMD_NEXT },
	{ 'H', CMD_TOP },
	{ 'L', CMD_BOT },
	{ 'M', CMD_MID },
	{ 'D' - '@', CMD_SDOWN },
	{ 'U' - '@', CMD_SUP },
	{ 'F' - '@', CMD_FWD },
	{ 'B' - '@', CMD_BACK },
	{ 'L' - '@', CMD_REDRAW },
	{ 'u', CMD_UNDO },
	{ 'q', CMD_QUIT },
	{ 'i', CMD_INFO },
	{ 'c', CMD_CHG },
	{ 'o', CMD_OPEN },
		{ KEY_OPEN, CMD_OPEN },
	{ 'a', CMD_ADD },
	{ 'd', CMD_DEL },
	{ 'D', CMD_DUP },
	{ 'S' - '@', CMD_IGNORE },
	{ 'Q' - '@', CMD_IGNORE },
	{ '!', CMD_ESCAPE },
	{ '\0', CMD_ERROR }
};

/* Identify Command
 *
 */
int get_command()
{
	register CMDCH *cmdchp;
	register chtype c;

	c = getch();
	cmdchp = cmdch;
	while ((cmdchp->cmd_ch != c) && (cmdchp->cmd_ch != '\0'))
		++cmdchp;
	blank_screen_line(mylastline);
	return(cmdchp->cmd_type);
}


/* FORM HANDLING
 *
 */
void update_context();

/* Read Form				Refresh the screen, obtain a user
 *					character, categorize it using the
 * command character definition table, dispatch on the command type using the
 * active context's command dispatch table, and, finally, save the last
 * context command.
 *
 */
void read_form()
{
	for (;;) {
		move(acp->c_scrline, acp->c_field->fld_col);
		(*acp->c_cmdtbl[acp->c_cmd = get_command()])();
		acp->c_lastcmd = acp->c_cmd;
	}
	/*NOTREACHED*/
}

/* Display Form Header		This routine displays the header of the active
 *				form by going through the context table and
 * displaying the current line for each of the awaiting forms.  At the end
 * this routine saves the number of lines in the header (the number of awaiting
 * contexts) plus one in the top-screen line field of the active context.
 *
 */
void display_form_header()
{
#	define A_HEADER (A_UNDERLINE | A_REVERSE)

	register CONTEXT *cp;
	register int scrlin, i;

	for (scrlin = 0; scrlin < contbl.tbl_count; scrlin++) {
		cp = get_context_table_entry(scrlin);
		display_form_line(cp, cp->c_line, (cp->c_listform) ?
			cp->c_tbl->tbl_ptr[cp->c_curline] : CP(cp->c_tbl),
			scrlin, A_HEADER);
	}
	blank_screen_line(scrlin);
	acp->c_scrtop = scrlin + 1;
}

/* Display Form Body			This routine displays the lines (i.e.,
 *					the body of the active form.  If this
 * is a list form (as opposed to a data form), then the number of lines in the
 * form, as well as the last line index must be reset to reflect the fact that
 * list forms may have new lines added, and old lines delete (whereas data
 * forms never have lines added or delete).
 *	The bottom accessible form line is computed by figuring how many lines 
 * could be displayed on the screen and noting whether the form's last line
 * might preceed such.
 *	Finally, for each line on the screen, some data, if only blanks
 * displayed.
 *
 */
void display_form_body()
{
	register int i, scrlin, lineno, botline, scrbot, scrtop;
	register LINE *linep;
	register char *dp;

	if (acp->c_listform) {
		acp->c_lines = acp->c_tbl->tbl_count;
		if ((acp->c_lastline = acp->c_lines - 1) < 0)
			acp->c_lastline = 0;
	}
	lineno = acp->c_topline;
	scrtop = acp->c_scrtop;
	scrbot = LINES - 1;

	botline = (LINES - scrtop) + (lineno - 1);
	if (botline > acp->c_lastline)
		botline = acp->c_lastline;
	acp->c_botline = botline;

	linep = acp->c_form->form_line;
	if (!acp->c_listform) {
		dp = CP(acp->c_tbl);
		while (linep->line_line < acp->c_topline)
			++linep;
	}

	for (scrlin = scrtop; scrlin <= scrbot; scrlin++) {
		if ((acp->c_lines) && (lineno <= botline) &&
		    ((acp->c_listform) || (linep->line_line == lineno))) {
			if (acp->c_listform)
				dp = acp->c_tbl->tbl_ptr[lineno];

			display_form_line(acp, linep, dp, scrlin, 0);

			if (!acp->c_listform)
				++linep;
		} else
			blank_screen_line(scrlin);

		lineno++;
	}
}

/* Display Form			This routine displays the form.  First the
 *				screen is cleared, and the header is output,
 * then, if the line count field indicates the form has no lines, i.e.,
 * is a list form, the display list form routine is executed, otherwise, the
 * display data form routine is utilized.
 *
 */
void display_form()
{
	display_form_header();
	display_form_body();
	update_context();
}


/* SCROLLING
 *
 */

/* Scroll Screen Up
 *
 */
void scrollup(lines)
int lines;
{
	register int topline;

	if (lines <= 0)
		return;

	if ((topline = acp->c_topline - lines) < 0)
		topline = 0;

	acp->c_topline = topline;
	display_form_body();
}


/* Scroll Screen Down
 *
 */
void scrolldown(lines)
int lines;
{
	register int botline;

	if (lines <= 0)
		return;

	botline = acp->c_botline + lines;
	if (botline > acp->c_lastline)
		botline = acp->c_lastline;

	acp->c_topline = (botline + 1) - (LINES - acp->c_scrtop);
	if (acp->c_topline < 0)
		acp->c_topline = 0;
	display_form_body();
}


/* CURSOR MOVEMENT COMMANDS
 *
 */

/* Move Cursor Right
 *
 */
void move_cursor_right()
{
	register int i;

	for (i = acp->c_fieldno + 1; i < acp->c_line->line_count; ++i)
		if (acp->c_line->line_field[i]->fld_chg != flash) {
			acp->c_fieldno = i;
			update_context();
			return;
		}
	flash();
}

/* Move Cursor Left		If the cursor is on the left-most field
 *				of the current line then declare an error
 * condition, otherwise, move left one field.
 *
 */
void move_cursor_left()
{
	register int i;

	for (i = acp->c_fieldno - 1; i >= 0; --i)
		if (acp->c_line->line_field[i]->fld_chg != flash) {
			acp->c_fieldno = i;
			update_context();
			return;
		}
	flash();
}

/* Move Cursor Up		This command cause the new current line in the
 *				active form to be the line above the present
 * current line.  If the new current line is not on the screen then scroll
 * the form down.  If the present current line is the first form line or there
 * are not lines on the form then indicate an error condition.
 *
 */
void move_cursor_up()
{
	display_current_form_line();
	acp->c_curline = (acp->c_listform) ?
		acp->c_curline - 1 : (acp->c_line - 1)->line_line;
	update_context();
}

/* Move Cursor Down
 *
 */
void move_cursor_down()
{
	display_current_form_line();
	acp->c_curline = (acp->c_listform) ?
		acp->c_curline + 1 : (acp->c_line + 1)->line_line;
	update_context();
}

/* Next Line			This command moves the cursor down one line
 *				and to the left as far as possible, sort of
 * a down and left command.  Care must be taken that the cursor isn't already
 * on the last form line (error).  Also, as in the down command, the screen
 * may need to be scrolled up if the new current line is below the bottom
 * of the screen.
 *
 */
void move_cursor_down_left()
{
	acp->c_fieldno = 0;
	move_cursor_down();
}

/* To Screen Top		This simple routine selects the top form
 *				line to be the current form line and resets
 * the field number to zero, causing the new current field to be the
 * left-most field on that line.
 *
 */
void move_cursor_to_top()
{
	display_current_form_line();
	acp->c_curline = acp->c_topline;
	acp->c_fieldno = 0;
	update_context();
}

/* To Screen Middle		Move the cursor to the middle of the form
 *				present on the screen.  As in the to bottom
 * of screen command, some calculation must be done to assure that the
 * new form line index is within bounds.
 *
 */
void move_cursor_to_middle()
{
	display_current_form_line();
	acp->c_curline = (acp->c_botline - acp->c_topline)/2 + acp->c_topline;
	acp->c_fieldno = 0;
	update_context();
}

/* To Screen Bottom		Move the cursor to the bottom of the screen
 *				or the bottom of the form on the screen if
 * the form doesn't cover the entire screen.
 *
 */
void move_cursor_to_bottom()
{
	display_current_form_line();
	acp->c_curline = acp->c_botline;
	acp->c_fieldno = 0;
	update_context();
}

/* Scroll Screen Down		Display more field lines at the bottom of the
 *				screen.  If the bottom of the form is displayed
 * on the screen then declare an error condition.
 *
 */
void smove_cursor_down()
{
	register int topline;

	topline = acp->c_topline;
	scrolldown(scrollhalf);
	acp->c_curline = (topline != acp->c_topline) ?
		acp->c_curline + (acp->c_topline - topline) : acp->c_lastline;
	update_context();
}

/* Scroll Screen Up		Display more form lines at the top of the
 *				screen, i.e., move the screen view up in the
 * form.  If the form's top line is displayed on the screen then declare
 * an error condition.
 *
 */
void smove_cursor_up()
{
	register int topline;

	topline = acp->c_topline;
	scrollup(scrollhalf);
	acp->c_curline = (topline != acp->c_topline) ?
		acp->c_curline + (topline - acp->c_topline) : 0;
	update_context();
}

/* Move Forward In Form
 *
 */
void move_screen_forward()
{
	scrolldown(scrollfull);
	acp->c_curline += scrollfull;
	update_context();
}

/* Move Backward In Form
 *
 */
void move_screen_backward()
{
	scrollup(scrollfull);
	acp->c_curline -= scrollfull;
	update_context();
}


/* MISCELLANEOUS COMMANDS
 *
 */

/* Undo Last Change
 *
 */
void undo_last_change()
{
	flash();
}


/* Supply User Information
 *
 */
void give_information()
{
	flash();
}

/* Re-Display The Form
 *
 */
void redisplay_form()
{
	clear();
	display_form();
}

/* Null Command
 *
 */
void null_command()
{
}

/* Escape To Shell Command
 *
 */
void escape_to_shell()
{
	register char *buffer;

	buffer = mkbuf(BUFSIZE);

	blank_screen_line(mylastline);
	refresh();
	endwin();

	putchar('!');
	fflush(stdout);

	if ((gets(buffer) != NULL) && (*buffer != '\0')) {
		system(buffer);
		printf("[Press a return to continue.]");
		fflush(stdout);
		gets(buffer);
	}
	refresh();
	free(buffer);
}

/* CONTEXT STRUCTURE
 *
 */
static CONTEXT contextbuf[CONTEXT_COUNT];

STRUCT_LIST context_slist = {
	sizeof(CONTEXT),
	0,
	0,
	CONTEXT_COUNT,
	STRUCTP(contextbuf)
};

/* Create A New Context		Allocate a new context structure, record the
 *				data block and form definition pointers; set
 * the current line and current field to be the first line in the form, and
 * the first field in the line.  Place the active context in the context table,
 * and set the new context as the active context.
 *
 */
void create_context(data, formp)
TABLE *data;
register FORM *formp;
{
	register CONTEXT *cp;

	cp = alloc_context();

	cp->c_tbl  = data;
	cp->c_form = formp;

	cp->c_line  = formp->form_line;
	cp->c_field  = formp->form_line->line_field[0];

	if (cp->c_lines = formp->form_count)
		cp->c_lastline = cp->c_lines - 1;
	else
		cp->c_listform = -1;

	cp->c_lindex = 0;
	cp->c_topline = 0;

	cp->c_cmdtbl[CMD_ERROR]  = flash;
	cp->c_cmdtbl[CMD_INFO]   = give_information;
	cp->c_cmdtbl[CMD_IGNORE] = null_command;
	cp->c_cmdtbl[CMD_ESCAPE] = escape_to_shell;
	cp->c_cmdtbl[CMD_REDRAW] = redisplay_form;
	cp->c_cmdtbl[CMD_RIGHT]  = move_cursor_right;
	cp->c_cmdtbl[CMD_LEFT]   = move_cursor_left;
	cp->c_cmdtbl[CMD_UNDO]   = flash;

	cp->c_cmdtbl[CMD_ADD]    = formp->form_add;
	cp->c_cmdtbl[CMD_QUIT]   = formp->form_quit;

	put_entry_in_context_table(acp);
	acp = cp;

	display_form();
}

/* Update Context		When the current field is changed using
 *				cursor-movement commands, or others, the
 * field specific information present in the active context structure and
 * the command routine table must be updated.
 *
 */
void update_context()
{
	register LINE *linep;
	register int i, nolines;

	nolines = 0;

	if (acp->c_curline < 0)
		acp->c_curline = 0;
	else if (acp->c_curline > acp->c_lastline)
		acp->c_curline = acp->c_lastline;

	scrollup(acp->c_topline - acp->c_curline);
	scrolldown(acp->c_curline - acp->c_botline);

	if (acp->c_listform) {
		if (!acp->c_tbl->tbl_count)
			--nolines;
		acp->c_lindex = acp->c_curline;
	} else {
		acp->c_lindex = 0;
     		while (((acp->c_form->form_line + acp->c_lindex)->line_line <
		        acp->c_curline) &&
		       ((acp->c_form->form_line + acp->c_lindex)->line_line <
		        acp->c_botline))
				++acp->c_lindex;

		acp->c_line = acp->c_form->form_line + acp->c_lindex;
		acp->c_curline = acp->c_line->line_line;
	}
	acp->c_scrline = acp->c_scrtop + (acp->c_curline - acp->c_topline);

	while (acp->c_fieldno >= acp->c_line->line_count)
		--acp->c_fieldno;
	while ((acp->c_line->line_field[acp->c_fieldno]->fld_chg == flash) &&
	       (acp->c_fieldno < acp->c_line->line_count - 1))
		acp->c_fieldno++;
	acp->c_field = acp->c_line->line_field[acp->c_fieldno];

	for (i = CMD_DOWN; i < CMD_CNT; i++)
		acp->c_cmdtbl[i] = flash;

	if (!nolines) {
		attron(A_BOLD);
		display_current_form_line();
		move(acp->c_scrline, acp->c_field->fld_col - 3);
		addch('-');
		addch('>');
		attroff(A_BOLD);

		if (acp->c_lindex) {
			acp->c_cmdtbl[CMD_UP]  = move_cursor_up;
			acp->c_cmdtbl[CMD_SUP] = smove_cursor_up;
		}

		if (acp->c_curline != acp->c_lastline) {
			acp->c_cmdtbl[CMD_DOWN]  = move_cursor_down;
			acp->c_cmdtbl[CMD_NEXT]  = move_cursor_down_left;
			acp->c_cmdtbl[CMD_SDOWN] = smove_cursor_down;
		}

		acp->c_cmdtbl[CMD_TOP] = move_cursor_to_top;
		acp->c_cmdtbl[CMD_BOT] = move_cursor_to_bottom;
		acp->c_cmdtbl[CMD_MID] = move_cursor_to_middle;

		if (acp->c_topline > 0)
			acp->c_cmdtbl[CMD_FWD] = move_screen_forward;

		if (acp->c_botline < acp->c_lastline)
			acp->c_cmdtbl[CMD_BACK] = move_screen_backward;

		if (!alwaysno)
			acp->c_cmdtbl[CMD_DEL]  = acp->c_form->form_del;
		acp->c_cmdtbl[CMD_DUP]  = acp->c_form->form_dup;

		acp->c_cmdtbl[CMD_CHG]  = acp->c_field->fld_chg;
		acp->c_cmdtbl[CMD_OPEN] = acp->c_field->fld_open;
	}
}

/* Deallocate Context Structure
 *
 */
void free_context(cp)
CONTEXT *cp;
{
	free_struct(&context_slist, cp);
}

/* Return From Read Form
 *
 */
void quit_read_form()
{
	free_context(acp);
	longjmp(outenv, 1);
	/*NOTREACHED*/
}

/* Quit Current Context
 *
 */
void quit_context()
{
	free_context(acp);
	acp = get_context_table_entry(--contbl.tbl_count);
	display_form();
}

/* Quit Sub-Form Context
 *
 */
void quit_sub_context()
{
	if (acp->c_modified)
		--get_context_table_entry(contbl.tbl_count - 1)->c_modified;
	quit_context();
}


/* ALLOWABLE FIELD CHARACTERS SPECIFICATION
 *
 */
char textchars[] = "\t !\"#$%&'()*+,-./0123456789:;<=>?@\
ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~";

char filechars[] = "!+,-.0123456789:;=@ABCDEFGHIJKLMNOPQRSTUVWXYZ_\
abcdefghijklmnopqrstuvwxyz";

char firstdig[] = "-0123456789";
char digits[] = "0123456789";
char fhexdigits[] = "-0123456789abcdefABCDEF";
char hexdigits[] = "0123456789abcdefABCDEF";
char foctdigits[] = "-01234567";
char octdigits[] = "01234567";
char verschars[] = "0123456789.";

ACSPEC	textacs		= { textchars,	textchars	},
	fileacs		= { filechars,	filechars	},
	dnacs		= { firstdig,	digits		},
	hnacs		= { fhexdigits,	hexdigits	},
	onacs		= { foctdigits,	octdigits	},
	*numacs[]	= { &dnacs,	&hnacs,		&onacs		},
	versacs		= { digits,	verschars	};


/* GENERAL FIELDS
 *
 */

/* Display Field
 *
 */
void display_field(fieldp, data, scrlin)
register FIELD *fieldp;
register char *data;
int scrlin;
{
	register chtype c;
	register int i;

	move(scrlin, fieldp->fld_col);

	for (i = 0; i < fieldp->fld_size; i++) {
		if (!(c = *data))
			c = ' ';
		else
			data++;
		addch(c);
	}
}

/* Change Field
 *
 */
char *change_field(fieldp, data, scrlin, acsp)
register FIELD *fieldp;
char *data;
int scrlin;
ACSPEC *acsp;
{
	register char *p, *q, *buf, *end;
	register chtype ch;
	register int size, col, bcol, lcol;

	buf  = memset(mkbuf(BUFSIZE), ' ', BUFSIZE);
	size = strlen(data);
	p    = strncpy(buf, data, size);
	end  = p + size;
	bcol = fieldp->fld_col;
	lcol = bcol + fieldp->fld_size;

	attron(A_REVERSE);
	display_field(fieldp, buf, scrlin);
	attroff(A_REVERSE);
	addch(' ');
	attron(A_REVERSE);

	col = bcol;
	for (;;) {
		move(scrlin, col);
		refresh();

		switch(ch = getch()) {
		case '\n':
		case '\r':
		case KEY_DOWN:
		case KEY_ENTER:
			/* restore field attributes */
			attroff(A_REVERSE);
			display_field(fieldp, buf, scrlin);
			move(scrlin, bcol);
			refresh();

			/* return field value if changed */
			*end = '\0';
			p = (!strcmp(data, buf)) ? NULL : create_string(buf);
			free(buf);
			return(p);
		case KEY_RIGHT:
			/* move cursor right */
			if (col >= lcol)
				break;
			++col;
			++p;
			continue;
		case DEL:
		case KEY_DC:
		eraseit:
			if (p < end) {	/* delete character under cursor */
				for (q = p; q < end; q++)
					*q = *(q + 1);
				*end-- = ' ';

				display_field(fieldp, buf, scrlin);

				/* was this the first char in buffer? */
				if (p == buf)
					continue;
				/* we are mid-field value? */
				if (p < end)
					continue;
				/* deleted last non-blank char? */
				if (p == end)
					/* locate new last non-blank char */
					while ((end > buf) &&
					      (*(end - 1) == ' '))
						--end;
			} else if ((p == end) && (p == buf))
				break;
		case KEY_LEFT:
			/* move cursor left */
			if (p <= buf)
				break;
			--col;
			--p;
			continue;
		default:
			/* special character? */
			if (ch == killer) {
				p = strncpy(memset(buf, ' ', BUFSIZE),
						data, size);
				end = buf + size;
				col = bcol;
				continue;
			} else if (ch == eraser)
				goto eraseit;

			/* space for characater? */
			if (col >= lcol)
				break;

			/* valid characater? */
			if (strchr((p == buf) ? acsp->acs_first :
			   acsp->acs_rest, ch) == NULL)
				break;

			/* save character */
			*p++ = ch;
			addch(ch);
			++col;

			/* update end of field value */
			if ((p > end) && (ch != ' '))
				end = p;

			continue;
		}
		/* error condition */
		flash();
	}
}


/* PROMPTS
 *
 */

/* Display Static Prompt
 *
 */
void display_prompt_field(fieldp, data, scrlin)
FIELD *fieldp;
char *data;
int scrlin;
{
	display_field(fieldp, fieldp->fld_prompt, scrlin);
}

/* Display Keyword Prompt
 *
 */
void display_token_field(fieldp, data, scrlin)
FIELD *fieldp;
char *data;
int scrlin;
{
	display_field(fieldp, keyword[fieldp->fld_token - T_DESCFILE], scrlin);
}


/* TEXT FIELDS
 *
 */

/* Display Data Name Field
 *
 */
void display_text_field(fieldp, data, scrlin, cp)
FIELD *fieldp;
char *data;
int scrlin;
CONTEXT *cp;
{
	display_field(fieldp, *LOCCHARPTR(data, fieldp->fld_sname,
				cp->c_form->form_sample), scrlin);
}

/* Change Data Name Field
 *
 */
void change_text_field()
{
	register char **namptrloc, *newname;

	namptrloc = LOCCHARPTR((acp->c_listform) ?
			acp->c_tbl->tbl_ptr[acp->c_curline] : CP(acp->c_tbl),
			acp->c_field->fld_sname, acp->c_form->form_sample);

	newname = change_field(acp->c_field, *namptrloc,
			acp->c_scrline, &textacs);
	if (newname == NULL)
		return;

	free_string(*namptrloc);
	*namptrloc = newname;
	--acp->c_modified;
}


/* NUMERIC FIELDS
 *
 */

/* Display Number Field
 *
 */
void display_numeric_field(fieldp, data, scrlin, cp)
FIELD *fieldp;
char *data;
int scrlin;
CONTEXT *cp;
{
	register char *buf;

	buf = convert_numeric_field(fieldp, data,
			cp->c_form->form_sample, fieldp->fld_snum);
	display_field(fieldp, buf, scrlin);
	free_string(buf);
}

/* Change Number Fields
 *
 */
int change_numeric_data(data, sample)
char *data, *sample;
{
	static int base[] = { 10, 0x10, 010 };
	static char *numname[] = { "decimal", "hexadecimal", "octal" };

	register int newnum, oldnum, numtype, *numptr;
	register char *buf, *newbuf;
	char *endptr;

	numptr = LOCINTEGER(data, acp->c_field->fld_snum, sample);
	oldnum = *numptr;
	buf    = convert_numeric_field(acp->c_field,
				data, sample, acp->c_field->fld_snum);

	numtype	= acp->c_field->fld_numtype;
	newbuf  = change_field(acp->c_field, buf,
				acp->c_scrline, numacs[numtype]);
	free_string(buf);

	if (newbuf == NULL) {
		botmsg("No change.");
		return(0);
	}

	newnum = strtol(newbuf, &endptr, base[numtype]);

	if (endptr != newbuf + strlen(newbuf)) {
		flash();
		botmsg("Illegal %s number: '%s'.", numname[numtype], newbuf);
		free_string(newbuf);
		return(0);
	}
	free_string(newbuf);
	*numptr = newnum;
	--acp->c_modified;
	return(-1);
}

void change_numeric_field()
{
	register int newnum;

	change_numeric_data((acp->c_listform) ?
		acp->c_tbl->tbl_ptr[acp->c_curline] : CP(acp->c_tbl),
		acp->c_form->form_sample);
	update_context();
}

/* Convert Binary To ASCII Decimal or Hexadecimal
 *
 */
char *convert_numeric_field(fieldp, data, sample, sample_nump)
FIELD *fieldp;
char *data, *sample;
int *sample_nump;
{
	register char *buf, *p;
	register int i;
	static char *numformat[] = { "%d", "%X", "%o" };

	buf = mkbuf(BUFSIZE);
	i = *LOCINTEGER(data, sample_nump, sample);

	sprintf(buf, numformat[fieldp->fld_numtype], i);
	p = create_string(buf);
	free(buf);
	return(p);
}


/* BIT FIELDS
 *
 */

/* Display Bit Field Prompt
 *
 */
void display_bit_field(fieldp, data, scrlin)
FIELD *fieldp;
int *data, scrlin;
{
	if (fieldp->fld_mask & *data) {
		move(scrlin, 0);
		addch('*');
	}
	display_token_field(fieldp, data, scrlin);
}

/* Select/De-Select Bit Field
 *
 */
void change_bit_field()
{
	*((int *)acp->c_tbl) ^= acp->c_field->fld_mask;
	--acp->c_modified;
	update_context();
}
