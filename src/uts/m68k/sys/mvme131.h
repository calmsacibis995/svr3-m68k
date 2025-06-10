/*	@(#)mvme131.h	1.0		*/

/*
 *	Status and Control registers for the MVME-131 and MVME-131XT CPU
 *		Status registers are read-only.
 *		Control registers are read/write.
 *		Cache control registers
 *		Port size is 8 bits
 */

/*
 *	Status Registers STAT0, STAT1, STAT2
 */

#define	STAT0	((char*)0xfffb0030)	/* Status register 0		*/

#define	STAT0_MODE	0x80		/* Power up reset vector mode	*/
#define	STAT0_AMSEL	0x40		/* Address modifier select	*/
#define	STAT0_SC	0x20		/* VME bus system controller	*/
#define	STAT0_ENVIR	0x10		/* Operating Enviroment Status 	*/
#define	STAT0_DB	0x0f		/* Serial Port default bauds	*/
#define	STAT0_DB0	0x03		/* Serial Port 1		*/
#define	STAT0_DB1	0x0c		/* Serial Port 2		*/

#define	STAT1	((char*)0xfffb0031)	/* Status register 1		*/

#define	STAT1_GP	0xff		/* General purpose/network addr	*/

#define	STAT2	((char*)0xfffb0032)	/* Status Register 2		*/

#define	STAT2_ABORT	0x20		/* Abort button switch		*/
#define	STAT2_SYSFAIL	0x10		/* VME bus SYSFAIL signal	*/
#define	STAT2_VMEBERR	0x08		/* VME bus BERR signal		*/
#define	STAT2_VMXBERR	0x04		/* VMX32bus BERR signal		*/
#define	STAT2_MMUBERR	0x02		/* MMU bus error signal		*/
#define	STAT2_LBTO	0x01		/* local bus timeout BERR signal*/


/*
 *	Control Registers CNT0, CNT1, CNT2, CNT3, CNT4, CNT5
 */

#define	CNT0	((char*)0xfffb000d)	/* Control register 0		*/

#define	CNT0_VBV	0xff		/* VME bus interrupt vector	*/
					/*  Also port A on Z8036 timer	*/

#define	CNT1	((char*)0xfffb000e)	/* Control register 1		*/

#define	CNT1_MPIRQ	0x80		/* Multiprocessor Interrupt Req	*/
					/*  Also port B on Z8036 timer	*/

#define	CNT2	((char*)0xfffb0038)	/* Control register 2		*/

#define	CNT2_VBIMSK	0xff		/* VME bus interrupt masks	*/

#define	CNT3	((char*)0xfffb0039)	/* Control register 3		*/

#define	CNT3_OVRIEN	0x80		/* Bus monitor FIFO overrun	*/
					/*  interrupt enable		*/
#define	CNT3_ALLIEN	0x40		/* All Interrupt Enable		*/
#define	CNT3_SYSFIEN	0x20		/* SYSFAIL Interrupt Enable	*/
#define	CNT3_IL		0x0e		/* VMEbus Interrupt request level */

#define	CNT4	((char*)0xfffb003a)	/* Control register 4		*/

#define	CNT5	((char*)0xfffb003b)	/* Control register 5		*/

#define	CNT5_VMXDIS	0x40		/* VMX32bus Disable		*/
#define	CNT5_VMXR	0x20		/* VMX32bus read only mode	*/
#define	CNT5_VMXDEN	0x10		/* VMX32bus decode enable	*/
#define	CNT5_VMXIRQ	0x08		/* VMX32bus Interrupt Request	*/
#define	CNT5_32_24	0x04		/* VME bus address size 32/24	*/
#define	CNT5_32_16	0x02		/* VME bus address size 32/16	*/
#define	CNT5_BDFAIL	0x01		/* Board failure/SYSFAIL signal	*/

/*
 *	Cache control registers XCCR and XCMR
 */

#define	XCCR	((char*)0xfffc0000)	/* Cache control register 	*/

#define	XCCR_VBCEN	0x80		/* VME bus cache enable		*/
#define	XCCR_MONEN	0x40		/* Monitor enable		*/
#define	XCCR_MLBK	0x20		/* Monitor Loop back		*/
#define	XCCR_MBLC	0x10		/* Monitor Block		*/
#define	XCCR_CCLR	0x08		/* Cache Clear			*/
#define	XCCR_CWEN	0x04		/* Cache Write Enable		*/
#define	XCCR_CREN	0x02		/* Cache Read Enable		*/
#define	XCCR_CTST	0x01		/* Cache Test			*/

#define	XCMR	((char*)0xfffc0001)	/* Cache mask register		*/

#define	XCMR_MSD	0x08		/* Mask supervisor data		*/
#define	XCMR_MSP	0x04		/* Mask supervisor program	*/
#define	XCMR_MUD	0x02		/* Mask user data		*/
#define	XCMR_MUP	0x01		/* Mask user program		*/

#define DATASPACE	0xfffd0000	/* Data access space		*/
#define	TAGSPACE	0xfffe0000	/* Tag access space		*/
