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
/***  FUNCTION:  ischrspec - determine whether a file is a UNIX    ***/
/***                         character special (raw) device file.  ***/
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
/***     begin is_character_special_file                           ***/
/***                                                               ***/
/***        return true for a character special file or            ***/
/***                                                               ***/
/***        return false otherwise                                 ***/
/***                                                               ***/
/***     end is_character_special_file                             ***/
/***                                                               ***/
/***                                                               ***/
/***  RETURN VALUES:  true - valid UNIX character special file     ***/
/***                                                       found.  ***/
/***                                                               ***/
/***                  false - invalid UNIX character special file  ***/
/***                                                       found.  ***/
/***                                                               ***/
/***                                                               ***/
/***  SOURCE LOCATION:  /usr/src/cmd/ipro/ischrspec.c              ***/
/***                                                               ***/
/***                                                               ***/
/*********************************************************************/
/*********************************************************************/


#include "iprodefs.h"


   BOOLEAN
ischrspec( mode )

   INT mode;
{
   INT filetype;

   /************************************************************/
   /************************************************************/
   /*                                                          */
   /* begin is_character_special_file                          */
   /*                                                          */
   /************************************************************/

      /*********************************************************/
      /*********************************************************/
      /*                                                       */
      /* return true for a character special file or           */
      /*                                                       */
      /*********************************************************/

   filetype = mode BITAND S_IFMT;

   if ( filetype EQ S_IFCHR )
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
   /* end is_character_special_file                            */
   /*                                                          */
   /************************************************************/
}
