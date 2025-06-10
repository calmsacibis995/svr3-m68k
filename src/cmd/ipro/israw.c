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
/***  FUNCTION:  israw - determine whether a file is a valid raw   ***/
/***                     UNIX device.                              ***/
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
/***     begin is_raw                                              ***/
/***                                                               ***/
/***        get the status of the file and                         ***/
/***                                                               ***/
/***        if the file is not a raw device then                   ***/
/***                                                               ***/
/***           return "file is not a raw device"                   ***/
/***                                                               ***/
/***        else                                                   ***/
/***                                                               ***/
/***           return "file is a raw device"                       ***/
/***                                                               ***/
/***        endif                                                  ***/
/***                                                               ***/
/***     end is_raw                                                ***/
/***                                                               ***/
/***                                                               ***/
/***  RETURN VALUES:  true - valid raw UNIX device found.          ***/
/***                                                               ***/
/***                  false - invalid raw UNIX device found.       ***/
/***                                                               ***/
/***                                                               ***/
/***  SOURCE LOCATION:  /usr/src/cmd/ipro/israw.c                  ***/
/***                                                               ***/
/***                                                               ***/
/*********************************************************************/
/*********************************************************************/


#include "iprodefs.h"


   BOOLEAN
israw( verbose, mesgtype, cmdname, fsname, mesgbuf, logfp )

   BOOLEAN verbose;
   CHARPTR mesgtype;
   CHARPTR cmdname;
   CHARPTR fsname;
   CHARPTR mesgbuf;
   FILEPTR logfp;
{
   STAT    statbuf;
   STATPTR statptr  = &statbuf;

   /************************************************************/
   /************************************************************/
   /*                                                          */
   /* begin is_raw                                             */
   /*                                                          */
   /************************************************************/

   if ( ( fsname EQ NULL )  OR  ( statptr NE &statbuf ) )
   {
      printmesg( TRUE, mesgtype, cmdname, fsname,
                 "israw: bad parameters/variables not initialized.",
                                                               logfp );

      return( FALSE );
   }

      /*********************************************************/
      /*********************************************************/
      /*                                                       */
      /* get the status of the file and                        */
      /*                                                       */
      /*********************************************************/

   if ( stat( fsname, statptr ) NE ZERO )
   {
      printmesg( TRUE, mesgtype, cmdname, fsname,
                               "israw: status not available.", logfp );

      return( FALSE );
   }

      /*********************************************************/
      /*********************************************************/
      /*                                                       */
      /* if the file is not a raw device then                  */
      /*                                                       */
      /*    return "file is not a raw device"                  */
      /*                                                       */
      /*********************************************************/

   if ( NOT ischrspec( statptr->st_mode ) )
   {
      printmesg( TRUE, mesgtype, cmdname, fsname,
                      "israw: file is not a raw device file.", logfp );

      return( FALSE );
   }

      /*********************************************************/
      /*********************************************************/
      /*                                                       */
      /* else                                                  */
      /*                                                       */
      /*    return "file is a raw device"                      */
      /*                                                       */
      /* endif                                                 */
      /*                                                       */
      /*********************************************************/

   else
   {
      return( TRUE );
   }

   /************************************************************/
   /************************************************************/
   /*                                                          */
   /* end is_raw                                               */
   /*                                                          */
   /************************************************************/
}
