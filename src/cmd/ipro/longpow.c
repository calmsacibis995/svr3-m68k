/*********************************************************************/
/*********************************************************************/
/***                                                               ***/
/***                                                               ***/
/***  (C) Copyright  Motorola, Inc.  1985, 1986, 1987              ***/
/***  All Rights Reserved                                          ***/
/***  Motorola Confidential/Proprietary                            ***/
/***                                                               ***/
/***                                                               ***/
/***  SOFTWARE PRODUCT:  IPRO                                      ***/
/***                                                               ***/
/***                                                               ***/
/***  FUNCTION:  longpow - calculates a long base raised to a      ***/
/***                       long power.                             ***/
/***                                                               ***/
/***                                                               ***/
/***  AUTHOR:  Norman L. Nealy                                     ***/
/***           Senior Software Development Engineer                ***/
/***           Motorola Microcomputer Division                     ***/
/***           3013 South 52nd Street  MS-J3                       ***/
/***           Tempe, Arizona  85282                               ***/
/***           (602) 438-5724/5600                                 ***/
/***                                                               ***/
/***                                                               ***/
/***  ORIGINAL DATE:  November 1, 1985                             ***/
/***                                                               ***/
/***                                                               ***/
/***  ALGORITHM:                                                   ***/
/***                                                               ***/
/***     begin long_power                                          ***/
/***                                                               ***/
/***        for each exponent greater than zero do                 ***/
/***                                                               ***/
/***           multiply the resultant with the base                ***/
/***                                                               ***/
/***        end for each exponent                                  ***/
/***                                                               ***/
/***        return the long result                                 ***/
/***                                                               ***/
/***     end long_power                                            ***/
/***                                                               ***/
/***                                                               ***/
/***  RETURN VALUES:  long result of the long base raised to the   ***/
/***                  long power.                                  ***/
/***                                                               ***/
/***                                                               ***/
/***  SOURCE LOCATION:  /usr/src/cmd/ipro/longpow.c                ***/
/***                                                               ***/
/***                                                               ***/
/*********************************************************************/
/*********************************************************************/


#include "iprodefs.h"


   LONG
longpow( base, exponent )

   LONG base;
   LONG exponent;
{
   LONG result   = ONE;

   /************************************************************/
   /************************************************************/
   /*                                                          */
   /* begin long_power                                         */
   /*                                                          */
   /************************************************************/

   if ( result NE ONE )
   {
      return( ONE );
   }

      /*********************************************************/
      /*********************************************************/
      /*                                                       */
      /* for each exponent greater than zero do                */
      /*                                                       */
      /*    multiply the resultant with the base               */
      /*                                                       */
      /* end for each exponent                                 */
      /*                                                       */
      /* return the long result                                */
      /*                                                       */
      /*********************************************************/

   while ( exponent-- GT ZERO )
   {
      result *= base;
   }

   return( result );

   /************************************************************/
   /************************************************************/
   /*                                                          */
   /* end long_power                                           */
   /*                                                          */
   /************************************************************/
}
