/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)curses:screen/curses.form	1.4.1.4"
/*
 * curses.h - this file is automatically made from caps, curses.form and
 *	curses.ed.
 *	Don't make changes directly to curses.h!
 */
#ifndef _SUBWIN

/*
	The definition for 'reg' is not standard, and is provided for
	compatibility reasons. User's are discouraged from using this.
*/
# define	reg	register

# ifndef 	NONSTANDARD
#ifndef stdin
#  include  <stdio.h>
#endif /* stdin */
  /*
   * This trick is used to distinguish between SYSV and V7 systems.
   * We assume that L_ctermid is only defined in stdio.h in SYSV
   * systems, but not in V7 or Berkeley UNIX.
   */
#  ifdef L_ctermid
#   define SYSV
#  endif
   /* Some V7 systems define L_ctermid - we list those here */
#  ifdef sun
#   undef SYSV
#  endif

#  include  <unctrl.h>
#  ifdef SYSV
#ifndef VINTR
#   include <termio.h>
#endif /* VINTR */
   typedef struct termio SGTTY;
#endif /* SYSV */
#  ifndef SYSV
#ifndef _SGTTYB_
#   include <sgtty.h>
#endif /* _SGTTYB_ */
   typedef struct sgttyb SGTTY;
/*
 * Here we attempt to improve portability by providing some #defines
 * for SYSV functions on non-SYSV systems.
 */
#   define memcpy(dst, src, len)	bcopy((src), (dst), (len))
#   define strchr			index
#   define strrchr			rindex
#  endif
# else		/* NONSTANDARD */
/*
 * NONSTANDARD is intended for a standalone program (no UNIX)
 * that manages screens.  The specific program is Alan Hewett's
 * ITC, which runs standalone on an 11/23 (at least for now).
 * It is unclear whether this code needs to be supported anymore.
 * It hasn't been tested in quite awhile.
 */
# define NULL 0
# endif		/* NONSTANDARD */

typedef	char bool;

/*
 * chtype is the type used to store a character together with attributes.
 * It can be set to "char" to save space, or "long" to get more attributes.
 */
# ifdef	CHTYPE
	typedef	CHTYPE chtype;
# else
	typedef unsigned short chtype;
# endif /* CHTYPE */

/* TRUE and FALSE get defined so many times, */
/* let's not get in the way of other definitions. */
#if	!defined(TRUE) || ((TRUE) != 1)
# define	TRUE	(1)
#endif
#if	!defined(FALSE) || ((FALSE) != 0)
# define	FALSE	(0)
#endif
#if	!defined(ERR) || ((ERR) != -1)
# define	ERR	(-1)
#endif
#if	!defined(OK) || ((OK) != 0)
# define	OK	(0)
#endif

/* values for win->_flags */
# define	_SUBWIN		01
# define	_ENDLINE	02
# define	_FULLWIN	04
# define	_SCROLLWIN	010
# define	_FLUSH		020
# define	_ISPAD		040
# define	_WINCHANGED	0100
# define	_WINMOVED	0200

/* _firstch is initially set to this */
# define	_NOCHANGE	-1

struct _win_st {
	short	_cury, _curx;
	short	_maxy, _maxx;
	short	_begy, _begx;
	short	_flags;
	chtype	_attrs;
	bool	_clear;
	bool	_leave;
	bool	_scroll;
	bool	_use_idl;	/* 0=no, 1=yes, 2=go by need_idl */
	bool	_use_keypad;	/* 0=no, 1=yes, 2=yes/timeout */
	bool	_use_meta;	/* T=use the meta key */
	bool	_nodelay;	/* T=don't wait for tty input */
	chtype	**_y;
	short	*_firstch;
	short	*_lastch;
	short	_tmarg,_bmarg;
	/* end of Vr2 structure */
	bool	_need_idl;	/* T=have done ins/del line/scroll recently */
	bool	_notimeout;	/* T=do wait for rest of function key */
	short	_pminrow, _pmincol, _sminrow, _smincol, _smaxrow, _smaxcol;
	short	_yoffset;
};

extern int	LINES, COLS;

typedef struct _win_st	WINDOW;
extern WINDOW	*stdscr, *curscr;

extern char	*Def_term, ttytype[];

#ifdef lint
/*
 * Various tricks to shut up lint about things that are perfectly fine.
 */
