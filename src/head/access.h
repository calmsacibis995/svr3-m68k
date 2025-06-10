/*		Copyright (c) 1985 Motorola, Inc.   */
/*		All Rights Reserved		*/

/*	@(#)access.h	1.2	10/3/85	*/

#define		DEV_SZ		32
#define		PATH_SZ	 	32
#define		SYS_SZ		20
#define		PERMS_FILE	"/etc/perms"
#define		ROOT		0
#define		ARG_COUNT	20
#define		MAX_STR		1000	/*  The largest char string array    */
#define		BS		512	/*  Block size                       */
#define		FMT		'7'	/*  The format slice                 */

struct  usrdev
  {
     char  real_dev[DEV_SZ];		/*  The real device to be accessed   */
     char  look_for[DEV_SZ];		/*  Device the user asked to access  */
     char  mntpt[PATH_SZ];		/*  The default mount point	     */
     char  fsize[SYS_SZ];		/*  Size of the file-system to make  */
     char  mkfs_dev[DEV_SZ];		/*  Dev to use for making fs's	     */
     char  modes[SYS_SZ];		/*  if readonly = R, otherwise RW[F] */
     char  pgm[DEV_SZ];			/*  the format program to use.       */
     char  fmt_dev[DEV_SZ];		/*  The device to be formatted       */
  };
