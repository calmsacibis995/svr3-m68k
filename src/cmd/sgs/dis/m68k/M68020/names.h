/*
 *	@(#)names.h	6.3	
 *		External definitions and defines for table driven 
 *		opcodes and register names for the MC68020
 *
 */

/*		from names.c		*/
extern char	*imm_char;
extern char	size[];
extern char	*opt_size[];
extern char	*cond_codes[];
extern char	*addregs[];
extern char	*reg_names[];
extern char	*suppressed_reg_names[];
extern char	*ctl_reg_names[];
extern char	*scale_string[];
extern char	*bit_ops[];
extern char	*bit_field_ops[];
extern char	*imm_fmt[];
extern char	*chk2_cmp2_fmt[];
extern char	*cas_cas2_fmt[];
extern char	*boolean_fmt[];
extern char	*miscell_8_fmt[];
extern char	*mul_div_ops[];
extern char	*shift_rot_fmt[];
extern char	*bcd_ops[];
extern char	*misc_ops[];
extern char	*other_ops[];

			/* Define index int ctl_reg_names	*/
#define	SFC	0			/* changing text_dis.c		      */
#define	DFC	1
#define	CACR	2
#define	SR	3
#define	USP	4
#define	VBR	5
#define	CAAR	6
#define	MSP	7
#define	ISP	8
#define	CCR	9

			/* Define index into bcd_ops    	*/
#define	NBCD	3
			/* Define indicies into other_ops	*/
#define BADOP	0
#define MOVEA	1
#define	MOVEP	2
#define	MOVES	3
#define	MOVEQ	4
#define	MOVEI	5
#define	MOVE	6
#define	RTM	7    
#define	CALLM	8  
#define	SWBET	9
#define	ILLEGAL	10
#define	LINK_L	11
#define	SWAP_W	12
#define	BKPT	13
#define	PEA	14
#define	EXT	15
#define	TRAP	16
#define	UNLK	17
#define	LINK	18
#define	MOVEC	19
#define	JSR	20
#define	JMP	21
#define	MOVEM	22
#define	DBcc	23
#define	Scc	24
#define	Tcc	25
#define	Bcc	26
#define	BSR	27
#define	BRA	28
#define	SUBQ	29
#define	ADDQ	30
#define	OR	31
#define	CMPA	32
#define	CMPM	33
#define	EOR	34
#define	CMP	35
#define	ABCD	36
#define	EXG	37
#define	AND	38
#define	SUB	39
#define	ADD	40
#define SWAP_L	41
#define TAS	42
			/* Define modifier indicies */
#define	AMOD	43
				/* AMOD - modifier for adda suba */
#define	XMOD	44
				/* XMOD - modifier for adds/subx */
