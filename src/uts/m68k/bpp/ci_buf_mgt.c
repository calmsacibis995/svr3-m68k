static	char ident[] = "%W%	%Q%	%G%	Copyright MOTOROLA, INC.";
/*MH*******************************************************************

MODULE NAME:	ci_buf_mgt.c

DESCRIPTION:	Contains the functions necessary to perform the
		processing of requests for BUFs.  Also provides the
		function to determine when BUFs are to be requested
		or returned (NOT CURRENTLY IMPLEMENTED).
		
FUNCTIONS:	get_buf_request
		get_buf_response
		ret_buf_request
		ret_buf_response
		buf_supply

UPDATE LOG:
     Name	  Date	  V/R  	Comments
-------------	--------  ---   -------------------------------
B. Volquardsen	05/09/86  1.0	Initial Coding
J. Holbrook	05/18/86  1.0	Initialized next in get_buf_request
				 to NULL.
B. Volquardsen	05/21/86  1.0	Forced routines dealing with buffers
				 to use local_resource[0] and initial-
				 ized get_buf_head in get_buf_request 
				 function to NULL.
B. Volquardsen  06/10/86  1.0   Changed to local_resource[CI_MAX_CPU+1].
B. Volquardsen  11/11/86  2.0   Changed "common" include path to "sys/bpp"
***********************************************************************/

#ifndef R2
#include "sys/cmn_err.h"
#define panic(foo)	cmn_err(CE_PANIC, foo)
#endif

#include "sys/bpp/types.h"
#include "sys/bpp/panic.h"
#include "sys/bpp/ce.h"
#include "sys/bpp/ci_config.h"
#include "sys/bpp/ci_const.h"
#include "sys/bpp/ci_types.h"

#define MIN(x,y) ((x) < (y) ? (x) : (y))

extern	RES	lc[];		/* local resource table	*/

/*FH************************* FUNCTION HEADER ****************************

FUNCTION NAME:	get_buf_response

ACTION:		Processes responses from shared memory manager for
		CE_GETBUF requests. The response epb's buf pointer
		contains a pointer to a linked list of buffers or else
		is NULL.  The buffers are released to the local shared
		buffer pool.

METHOD:

UPDATE LOG:
     Name	  Date	  Rev	  Comments
--------------	--------  ---   ------------------------------------------
B. Volquardsen	05/09/86  1.0	Initial coding (copied from ce_bsh.c) and
				 modified slightly for UNIX environment.
B. Volquardsen  05/21/86  1.0   Forced routine to use local_resource[0].
B. Volquardsen  06/10/86  1.0   Changed to local_resource[CI_MAX_CPU+1].
*************************************************************************/

VOID get_buf_response(sepb)

CE_EPB *sepb;
{
	extern   UBYTE	*get_vme_address();
	extern	 RES	local_resource[];

	USHORT	 remote_cpu = CI_MAX_CPU+1;
	register BUF 	*sbuf;
	register BUF 	*next;
	register BUF	*vme_buf;
	register CE_EPB	*vme_epb = (CE_EPB *)get_vme_address(sepb);

	local_resource[remote_cpu].buf_request_pending = FALSE;

	if (vme_epb->hdr.type == CE_GETBUF_RESPONSE)
	{
		sbuf = vme_epb->hdr.buf;
		while (sbuf != (BUF *)NULL)
		{
			vme_buf = (BUF *)get_vme_address(sbuf);
			next = vme_buf->next;
			release_sbuf(remote_cpu, sbuf);
			sbuf = next;
		}
	}
	release_sepb(sepb);
}

/*FH************************* FUNCTION HEADER ****************************

FUNCTION NAME:	get_buf_request

ACTION:  	This routine handles requests for shared buffers from the
		local pool by the remote processor.  The number of buffers
		requested is in the buf_count field of the the epb.  It
		provides the smaller of the number of buffers requested or the
		number of buffers which will reduce the pool count to the
		minimum declared in the shared resource object.  A linked list
		of buffers is returned in a CE_GETBUF_RESPONSE epb.

METHOD:

UPDATE LOG:
     Name	  Date	  Rev	Comments
-------------	--------  ---   -----------------------------------------
Dana Holgate	 1/1/86	  1.0	Initial Release
J. Holbrook	05/08/86  1.0	Changed to manage get buf requests
				 from the remote processor.
J. Holbrook	05/18/86  1.0	Initialized next to NULL.
B. Volquardsen	05/19/86  1.0	Fixed initialization of "avail_cnt" and
				 deleted unused variables "err" & "qid".
B. Volquardsen	05/21/86  1.0	Forced routine to use local_resource[0]
				 for lrptr initialization and initialized
				 get_buf_head to NULL.
*************************************************************************/


