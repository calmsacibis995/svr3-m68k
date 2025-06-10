/*		Copyright (c) 1985 AT&T		*/
/*		All Rights Reserved		*/
/*	@(#)mc68153.h	6.1		*/
/*
 * mc68153 bus interrupter module
 */

#define	 F	0x80	/* flag */
#define  FAC	0x40	/* flag auto-clear */
#define  EXT	0x20	/* extern/internal */
#define  IRE	0x10	/* interrupt enable */
#define  IRAC	0x08 	/* interrupt auto-clear */
#define  IMASK	0x07	/* interrupt level mask */


	struct bim
		{
		char crlreg0,	fill2; 	/* control register 0 */
		char crlreg1,	fill4; 	/* control register 1 */
		char crlreg2,	fill6;	/* control register 2 */
		char crlreg3,	fill8;	/* control register 3 */
		char vctreg0,	fill10;	/* vector  register 0 */
		char vctreg1,	fill12;	/* vector  register 1 */
		char vctreg2,	fill14;	/* vector  register 2 */
		char vctreg3,	fill16;	/* vector  register 3 */
		};
