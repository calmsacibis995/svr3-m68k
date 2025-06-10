#include  "sys/bpp/types.h"


/*

	This is the hardware interface to the first VME 330 board.

*/


/* control register */
static UBYTE *control =  (UBYTE *)(0xdc0000+0x2804);

/* data register */
static UBYTE *data_ptr = (UBYTE *)(0xdc0000+0x2805);

/* status register */
static UBYTE *status = (UBYTE *)(0xdc0000+0x2806);

/* interrupt register -- any of the
   last 512 bytes of the 128k window
   will do
*/
static UBYTE *interrupt = (UBYTE *)(0xdc0000+0x1fe00);

/*
	board reset address  -- this is the
	same as the control register address.
	No hardware reset function on this
	board
*/
static UBYTE *reset = (UBYTE *)(0xdc0000+0x2807);

/* interrupt mask */
static UBYTE intr_mask = 0xff;

/* reset mask */
static UBYTE rst_mask = 0x01;


v30_1_ctrl_write(data)
ULONG data;
{
}
v30_1_data_write(data)
ULONG data;
{
	*data_ptr = data;
	*interrupt = intr_mask;
}
v30_1_intr_write()
{
	*interrupt = intr_mask;
}
v30_1_status_read()
{
	return((ULONG)*status);
}
v30_1_rst_write()
{
	*data_ptr = rst_mask;
	*interrupt = intr_mask;
}
v30_1_probe()
{
	return(bprobe(data_ptr,-1));
}

