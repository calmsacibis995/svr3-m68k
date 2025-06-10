/*		Copyright (c) 1985 Motorola, Inc.		*/
/*		All Rights Reserved		*/
/*  @(#)clkZ8036.c	6.9       */
/*
	This file contains the routines required to 
	manipulate the Z8036 hardware clock on the vm04.
*/

/* This declaration is used to force the object for this module to be extracted
   from the library before other clock handlers (which have same entry points*/

int	Z8036clk;


#include "sys/types.h"


#define	CLK_ADDR 0xfffb0000	/* clock address */
#define	COUNT	33333		/* counter initial value (1/60 second) */
#define	CNT2	0xfffb0038	/* VERSAbus interrupt mask */

/* bit defines for Master Interrupt Control Register */
#define	RESET	0x01		/* reset Z8036 */
#define	ZERO	0x00		/* clear reset bit */
#define	MIE	0x80		/* Master Interrupt Enable */
#define	NV	0x20		/* No Vector Returned During IACK */
#define	RJA	0x02		/* Right Justified Address */
#define	DLC	0x40		/* Disable Lower Chain */

/* bit defines for Master Configuration Control Register */
#define	CT1E	0x40		/* Master Interrupt Enable */
#define PBE	0x80		/* Port B enable (VM04 compatibility) */
#define PAE	0x04		/* Port A enable (VM04 compatibility) */

/* bit defines for Counter/timer Mode Specification  Register */
#define	CSC	0x80		/* Continuous cycle */

/* bit defines for Counter/timer Command/Status  Register */
#define	CLIPIUS	0x20		/* Clear IP & IUS */
#define	GCB	0x04		/* Allow Countdown Sequence */
#define	TCB	0x02		/* Trigger Countdown Sequence */
#define	IE	0xc0		/* Interrupt Enable */

struct clk_map
{
	/* only registers relevant to counter 1 are defined here */

	char c_intctl;		/* master interrupt control register */
	char c_confctl;		/* master config control register */
	char fill1[2];
	char c_intvec;		/* counter interrupt vector */
	char fill2[5];
	char c_cs1;		/* counter 1 command/status */
	char fill3[5];
	unsigned short c_cc1;	/* counter 1 current count */
	char fill4[4];
	unsigned short c_tc1;	/* counter 1 time constant */
	char fill5[4];
	char c_mode1;		/* counter 1 mode specification */
};

extern time_t time;			/* time of day in seconds */


/* initialize the clock */
clkstart ()
{
	register struct clk_map *clk;
	clk = (struct clk_map *) CLK_ADDR;

	/* skip reset because ports a & b are control regs */

	/* clk->c_intctl = RESET;		/* reset Z8036 */
	/* clk->c_intctl = ZERO;		/* clear reset */

	clk->c_intctl = MIE | NV | RJA | DLC;
	clk->c_mode1 = CSC;		/* continuous count mode */
	clk->c_tc1 = COUNT;		/* set time constant for 1/60 sec. */
	clk->c_confctl = CT1E | PAE | PBE;	/* enable counter 1 */
	clk->c_cs1 = IE | GCB | TCB;	/* enable interrupts and start */

	*((char *) CNT2 ) &= ~0x02;	/* enable timein ints */
}

clkset (oldtime)
time_t	oldtime;
{
	time = oldtime;			/* reset time of day (sec) */
}

clkreld (on)
int on;
{
	register struct clk_map *clk;
#ifdef DEBUG
	static char tick = 0;
	static char sec = 1;

	if (++tick >= 60)
		{
		sec = (sec + 1) & 0xf;
		tick = 0;
		}
	*((char *) 0xfffb0068) = sec;	/* write the seconds to the panel */
#endif

	clk = (struct clk_map *) CLK_ADDR;
	if (on)
		clk->c_cs1 = CLIPIUS | GCB;	/* clear IP & IUS */
	else
		clk->c_cs1 = CLIPIUS;   /* stop clock,clear IP & IUS */
	return(0);
}