# ifndef CURSES				/* if not internal to curses source */
struct screen {
	int _nobody_;
};
# endif /* CURSES */
#endif /* lint */

typedef struct screen	SCREEN;

# ifndef NOMACROS

#  ifndef MINICURSES
/*
 * psuedo functions for standard screen
 */
# define	addch(ch)	waddch(stdscr, ch)
# define	getch()		wgetch(stdscr)
# define	addstr(str)	waddstr(stdscr, str)
# define	getstr(str)	wgetstr(stdscr, str)
# define	move(y, x)	wmove(stdscr, y, x)
# define	clear()		wclear(stdscr)
# define	erase()		werase(stdscr)
# define	clrtobot()	wclrtobot(stdscr)
# define	clrtoeol()	wclrtoeol(stdscr)
# define	insertln()	winsertln(stdscr)
# define	deleteln()	wdeleteln(stdscr)
# define	refresh()	wrefresh(stdscr)
# define	inch()		winch(stdscr)
# define	insch(c)	winsch(stdscr,c)
# define	delch()		wdelch(stdscr)
# define	standout()	wstandout(stdscr)
# define	standend()	wstandend(stdscr)
# define	attron(at)	wattron(stdscr,at)
# define	attroff(at)	wattroff(stdscr,at)
# define	attrset(at)	wattrset(stdscr,at)
# define	echochar(ch)	wechochar(stdscr, ch)

# define	setscrreg(t,b)	wsetscrreg(stdscr, t, b)
# define	wsetscrreg(win,t,b)	(win->_tmarg=(t),win->_bmarg=(b))

/*
 * mv functions
 */
#define	mvwaddch(win,y,x,ch)	(wmove(win,y,x)==ERR?ERR:waddch(win,ch))
#define	mvwgetch(win,y,x)	(wmove(win,y,x)==ERR?ERR:wgetch(win))
#define	mvwaddstr(win,y,x,str)	(wmove(win,y,x)==ERR?ERR:waddstr(win,str))
#define	mvwgetstr(win,y,x,str)	(wmove(win,y,x)==ERR?ERR:wgetstr(win,str))
#define	mvwinch(win,y,x)	(wmove(win,y,x)==ERR?ERR:winch(win))
#define	mvwdelch(win,y,x)	(wmove(win,y,x)==ERR?ERR:wdelch(win))
#define	mvwinsch(win,y,x,c)	(wmove(win,y,x)==ERR?ERR:winsch(win,c))
#define	mvaddch(y,x,ch)		mvwaddch(stdscr,y,x,ch)
#define	mvgetch(y,x)		mvwgetch(stdscr,y,x)
#define	mvaddstr(y,x,str)	mvwaddstr(stdscr,y,x,str)
#define	mvgetstr(y,x,str)	mvwgetstr(stdscr,y,x,str)
#define	mvinch(y,x)		mvwinch(stdscr,y,x)
#define	mvdelch(y,x)		mvwdelch(stdscr,y,x)
#define	mvinsch(y,x,c)		mvwinsch(stdscr,y,x,c)

#  else /* MINICURSES */
/*
 * MINICURSES is not documented or supported anymore.  The intent was
 * to avoid the window handling stuff for a smaller curses.  However,
 * the bulk of the code is below refresh (3/4 or so) so the savings
 * are insignificant.  This version of curses just isn't well suited
 * to a 16 bit processor, except with a tiny application.
 */

# define	addch(ch)		m_addch(ch)
# define	addstr(str)		m_addstr(str)
# define	move(y, x)		m_move(y, x)
# define	clear()			m_clear()
# define	erase()			m_erase()
# define	refresh()		m_refresh()
# define	standout()		wstandout(stdscr)
# define	standend()		wstandend(stdscr)
# define	attron(at)		wattron(stdscr,at)
# define	attroff(at)		wattroff(stdscr,at)
# define	attrset(at)		wattrset(stdscr,at)
# define	mvaddch(y,x,ch)		move(y, x), addch(ch)
# define	mvaddstr(y,x,str)	move(y, x), addstr(str)
# define	initscr			m_initscr
# define	newterm			m_newterm
# define	echochar(ch)		(m_addch(ch) == ERR ? ERR : m_refresh())

