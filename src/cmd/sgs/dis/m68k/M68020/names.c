/*
 *	@(#)names.c	6.6		
 *		contains format strings and constant strings for
 *		M68020 disassembler.
 */

	static char SCCSID[] = "@(#)names.c	6.6		85/05/31";

#include	"dis.h"
#include	<setjmp.h>

	      /* Common variables  */
unsigned short curinstr;	/* for saving first part of instruction
				   when cur2bytes is used to read displ */
unsigned short oldinstr = 0;	/* to save previous instruction for
				   testing that swbeg follows jump	*/

char	conv_temp[NHEX];	/* Temporary location for ascii	*/
				/* representation of operands.	*/

char	comp_temp[NHEX];	/* temp for calling compoff	*/

int	errlev = 0;	/* Errors encountered during the	*/
				/* disassembly, probably due to being	*/
				/* out of sync.				*/
#ifdef SDB
			/* data for sdb only */
unsigned short	cur2bytes;
unsigned long	cur4bytes;
int		debugflag;
long		loc;			/* byte location being disassembled */
char		mneu[NLINE];		/* line to be printed if desired    */
char		object[NHEX];		/* reserved for future use	    */
#else
			/* data for dis only */
int		debugflag=0;
jmp_buf		env;		/* used by set/longjmp when an error is
				   located down in a subroutine these
				   routines allow continuing after resync */
#endif

	      /* Format strings for dis_util to use */

				/* used by convert: */
			/***TEMP, hex format changed to dec for testing ***/
char	*cvt_hex_lead = "%#x";
char	*cvt_oct_lead = "%#o";
char	*cvt_hex_nolead = "%.4x";
char	*cvt_oct_nolead = "%.5o";
				/* used by printline */
char	*print_octal_fmt = "%-40s%s\n";
char	*print_hex_fmt	 = "%-41s%s\n";

char	*imm_char = "&";
char	size[] = {'b','w','l'};
char	*opt_size[] =
	{ "", ".b", ".w", ".l" };

char	*cond_codes[] =
	{
		"t",	"f",	"hi",	"ls",
		"cc",	"cs",	"ne",	"eq",
		"vc",	"vs",	"pl",	"mi",
		"ge",	"lt",	"gt",	"le"
	};

char 	*addregs[] = 
	{ "%a0","%a1","%a2","%a3","%a4","%a5","%fp","%sp" };

char	*reg_names[] =
	{ "%d0", "%d1", "%d2", "%d3", "%d4", "%d5", "%d6", "%d7",
	  "%a0", "%a1", "%a2", "%a3", "%a4", "%a5", "%fp", "%sp", "%pc" 
	};

char	*suppressed_reg_names[] =
	{ "%zd0", "%zd1", "%zd2", "%zd3", "%zd4", "%zd5", "%zd6", "%zd7",
	  "%za0", "%za1", "%za2", "%za3", "%za4", "%za5", "%zfp", "%zsp", "%zpc" 
	};

char	*ctl_reg_names[] = {		/* Warning: don't change order without*/
		"%sfc",			/* changing text_dis.c		      */
		"%dfc",
		"%cacr",
		"%sr",
		"%usp",
		"%vbr",
		"%caar",
		"%msp",
		"%isp",
		"%cc",
		"ILLEGAL CTL REG NAME"
	};

char	*scale_string[]=
	{ ".w", ".w*2", ".w*4", ".w*8",
	  ".l", ".l*2", ".l*4", ".l*8" };

char	*bit_ops[] = {
		"btst    ",
		"bchg    ",
		"bclr    ",
		"bset    "
	};

char	*bit_field_ops[] = {
		"bftst   ",
		"bfextu  ",
		"bfchg   ",
		"bfexts  ",
		"bfclr   ",
		"bfffo   ",
		"bfset   ",
		"bfins   "
	};

char	*imm_fmt[] = {
		"ori%-5s%s,",
		"andi%-4s%s,",
		"subi%-4s%s,",
		"addi%-4s%s,",
		"MISC_OP",
		"eori%-4s%s,",
		"cmpi%-4s%s,",
		"MOVES"
	};

char	*chk2_cmp2_fmt[] = {
		"cmp2%-4s",
		"chk2%-4s"
	};

char	*cas_cas2_fmt[] = {
		"cas%-5s%s,%s,%s",
		"cas2%-4s%s:%s,%s:%s,(%s):(%s)"
	};

char	*boolean_fmt[] = {
		"negx%-4s%s",
		"clr%-5s%s",
		"neg%-5s%s",
		"not%-5s%s",
		"BAD BOOLEAN OP",
		"tst%-5s%s"
	};

char	*miscell_8_fmt[] = {
		"chk.l    %s,%%d%d",
		"UNASSIGNED",
		"chk.w    %s,%%d%d",
		"lea      %s,%%a%d",
		"extb.l   %s"
	};

char	*mul_div_ops[] = {
		"mulu.w  ",
		"mulu.l  ",
		"muls.w  ",
		"muls.l  ",
		"divu.w  ",
		"divu.l  ",
		"divs.w  ",
		"divs.l  ",
		"mulu.w  ",
		"mulu.l  ",
		"muls.w  ",
		"muls.l  ",
		"divu.w  ",
		"divul.l ",
		"divs.w  ",
		"divsl.l "
	};
char	*shift_rot_fmt[] = {
		"asr%-5s%s,%s",
		"asl%-5s%s,%s",
		"lsr%-5s%s,%s",
		"lsl%-5s%s,%s",
		"roxr%-4s%s,%s",
		"roxl%-4s%s,%s",
		"ror%-5s%s,%s",
		"rol%-5s%s,%s"
	};

char	*bcd_ops[] = {
		"sbcd.b  ",
		"pack    ",
		"unpk    ",
		"nbcd.b  "
	};

char	*misc_ops[] =
	{
	       "reset",
	       "nop",
	       "stop    &",
	       "rte",
	       "rtd     ",
	       "rts",
	       "trapv",
	       "rtr"
	};

char	*other_ops[] = {
		"BAD OP",	/* ASM notation */
		"mov",		/* movea */
		"movp",		/* movep */
		"movs",		/* moves */
		"moveq.l ",
		"mov",		/* movei */
		"mov",
		"rtm     ",
		"callm   ",
		"swbet   %s",
		"illegal ",
		"link.l  %s,",
		"swap.w  %s",
		"bkpt    %s",
		"pea.l   ",
		"ext%-5s",
		"trap    &",
		"unlk    %s",
		"link    %s,",
		"mov",		/* movec */
		"jsr     ",
		"jmp     ",
		"movm",		/* movem */
		"db",
		"s",
		"t",
		"b",
		"bsr",
		"bra",
		"subq",
		"addq",
		"or",
		"cmpa",
		"cmpm",
		"eor",
		"cmp",
		"abcd.b  ",
		"exg     %s,%s",
		"and",
		"sub",
		"add",
		"swap.l",
		"tas.b  ",
		"a",			/* AMOD - modifier for adda suba */
		"x",			/* XMOD - modifier for adds/subx */
	};
