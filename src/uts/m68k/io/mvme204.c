/*	@(#)	mvme204.c	1.0	*/
/*
 * MVME204-1/-2 DUAL PORTED DYNAMIC MEMORY VMEmodule
 * related routines.  Also see "sys/mvme204.h" for
 * further information.
 *
 * COPYRIGHT 1986 BY MOTOROLA INC.
 */

#include "sys/param.h"
#include "sys/types.h"
#include "sys/utsname.h"
#include "sys/elog.h"
#include "sys/erec.h"
#include "sys/cmn_err.h"
#include "sys/mvme204.h"

/* The following values are based on the
 * VME 2016 TOWER, BOARD STRAPPING AND CONFIGURATION GUIDE (Rev B),
 * written by Thomas S. Gardner (4/86), Motorola Computer Systems, Inc.
 */
static unsigned char *csr_loc[] = {
	(unsigned char *) 0xffffbe01, (unsigned char *) 0xffffbe03,
	(unsigned char *) 0xffffbe05, (unsigned char *) 0xffffbe07,
	(unsigned char *) 0xffffbe09, (unsigned char *) 0xffffbe0b,
	(unsigned char *) 0xffffbe0d, NULL
};

#define CSR_CNT (sizeof(csr_loc)/sizeof(char *))


/* The pointers to the MVME204 CSR's are held here;
 * the last one is followed by NULL pointers until the array is full.
 */
unsigned char *m204csr[CSR_CNT];

/* m204init()
 *
 * Locate MVME204-1/-2 CSR's and save their addresses in
 * the m204csr[] array by sensing each possible MVME204
 * CSR location; if no bus error occurs, then clear any
 * previous parity error indication, set enable parity
 * error report bit, and, save the address in the
 * m204csr[] array.  The last valid entry in the m204csr[]
 * array is NULL.
 */
void m204init()
{
	extern int bprobe();	/* returns non-zero on error */

	register unsigned char syn, *csr;
	register int i, j;

	j = 0;
	for (i = 0; (csr = csr_loc[i]) != NULL; i++)
		if (!bprobe(csr, -1)) {
			syn = *csr;
			*csr = (syn & ~M204_PE) | M204_EPER;
			m204csr[j++] = csr;
		}
	m204csr[j] = NULL;
	return;
}

/* m204parity()
 *
 * Check csr's on MVME-204 memory boards for parity error conditions.
 * If there is one (and there should at most one) then:
 *	- If the parity error is not a repeat of the last parity condition,
 *	  i.e., the board, syndrome bits, or, error location is different
 *	  from the last parity error.
 *		* save this error's board, syndrome bits, and location
 *		* reset parity error instance counter to zero.
 *	- Inform the console of the error condition if the parity error
 *	  instance counter is equivalent to zero modulo M204_MINCNT.
 *	- Increment parity error instance counter.
 *	- Log the error.
 *	- if this was a user mode parity error, then, attempt to fix the error:
 *		* Disable bus interrupt on parity error.
 *		* Read, then, write the 4 bytes starting at the error location.
 *		* Re-enable bus interrupt on parity error.
 *		* Clear the csr parity error bit.
 * Return non-zero if any csr's indicate a parity error condition.
 */
/* mode argument values */
#define KERNEL 0
#define USER 1

int m204parity(loc, mode)
char *loc;
int mode;
{
	extern void logfailmem();

	register unsigned char syn, *csr;
	register char *p;
	register int i, j, ch, status;

	static char *modename[] = { "Kernel", "User" };
	static char *parity_msg =
		"MVME204 %s Mode Parity Error; csr addr = 0x%x; value = 0x%x\n";

      static unsigned char *lcsr = (unsigned char *)-1;	/* last csr address */
	static unsigned char lsyn = 0;			/* last syndrome bits */
	static char *lloc = (char *)-1;			/* last err location */
	static int count = 0;				/* similiar counter */

	status = 0;
	for (i = 0; (csr = m204csr[i]) != NULL; i++)
		if ((syn = *csr) & M204_PE) {
			if ((lcsr != csr) || (lsyn != syn) || (lloc != loc)) {
				lcsr = csr;
				lsyn = syn;
				lloc = loc;
				count = 0;
			}
			if (count % M204_MINCNT == 0)
				cmn_err(CE_WARN, parity_msg,
					modename[mode], csr, syn);
			count++;

			logmemory(syn, csr, E_UME);

			if (mode == USER) {
				*csr = syn & ~M204_EPER;
				p = (char *)loc;
				for (j = 0; j < 4; j++) {
					ch = fubyte(p);
					subyte(p, ch);
					p++;
				}
				*csr = (syn & ~M204_PE) | M204_EPER;
			}
			status--;
		}
	return(status);
}
