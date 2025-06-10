/*	@(#)dis.h	6.1		*/
/* dis.h */
/*	static char SCCSID[] = "dis.h:	2.1 83/06/28";	*/



#define		NCPS	8	/* Number of chars per symbol.		*/
#define		NHEX	40	/* Maximum # chars in object per line.	*/
#define		NLINE	33	/* Maximum # chars in mnemonic per line.*/
#define		FAIL	0
#define		TRUE	1
#define		FALSE	0
#define		LEAD	1
#define		NOLEAD	0
#define		TERM	0

#define LNNOBLKSZ	1024	/* size of blocks of line numbers		*/
#define SYMBLKSZ	1024	/* size of blocks of symbol table entries	*/
#define	STRNGEQ		0	/* used in string compare operation */

#define BIT0(x) ( (x) & 0x1L)           /* -------x bit 0               */
#define	BIT3(x) ( (x >> 3) & 0x1L)	/* ----x--- bit 3		*/
#define	BIT5(x)	 ( (x >> 5) & 0x1L)	/* --x----- bit 5		*/
#define	BIT6(x)	 ( (x >> 6) & 0x1L)	/* -x------ bit 6		*/
#define	BIT7(x)	 ( (x >> 7) & 0x1L)	/* x------- bit 7		*/
#define BIT8(x)	( (x >> 8) & 0x1L)
#define BIT10(x) ( (x >> 10) & 0x1L)
#define BIT11(x) ( (x >> 11) & 0x1L)
#define BIT12(x) ( (x >> 12) & 0x1L)
#define BIT15(x) ( (x >> 15) & 0x1L)
#define	BITS15_8(x) ( (x >> 8) & 0xffL)	
#define BITS15_7(x) ( (x >> 7) & 0x1ffL)
#define BITS14_12(x) ( (x >> 12) & 0x7L)
#define	BITS11_9(x) ( (x >> 9) & 0x7L)	/* bits 11 through 9		*/
#define	BITS11_8(x) ( (x >> 8) & 0xfL)	/* bits 11 through 8		*/
#define	BITS10_9(x) ( (x >> 9) & 0x3L)	/* bits 10 through 9		*/
#define	BITS10_8(x) ( (x >> 8) & 0x7L)	/* bits 10 through 8		*/
#define	BITS8_6(x) ( (x >> 6) & 0x7L)	/* bits 8 through 6		*/
#define BITS8_3(x) ( (x >> 3) & 0x3fL)	/* bits 8 through 3		*/
#define	BITS7_6(x) ( (x >> 6) & 0x3L)	/* bits 7 through 6		*/
#define	BITS5_4(x) ( (x >> 4) & 0x3L)	/* bits 5 through 4		*/
#define	BITS5_3(x) ( (x >> 3) & 0x7L)	/* bits 5 through 3		*/
#define	BITS5_0(x) ( x & 0x3fL)		/* bits 5 through 0		*/
#define	BITS4_3(x) ( (x >> 3) & 0x3L)	/* bits 4 through 3		*/
#define	BITS3_0(x) ( x & 0xfL)		/* bits 3 through 0		*/
#define	BITS2_0(x) ( x & 0x7L)		/* bits 2 through 0		*/
#define	LOW8(x) ( x & 0xffL)		/* low 8 bits of quantity	*/
#define HIOF32(x) ( (x >> 31) & 0x1L)	/* sign bit of 32 bit quantity	*/
#define	HI4OF16(x) ((x >> 12) & 0xfL)