/*
 * These functions don't exist in minicurses, so we define them
 * to nonexistent functions to help the user catch the error.
 */
#define	getch		m_getch
#define	getstr		m_getstr
#define	clrtobot	m_clrtobot
#define	clrtoeol	m_clrtoeol
#define	insertln	m_insertln
#define	deleteln	m_deleteln
#define	inch		m_inch
#define	insch		m_insch
#define	delch		m_delch
/* mv functions that aren't valid */
#define	mvwaddch	m_mvwaddch
#define	mvwgetch	m_mvwgetch
#define	mvwaddstr	m_mvaddstr
#define	mvwgetstr	m_mvwgetstr
#define	mvwinch		m_mvwinch
#define	mvwdelch	m_mvwdelch
#define	mvwinsch	m_mvwinsch
#define	mvgetch		m_mvwgetch
#define	mvgetstr	m_mvwgetstr
#define	mvinch		m_mvwinch
#define	mvdelch		m_mvwdelch
#define	mvinsch		m_mvwinsch
/* Real functions that aren't valid */
#define box		m_box
#define delwin		m_delwin
#define longname	m_longname
#define makenew		m_makenew
#define mvprintw	m_mvprintw
#define mvscanw		m_mvscanw
#define mvwin		m_mvwin
#define mvwprintw	m_mvwprintw
#define mvwscanw	m_mvwscanw
#define newwin		m_newwin
#define _outchar	m_outchar
#define overlay		m_overlay
#define overwrite	m_overwrite
#define printw		m_printw
#define putp		m_putp
#define scanw		m_scanw
#define scroll		m_scroll
#define subwin		m_subwin
#define touchwin	m_touchwin
#define _tscroll	m_tscroll
#define _tstp		m_tstp
#define vidattr		m_vidattr
#define waddch		m_waddch
#define waddstr		m_waddstr
#define wclear		m_wclear
#define wclrtobot	m_wclrtobot
#define wclrtoeol	m_wclrtoeol
#define wdelch		m_wdelch
#define wdeleteln	m_wdeleteln
#define werase		m_werase
#define wgetch		m_wgetch
#define wgetstr		m_wgetstr
#define winsch		m_winsch
#define winsertln	m_winsertln
#define wmove		m_wmove
#define wprintw		m_wprintw
#define wrefresh	m_wrefresh
#define wscanw		m_wscanw
#define setscrreg	m_setscrreg
#define wsetscrreg	m_wsetscrreg

#  endif /* MINICURSES */

/*
 * psuedo functions
 */

#define	getyx(win,y,x)		y = win->_cury, x = win->_curx
#define	getbegyx(win,y,x)	y = win->_begy, x = win->_begx
#define	getmaxyx(win,y,x)	y = win->_maxy, x = win->_maxx
#define getsyx(y,x)		_getsyx(&y,&x)
#define	winch(win)		(win->_y[win->_cury][win->_curx])

WINDOW	*initscr(), *newwin(), *subwin(), *newpad(), *subpad();
char	*longname(), *keyname();
char	erasechar(), killchar();
int	wgetch();	/* because it can return KEY_*, for instance. */
SCREEN	*newterm(), *set_term();
char	*slk_label();

/* Various video attributes */
#define A_STANDOUT	0000200
#define	_STANDOUT	A_STANDOUT    /* for compatability with old curses */
#define A_UNDERLINE	0000400
#define A_REVERSE	0001000
#define A_BLINK		0002000
#define A_DIM		0004000
#define A_BOLD		0010000
#define A_ALTCHARSET	0100000

/* The next two are subject to change so don't depend on them */
#define A_INVIS		0020000
#define A_PROTECT	0040000

#define A_NORMAL	0000000
#define A_ATTRIBUTES	0177600
#define A_CHARTEXT	0000177

/*
 * Standard alternate character set.  The current ACS world is evolving,
 * so we support only a widely available subset: the line drawing characters
 * from the VT100, plus a few from the Teletype 5410.  Eventually there
 * may be support of more sophisticated ACS line drawing, such as that
 * in the Teletype 5410, the HP line drawing set, and the like.  There may
 * be support for some non line oriented characters as well.
 *
 * Line drawing ACS names are of the form ACS_trbl, where t is the top, r
 * is the right, b is the bottom, and l is the left.  t, r, b, and l might
 * be B (blank), S (single), D (double), or T (thick).  The subset defined
 * here only uses B and S.
 */
