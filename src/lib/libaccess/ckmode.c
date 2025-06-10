/*		Copyright (c) 1985 Motorola, Inc.   */
/*		All Rights Reserved		*/
/*	@(#)ckmode.c	1.1	8/21/85	*/

#include	"access.h"
#include	<stdio.h>
#include	<string.h>	/* wjc - MCS - 5/15/86 */

/*	ckmode():	Determines if the given device has a specific
 *			permission flag set.  If the permission asked
 *			for does not exist then the routine will return
 *			a ( 1 ).  This will cause a 
 *			if ( ckmode() )
 *			 {
 *			   print error; exit
 *			 }
 *			to work properly.
 */

ckmode ( dev, ask )
struct usrdev *dev;
char    ask;
{
  if  (  strchr( dev->modes, ask )  ==  NULL  )
   {
      return( 1 );
   }
 else
   {
     return( 0 );
   }
}
