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
/***  FUNCTION:  isdir - determine whether a file is a UNIX        ***/
/***                     directory file.                           ***/
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
/***     begin is_directory_file                                   ***/
/***                                                               ***/
/***        return true for a directory file or                    ***/
/***                                                               ***/
/***        return false otherwise                                 ***/
/***                                                               ***/
/***     end is_directory_file                                     ***/
/***                                                               ***/
/***                                                               ***/
/***  RETURN VALUES:  true - valid UNIX directory file found.      ***/
/***                                                               ***/
/***                  false - invalid UNIX directory file found.   ***/
/***                                                               ***/
/***                                                               ***/
/***  SOURCE LOCATION:  /usr/src/cmd/ipro/isdir.c                  ***/
/***                                                               ***/
/***                                                               ***/
/*********************************************************************/
/*********************************************************************/


#include "iprodefs.h"


   BOOLEAN
isdir( mode )

   INT mode;
{
   INT filetype;

   /************************************************************/
   /************************************************************/
   /*                                                          */
   /* begin is_directory_file                                  */
   /*                                                          */
   /************************************************************/

      /*********************************************************/
      /*********************************************************/
      /*                                                       */
      /* return true for a directory file or                   */
      /*                                                       */
      /*********************************************************/

   filetype = mode BITAND S_IFMT;

   if ( filetype EQ S_IFDIR )
   {
      return( TRUE );
   }

      /*********************************************************/
      /*********************************************************/
      /*                                                       */
      /* return false otherwise                                */
      /*                                                       */
      /*********************************************************/

   else
   {
      return( FALSE );
   }

   /************************************************************/
   /************************************************************/
   /*                                                          */
   /* end is_directory_file                                    */
   /*                                                          */
   /************************************************************/
}
