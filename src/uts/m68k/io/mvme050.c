/*		Copyright (c) 1985 Motorola Inc.		*/
/*		All Rights Reserved				*/
/*
 *	This file initializes the Motorola mvme050 system controller
 *	and handles I/O to the front panel display and switches
 */
#ident	"@(#)kern-port:io/mvme050.c	7.6"
#include "sys/mc68153.h"

#define  BIM1		0xc0	/* bim 1 offset */
#define  BIM2		0xe0	/* bim 2 offset */

#define  DISPLAY_ADDR   0xa0	/* display address offset */
#define  SWITCH_ADDR    0xa0	/* switch address offset */
#define  BLANK_ADDR	0x8a	/* blank address offset */
#define  BLANK	 	0x0	/* blank out display register */
#define  UNBLANK	0x8	/* unblank display register */

#define  BLANK_CMD	1	/* turn off display */

char	m050_0;			/* board exist flag */

m050init ()
{
	extern int	m050_addr[];
	extern int	m050_ivec[];
	extern int	m5_ivec[];
	register vec = m050_ivec[0]/4;	/* get base vector # */
	register struct bim *bim_ptr;

	if( bprobe( m050_addr[0], -1 ) )  /* if it's not there */
		return;
	m050_0 = -1;			/* set to indicate the board is there */

	bim_ptr = (struct bim *) (m050_addr[0] + BIM1);

	/* special case for serial ports */
	bim_ptr->vctreg0 = m5_ivec[0]/4;
	bim_ptr->vctreg1 = m5_ivec[1]/4;
	bim_ptr->crlreg0 = (EXT | IRE | 5);
	bim_ptr->crlreg1 = (EXT | IRE | 5);

	/* general case */
	vec++;
	vec++;
	bim_ptr->vctreg2 = vec++;
	bim_ptr->vctreg3 = vec++;

#ifdef	VME120
	bim_ptr->crlreg2 = (IRE | 6);
#else
	bim_ptr->crlreg2 = 0;
#endif
	bim_ptr->crlreg3 = 0;

	bim_ptr = (struct bim *) (m050_addr[0] + BIM2);

	bim_ptr->vctreg0 = vec++;
	bim_ptr->vctreg1 = vec++;
	bim_ptr->vctreg2 = vec++;
	bim_ptr->vctreg3 = vec;

	bim_ptr->crlreg0 = 0;		/* disabled */
	bim_ptr->crlreg1 = 0;		/* disabled */
	bim_ptr->crlreg2 = 0;		/* disabled */
	bim_ptr->crlreg3 = 0;		/* disabled */
}


m050read (dev)
int	dev;
{
	extern int	m050_addr[];

	if(m050_0)
		passc (*(char *) (m050_addr[0] + SWITCH_ADDR));
}


m050write (dev)
int	dev;
{
	extern int	m050_addr[];
	register int	c;

	if ( m050_0 && (c = cpass()) != -1)
		*(char *) (m050_addr[0] + DISPLAY_ADDR) = c;
}


m050ioctl (dev, cmd, arg, mode)
int	dev, cmd, mode;
char	*arg;
{
	extern int	m050_addr[];
	register char	*cp;

	if(!m050_0)
		return;
	cp = (char *) (m050_addr[0] + BLANK_ADDR);

	if (cmd == BLANK_CMD)
		*cp = BLANK;
	else
		*cp = UNBLANK;
}

m050intr(r0,r1,r2,r3,r4,r5,r6,r7,r8,r9,r10,r11,r12,r13,r14,ps,pc,sform)
short	sform;
{	register unsigned vec = sform&0xfff;
	register unsigned ivec;
	register struct bim *bim_ptr;
	register char *bp;
	extern m050_ivec[];

	printf("M050: interrupt at %x\n", vec );

	ivec = vec;
	ivec /= 4;

	vec -= m050_ivec[0];
	vec /= 4;
	/* vec == interrupt # */

	if( vec >= 8 )
		return;
	
	if( vec < 4 )
		bim_ptr = (struct bim *) (m050_addr[0] + BIM1);
	else {
		vec -= 4;
		bim_ptr = (struct bim *) (m050_addr[0] + BIM2);
	}
	switch(vec) {
	case 0:	bp = &bim_ptr->crlreg0;	break;
	case 1:	bp = &bim_ptr->crlreg1;	break;
	case 2:	bp = &bim_ptr->crlreg2;	break;
	case 3:	bp = &bim_ptr->crlreg3;	break;
	}

	if(( *bp&IRAC ) == 0 ) {	/* not setup by m050prime? */
		printf("MVME050: unexpected interrupt on vector %x\n", ivec );
		*bp = 0;	/* disable further interrupts */
	}
}

m050prime(vec,vector,control)
register unsigned vec;
register unsigned vector;
register unsigned control;
{	register struct bim *bim_ptr;
	register char *bp;

	if( vec >= 8 )
		return;

	if( vec < 4 )
		bim_ptr = (struct bim *) (m050_addr[0] + BIM1);
	else {
		vec -= 4;
		bim_ptr = (struct bim *) (m050_addr[0] + BIM2);
	}

	if( vector ) {
		switch(vec) {
		case 0:	bp = &bim_ptr->vctreg0;	break;
		case 1:	bp = &bim_ptr->vctreg1;	break;
		case 2:	bp = &bim_ptr->vctreg2;	break;
		case 3:	bp = &bim_ptr->vctreg3;	break;
		}
		*bp = vector;
	}

	switch(vec) {
	case 0:	bp = &bim_ptr->crlreg0;	break;
	case 1:	bp = &bim_ptr->crlreg1;	break;
	case 2:	bp = &bim_ptr->crlreg2;	break;
	case 3:	bp = &bim_ptr->crlreg3;	break;
	}
	*bp = control;
}
