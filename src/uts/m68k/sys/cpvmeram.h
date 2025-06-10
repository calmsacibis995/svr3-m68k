/*	@(#)	cpvmeram.h	1.0	*/
/*
 * See the CLEARPOINT VMERAM USER's MANUAL, Revision 1.0 (Preliminary),
 * Chapter 3, "Use Of The Control And Status Register" for more
 * information on the following CSR values.
 */
#define CP_ERROR	0x8000	/* Error since last SYSRESET	*/
#define CP_CORDA	0x4000	/* Correction Disable		*/
#define CP_WBEDC	0x2000	/* Write Bad EDC		*/
#define CP_BEDIS	0x1000	/* Bus Error Disable		*/
#define CP_ERTYP	0x0800	/* Error Type (double/single)	*/
#define CP_ROWIE	0x0400	/* Row In Error			*/
#define CP_SYNDR	0x03ff	/* Syndrome Bits		*/

/* timeout() time parameter for checking CSR's for
 * single-bit error corrections
 */
#define CP_CHKCSR	(1 * HZ)

/* minimum count for consecutive similiar error
 * conditions before message is displayed on console
 */
#define CP_MINCNT 20
