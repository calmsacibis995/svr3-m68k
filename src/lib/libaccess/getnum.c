/*		Copyright (c) 1985 Motorola, Inc.   */
/*		All Rights Reserved		*/
/*	@(#)getnum.c	1.1	8/21/85	*/

#include	"access.h"
#include	<stdio.h>

getnum( value )
char	*value;
{
  register n;
  char     *strpbrk();

 /*   Skip the leading white space.  */
  if  (  (value = strpbrk( value, "0123456789" ) )  == NULL )
   {
#ifdef EBUG
      fprintf( stderr, "Invalid numeric argument: %s\n", value );
#endif
      return ( -1 );
   }

 /*   Calculate the numeric value. */
  n = 0;
  while ( *value >= '0' && *value <= '9')
	n = n*10 + *value++ - '0';

 /*  
  *  Now add the value of the alphabetical characters.
  *  Note if another numberic character appear this is an error.
  */
  while  (  *value < '0'  ||  *value > '9' )
    {
      switch ( *value++ )
        {
          case  ' ':
          case '\t':
                     continue;   /*  skip  whitespace  */
          case  'k':
                     n *= 1024;
	             continue;
          case  'b':
	             n *= 512;
                     continue;
          case  'w':
                     n *= 2;
                     continue;
          case  '*':
          case  'x':
	             return ( n * getnum( value ) ); 
          case  ':':
                     return ( n );  /* given fsize:inodes */
          case '\0':
		     return ( n );
          default  :
                     fprintf( stderr, "Unknown numeric multipier\n" );
                     exit ( 1 );
        }
      fprintf( stderr,  " Mulitple numeric args given for a single flag.\n" );
    }
}
