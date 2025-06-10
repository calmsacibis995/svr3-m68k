/*	@(#)smd.h	7.3		*/
/*
 *	Copyright (c) 1985	AT&T 
 *	All Rights Reserved.
 */
/*	@(#)smd.h	5.1.1.4 MODIFIED		*/
/*
 *	The following partitioning defines "sizes" for each of
 *	the two "16" parts of a 32mb fixed/removable CMD disk
 *	or the "16" part of a 96mb fixed/removable CMD disk.
 *	16:16mb nominal 13.5:13.5 formatted
 *		   or
 *	80:16mb nominal 67.4:13.5 formatted
 */
#define CMD16\
	25984,	7,		/* cyl   7 thru 818 */\
	22784,	107,		/* cyl 107 thru 818 */\
	19584,	207,		/* cyl 207 thru 818 */\
	16384,	307,		/* cyl 307 thru 818 */\
	13184,	407,		/* cyl 407 thru 818 */\
	9984,	507,		/* cyl 507 thru 818 */\
	6784,	607,		/* cyl 607 thru 818 */\
	26336,	0,		/* cyl   0 thru 822 */

/*
 *	The following partitioning defines the "sizes" of
 *	the "80" portion of a 96mb fixed/removable CMD disk.
 *	80:16mb nominal 67.4:13.5 formatted
 */
#define CMD80\
	130080,	6,		/* cyl   6 thru 818 */\
	110880,	126,		/* cyl 126 thru 818 */\
	77760,	333,		/* cyl 333 thru 818 */\
	51840,	495,		/* cyl 495 thru 818 */\
	38880,	576,		/* cyl 576 thru 818 */\
	25920,	657,		/* cyl 657 thru 818 */\
	12960,	738,		/* cyl 738 thru 818 */\
	131680,	0,		/* cyl   0 thru 822 */
/*
 *	The following partitioning defines "sizes" of each
 *	of the two "25" portions of the 50mb LARK disk.
 *	25:25mb nominal 20.4:20.4 formatted
 */
#define LMD25\
	38144,	15,		/* cyl  15 thru 610 */\
	33344,	90,		/* cyl  90 thru 610 */\
	28544,	165,		/* cyl 165 thru 610 */\
	23744,	240,		/* cyl 240 thru 610 */\
	18944,	315,		/* cyl 315 thru 610 */\
	14144,	390,		/* cyl 390 thru 610 */\
	9344,	465,		/* cyl 465 thru 610 */\
	39936,	0,		/* cyl   0 thru 623 */
/*
 *	The following partitioning defines "sizes" of each
 *	of the two "8" portions of the 16mb LARK disk.
 *	8:8mb nominal  6.8:6.8  formatted
 */
#define LMD8\
	12736,	4,		/* cyl   4 thru 202 */\
	11136,	29,		/* cyl  29 thru 202 */\
	9536,	54,		/* cyl  54 thru 202 */\
	7936,	79,		/* cyl  79 thru 202 */\
	6336,	104,		/* cyl 104 thru 202 */\
	4736,	129,		/* cyl 129 thru 202 */\
	3136,	154,		/* cyl 154 thru 202 */\
	13184,	0,		/* cyl   0 thru 205 */
/*
 *      DISREGARD. FU168 IS NOT SUPPORTED. 
 *	The following partitioning defines "sizes" of 
 *	of the 168mb Fujitsu disk.
 *	168mb nominal  134.84mb  formatted
 */
#if 0
#define FU168\
	260160,	6,		/* cyl   6 thru 822 */\
	221760,	126,		/* cyl 126 thru 822 */\
	155520,	333,		/* cyl 333 thru 822 */\
	103680,	495,		/* cyl 495 thru 822 */\
	77760,	576,		/* cyl 576 thru 822 */\
	51840,	657,		/* cyl 657 thru 822 */\
	25920,	738,		/* cyl 738 thru 822 */\
	263360,	0,		/* cyl   0 thru 822 */
#endif
/*
 *	The following partitioning defines "sizes" of 
 *	of the 337mb Fujitsu disk.
 *	337mb nominal  269.68mb  formatted
 *	350 alternate tracks reserved at: cylinders 787 to 821 inclusive
 *	Cylinders 1 and 822 are diagnostic cylinders
 *
 */
#define FU337\
	502400,	2,		/* cyl   2 thru 786 */\
	459520,	69,		/* cyl  69 thru 786 */\
	379520,	194,		/* cyl 194 thru 786 */\
	299520,	319,		/* cyl 319 thru 786 */\
	219520,	444,		/* cyl 444 thru 786 */\
	139520,	569,		/* cyl 569 thru 786 */\
	59520,	694,		/* cyl 694 thru 786 */\
	526720,	0,		/* cyl   0 thru 822 */

/* pictorial view of slices for FU337
 * IO to the area labeled 'alt' will cause
 * an error.  Bottom row of numbers
 * are cylinders, second row size of area in bytes.
                                                    6-------|
                                            5---------------|
                                    4-----------------------|
                            3-------------------------------|
                    2---------------------------------------|
            1-----------------------------------------------|
     0------------------------------------------------------|
7-----------------------------------------------------------|--alt--|
|655K|21.95M|40.96M |40.96M |40.96M |40.96M |40.96M |30.47M | 11.47M|
| 0-1| 2-68 |69-193 |194-318|319-443|444-568|569-693|694-786|787-821|

*/

/*
 *	The following is a set of empty partitions
 *	referenced in place of real partitioning wherever a volume
 *	could exist but does not in the current configuration.
 */
#define NULLSMD\
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
