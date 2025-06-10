/*	@(#)m360io.h	7.3 	*/
/*
 *	Copyright (c) 1985	Motorola Inc.  
 *		All Rights Reserved.
 */

/*
 *	This files creates "m360_sizes" for use
 *		by the MVME360 disk controller driver.
 */


#include "sys/io/smd.h"
#include "sys/io/sa800.h"
#include "sys/io/sa400.h"

/*
 *	M360 disk types slicing
 *	See m360space.h for the
 *	definition of disk types.
 */

struct	size m360_sizes[M360TYPES][8] = {
	NULLSMD			/* 2nd volume in single volume configurations */
	FU337			/* 337 Mbyte Fujitsu */
};

