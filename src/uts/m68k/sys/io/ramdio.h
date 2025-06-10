/*
 *	Copyright (c) 1986 Motorola
 *	All Rights Reserved	
 *
 *		@(#)ramdio.h	6.1
 *
 *	slice table for ramdisk driver
 *		see "sys/io.h"
 *			and		
 *		/usr/src/usr/m68k/io/ramdisk.c
 *
 *	swap (slice 1) must be at least BSIZE, or any attempt to
 *	access it will cause an 'i/o error in swap' panic
 */

struct	size ramd_sizes[8] =
{
	/* size		starting address */
	0x10c400,	0xf3800,
	0x400,		0x1ffc00,
	0x0,		0,
	0x0,		0,
	0x0,		0,
	0x0,		0,
	0x0,		0,
	0x0,		0,
};

