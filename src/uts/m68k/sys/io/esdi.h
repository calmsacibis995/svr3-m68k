
/*	@(#)esdi.h	*/
/*
 *	Copyright (c) 1987  Motorola 
 *	All Rights Reserved.
 */
/* CDC WREN 3 Drive  Model 94166 - 182
   Unformatted capacity	= 182.1 MB
   formatted capacity = 160.7 MB
   969 cylinders
   9 tracks per cylinder
   35 sectors per track ( actually 36, but one sector is reserved for slipping)
   cyl 0 and cyl 1 are configuration and diagnostic cylinders. 
   144 alternate tracks are reserved on cylinders 952 through 967 inclusive.
   cyl 968 contains manufacturers defect list which will be preserved. 
*/
#define CDC182MB \
	{ 0,630,299250 },    /* Partition 0, cyl 2 -   cyl 951 inclusive */\
        { 0,25200,274680 },  /* Partition 1, cyl 80 -  cyl 951 inclusive */\
        { 0,50400,249480 },  /* Partition 2, cyl 160 - cyl 951 inclusive */\
	{ 0,100800,199080 }, /* Partition 3, cyl 320 - cyl 951 inclusive */\
	{ 0,151200,148680 }, /* Partition 4, cyl 480 - cyl 951 inclusive */\
	{ 0,201600,98280 },  /* Partition 5, cyl 640 - cyl 951 inclusive */\
	{ 0,252000,47880 },  /* Partition 6, cyl 800 - cyl 951 inclusive */\
	{ 0,0,304920 },      /* Partition 7, cyl 0 -   cyl 967 inclusive */

