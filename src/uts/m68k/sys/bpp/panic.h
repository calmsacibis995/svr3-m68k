
/*%W%	%Q%	%G%*/
/*IH*********************** INCLUDE HEADER ****************************
**
**  INCLUDE NAME:	panic.h
**
**  DESCRIPTION:	This file contains panic code definitions.
**
**  NOTES:
**
**  UPDATE LOG:
**  Date	Name		V/R	Comments
**  --------	-----------   	---	------------------------------
**  05/08/86	Jim Holbrook	1.0	Original coding.
**  05/21/86	B. Volquardsen	1.0	Added buffer panic codes 11-13.
**  05/23/86	Jim Holbrook	1.0	Changed defines to quote strings.
**
**********************************************************************/

#define	CI_BUFF_ERR_PANIC	"ci: buffer error panic"	/* invalid buffer pool condition */
#define CI_NORSP_PANIC		"ci: no response epb panic"	/* No epb to send response */
#define	CI_VRTX_QERR_PANIC	"ci: vrtx queue error panic"	/* VRTX queue post error */
#define CI_VRTX_ERR_PANIC	"ci: vrtx error panic"	/* VRTX initialization problem */
#define CI_NO_PIPE_PANIC	"ci: no pipe panic"	/* Pipe doesn't exist */
#define CI_BAD_VERSION_PANIC	"ci: incompatible versions"	/* inconsistent BPP versions */
#define	CI_BAD_EPB_POOL_PANIC	"ci: bad epb pool"	/* invalid pool state */
#define	CI_BAD_BUF_POOL_PANIC	"ci: bad buf pool"	/* invalid pool state */
#define CI_NO_RSPEPB_PANIC	"ci: no epb for response"	/* no shared EPB to send response */
#define CI_BUF_INIT_PANIC	"ci: buf init failed"	/* buffer pool initialization failed */
#define CI_BUF_NOT_ACTIVE_PANIC "ci: buffer not active"	/* attempt to release a free buffer */
#define CI_BUF_NOT_FREE_PANIC	"ci: buffer not free"	/* got an ACTIVE buffer from pool */
#define CI_BAD_BUF_ID_PANIC	"ci: bad buf id"	/* invalid buffer ID field on get_sbuf*/
#define CI_BAD_MSG_LENGTH	"ci: bad buf msg length"	/* invalid BUF length field */
#define CI_PWRITE_BAD_CPU	"ci: pwrite bad cpu"	/* cpu number out of configured range */
#define CI_PREAD_BAD_CPU	"ci: pread bad cpu"	/* cpu number out of configured range */
#define CI_PEPB_REL_ERR		"ci: bad pepb release"	/* error status returnedby VRTX */
