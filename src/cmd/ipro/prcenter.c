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
/***  SUBROUTINE:  prcenter - prints a centered text line.         ***/
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
/***     begin print_center                                        ***/
/***                                                               ***/
/***        print the text message on the screen centered          ***/
/***                                                               ***/
/***     end print_center                                          ***/
/***                                                               ***/
/***                                                               ***/
/***  SOURCE LOCATION:  /usr/src/cmd/ipro/prcenter.c               ***/
/***                                                               ***/
/***                                                               ***/
/*********************************************************************/
/*********************************************************************/


#include "iprodefs.h"


   VOID
prcenter( fp, mesg )

   FILEPTR fp;
   CHARPTR mesg;
{
   INT     i;

   /************************************************************/
   /************************************************************/
   /*                                                          */
   /* begin print_center                                       */
   /*                                                          */
   /************************************************************/

   if ( ( fp EQ NULL )  OR  ( mesg EQ NULL ) )
   {
      return;
   }

      /*********************************************************/
      /*********************************************************/
      /*                                                       */
      /* print the text message on the screen centered         */
      /*                                                       */
      /*********************************************************/

   i = ( SCRNWDTH - strlen( mesg ) ) / TWO;

   fprintf( fp, "%*.*s", i, i,
                          "                                        " );
   fprintf( fp, mesg );

   fprintf( fp, "\n" );

   return;

   /************************************************************/
   /************************************************************/
   /*                                                          */
   /* end print_center                                         */
   /*                                                          */
   /************************************************************/
}
