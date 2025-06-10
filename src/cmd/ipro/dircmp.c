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
/***  FUNCTION:  dircmp - compare two directory entries for        ***/
/***                      sorting purposes.                        ***/
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
/***     begin directory_entry_comparison                          ***/
/***                                                               ***/
/***        pick up the directory entrys' inode numbers and        ***/
/***                                                               ***/
/***        if the second entry's inode number is zero then        ***/
/***                                                               ***/
/***           return "do not exchange the entries"                ***/
/***                                                               ***/
/***        else if the first entry's inode number is zero then    ***/
/***                                                               ***/
/***           return "exchange the entries"                       ***/
/***                                                               ***/
/***        else if the first entry's modification time is         ***/
/***                              greater than the second entry's  ***/
/***                                       modification time then  ***/
/***                                                               ***/
/***           return "do not exchange the entries"                ***/
/***                                                               ***/
/***        else if the first entry's modification time is less    ***/
/***                                      than the second entry's  ***/
/***                                       modification time then  ***/
/***                                                               ***/
/***           return "exchange the entries"                       ***/
/***                                                               ***/
/***        else if the first name is less than the second name    ***/
/***                                                         then  ***/
/***           return "do not exchange the entries"                ***/
/***                                                               ***/
/***        else if the first name is greater than the second      ***/
/***                                                    name then  ***/
/***           return "exchange the entries"                       ***/
/***                                                               ***/
/***        else                                                   ***/
/***                                                               ***/
/***           all fields are identical and                        ***/
/***                                                               ***/
/***           return "do not exchange the entries"                ***/
/***                                                               ***/
/***        endif                                                  ***/
/***                                                               ***/
/***     end directory_entry_comparison                            ***/
/***                                                               ***/
/***                                                               ***/
/***  RETURN VALUES:  less than 0 - do not exchange directory      ***/
/***                                entries.                       ***/
/***                                                               ***/
/***                  greater than 0 - exchange directory          ***/
/***                                   entries.                    ***/
/***                                                               ***/
/***                                                               ***/
/***  NOTES:  No exchanges should take place if the filesystem     ***/
/***          has just been reorganized.                           ***/
/***                                                               ***/
/***                                                               ***/
/***  ASSUMPTIONS:  both parameters point to valid directory       ***/
/***                entries.                                       ***/
/***                                                               ***/
/***                                                               ***/
/***  SOURCE LOCATION:  /usr/src/cmd/ipro/dircmp.c                 ***/
/***                                                               ***/
/***                                                               ***/
/*********************************************************************/
/*********************************************************************/


#include "iprodefs.h"


   INT
dircmp( dirlnk1, dirlnk2 )

   DIRPTR dirlnk1;
   DIRPTR dirlnk2;
{
   INT    inum1;
   INT    inum2;
   INT    inumdiff;
   INT    namediff;

   /************************************************************/
   /************************************************************/
   /*                                                          */
   /* begin directory_entry_comparison                         */
   /*                                                          */
   /************************************************************/

   if ( ( dirlnk1 EQ NULL )  OR  ( dirlnk2 EQ NULL ) )
   {
      printmesg( TRUE, FATAL, DEFCMDNAME, DEFFSNAME,
                  "dircmp: bad parameters/variables not initialized.",
                                                                NULL );

      exit( M_ONE );
   }

      /*********************************************************/
      /*********************************************************/
      /*                                                       */
      /* pick up the directory entrys' inode numbers and       */
      /*                                                       */
      /* if the second entry's inode number is zero then       */
      /*                                                       */
      /*    return "do not exchange the entries"               */
      /*                                                       */
      /*********************************************************/

   if ( (inum2 = dirlnk2->d_ino) EQ ZERO )
   {
      return( M_ONE );
   }

      /*********************************************************/
      /*********************************************************/
      /*                                                       */
      /* else if the first entry's inode number is zero then   */
      /*                                                       */
      /*    return "exchange the entries"                      */
      /*                                                       */
      /*********************************************************/

   if ( (inum1 = dirlnk1->d_ino) EQ ZERO )
   {
      return( ONE );
   }

      /*********************************************************/
      /*********************************************************/
      /*                                                       */
      /* else if the first entry's modification time is        */
      /*                       greater than the second entry's */
      /*                                modification time then */
      /*                                                       */
      /*    return "do not exchange the entries"               */
      /*                                                       */
      /*********************************************************/

   if ( (inumdiff = dirlnk1->d_ino - dirlnk2->d_ino) GT ZERO )
   {
      return( M_TWO );
   }

      /*********************************************************/
      /*********************************************************/
      /*                                                       */
      /* else if the first entry's modification time is less   */
      /*                               than the second entry's */
      /*                                modification time then */
      /*                                                       */
      /*    return "exchange the entries"                      */
      /*                                                       */
      /*********************************************************/

   else if ( inumdiff LT ZERO )
   {
      return( TWO );
   }

      /*********************************************************/
      /*********************************************************/
      /*                                                       */
      /* else if the first name is less than the second name   */
      /*                                                  then */
      /*    return "do not exchange the entries"               */
      /*                                                       */
      /*********************************************************/

   if ( (namediff = strncmp( dirlnk1->d_name, dirlnk2->d_name,
                                                    DIRSIZ )) LT ZERO )
   {
      return( M_THREE );
   }

      /*********************************************************/
      /*********************************************************/
      /*                                                       */
      /* else if the first name is greater than the second     */
      /*                                             name then */
      /*    return "exchange the entries"                      */
      /*                                                       */
      /*********************************************************/

   else if ( namediff GT ZERO )
   {
      return( THREE );
   }

      /*********************************************************/
      /*********************************************************/
      /*                                                       */
      /* else                                                  */
      /*                                                       */
      /*    all fields are identical and                       */
      /*                                                       */
      /*    return "do not exchange the entries"               */
      /*                                                       */
      /* endif                                                 */
      /*                                                       */
      /*********************************************************/

   else
   {
      return( M_FOUR);
   }

   /************************************************************/
   /************************************************************/
   /*                                                          */
   /* end directory_entry_comparison                           */
   /*                                                          */
   /************************************************************/
}
