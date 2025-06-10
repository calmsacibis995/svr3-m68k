/*		Copyright (c) 1985 AT&T		*/
/*		All Rights Reserved		*/
/*	@(#)io.h	6.3	 */

struct size {
	daddr_t nblocks;
	int	cyloff;
};

#include "sys/io/m320io.h"
#include "sys/io/m360io.h"
#include "sys/io/m323io.h" 
#include "sys/io/ramdio.h"
