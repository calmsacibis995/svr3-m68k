/*
 * File name:	checksum.c
 *
 * Description:	This file contains code to calculate a crc checksum
 *		for an array of characters.
 *
 * @(#) Copyright 1984 by Four-Phase Systems (ISG) of Motorola, Inc.
 *
 * Contents:
 *
 *	make_crctable()		Function that initializes the crc logic.
 *	checksum()		Function that calculates the checksum.
 *
 * Revision history:
 *
 *	07/05/84	RES	Created the functions.
 *
 */

#define	POLY	0xedb88320
static char *Version = "@(#)checksum.c 1.1 - 3/13/85";



/*
 * Function:	make_crctable()
 *
 * Description:	This function initializes a table of polynomials that are
 *		used by the checksum algorithm.  It is to be called once
 *		at the beginning of any program that uses the checksum
 *		function.
 *
 * Parameters:
 *
 *	table		Entry:  Pointer to an array of 256 longs.
 *			Exit:   The table will be filled with the polynomials.
 *
 * Globals:	None.
 *
 * Input:	None.
 *
 * Output:	None.
 *
 * Calls:	None.
 *
 * Returns:	Always returns normal termination.
 *
 * Status:	Tested.
 *
 * Pseudo-code:
 *
 *	Initialize polynomial to predefined 32 bit value. (xedb88320)
 *	Perform loop 256 times.
 *		Get loop counter variable value;
 *		Mask out first bit of loop counter variable value;
 *		Shift loop counter variable value right one bit.
 *		If (first bit on)
 *			XOR polynomial value into loop counter variable.
 *		Set next table cell with value of loop counter variable.
 *	Continue loop.
 *	Return with normal termination status.
 *
 */

make_crctable(table)
	register	unsigned	long	*table;
{
	register	short	loop_counter;
	register	unsigned	long	poly = POLY;
	register	unsigned	long	temp;
	register	unsigned	short	x;

	/* Fill the polynomial table now */
	for (loop_counter = 0; loop_counter < 256; ++loop_counter)
	{
		temp = (unsigned long)loop_counter;
		x = temp & 1;
		temp >>= 1;
		if (x)
			temp ^= poly;
		*table++ = temp;
	}
}


/*
 * Function:	checksum()
 *
 * Description:	This function will scan a user input array of characters
 *		and build a 32 bit polynomial checksum based on the
 *		contents of the array.
 *
 * Parameters:
 *
 *	table		Entry: Pointer to table of 32 bit polynomials.
 *			Exit:  Unchanged.
 *
 *	buffer		Entry: Pointer to character array to checksum.
 *			Exit:  Garbage.
 *
 *	length		Entry: Number of characters in buffer.
 *			Exit:  Unchanged.
 *
 *	crc		Entry: Value to be used as starting checksum.
 *			Exit:  Unchanged.
 *
 *	Note:  The crc entry may be the result of a previous checksum
 *		scan.  If it is used in this case, then the result will
 *		be a checksum that is logically calculated based on the
 *		two data buffers.  This logical calculation can be used
 *		with any number of buffers.
 *
 *		If you do not want to calculate logical checksums with
 *		more than one buffer of data this value must contain
 *		a -1.  Also, use -1 for the absolute first time that
 *		this checksum function is called.
 *
 * Globals:	None.
 *
 * Input:	None.
 *
 * Output:	None.
 *
 * Calls:	None.
 *
 * Returns:	The checksum value cakculated.
 *
 * Status:	Tested.
 *
 * Pseudo-code:
 *
 *	Perform a loop based on user length.
 *		Obtain the next character from the users buffer.
 *		XOR character into current value of checksum.
 *		Obtain first byte of checksum value. (first 8 bits)
 *		Logically shift checksum right one byte. (8 bits)
 *		XOR table[first byte of checksum value] into the checksum.
 *	Continue the loop.
 *	Return the value of the calculated checksum.
 *
 */

long	checksum(table, buffer, length, crc)
	unsigned	long	*table;
	register	char	*buffer;
	register	short	length;
	register	unsigned	long	crc;
{
	register	unsigned	char	cur_char;
	register	short	loop_counter;

	/* Calculate the new checksum now */
	for (loop_counter = 0; loop_counter < length; ++loop_counter)
	{
		cur_char = *buffer++;
		crc ^= cur_char;
		cur_char = crc & 0xff;
		crc >>= 8;
		crc ^= table[cur_char];
	}
	return (crc);
}
