

/*IH***************************** INCLUDE HEADER *******************************
**
**  MODULE NAME:	config.h
**
**  DESCRIPTION:    	This include file contains definitions pertaining
**			to the configuration of the Common Interface.
**
**  NOTES:
**
*  CHANGES:     DESCRIPTION                       DATE     ENGINEER
*  ___________________________________________________________________
*  Original coding.				04/16/86	jeh
*  Changed to use ptrs to functions		05/02/86	jeh
*  Added fields in ci_cputab_t structure to 	06/06/86	wlv
*   allow number and size of BPE's, EPB's, 
*   and buffers to be separately configured
*   on an individual board basis.		
*
*******************************************************************************/


#define	CI_MAX_CPU	3	/* Number remote cpu's configured */

/* Board type definitions */

#define	VME330_BOARD	1
#define VME333_BOARD	2
#define VME372_BOARD	3
#define VME131_BOARD	4

/* ICC configuration information structure */

typedef struct ci_cputab_t
{
    UBYTE	cpu_number;		/* BPP cpu designation for this
					   processor.  this number is used
					   for the cpu number in designating
					   the interface point (IP) to BPP. */

    UBYTE	cpu_type;		/* Identifies the type of board */

    ULONG	(*init_func)();		/* pointer to board-specific init
					   function to execute instead of
					   the standard initialization
					   function.  This field is NULL if
					   no special function exists. */

    ULONG	(*probe)();		/* function to probe for presence of
					   board.  Currently this is either
					   bprobe or sprobe. */

    ULONG	(*ctrl_write)();	/* pointer to function that writes
					   control register address */

    ULONG	(*data_write)();	/* Pointer to function to write data
					   to cpu.  We assume that every cpu 
					   can receive at least a byte. */

    ULONG	(*status_read)();	/* pointer to function to read status
					   from the cpu */

    ULONG	(*intr_write)();	/* pointer to function that interrupts
					   the cpu.  The interrupt mask is
					   passed as the argument to this
					   function call. */

    ULONG	icc_intr_mask;		/* Value which must be written to 
					   icc_interrupt to interrupt the cpu.
					   Not all icc's require a specific
					   value, but some may. */

    ULONG	(*rst_write)();		/* Pointer to function that performs
					   cpu reset.  The rest mask is
					   passed as an argument to this
					   function. */

    ULONG	icc_rst_mask;		/* Value to write to the icc_reset
					   address to execute a board reset */

    UBYTE	icc_intr_level;		/* interrupt level that this cpu should
					   use in talking to the Unix host */

    UBYTE	icc_intr_vector;	/* interrupt vector that this cpu will
					   present to the Unix host.  If the
					   cpu cannot generate a vector, this
					   may be one of the host's auto
					   vectors */

    UBYTE	init_type;		/* currently unused -- to encode and
					   board-specific information we may
					   encounter */

    UBYTE	cpu_alive;		/* flag for indication of whether
					   this cpu is alive or dead */

    UBYTE	cpu_running;		/* flag to indicate that this cpu has
					   been initialized and is running */

    SHORT	num_local_bpes;		/* number of buffer pipe envelopes to
					   allocate for local (host) use. */

    SHORT	num_remote_bpes;	/* number of buffer pipe envelopes to
					   allocate for remote board use. */

    SHORT	epb_size;		/* Size of EPB's for this board	*/

    SHORT	num_local_epbs;		/* number of EPB's to allocate
					   for local (host) use.	*/

    SHORT	num_remote_epbs;	/* number of EPB's to allocate
					   for remote board use.	*/

    SHORT	buf_size;		/* Size of buffers to allocate
					   for this board.		*/

    SHORT	num_local_bufs;		/* number of buffers to allocate
					   for local (host) use		*/

    SHORT	num_remote_bufs;	/* number of buffers to allocate
					   for remote board use		*/

    SHORT	min_remote_bufs;	/* minimum number of buffers that
					   board should hang onto.	*/

    SHORT	max_remote_bufs;	/* maximum number of buffers that
					   board should hang onto.	*/

} CI_CPUTAB_T;

