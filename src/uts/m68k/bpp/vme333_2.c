#include  "sys/bpp/types.h"
/*

	This is the hardware interface for the second VME 333 board

*/


/* control register */
static USHORT *control = (USHORT *)(0xffff3900);

/* status register */
static USHORT *status = (USHORT *)(0xffff3900);

/* interrupt mask */
static USHORT intr_mask = 0x0100;

/* reset mask */
static USHORT rst_mask = 0x0200;

/* Multiple storage areas for reading and verifying VME333 status */
USHORT v33_2_status_1 = 0;		
USHORT v33_2_status_2 = 0;		
USHORT v33_2_status_3 = 0;		


v33_2_ctrl_write(data)
UBYTE data;
{
}
v33_2_data_write(data)
UBYTE data;
{
	*control = (data | intr_mask);
}
v33_2_intr_write()
{
	*control = intr_mask;
}
v33_2_status_read()
{
	v33_2_status_1 = (ULONG)*status;
	v33_2_status_2 = (ULONG)*status;

	/*
	**  Read status twice - if the two values don't match, then
	**   repeat the status reads until they do.  This code attempts
	**   to solve the problem that intermittently occurs when two
	**   VME333 boards are installed in a system:  the second board
	**   will not always respond and is therefore marked as "not
	**   running" by the host BPP driver.  Note, however, that this
	**   code has apparently not eliminated the problem, only reduced
	**   the frequency of occurance slightly.
	*/

	while (v33_2_status_1 != v33_2_status_2)
	{
		v33_2_status_3 = v33_2_status_1;
		v33_2_status_1 = v33_2_status_2;
		v33_2_status_2 = (ULONG)*status;
	}

	return(v33_2_status_1 & 0x07);
	
}
v33_2_rst_write()
{
	*control = rst_mask;
}
v33_2_probe()
{
	return(sprobe(control,-1));
}
