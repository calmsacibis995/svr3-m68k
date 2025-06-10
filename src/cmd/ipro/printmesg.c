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
/***  SUBROUTINE:  printmesg - print a message to the standard     ***/
/***                           error file.                         ***/
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
/***     begin print_message                                       ***/
/***                                                               ***/
/***        if messages are enabled then                           ***/
/***                                                               ***/
/***           pick up default string values for any parameters    ***/
/***           that are null and                                   ***/
/***                                                               ***/
/***           print out the message to the standard error file    ***/
/***                                                               ***/
/***        endif                                                  ***/
/***                                                               ***/
/***        print out the message to the log file, if selected     ***/
/***                                                               ***/
/***        return                                                 ***/
/***                                                               ***/
/***     end print_message                                         ***/
/***                                                               ***/
/***                                                               ***/
/***  SOURCE LOCATION:  /usr/src/cmd/ipro/printmesg.c              ***/
/***                                                               ***/
/***                                                               ***/
/*********************************************************************/
/*********************************************************************/


#include "iprodefs.h"


   VOID
printmesg( verbose, mesgtype, cmdname, fsname, message, logfp )

   BOOLEAN verbose;
   CHARPTR mesgtype;
   CHARPTR cmdname;
   CHARPTR fsname;
   CHARPTR message;
   FILEPTR logfp;
{

   /************************************************************/
   /************************************************************/
   /*                                                          */
   /* begin print_message                                      */
   /*                                                          */
   /************************************************************/

      /*********************************************************/
      /*********************************************************/
      /*                                                       */
      /* if messages are enabled then                          */
      /*                                                       */
      /*********************************************************/

   if ( verbose )
   {
         /******************************************************/
         /******************************************************/
         /*                                                    */
         /* pick up default string values for any parameters   */
         /* that are null and                                  */
         /*                                                    */
         /******************************************************/

      if ( mesgtype EQ NULL )
      {
         mesgtype = DEFMSGTYPE;
      }

      if ( cmdname EQ NULL )
      {
         cmdname = DEFCMDNAME;
      }

      if ( fsname EQ NULL )
      {
         fsname = DEFFSNAME;
      }

      if ( message EQ NULL )
      {
         message = DEFMESSAGE;
      }

         /******************************************************/
         /******************************************************/
         /*                                                    */
         /* print out the message to the standard error file   */
         /*                                                    */
         /******************************************************/

      fprintf( stderr, "%s: %s: %s: %s\n", mesgtype, cmdname, fsname,
                                                             message );
   }

   if ( logfp NE NULL )
   {
      fprintf( logfp, "%s: %s: %s: %s\n", mesgtype, cmdname, fsname,
                                                             message );
   }

      /*********************************************************/
      /*********************************************************/
      /*                                                       */
      /* endif                                                 */
      /*                                                       */
      /* print out the message to the log file, if selected    */
      /*                                                       */
      /* return                                                */
      /*                                                       */
      /*********************************************************/

   return;

   /************************************************************/
   /************************************************************/
   /*                                                          */
   /* end print_message                                        */
   /*                                                          */
   /************************************************************/
}
