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
/***  FUNCTION:  isdevice - determine whether a file is a valid    ***/
/***                        UNIX device.                           ***/
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
/***     begin is_device                                           ***/
/***                                                               ***/
/***        get the status of the file and                         ***/
/***                                                               ***/
/***        if the file is not a device then                       ***/
/***                                                               ***/
/***           return "file is not a device"                       ***/
/***                                                               ***/
/***        else                                                   ***/
/***                                                               ***/
/***           return "file is a device"                           ***/
/***                                                               ***/
/***        endif                                                  ***/
/***                                                               ***/
/***     end is_device                                             ***/
/***                                                               ***/
/***                                                               ***/
/***  RETURN VALUES:  true - valid UNIX device found.              ***/
/***                                                               ***/
/***                  false - invalid UNIX device found.           ***/
/***                                                               ***/
/***                                                               ***/
/***  SOURCE LOCATION:  /usr/src/cmd/ipro/isdevice.c               ***/
/***                                                               ***/
/***                                                               ***/
/*********************************************************************/
/*********************************************************************/


#include "iprodefs.h"


   BOOLEAN
isdevice( verbose, mesgtype, cmdname, fsname, mesgbuf, logfp )

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
   /* begin is_device                                          */
   /*                                                          */
   /************************************************************/

   printmesg( verbose, INFORMATIVE, cmdname, fsname,
                                         "Device File Check.", logfp );

   if ( ( fsname EQ NULL )  OR  ( statptr NE &statbuf ) )
   {
      printmesg( TRUE, mesgtype, cmdname, fsname,
                 "isdevice: bad parameters/variables not initialized.",
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
                            "isdevice: status not available.", logfp );

      return( FALSE );
   }

      /*********************************************************/
      /*********************************************************/
      /*                                                       */
      /* if the file is not a device then                      */
      /*                                                       */
      /*    return "file is not a device"                      */
      /*                                                       */
      /*********************************************************/

   if ( NOT isdev( statptr->st_mode ) )
   {
      printmesg( TRUE, mesgtype, cmdname, fsname,
                       "isdevice: file is not a device file.", logfp );

      return( FALSE );
   }

      /*********************************************************/
      /*********************************************************/
      /*                                                       */
      /* else                                                  */
      /*                                                       */
      /*    return "file is a device"                          */
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
   /* end is_device                                            */
   /*                                                          */
   /************************************************************/
}
