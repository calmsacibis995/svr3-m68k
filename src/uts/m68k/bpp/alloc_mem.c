/*MH***************************** MODULE HEADER *******************************
**
**  MODULE NAME:	alloc_mem.c
**
**  DESCRIPTION:    	alloc_mem() attempts to get a contiguous
**			block of memory for the EPB, BPE and BUF 
**			pools that is not split across a four (4)
**			megabyte boundary.
**
**  Global Data:	If successful, memadr and memsz are set up
**			for use by ci_init().
**
**  External Refs:
**
**  Input Parameters:	chunksz - size of needed memory block
**
**  Return Parameters: 	SUCCESS
**			FAILURE
**			errflag
**
**  NOTES:
**
**  CHANGES:     DESCRIPTION                       DATE     ENGINEER
**  ___________________________________________________________________
**  Extracted from bppci.c and deleted Unix	11/12/86	wlv
**   version 5.1 compatibility code.
*******************************************************************************/

#include "sys/errno.h"
#include "sys/param.h"
#include "sys/sysmacros.h"
#include "sys/types.h"
#include "sys/signal.h"
#include "sys/dir.h"
#ifndef R2
#include "sys/immu.h"
#include "sys/region.h"
#endif
#include "sys/proc.h"
#include "sys/user.h"

#include "sys/bpp/types.h"
#include "sys/bpp/ce.h"
#include "sys/bpp/ci_types.h"
#include "sys/bpp/ci_const.h"
#include "sys/bpp/ci_config.h"
#define FOUR_MEG_SPLIT(a, s)	((a & 0xc00000) != ((a+s-1) & 0xc00000))

#ifdef DEBUG
#define DEBUG1(str,parm) { printf(str,parm); }
#define DEBUG2(str,parm1,parm2) { printf(str,parm1,parm2); }
#else
#define DEBUG1(str,parm) 
#define DEBUG2(str,parm1,parm2) 
#endif

/* GLOBAL VARIABLES */
static	BYTE	devname[] = "bppci_";	/* device name for printf()*/
	ADDR	memadr;			/* physical memory address */
	LONG	memsz;			/* memory size */



alloc_mem(chunksz)
LONG    chunksz;
{
        LONG    i, n;
        LONG    clicks;
        ADDR    addr_a, addr_b;
        LONG    size_a, size_b;
        BYTE    errflg;

        DEBUG1("%salloc\n",devname);    /* #### */

        clicks = btoc(chunksz);
        memsz  = ctob(clicks);

#ifdef R2
	memadr = ctob(mmualloc(clicks));
        if (memadr == 0)
                goto memerror;

	addr_a = memadr;

        if(memadr < OFF_BOARD_MEM)
        {                               /* not off-board, keep trying */
                errflg = alloc_mem(chunksz);
                mmufree(btoc(memsz), btoc(addr_a));
                return(errflg);
        }
        else
        {                               /* got off-board              */

                if (FOUR_MEG_SPLIT(memadr, memsz))
                {                       /* straddling 4-meg boundary  */
                                        /* try one more time          */
                        addr_a = memadr;
                        addr_b = memadr + 0x0400000;
                        size_a = addr_b - addr_a;
                        size_b = memsz - size_a;

                        mmufree(btoc(size_b), btoc(addr_b));
                        memadr = ctob(mmualloc(clicks));
                        if ( memadr == 0 )
                        {
                                mmufree(btoc(size_a), btoc(addr_a));
                                goto memerror;
                        }
                        if (FOUR_MEG_SPLIT(memadr, memsz))
                                goto memerror;
                        mmufree(btoc(size_a), btoc(addr_a));
                }
                return(SUCCESS);
        }
#else
	/****************************************/
	/* NEW MEMORY ALLOCATION SCHEME FOR 5.3 */
	/****************************************/

	memadr = getcpages(clicks, 1);
	if (memadr == 0)
		goto memerror;

	addr_a = memadr;

	if (FOUR_MEG_SPLIT(memadr, memsz)) 
	{
		/*
		 * WE are straddling a 4 Meg boundary.  Release memory
		 * past the boundary, try to alloc again, and if succesful,
		 * then release the stuff we were holding on to below the
		 * boundary.
		 */
		addr_a = memadr;
		addr_b = memadr + 0x0400000;
		size_a = addr_b - addr_a;
		size_b = memsz - size_a;

		DEBUG2("alloc_mem: straddle: addr_a = %x size_a = %x\n",
			addr_a, memsz);
		DEBUG2("                     addr_b = %x size_b = %x\n",
			addr_b, size_b);

		freecpages(btoc(addr_b), btoc(size_b));

		memadr = getcpages(clicks, 1);
		if (memadr == 0) 
		{
			freecpages(addr_a, btoc(size_a));
			goto memerror;
		}

		if (FOUR_MEG_SPLIT(memadr, memsz))
			goto memerror;

		freecpages(addr_a, btoc(size_a));
	}

	DEBUG2("%sinit: SUCCESS! memadr = %x, memsz = %x\n", memadr, memsz);

	return(SUCCESS);

#endif

memerror:
        printf("\n%sinit:  memory allocation error!\n",devname);

        return(FAILURE);
}
