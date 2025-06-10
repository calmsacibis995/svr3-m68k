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
/***  FUNCTION:  loadfrelst - load the free list block numbers     ***/
/***                          into the free list section header    ***/
/***                          blocks.                              ***/
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
/***     begin load_free_list                                      ***/
/***                                                               ***/
/***        get two working blocks                                 ***/
/***                                                               ***/
/***        write out null blocks for each valid entry in the      ***/
/***        super block free list section                          ***/
/***                                                               ***/
/***        load the free list section header block with the       ***/
/***        section block count                                    ***/
/***                                                               ***/
/***        for each free list section do                          ***/
/***                                                               ***/
/***           load the section header block with contiguous free  ***/
/***           list block numbers                                  ***/
/***                                                               ***/
/***           terminate the last section header block             ***/
/***                                                               ***/
/***           write out the free list section header block and    ***/
/***                                                               ***/
/***           write out null blocks to the remainder of the free  ***/
/***           list section                                        ***/
/***                                                               ***/
/***        end for each free list section                         ***/
/***                                                               ***/
/***        return success                                         ***/
/***                                                               ***/
/***     end load_free_list                                        ***/
/***                                                               ***/
/***                                                               ***/
/***  RETURN VALUES:  true - success, no errors detected.          ***/
/***                                                               ***/
/***                  false - failure, error detected in this      ***/
/***                          routine or erroneous value returned  ***/
/***                          from an invoked routine.             ***/
/***                                                               ***/
/***                                                               ***/
/***  NOTES:  All section headers are filled complete, upwards     ***/
/***          from the bottom.  The excess at the top is taken     ***/
/***          care of in the super block.                          ***/
/***                                                               ***/
/***                                                               ***/
/***  ASSUMPTIONS:  the blocks have been reordered.                ***/
/***                                                               ***/
/***                                                               ***/
/***  SOURCE LOCATION:  /usr/src/cmd/ipro/loadfrelst.c             ***/
/***                                                               ***/
/***                                                               ***/
/*********************************************************************/
/*********************************************************************/


#include "iprodefs.h"


   BOOLEAN
