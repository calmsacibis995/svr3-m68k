/*
 *	Copyright (c) 1984, Communication Machinery Corporation
 */

/*
 *	This file contains material and information which is
 *	proprietary to Communication Machinery Corporation, and
 *	which may not be divulged without the express written
 *	permission of Communication Machinery Corporation.
 */

/*
 * This file defines values used by the networking software which on the
 * standard 4.2 software were picked up out of <sys/param.h>.  Since it
 * is not assured that the values defined in the 4.2 <sys/param.h> will be
 * in another system's <sys/param.h> file, the values actually needed are
 * supplied in this file.
 *
 * Created September 16, 1984 - Deb Brown, CMC
 */

#define MAXPATHLEN	1024		/* max length of file name */
#define NCARGS		10240		/* # chars in exec arglist */
#define NBBY		8		/* number bits per byte */
