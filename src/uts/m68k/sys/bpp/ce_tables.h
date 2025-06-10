/*%W%	%Q%	%G%*/
/*IH*********************** INCLUDE HEADER ****************************
**
**  INCLUDE NAME:	ce_tables.h
**
**  DESCRIPTION:	This file defines all of the table structures
**			used by the common environment initialization
**			routine, "ce_init()".
**
**  NOTES:		These structure definitions are compatible
**			with VRTX version 3.12.  The instantiation
**			of these structures occurs in "ce_tables.c".
**
**  UPDATE LOG:
**  Date	Name		V/R	Comments
**  --------	--------------	---	-----------------------------
**  02/11/86	Roy K. Larsen	1.0	Original coding.
**  03/11/86	Dana Holgate	1.0	Stolen from o_tables.h
**  04/03/86	B. Volquardsen	1.0	Added "hw_init_entry".
**  04/05/86	B. Volquardsen	1.0	Changed "evt_entry" & "tcreate_
**					params" definitions to use PTF
**					typedef instead of BYTE*.
**********************************************************************/

/*
**  The following is the structure for the stack table.
*/

struct	stack_entry
{
	BYTE	 tid;
	SHORT	 stack_size;
	BYTE	*stack;
};

/*
**  The following structure defines an EVT table entry.
**  value in the table.
*/

struct	evt_entry
{
	PTF	*vector_addr;
	PTF 	exception_handler;
};

/*
**  The following structure is used to create all VRTX tasks. 
**  	It defines the task creation table elements.
*/

struct tcreate_params
{
	PTF 	task_addr;		/* task entry point  	*/
	DEFAULT	tid;			/* task id		*/
	DEFAULT	pri;			/* task priority	*/
	DEFAULT mode;			/* task mode		*/
};

/*
**  The following structure defines the queue creation table elements.
*/

struct	qcreate_params
{
	DEFAULT	qid;
	DEFAULT	qsize;
};

/*
**  The following structure defines partition creation table elements.
**  It is not currently used for ce_init() because of rom variables
**  that define block sizes for VRTX partitions.
*/

struct	pcreate_params
{
	DEFAULT	pid;			/* partition ID number	*/
	UBYTE 	*paddr;			/* partition base addr	*/
	DEFAULT	psize;			/* partition size 	*/
	DEFAULT bsize;			/* partition block size	*/
};

/*
**  The following structure defines the hardware initialization table 
**  structure.
*/

struct 	hw_init_entry
{
	UBYTE	*reg_addr;		/* h/w register address */
	UBYTE	value;			/* value to be set	*/
	BOOLEAN read_flag;		/* read register flag	*/
};
