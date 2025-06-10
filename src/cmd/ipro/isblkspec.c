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
/***  FUNCTION:  isblkspec - determine whether a file is a UNIX    ***/
/***                         block special (non-raw) device file.  ***/
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
/***     begin is_block_special_file                               ***/
/***                                                               ***/
/***        return true for a block special file or                ***/
/***                                                               ***/
/***        return false otherwise                                 ***/
/***                                                               ***/
/***     end is_block_special_file                                 ***/
/***                                                               ***/
/***                                                               ***/
/***  RETURN VALUES:  true - valid UNIX block special file found.  ***/
/***                                                               ***/
/***                  false - invalid UNIX block special file      ***/
/***                                                       found.  ***/
/***                                                               ***/
/***                                                               ***/
/***  SOURCE LOCATION:  /usr/src/cmd/ipro/isblkspec.c              ***/
/***                                                               ***/
/***                                                               ***/
/*********************************************************************/
/*********************************************************************/


#include "iprodefs.h"


   BOOLEAN
isblkspec( mode )

   INT mode;
{
   INT filetype;

   /************************************************************/
   /************************************************************/
   /*                                                          */
   /* begin is_block_special_file                              */
   /*                                                          */
   /************************************************************/

      /*********************************************************/
      /*********************************************************/
      /*                                                       */
      /* return true for a block special file or               */
      /*                                                       */
      /*********************************************************/

   filetype = mode BITAND S_IFMT;

   if ( filetype EQ S_IFBLK )
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
   /* end is_block_special_file                                */
   /*                                                          */
   /************************************************************/
}