#define ACS_BSSB	(acs_map['l'])
#define ACS_SSBB	(acs_map['m'])
#define ACS_BBSS	(acs_map['k'])
#define ACS_SBBS	(acs_map['j'])
#define ACS_SBSS	(acs_map['u'])
#define ACS_SSSB	(acs_map['t'])
#define ACS_SSBS	(acs_map['v'])
#define ACS_BSSS	(acs_map['w'])
#define ACS_BSBS	(acs_map['q'])
#define ACS_SBSB	(acs_map['x'])
#define ACS_SSSS	(acs_map['n'])

/*
 * Human readable names for the most commonly used characters.
 * "Upper", "right", etc. are chosen to be consistent with the vt100 manual.
 */
#define ACS_ULCORNER	ACS_BSSB
#define ACS_LLCORNER	ACS_SSBB
#define ACS_URCORNER	ACS_BBSS
#define ACS_LRCORNER	ACS_SBBS
#define ACS_RTEE	ACS_SBSS
#define ACS_LTEE	ACS_SSSB
#define ACS_BTEE	ACS_SSBS
#define ACS_TTEE	ACS_BSSS
#define ACS_HLINE	ACS_BSBS
#define ACS_VLINE	ACS_SBSB
#define ACS_PLUS	ACS_SSSS
#define ACS_S1		(acs_map['o'])		/* scan line 1 */
#define ACS_S9		(acs_map['s'])		/* scan line 9 */
#define ACS_DIAMOND	(acs_map['`'])		/* diamond */
#define ACS_CKBOARD	(acs_map['a'])		/* checker board (stipple) */
#define ACS_DEGREE	(acs_map['f'])		/* degree symbol */
#define ACS_PLMINUS	(acs_map['g'])		/* plus/minus */
#define ACS_BULLET	(acs_map['~'])		/* bullet */
	/* Teletype 5410/5420 symbols */
#define ACS_LARROW	(acs_map[','])		/* arrow pointing left */
#define ACS_RARROW	(acs_map['+'])		/* arrow pointing right */
#define ACS_DARROW	(acs_map['.'])		/* arrow pointing down */
#define ACS_UARROW	(acs_map['-'])		/* arrow pointing up */
#define ACS_BOARD	(acs_map['h'])		/* board of squares */
#define ACS_LANTERN	(acs_map['i'])		/* lantern symbol */
#define ACS_BLOCK	(acs_map['0'])		/* solid square block */

extern chtype *acs_map;

/* Funny "characters" enabled for various special function keys for input */
/* This list is created from caps and curses.ed. Do not edit it! */
#define KEY_BREAK	0401		/* break key (unreliable) */
#define KEY_DOWN	0402		/* Sent by terminal down arrow key */
#define KEY_UP		0403		/* Sent by terminal up arrow key */
#define KEY_LEFT	0404		/* Sent by terminal left arrow key */
#define KEY_RIGHT	0405		/* Sent by terminal right arrow key */
#define KEY_HOME	0406		/* Sent by home key. */
#define KEY_BACKSPACE	0407		/* Sent by backspace key */
#define KEY_F0		0410		/* function key f0. */
#define KEY_F(n)	(KEY_F0+(n))	/* Space for 64 function keys is reserved. */
#define KEY_DL		0510		/* Sent by delete line key. */
#define KEY_IL		0511		/* Sent by insert line. */
#define KEY_DC		0512		/* Sent by delete character key. */
#define KEY_IC		0513		/* Sent by ins char/enter ins mode key. */
#define KEY_EIC		0514		/* Sent by rmir or smir in insert mode. */
#define KEY_CLEAR	0515		/* Sent by clear screen or erase key. */
#define KEY_EOS		0516		/* Sent by clear-to-end-of-screen key. */
#define KEY_EOL		0517		/* Sent by clear-to-end-of-line key. */
#define KEY_SF		0520		/* Sent by scroll-forward/down key */
#define KEY_SR		0521		/* Sent by scroll-backward/up key */
#define KEY_NPAGE	0522		/* Sent by next-page key */
#define KEY_PPAGE	0523		/* Sent by previous-page key */
#define KEY_STAB	0524		/* Sent by set-tab key */
#define KEY_CTAB	0525		/* Sent by clear-tab key */
#define KEY_CATAB	0526		/* Sent by clear-all-tabs key. */
#define KEY_ENTER	0527		/* Enter/send (unreliable) */
#define KEY_SRESET	0530		/* soft (partial) reset (unreliable) */
#define KEY_RESET	0531		/* reset or hard reset (unreliable) */
#define KEY_PRINT	0532		/* print or copy */
#define KEY_LL		0533		/* Sent by home-down key */
					/* The keypad is arranged like this: */
					/*    a1    up    a3   */
					/*   left   b2  right  */
					/*    c1   down   c3   */