loadfrelst( verbose, mesgtype, cmdname, fsname, mesgbuf,
                  fsfd, logfp, bsize, nfsblkcnt, ntotfreblks,
                         ntotfresect, npresectcnt, wrkblks, wrkblkidx )

   BOOLEAN    verbose;
   CHARPTR    mesgtype;
   CHARPTR    cmdname;
   CHARPTR    fsname;
   CHARPTR    mesgbuf;
   INT        fsfd;
   FILEPTR    logfp;
   LONG       bsize;
   LONG       nfsblkcnt;
   LONG       ntotfreblks;
   LONG       ntotfresect;
   LONG       npresectcnt;
   CHARPTR    wrkblks;
   LONG       wrkblkidx;
{
   LONG       fresectno;
   LONG       secthdridx;
   CHARPTR    nullsecthdr;
   FREEBLOCK *secthdrptr;
   LONG       sectoffset;

   /************************************************************/
   /************************************************************/
   /*                                                          */
   /* begin load_free_list                                     */
   /*                                                          */
   /************************************************************/

   printmesg( verbose, INFORMATIVE, cmdname, fsname,
                                     "Loading The Free List.", logfp );

   if ( ( fsfd EQ CLOSED )       OR
             ( bsize LE ZERO )        OR
                  ( nfsblkcnt LE ZERO )    OR
                       ( ntotfreblks LT ZERO )  OR
                            ( ntotfresect LT ZERO )  OR
                                 ( npresectcnt LT ZERO )  OR
                                      ( wrkblks EQ NULL )      OR
                                                ( wrkblkidx LT ZERO ) )
   {
      printmesg( TRUE, mesgtype, cmdname, fsname,
               "loadfrelst: bad parameters/variables not initialized.",
                                                               logfp );

      return( FALSE );
   }

      /*********************************************************/
      /*********************************************************/
      /*                                                       */
      /* get two working blocks                                */
      /*                                                       */
      /*********************************************************/

   if ( (nullsecthdr = GETWRKBLK( wrkblks, WRKBLKCNT,
                                        wrkblkidx, bsize )) EQ NULL )
   {
      sprintf( mesgbuf, 
          "loadfrelst: can't get first working block %ld of %ld %ld.",
             /*********************************************************/

   if ( NOT initblocks( verbose, mesgtype, cmdname, fsname, mesgbuf,
                         fsfd, logfp, bsize, nfsblkcnt - ntotfreblks,
                           nfsblkcnt - ntotfreblks + npresectcnt - ONE, 
                                           NULL, wrkblks, wrkblkidx ) )
   {
      printmesg( TRUE, mesgtype, cmdname, fsname,
                        "loadfrelst: write initial null blocks.",
                                                                          logfp );

      return( FALSE );
   }

   secthdrptr = (FREEBLOCK *)nullsecthdr;

      /*********************************************************/
      /*********************************************************/
      /*                                                       */
      /* write out null blocks for each valid entry in the     */
      /* super block free list section                         */
      /*                                                       */
      /*********************************************************/

   if ( NOT initblocks( verbose, mesgtype, cmdname, fsname, mesgbuf,
                         fsfd, logfp, bsize, nfsblkcnt - ntotfreblks,
                           nfsblkcnt - ntotfreblks + npresectcnt - ONE, 
                                           NULL, wrkblks, wrkblkidx ) )
   {
      printmesg( TRUE, mesgtype, cmdname, fsname,
                        "loadfrelst: write initial null blocks.",
                                                               logfp );

      return( FALSE );
   }

      /*********************************************************/
      /*********************************************************/
      /*                                                       */
      /* load the free list section header block with the      */
      /* section block count                                   */
      /*                                                       */
      /* for each free list section do                         */
      /*                                                       */
      /*    load the section header block with contiguous free */
      /*    list block numbers                                 */
      /*                                                       */
      /*********************************************************/

   secthdrptr->df_nfree = NICFREE;

   for ( fresectno = ZERO; fresectno LT ntotfresect; fresectno++ )
   {
      sectoffset = nfsblkcnt - ( ntotfresect - fresectno ) * NICFREE;

      for ( secthdridx = ZERO; secthdridx LT NICFREE; secthdridx++ )
      {
         secthdrptr->df_free[ secthdridx ] = 
                                     sectoffset + NICFREE - secthdridx;
      }

         /******************************************************/
         /******************************************************/
         /*                                                    */
         /* terminate the last section header block            */
         /*                                                    */
         /******************************************************/

      if ( fresectno EQ ( ntotfresect - ONE ) )
      {
         secthdrptr->df_free[ ZERO ] = ZERO;
      }

         /******************************************************/
         /******************************************************/
         /*                                                    */
         /* write out the free list section header block and   */
         /*                                                    */
         /* write out null blocks to the remainder of the free */
         /* list section                                       */
         /*                                                    */
         /******************************************************/

      if ( NOT fsblockio( verbose, mesgtype, cmdname, fsname,
                               mesgbuf, fsfd, logfp, bsize, WRITE,
                                    (CHARPTR)secthdrptr, sectoffset ) )
      {
         printmesg( TRUE, mesgtype, cmdname, fsname,
                             "loadfrelst: write section header block.",
                                                               logfp );

         return( FALSE );
      }

      if ( NOT initblocks( verbose, mesgtype, cmdname, fsname, mesgbuf,
                                fsfd, logfp, bsize, sectoffset + ONE,
                                     sectoffset + NICFREE - ONE,
                                           NULL, wrkblks, wrkblkidx ) )
      {
         printmesg( TRUE, mesgtype, cmdname, fsname,
                       "loadfrelst: write null blocks to section.",
                                                               logfp );

         return( FALSE );
      }
   }

      /*********************************************************/
      /*********************************************************/
      /*                                                       */
      /* end for each free list section                        */
      /*                                                       */
      /* return success                                        */
      /*                                                       */
      /*********************************************************/

   return( TRUE );

   /************************************************************/
   /************************************************************/
   /*                                                          */
   /* end load_free_list                                       */
   /*                                                          */
   /************************************************************/
}
