/*		Copyright (c) 1985 Motorola, Inc.   */
/*		All Rights Reserved		*/
/*	@(#)strins.c	1.1	8/21/85	*/

#include	"access.h"
#include	<stdio.h>
#include	<string.h>	/* wjc - MCS - 5/15/86 */

strins( string, ins, place )
char	*string;
char	*ins;
int	place;
{
  char	temp[MAX_STR];

 /*  The farthest place into the string is the end */
  if (  strlen( string )  <  place )  place = strlen( string );

 /*  Copy up to the place  */
  strncpy( temp, string, place );
  temp[place] = '\0';

 /*  Insert the new characters  */
  strcat( temp, ins );

 /*  Copy the rest of the string */
  strcat( temp, (string+place) );

 /*  Return the new string    */
  strcpy( string, temp );
}
