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
/***  FUNCTION:  initblocks - initialize a contiguous set of       ***/
/***                          filesystem blocks.                   ***/
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
/***     begin initialize_filesystem_blocks                        ***/
/***                                                               ***/
/***        get a working block                                    ***/
/***                                                               ***/
/***        write out initialized blocks to the filesystem within  ***/
/***        a specified range                                      ***/
/***                                                               ***/
/***     end initialize_filesystem_blocks                          ***/
/***                                                               ***/
/***                                                               ***/
/***  RETURN VALUES:  true - success, no errors detected.          ***/
/***                                                               ***/
/***                  false - failure, error detected in this      ***/
/***                          routine or erroneous value returned  ***/
/***                          from an invoked routine.             ***/
/***                                                               ***/
/***                                                               ***/
/***  SOURCE LOCATION:  /usr/src/cmd/ipro/initblocks.c             ***/
/***                                                               ***/
/***                                                               ***/
/*********************************************************************/
/*********************************************************************/


#include "iprodefs.h"


   BOOLEAN
initblocks( verbose, mesgtype, cmdname, fsname,
                   mesgbuf, fsfd, logfp, bsize, lofsblkno,
                            hifsblkno, blkinitchr, wrkblks, wrkblkidx )

   BOOLEAN verbose;
   CHARPTR mesgtype;
   CHARPTR cmdname;
   CHARPTR fsname;
   CHARPTR mesgbuf;
   INT     fsfd;
   FILEPTR logfp;
   LONG    bsize;
   LONG    lofsblkno;
   LONG    hifsblkno;
   LONG    blkinitchr;
   CHARPTR wrkblks;
   LONG    wrkblkidx;
{
   CHARPTR nullblock;
   LONG    fsblkno;

   /************************************************************/
   /************************************************************/
   /*                                                          */
   /* begin initialize_filesystem_blocks                       */
   /*                                                          */
   /************************************************************/

   if ( ( fsfd EQ CLOSED )      OR
                ( bsize LE ZERO )       OR
                        ( lofsblkno LT ZERO )   OR
                                ( hifsblkno LT ZERO )  OR
                                        ( wrkblks EQ NULL )     OR
                                                ( wrkblkidx LT ZERO ) )
   {
      printmesg( TRUE, mesgtype, cmdname, fsname,
               "initblocks: bad parameters/variables not initialized.",
                                                               logfp );

      return( FALSE );
   }

      /*********************************************************/
      /*********************************************************/
      /*                                                       */
      /* get a working block                                   */
      /*                                                       */
      /*********************************************************/

   if ( (nullblock = GETWRKBLK( wrkblks, WRKBLKCNT,
                                          wrkblkidx, bsize )) EQ NULL )
   {
      sprintf( mesgbuf, 
           "initblocks: can't get first working block %ld of %ld %ld.",
                                   wrkblkidx - ONE, WRKBLKCNT, bsize );

      printmesg( TRUE, mesgtype, cmdname, fsname, mesgbuf, logfp );

      return( FALSE );
   }

   if ( NOT memops( verbose, mesgtype, cmdname, fsname,
                          mesgbuf, logfp, INIT_MEM, NULL, NULL,
                                 nullblock, NULL, bsize, blkinitchr ) )
   {
      printmesg( TRUE, mesgtype, cmdname, fsname,
                  "initblocks: can't initialize first working block.",
                                                               logfp );

      return( FALSE );
   }

      /*********************************************************/
      /*********************************************************/
      /*                                                       */
      /* write out initialized blocks to the filesystem within */
      /* a specified range                                     */
      /*                                                       */
      /*********************************************************/

   for ( fsblkno = lofsblkno; fsblkno LE hifsblkno; fsblkno++ )
   {
      if ( NOT fsblockio( verbose, mesgtype, cmdname, fsname,
                                 mesgbuf, fsfd, logfp, bsize,
                                        WRITE, nullblock, fsblkno ) )
      {
         printmesg( TRUE, mesgtype, cmdname, fsname,
                     "initblocks: write initialized filesystem block.",
                                                               logfp );

         return( FALSE );
      }
   }

   return( TRUE );

   /************************************************************/
   /************************************************************/
   /*                                                          */
   /* end initialize_filesystem_blocks                         */
   /*                                                          */
   /************************************************************/
}
