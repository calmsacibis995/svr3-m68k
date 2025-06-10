#ifdef CETRACE
/*MH*******************************************************************

MODULE NAME:	trace.c

DESCRIPTION:	Module trace function.  This routine manages a wrap-
		around trace buffer.
		
UPDATE LOG:
     Name	  Date	  Rev	  Comments
-------------	--------  ---   ---------------------------------------
J. Holbrook	05/21/86  1.0   Initial coding.
B. Volquardsen  11/11/86  2.0   Changed "common" include path to "sys/bpp"
***********************************************************************/

#include "sys/bpp/types.h"
#include "sys/bpp/trace.h"

VOID ci_trace(trace_hdr, arg1, arg2)

    register TRACE_T	*trace_hdr;
    ULONG	arg1, arg2;
{
	register ULONG	*ptr = trace_hdr->trace_buf;
	register ULONG  index = trace_hdr->trace_ptr;

	ptr[index++] = arg1;
	ptr[index++] = arg2;
	
	index = index & trace_hdr->length_mask;
	trace_hdr->trace_ptr = index;
	ptr[index] = 0x3e3e3e3e;
}
#endif