get_buf_request(sepb)

    CE_EPB *sepb;
{
	extern   BUF 	*get_sbuf();
	extern	 UBYTE	*get_vme_address();
	extern 	 CE_EPB *get_sepb();

    		 USHORT  remote_cpu;
	register DEFAULT i;
		 USHORT  get_cnt;
		 USHORT  avail_cnt;
		 BUF 	*get_buf_head;
		 BUF 	*sbuf;
	register BUF 	*next;
		 CE_EPB	*rsp_epb;	/* response epb pointer */
		 CE_EPB	*vme_epb;	/* MAPPED epb pointer */
	register RES	*res_ptr;
	register BUF	*vme_buf;	/* MAPPED pointer to shared BUF */
		 POOL_T pool;		/* temporary holding place for
					   EPB pool id */

	avail_cnt = 0;
	res_ptr = &(local_resource[CI_MAX_CPU+1]);

	if ( res_ptr->current_bufs > res_ptr->minimum_bufs)
		avail_cnt = (res_ptr->current_bufs - res_ptr->minimum_bufs);
	get_cnt = MIN(sepb->buf_count, avail_cnt);

	remote_cpu = GET_CPU(sepb->hdr.src_id);

	if (rsp_epb = get_sepb(remote_cpu))
	{  /* get an epb for the response */
	    vme_epb = (CE_EPB *) get_vme_address(rsp_epb);
	    pool = vme_epb->hdr.pool;
	    copy_stp(sepb,rsp_epb,sizeof(CE_EPB));
	    vme_epb->hdr.pool = pool;
	    release_sepb(sepb);
	}
	else
	    panic(CI_NO_RSPEPB_PANIC);	/* die if we can't respond */

	get_buf_head = next = (BUF *)NULL;

	for (i = 0; i < get_cnt; i++)
	{
		if ((sbuf = get_sbuf(remote_cpu)) == (BUF *) NULL)
		{  /* inconsistent buffer pool condition -- panic */
		    panic(CI_BUFF_ERR_PANIC);
		}
		get_buf_head = sbuf;
		vme_buf = (BUF *)get_vme_address(sbuf);
		vme_buf->next = next;
		next = sbuf;
	}

	swap_id(rsp_epb);
	vme_epb->hdr.type  = CE_GETBUF_RESPONSE;
	vme_epb->hdr.buf   = get_buf_head;
	vme_epb->buf_count = get_cnt;

	if ( pwrite(rsp_epb) != NO_ERROR )
	{
		/*
		**  Pipe must have died before we tried to send response
		**  so just release all the response resources.
		*/
		for ( sbuf = vme_epb->hdr.buf,i = 1; i <= get_cnt; i++)
		{
			vme_buf = (BUF *)get_vme_address(sbuf);
			next = vme_buf->next;
			release_sbuf(remote_cpu,sbuf);
			sbuf = next;
		}
		release_sepb(rsp_epb);
	}
	return(0);
}



/*FH************************* FUNCTION HEADER ****************************

FUNCTION NAME:	ret_buf_request

ACTION:  	This routine handles requests for to return shared buffers
		by the remote processor.

METHOD:

UPDATE LOG:
     Name	  Date	  Rev	  Comments
-------------	--------  ---   ----------------------
J. Holbrook	05/08/86  1.0	Coded dummy funtion.
B. Volquardsen  05/21/86  1.0	Forced to use local_resource[0].
B. Volquardsen  06/10/86  1.0	Changed to local_resource[CI_MAX_CPU+1].
*************************************************************************/

ret_buf_request(sepb)
    CE_EPB	*sepb;

{
	extern   UBYTE	*get_vme_address();
	extern	 RES	local_resource[];

	USHORT	 remote_cpu;
	register BUF 	*sbuf;
	register BUF 	*next;
	register BUF	*vme_buf;
		 CE_EPB	*rsp_epb;	/* response epb pointer */
	register CE_EPB	*vme_epb;	/* mapped input epb ptr	*/
	POOL_T		pool;		/* temporary holding place for
					   EPB pool id */

	vme_epb = (CE_EPB *)get_vme_address(sepb);
	remote_cpu = GET_CPU(vme_epb->hdr.src_id);
	local_resource[CI_MAX_CPU+1].buf_request_pending = FALSE;

	sbuf = vme_epb->hdr.buf;
	while (sbuf != (BUF *)NULL)
	{
		vme_buf = (BUF *)get_vme_address(sbuf);
		next = vme_buf->next;
		release_sbuf(remote_cpu, sbuf);
		sbuf = next;
	}

	if (rsp_epb = get_sepb(remote_cpu))
	{  /* get an epb for the response */
	    vme_epb = (CE_EPB *) get_vme_address(rsp_epb);
	    pool = vme_epb->hdr.pool;
	    copy_stp(sepb,rsp_epb,sizeof(CE_EPB));
	    vme_epb->hdr.pool = pool;
	}
	else
	    panic(CI_NO_RSPEPB_PANIC);	/* die if we can't respond */

	release_sepb(sepb);

	swap_id(rsp_epb);
	vme_epb->hdr.type  = CE_RETBUF_RESPONSE;
	vme_epb->hdr.buf   = (BUF *) NULL;

	pwrite(rsp_epb);
	return(0);

}

/*FH************************* FUNCTION HEADER ****************************

FUNCTION NAME:	ret_buf_response

ACTION:  	This routine handles responses to requests to return 
		shared buffers.

METHOD:

UPDATE LOG:
     Name	  Date	     Vers/Rev	  Comments
-------------	--------     --------    ----------------------
J. Holbrook	05/08/86	1.0	Coded dummy funtion.
*************************************************************************/


ret_buf_response(sepb)

    CE_EPB	*sepb;

{
    USHORT	remote_cpu;
}

