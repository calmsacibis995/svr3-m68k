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
/***  FUNCTION:  printtime - print the local time to the screen.   ***/
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
/***     begin print_local_time                                    ***/
/***                                                               ***/
/***        print the local time to the screen                     ***/
/***                                                               ***/
/***     end print_local_time                                      ***/
/***                                                               ***/
/***                                                               ***/
/***  RETURN VALUES:  positive - local time in seconds.            ***/
/***                                                               ***/
/***                  non-positive - error.                        ***/
/***                                                               ***/
/***                                                               ***/
/***  SOURCE LOCATION:  /usr/src/cmd/ipro/prtime.c                 ***/
/***                                                               ***/
/***                                                               ***/
/*********************************************************************/
/*********************************************************************/


#include "iprodefs.h"


   LONG
printtime( verbose, mesgtype, cmdname, fsname, mesgbuf, logfp )

   BOOLEAN       verbose;
   CHARPTR       mesgtype;
   CHARPTR       cmdname;
   CHARPTR       fsname;
   CHARPTR       mesgbuf;
   FILEPTR       logfp;
{
   LONG          gmtime;
   LONG          lcltime;
   LONG          time();
   TIMEPTR       localtime();
   TIME          timercd;
   TIMEPTR       timeptr     = &timercd;
   EXTERNAL LONG timezone;

   /************************************************************/
   /************************************************************/
   /*                                                          */
   /* begin print_local_time                                   */
   /*                                                          */
   /************************************************************/

      /*********************************************************/
      /*********************************************************/
      /*                                                       */
      /* print the local time to the screen                    */
      /*                                                       */
      /*********************************************************/

   time( &gmtime );

   lcltime = gmtime;

   timeptr = localtime( &lcltime );

   sprintf( mesgbuf,
             "CURRENT TIME:                           %02d:%02d:%02d",
                 timeptr->tm_hour, timeptr->tm_min, timeptr->tm_sec );

   printmesg( verbose, INFORMATIVE, cmdname, fsname, mesgbuf, logfp );

   return( gmtime - timezone );

   /************************************************************/
   /************************************************************/
   /*                                                          */
   /* end print_local_time                                     */
   /*                                                          */
   /************************************************************/
}
