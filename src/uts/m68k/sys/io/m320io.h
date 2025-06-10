/* @(#)m320io.h	6.4  */

/*
  Slicing tables for mvme320 controller
*/

/* Supported on VME/10, VME120, and VME131 systems */

/*
 *      The slicing for the mvme320 disk controller
 *	is as follows:
 *
 *		Minor Number[0:2] defines one of 8 slices and
 *		Minor Number[3:4] defines the following devices:
 *
 *			0: hard device 0 (winchester)
 *			1: hard device 1 (winchester)
 *			2: floppy device 2
 *			3: floppy device 3
 *
 *		Minor Number[5] defines one of 2 controllers.
 *		Minor Number[6:7] is not used.
 */
#include "sys/io/win.h"
#include "sys/io/sa400.h"

struct	size m320_sizes[4][8] =
{
        DISK01
	DISK23
	WFLOPPY
	WFLOPPY
};