#define KEY_A1		0534		/* Upper left of keypad */
#define KEY_A3		0535		/* Upper right of keypad */
#define KEY_B2		0536		/* Center of keypad */
#define KEY_C1		0537		/* Lower left of keypad */
#define KEY_C3		0540		/* Lower right of keypad */
#define KEY_BTAB	0541		/* Back tab key */
#define KEY_BEG		0542		/* beg(inning) key */
#define KEY_CANCEL	0543		/* cancel key */
#define KEY_CLOSE	0544		/* close key */
#define KEY_COMMAND	0545		/* cmd (command) key */
#define KEY_COPY	0546		/* copy key */
#define KEY_CREATE	0547		/* create key */
#define KEY_END		0550		/* end key */
#define KEY_EXIT	0551		/* exit key */
#define KEY_FIND	0552		/* find key */
#define KEY_HELP	0553		/* help key */
#define KEY_MARK	0554		/* mark key */
#define KEY_MESSAGE	0555		/* message key */
#define KEY_MOVE	0556		/* move key */
#define KEY_NEXT	0557		/* next object key */
#define KEY_OPEN	0560		/* open key */
#define KEY_OPTIONS	0561		/* options key */
#define KEY_PREVIOUS	0562		/* previous object key */
#define KEY_REDO	0563		/* redo key */
#define KEY_REFERENCE	0564		/* ref(erence) key */
#define KEY_REFRESH	0565		/* refresh key */
#define KEY_REPLACE	0566		/* replace key */
#define KEY_RESTART	0567		/* restart key */
#define KEY_RESUME	0570		/* resume key */
#define KEY_SAVE	0571		/* save key */
#define KEY_SBEG	0572		/* shifted beginning key */
#define KEY_SCANCEL	0573		/* shifted cancel key */
#define KEY_SCOMMAND	0574		/* shifted command key */
#define KEY_SCOPY	0575		/* shifted copy key */
#define KEY_SCREATE	0576		/* shifted create key */
#define KEY_SDC		0577		/* shifted delete char key */
#define KEY_SDL		0600		/* shifted delete line key */
#define KEY_SELECT	0601		/* select key */
#define KEY_SEND	0602		/* shifted end key */
#define KEY_SEOL	0603		/* shifted clear line key */
#define KEY_SEXIT	0604		/* shifted exit key */
#define KEY_SFIND	0605		/* shifted find key */
#define KEY_SHELP	0606		/* shifted help key */
#define KEY_SHOME	0607		/* shifted home key */
#define KEY_SIC		0610		/* shifted input key */
#define KEY_SLEFT	0611		/* shifted left arrow key */
#define KEY_SMESSAGE	0612		/* shifted message key */
#define KEY_SMOVE	0613		/* shifted move key */
#define KEY_SNEXT	0614		/* shifted next key */
#define KEY_SOPTIONS	0615		/* shifted options key */
#define KEY_SPREVIOUS	0616		/* shifted prev key */
#define KEY_SPRINT	0617		/* shifted print key */
#define KEY_SREDO	0620		/* shifted redo key */
#define KEY_SREPLACE	0621		/* shifted replace key */
#define KEY_SRIGHT	0622		/* shifted right arrow */
#define KEY_SRSUME	0623		/* shifted resume key */
#define KEY_SSAVE	0624		/* shifted save key */
#define KEY_SSUSPEND	0625		/* shifted suspend key */
#define KEY_SUNDO	0626		/* shifted undo key */
#define KEY_SUSPEND	0627		/* suspend key */
#define KEY_UNDO	0630		/* undo key */

# endif /* NOMACROS */
#endif /* _SUBWIN */
