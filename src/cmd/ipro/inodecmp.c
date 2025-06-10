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
/***  FUNCTION:  inodecmp - compare the modification time fields   ***/
/***                        of two inodes for sorting purposes.    ***/
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
/***     begin inode_comparison                                    ***/
/***                                                               ***/
/***        if the first time is less than the second time then    ***/
/***                                                               ***/
/***           return "do not exchange the inodes"                 ***/
/***                                                               ***/
/***        else if the first time is greater than the second      ***/
/***                                                    time then  ***/
/***           return "exchange the inodes"                        ***/
/***                                                               ***/
/***        else                                                   ***/
/***                                                               ***/
/***           the times are identical and                         ***/
/***                                                               ***/
/***           return "do not exchange the inodes"                 ***/
/***                                                               ***/
/***        endif                                                  ***/
/***                                                               ***/
/***     end inode_comparison                                      ***/
/***                                                               ***/
/***                                                               ***/
/***  RETURN VALUES:  less than 0 - do not exchange inodes.        ***/
/***                                                               ***/
/***                  greater than 0 - exchange inodes.            ***/
/***                                                               ***/
/***                                                               ***/
/***  NOTES:  No exchanges should take place if the filesystem     ***/
/***          has just been reorganized.                           ***/
/***                                                               ***/
/***                                                               ***/
/***  ASSUMPTIONS:  both parameters point to valid inode           ***/
/***                information structures.                        ***/
/***                                                               ***/
/***                                                               ***/
/***  SOURCE LOCATION:  /usr/src/cmd/ipro/inodecmp.c               ***/
/***                                                               ***/
/***                                                               ***/
/*********************************************************************/
/*********************************************************************/


#include "iprodefs.h"


   INT
inodecmp( info1, info2 )

   INODEINFO *info1;
   INODEINFO *info2;
{
   LONG       timediff;

   /************************************************************/
   /************************************************************/
   /*                                                          */
   /* begin inode_comparison                                   */
   /*                                                          */
   /************************************************************/

   if ( ( info1 EQ NULL )  OR  ( info2 EQ NULL ) )
   {
      printmesg( TRUE, FATAL, DEFCMDNAME, DEFFSNAME,
                 "inodecmp: bad parameters/variables not initialized.",
                                                                NULL );

      exit( M_ONE );
   }

      /*********************************************************/
      /*********************************************************/
      /*                                                       */
      /* if the first time is less than the second time then   */
      /*                                                       */
      /*    return "do not exchange the inodes"                */
      /*                                                       */
      /*********************************************************/

   if ( (timediff = info1->modtime - info2->modtime) LT ZERO )
   {
      return( M_ONE );
   }

      /*********************************************************/
      /*********************************************************/
      /*                                                       */
      /* else if the first time is greater than the second     */
      /*                                             time then */
      /*    return "exchange the inodes"                       */
      /*                                                       */
      /*********************************************************/

   else if ( timediff GT ZERO )
   {
      return( ONE );
   }

      /*********************************************************/
      /*********************************************************/
      /*                                                       */
      /* else                                                  */
      /*                                                       */
      /*    the times are identical and                        */
      /*                                                       */
      /*    return "do not exchange the inodes"                */
      /*                                                       */
      /* endif                                                 */
      /*                                                       */
      /*********************************************************/

   else
   {
      return( M_TWO );
   }

   /************************************************************/
   /************************************************************/
   /*                                                          */
   /* end inode_comparison                                     */
   /*                                                          */
   /************************************************************/
}
