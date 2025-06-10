

/*IH*********************** INCLUDE HEADER ****************************
**
**  INCLUDE NAME:	trace.h
**
**  DESCRIPTION:	This file trace function definitions.
**
**  NOTES:
**
**  UPDATE LOG:
**  Date	Name		V/R	Comments
**  --------	-----------   	---	-----------------------------
**  05/21/86	Jim Holbrook	1.0	Original coding.
**
**********************************************************************/

#ifdef CETRACE

typedef struct trace_t
{
    ULONG	*trace_buf;	/* pointer to trace buffer */
    ULONG	trace_ptr;	/* index to current entry */
    USHORT	length_mask;	/* mask for wrap -- buffer is assumed
				   to be some modulo 2 length, i.e.,
				   0x3ff for 1024-byte buffer. */
} TRACE_T;

#endif

