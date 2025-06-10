/*		Copyright (c) 1985 Motorola, Inc.   */
/*		All Rights Reserved		*/

/*	@(#)getperms.c	1.1	created:8/21/85	*/

#include	<stdio.h>
#include	"access.h"

/*	Open the file PERMS_FILE to determine if it
 *	is okay for the average user to access this device.
 *
 *	This function returns:
 *	the real device to be mounted,
 *      ( given the alias which may also match the real device )
 *	the size of a file-system that can be made,
 *      if the file system is readonly R or read/write RW,
 *	the format program to use,      
 *	and what device if any may be formatted.
 */

getperms( dsk )

struct  usrdev   *dsk;

{
  FILE	*map;
  char	alias[DEV_SZ];



 /*  If you cannot open the permissions file stop */

  if  ( (map = fopen( PERMS_FILE, "r" ))  ==  NULL )
   {
      fprintf( stderr, "Could not open the permissions file, %s.\n", PERMS_FILE );
      exit( 2 );
   }


 /*	Read the file until a match of the real device or an alias is found */

  while (  (fscanf( map, "%s%s%s%s%s%s%*[^\n]", dsk->real_dev, alias, dsk->mntpt, dsk->fsize, dsk->modes, dsk->pgm )  !=  EOF )  )
    {
#ifdef  EBUG2
	fprintf( stderr, "getperms read:  real_dev = %s\n", dsk->real_dev );
	fprintf( stderr, "                alias    = %s\n", alias  );
	fprintf( stderr, "                mntpt    = %s\n", dsk->mntpt  );
	fprintf( stderr, "                fsize    = %s\n", dsk->fsize );
	fprintf( stderr, "                mode     = %s\n", dsk->modes );
	fprintf( stderr, "                pgm      = %s\n", dsk->pgm );
#endif
       if ( (strcmp( dsk->look_for, alias )  ==  0 ) | (strcmp( dsk->look_for, dsk->real_dev ) == 0 ) )
        {
          strcpy( dsk->fmt_dev, dsk->real_dev );   /* generate the fmt dev */
          dsk->fmt_dev[ strlen( dsk->fmt_dev ) - 1 ] = FMT;
          strins ( dsk->fmt_dev, "r", 5 );

          strcpy( dsk->mkfs_dev, dsk->real_dev );   /* generate the mkfs dev */
          strins ( dsk->mkfs_dev, "r", 5 );
#ifdef  EBUG1
	fprintf( stderr, "\ngetperms  SUCCEEDED \n" );
	fprintf( stderr, "and will return:  real_dev = %s\n", dsk->real_dev );
	fprintf( stderr, "                  alias    = %s\n", alias  );
	fprintf( stderr, "                  mntpt    = %s\n", dsk->mntpt  );
	fprintf( stderr, "                  fsize    = %s\n", dsk->fsize );
	fprintf( stderr, "                  mkfs_de  = %s\n", dsk->mkfs_dev );
	fprintf( stderr, "                  mode     = %s\n", dsk->modes );
	fprintf( stderr, "                  pgm      = %s\n", dsk->pgm );
	fprintf( stderr, "                  fmt_dev  = %s\n", dsk->fmt_dev );
#endif
          return( 0 );
        }
    }
#ifdef  EBUG1
	fprintf( stderr, "\ngetperms FAILED to find a match\n" );
#endif
    return( 1 );
}
