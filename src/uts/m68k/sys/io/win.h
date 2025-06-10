/*	@(#)win.h	7.5		*/
/*
 *	Partitioning for winchester disks supported by
 *	controllers M68RWIN1/2, MVME315/319 and MVME320
 */

/*
 * 5mb winchester disk
 * 5mb nominal 4.8mb available 
 * Used to define drives 0 and 1 
 *
 * 306 cylinders, 2 heads, 16 blocks/track
 * 12 cylinders set aside for alternates
 */
#define WD5\
	9408,	 0,\
	8232,	 1176,\
	7056,	 2352,\
	5880,	 3528,\
	4704,	 4704,\
	3528,	 5880,\
	2352,	 7056,\
	1176,	 8232,

/*
 * 15mb winchester disk
 * 15mb nominal 14.1mb available
 * Used to define drives 0 and 1 
 *
 * Micropolis
 * 306 cylinders, 6 heads, 16 blocks/track
 * 12 cylinders set aside for alternates
 */
#define WD15\
	24504,   192,\
	21168,   7056,\
	17640,	 10584,\
	14112,	 14112,\
	10584,	 17640,\
	7056,	 21168,\
	3528,	 24696,\
	28224,   0,

/*
* 40 mb GENERIC winchester disk
* 40 mb nominal  
* Used to define drives 0 and/or 1
*
* Micropolis
* 830 cylinders, 6 heads, 16 blocks/track
* 12 cylinders set aside for alternates
*
* Vertex
* 987 cylinders, 5 heads, 16 blocks/track
* 12 cylinders set aside for alternates
*
* Tandon
* 981 cylinders, 5 heads, 16 blocks/track
* 12 cylinders set aside for alternates
*
* Miniscrib
* 1024 cylinders, 5 heads, 16 blocks/track
* 12 cylinders set aside for alternates
*/
#define WD40\
	77328,	192,\
	58728,	18792,\
	53328,	24192,\
	49296,	28224,\
	38880,	38640,\
	30696,	46824,\
	14448,	63072,\
	77520,	0,

/*
 * 70mb GENERIC winchester disk
 * 70mb nominal 64.3mb available 
 * Used to define drives 0 and 1 
 *
 * Micropolis #1325 and Miniscribe #6085
 * 1024 cylinders, 8 heads, 16 blocks/track
 * 13 cylinders set aside for alternates
 *
 * Toshiba MK-56FA
 * 830 cylinders, 10 heads, 16 blocks/track
 * 10 cylinders set aside for alternates
 *
 * Priam
 * 1166 cylinders, 7 heads, 16 blocks/track
 * 101 tracks set aside for alternates
 * ( This drive does NOT have partitions on
 *   cylinder boundaries. )
 *
 * note: Not all of the alternate cylinders are
 *       expected to be used.  The relatively
 *       large number of alternates was used to
 *       "fill" to cylinder boundaries on the
 *	 drives.
 */
/* pictorial view of slices for WD70
							           6--------|
						          5-----------------|
					         4--------------------------|
				       3------------------------------------|
	                     2----------------------------------------------|
                   1--------------------------------------------------------|
         0------------------------------------------------------------------|
7---------------------------------------------------------------------------|
|  640K  |  13440K |  8640K  |  7680K  |  7680K  |  8000K |  9600K |  9792K |

*/

#define WD70\
	128640,	1280,\
	101760,	28160,\
	84480,	45440,\
	69120,	60800,\
	53760,	76160,\
	37760,	92160,\
	18560,	111360,\
	131072,	0,

/*
 * 140mb winchester disk
 * 140mb nominal 110mb available
 * Used to define drives 0 and 1
 * 
 * Maxtor XT1140
 * 918 cylinders, 15 heads, 16 blocks/track
 * 10 cylinders set aside for alternates
 *
 */
#define WD140\
	217200,	1680,\
	186480,	32400,\
	154080,	64800,\
	125280,	93600,\
	96480,	122400,\
	67680,	151200,\
	38880,	180000,\
	220320,	0,

/*
 *
 */
