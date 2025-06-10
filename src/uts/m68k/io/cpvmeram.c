/*	@(#)	cpvmeram.c	1.0	*/
/*
 * VMERAM related routines.
 * Also see "sys/cpvmeram.h" for further information.
 *
 * COPYRIGHT 1986 BY MOTOROLA INC.
 */

#include "sys/param.h"
#include "sys/types.h"
#include "sys/utsname.h"
#include "sys/elog.h"
#include "sys/erec.h"
#include "sys/cmn_err.h"
#include "sys/cpvmeram.h"

/* The following values are based on the
 * VME 2016 TOWER, BOARD STRAPPING AND CONFIGURATION GUIDE (Rev D),
 * written by Thomas S. Gardner, Motorola Computer Systems, Inc.
 */
static unsigned short *csr_loc[] = {
	(unsigned short *) 0xfffffee0, (unsigned short *) 0xfffffee2,
	(unsigned short *) 0xfffffee4, (unsigned short *) 0xfffffee6,
	(unsigned short *) 0xfffffee8, (unsigned short *) 0xfffffeea,
	(unsigned short *) 0xfffffeec, (unsigned short *) 0xfffffeee,
	(unsigned short *) 0xfffffef0, (unsigned short *) 0xfffffef2,
	(unsigned short *) 0xfffffef4, (unsigned short *) 0xfffffef6,
	(unsigned short *) 0xfffffef8, (unsigned short *) 0xfffffefa,
	(unsigned short *) 0xfffffefc, (unsigned short *) 0xfffffefe,
	NULL
};

#define CSR_CNT (sizeof(csr_loc)/sizeof(unsigned short *))

/* The pointers to the VMERAM's CSR's are held here;
 * the last one is followed by NULL pointers until the array is full.
 */
unsigned short *cpramcsr[CSR_CNT];

/* cpinit()
 *
 * Locate VMERAM VMERAM csr's and save their addresses in
 * the cpramcsr[] array by sensing each possible VMERAM
 * csr location; if no bus error occurs, then clear any
 * previous parity error indication, set enable parity
 * error report bit, and, save the address in the
 * cpramcsr[] array.  All empty entries, and always the
 * last entry, in the cpramcsr[] array are NULL.
 * Set-up single-bit correction logger function.
 */
/* initial mask for csr */
#define CP_INIT	~(CP_ERROR | CP_CORDA | CP_WBEDC | CP_BEDIS)

void cpinit()
{
	extern int bprobe();	/* returns non-zero on error */
	extern void timeout();

	void cptimer();

	register unsigned short syn, *csr;
	register int i, j;

	j = 0;
	for (i = 0; (csr = csr_loc[i]) != NULL; i++)
		if (!bprobe(csr, -1)) {
			syn = *csr;
			*csr = syn & CP_INIT;
			cpramcsr[j++] = csr;
		}
	cpramcsr[j] = NULL;

	timeout(cptimer, 0, CP_CHKCSR);
	return;
}

/* cpparity()
 *
 * Check csr's on VMERAM memory boards for parity error conditions.
 * If there is one (and there should only be one) then:
 *	- If the parity error is not a repeat of the last parity condition,
 *	  i.e., the board, syndrome bits, or, error location is different
 *	  from the last parity error.
 *		* save this error's board, syndrome bits, and location
 *		* reset parity error instance counter to zero.
 *	- Inform the console of the error condition if the parity error
 *	  instance counter is equivalent to zero modulo CP_MINCNT.
 *	- Increment parity error instance counter.
 *	- Log the error.
 *	- if this was a user mode parity error, then, attempt to fix the error:
 *		* Disable bus interrupt on parity error and error correction
 *		  for the offending board.
 *		* Read, then, write the 4 bytes starting at the error location.
 *		* Re-enable bus interrupt on parity error and correction logic.
 *		* Clear the csr parity error and error type bits.
 * Return non-zero if any csr's indicate a parity error condition.
 */
/* csr mask after parity error fix */
#define CP_CORRECTED (~(CP_ERROR | CP_BEDIS | CP_CORDA | CP_ERTYP))

/* mode argument values */
#define KERNEL 0
#define USER 1

int cpparity(loc, mode)
char *loc;
int mode;
{
	register unsigned short syn, *csr;
	register char *p;
	register int i, j, ch, status;

	static char *modename[] = { "Kernel", "User" };
	static char *parity_msg =
"VMERAM %s Mode Uncorrectable Error: csr addr = 0x%x; value = 0x%x\n";

    static unsigned short *lcsr = (unsigned short *)-1;	/* last csr address */
	static unsigned short lsyn = 0;			/* last syndrome bits */
	static char *lloc = (char *)-1;			/* last location */
	static int count = 0;				/* instance counter */

	status = 0;
	for (i = 0; (csr = cpramcsr[i]) != NULL; i++)
		if ((syn = *csr) & CP_ERROR) {
			if ((lcsr != csr) || (lsyn != syn) || (lloc != loc)) {
				lcsr = csr;
				lsyn = syn;
				lloc = loc;
				count = 0;
			}
			if (count % CP_MINCNT == 0)
				cmn_err(CE_WARN, parity_msg,
					modename[mode], csr, syn);
			++count;

			logmemory(syn, csr, E_UME);

			if (mode == USER) {
				*csr = syn | (CP_BEDIS | CP_CORDA);
				p = loc;
				for (j = 0; j < 4; j++) {
					ch = fubyte(p);
					subyte(p, ch);
					p++;
				}
				*csr = syn & CP_CORRECTED;
			}
			status--;
		}
	return(status);
}

/* cptimer()
 *
 * Check for corrected parity error conditions:
 *
 * Since the VMERAM VME Memory Board has single-bit error
 * correction capability, we must scan every CP_CHKCSR ticks to
 * determine whether any such errors have occurred.
 * A double-bit error is not correctable, and consequently results in a
 * bus error condition being generated, which is handled by cpparity().
 * In the case where an error condition is found,
 * only the error log file is informed.
 * Only the last corrected parity error's syndrome bits are recorded
 * in any given CP_CHKCSR ticks; intervening errors are lost..
 */
static void cptimer()
{
	extern void timeout(), logfixmem();

	register unsigned short syn, *csr, **csrp;

	csrp = cpramcsr;
	while ((csr = *csrp++) != NULL)
		if ((syn = *csr) & CP_ERROR) {
			logmemory(syn, csr, E_SBME);
			*csr = syn & ~CP_ERROR;
		}

	timeout(cptimer, 0, CP_CHKCSR);
	return;
}
