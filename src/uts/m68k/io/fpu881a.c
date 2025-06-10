/* @(#)fpu881a.c	6.2	 */

#include "sys/param.h"
#include "sys/types.h"
#include "sys/sysmacros.h"
#include "sys/fs/s5dir.h"
#include "sys/signal.h"
#include "sys/user.h"
#include "sys/trap.h"

extern void chk_fpu();
extern void fpu_UN_c();
extern void fsave();
extern void bcopy();


short do_fpu = 0;	/* Assume no fpu unless M68881
			** initialization decides otherwise.
			*/
fpu_info *fpu_ptr = &u.u_fpu;	/* Having this address makes assembly */
				/* language coding easier.	 */

char reset_fsave[8]; /* only 4 bytes needed for null FSAVE */
fregs reset_fregs;   /* null programmer's model */




void fpu_proc_init()	/* initialize u-vector 881-related info */
{
	bcopy(reset_fsave,u.u_fpu.fsave,sizeof(reset_fsave));

	frestore(u.u_fpu.fsave);	/* init the 881 */

	/* Clear the u-vector image of the 881 programmer's model 
	** in case sdb writes or reads from a pristine 881.	  */

	bcopy(&reset_fregs,&u.u_fpu.regs,sizeof(fregs));

	/* Init the exception operand, in case someone reads it 
	** at the wrong time. */

	bcopy(reset_fregs.reg,u.u_fpu.excop,sizeof(u.u_fpu.excop));

	u.u_fpu.ustate = 0;	/* Indicate no unusual conditions present in the
				** u-vector's 881 data area.		 */
}








/* initialize M68881 floating point data items, check existence of fpu */

void fpuinit()
{
	/* The following routine sets "do_fpu" true if we are able to 
	** execute an 881 instruction.  */

	chk_fpu(reset_fsave,&reset_fregs);
	if (do_fpu)
		fpu_proc_init();	/* init u-vector 881-related info */
	else
		u.u_fpu.fsave[0] = 0;	/* Indicate 881 in reset state (unused) 
					** in case sdb examines it.	*/
}



fpu_trap(type,code)	/* Handle an fpu exception; called from "os/trap.c" */
register type;
{
register code;

	switch (type) {

	case FPTBSUN: /* FPU exc. - Branch or Set on UNordered cond.*/
		code = KFPBSUN;
		fpu_UN_c(); /* Clear UN bit in the condition code byte */
				/* of the 881 status reg.  Otherwise	   */
				/* exception will recur.	           */
		break;
	case FPTINEX: /* FPU exc. - INEXact result */
		code = KFPINEX; break;
	case FPTDZ:	/* FPU exc. - Divide by Zero */
		code = KFPDZ; break;
	case FPTUNFL: /* FPU exc. - UNderFLow */
		code = KFPUNFL; break;
	case FPTOPER: /* FPU exc. - OPerand ERror */
		code = KFPOPER; break;
	case FPTOVFL: /* FPU exc. - OVerFLow */
		code = KFPOVFL; break;
	case FPTSNAN: /* FPU exc. - Signalling NAN */
		code = KFPSNAN; break;
	default:
		code = 0;	 /* illegal value */
		break;
	}

	fsave(u.u_fpu.fsave);		/* save 881 internal state */
					/* of the current process  */
	/* Save away the offending exceptional operand from the "fsave" area */
	bcopy(&u.u_fpu.fsave[8],u.u_fpu.excop,sizeof(u.u_fpu.excop));
	return(code);
}



fpu_wrt_ok(p) 	/* Is it OK for sdb to update a given 881 value 
		** in the u-vector of the child process?
		*/
register int *p;
{
register i;
register j;

	for (i=0; i<3; i++)	/* 3 longwords per fpi */
	for (j=0; j<8; j++)	/* 8 fpi registers     */
	if (p == (int *)&(u.u_fpu.regs.reg[j][i])) /* fp0..fp7 */
	       return(1);	/* OK to write u-vector location */

	if (   p == (int *)&(u.u_fpu.regs.control)	  /* control, status */
	    || p == (int *)&(u.u_fpu.regs.status))
	       return(1);	/* OK to write u-vector location */

	return(0);		/* otherwise, can't write the location */
}
