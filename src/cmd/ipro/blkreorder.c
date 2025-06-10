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
/***  FUNCTION:  blkreorder - reorder all of the data and          ***/
/***                          indirect blocks of all files in the  ***/
/***                          filesystem.                          ***/
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
/***     begin reorder_data_and_indirect_blocks                    ***/
/***                                                               ***/
/***        get two working blocks                                 ***/
/***                                                               ***/
/***        for each entry (sequential) in the block table do      ***/
/***                                                               ***/
/***           pick up the destination of the current block        ***/
/***                                                               ***/
/***           if the block is free or already processed then      ***/
/***                                                               ***/
/***              skip the block                                   ***/
/***                                                               ***/
/***           endif                                               ***/
/***                                                               ***/
/***           mark the block as already processed                 ***/
/***                                                               ***/
/***           if the block is in-place then                       ***/
/***                                                               ***/
/***              skip the block                                   ***/
/***                                                               ***/
/***           endif                                               ***/
/***                                                               ***/
/***           the head of a ripple insertion chain has been       ***/
/***           found                                               ***/
/***                                                               ***/
/***           read in (ripple) the chain head block               ***/
/***                                                               ***/
/***           while the block's destination is within the         ***/
/***                  original block section of the filesystem do  ***/
/***                                                               ***/
/***              pick up the destination of the block's           ***/
/***              destination                                      ***/
/***                                                               ***/
/***              if the block's destination is free or already    ***/
/***                                               processed then  ***/
/***                                                               ***/
/***                 the tail of a ripple insertion chain has      ***/
/***                 been found                                    ***/
/***                                                               ***/
/***              endif                                            ***/
/***                                                               ***/
/***              mark the block's destination as already          ***/
/***              processed                                        ***/
/***                                                               ***/
/***              read in (ripple) the block's destination and     ***/
/***                                                               ***/
/***              write out (insert) the block to its destination  ***/
/***                                                               ***/
/***              resync for the next chain block                  ***/
/***                                                               ***/
/***           end while the block's destination                   ***/
/***                                                               ***/
/***           write out (insert) the chain tail block to its      ***/
/***           destination                                         ***/
/***                                                               ***/
/***        end for each entry in the block table                  ***/
/***                                                               ***/
/***        return success                                         ***/
/***                                                               ***/
/***     end reorder_data_and_indirect_blocks                      ***/
/***                                                               ***/
/***                                                               ***/
/***  RETURN VALUES:  true - success, no errors detected.          ***/
/***                                                               ***/
/***                  false - failure, error detected in this      ***/
/***                          routine or erroneous value returned  ***/
/***                          from an invoked routine.             ***/
/***                                                               ***/
/***                                                               ***/
/***  ASSUMPTIONS:  the block table has been loaded at this time.  ***/
/***                                                               ***/
/***                the free block list has not been loaded.       ***/
/***                                                               ***/
/***                                                               ***/
/***  SOURCE LOCATION:  /usr/src/cmd/ipro/blkreorder.c             ***/
/***                                                               ***/
/***                                                               ***/
/*********************************************************************/
/*********************************************************************/


#include "iprodefs.h"


   BOOLEAN
blkreorder( verbose, mesgtype, cmdname, fsname, mesgbuf,
                    fsfd, logfp, blocktable, bsize, inodesect,
                              fsblkcnt, blktblcnt, wrkblks, wrkblkidx )

   BOOLEAN    verbose;
   CHARPTR    mesgtype;
   CHARPTR    cmdname;
   CHARPTR    fsname;
   CHARPTR    mesgbuf;
   INT        fsfd;
   FILEPTR    logfp;
   BLOCKINFO *blocktable;
   LONG       bsize;
   LONG       inodesect;
   LONG       fsblkcnt;
   LONG       blktblcnt;
   CHARPTR    wrkblks;
   LONG       wrkblkidx;
{
   LONG       blktblidx;
   LONG       blkno1;
   LONG       blkno2;
   CHARPTR    block1;
   CHARPTR    block2;
   CHARPTR    tmpblock;

   /************************************************************/
   /************************************************************/
   /*                                                          */
   /* begin reorder_data_and_indirect_blocks                   */
   /*                                                          */
   /************************************************************/

   printmesg( verbose, INFORMATIVE, cmdname, fsname,
                   "Reordering The Data And Indirect Blocks.", logfp );

   if ( ( fsfd EQ CLOSED )      OR
             ( blocktable EQ NULL )  OR
                  ( bsize LE ZERO )       OR
                       ( inodesect LE ZERO )   OR
                            ( fsblkcnt LE ZERO )    OR
                                 ( blktblcnt LE ZERO )   OR
                                      ( wrkblks EQ NULL )     OR
                                                ( wrkblkidx LT ZERO ) )
   {
      printmesg( TRUE, mesgtype, cmdname, fsname,
               "blkreorder: bad parameters/variables not initialized.",
                                                               logfp );

      return( FALSE );
   }

      /*********************************************************/
      /*********************************************************/
      /*                                                       */
      /* get two working blocks                                */
      /*                                                       */
      /*********************************************************/

   if ( (block1 = GETWRKBLK( wrkblks, WRKBLKCNT, wrkblkidx, bsize )) EQ
                                                                 NULL )
   {
      sprintf( mesgbuf, 
           "blkreorder: can't get first working block %ld of %ld %ld.",
                                   wrkblkidx - ONE, WRKBLKCNT, bsize );

      printmesg( TRUE, mesgtype, cmdname, fsname, mesgbuf, logfp );

      return( FALSE );
   }

   if ( (block2 = GETWRKBLK( wrkblks, WRKBLKCNT, wrkblkidx, bsize )) EQ
                                                                 NULL )
   {
      sprintf( mesgbuf, 
          "blkreorder: can't get second working block %ld of %ld %ld.",
                                   wrkblkidx - ONE, WRKBLKCNT, bsize );

      printmesg( TRUE, mesgtype, cmdname, fsname, mesgbuf, logfp );

      return( FALSE );
   }

      /*********************************************************/
      /*********************************************************/
      /*                                                       */
      /* for each entry (sequential) in the block table do     */
      /*                                                       */
      /*********************************************************/

   for ( blktblidx = ZERO; blktblidx LT blktblcnt; blktblidx++ )
   {
         /******************************************************/
         /******************************************************/
         /*                                                    */
         /* pick up the destination of the current block       */
         /*                                                    */
         /* if the block is free or already processed then     */
         /*                                                    */
         /*    skip the block                                  */
         /*                                                    */
         /* endif                                              */
         /*                                                    */
         /* mark the block as already processed                */
         /*                                                    */
         /******************************************************/

      if ( (blkno1 = blocktable[ blktblidx ]) EQ ZERO )
      {
         continue;
      }

      blocktable[ blktblidx ] = ZERO;

         /******************************************************/
         /******************************************************/
         /*                                                    */
         /* if the block is in-place then                      */
         /*                                                    */
         /*    skip the block                                  */
         /*                                                    */
         /* endif                                              */
         /*                                                    */
         /* the head of a ripple insertion chain has been      */
         /* found                                              */
         /*                                                    */
         /* read in (ripple) the chain head block              */
         /*                                                    */
         /******************************************************/

      if ( blkno1 EQ ( blktblidx + inodesect ) )
      {
         continue;
      }

      if ( NOT fsblockio( verbose, mesgtype, cmdname, fsname,
                               mesgbuf, fsfd, logfp, bsize, READ,
                                      block1, blktblidx + inodesect ) )
      {
         printmesg( TRUE, mesgtype, cmdname, fsname,
                       "blkreorder: ripple chain head block.", logfp );

         return( FALSE );
      }

         /******************************************************/
         /******************************************************/
         /*                                                    */
         /* while the block's destination is within the        */
         /*        original block section of the filesystem do */
         /*                                                    */
         /******************************************************/

      while ( INRANGE( inodesect, blkno1, fsblkcnt - ONE ) )
      {
            /***************************************************/
            /***************************************************/
            /*                                                 */
            /* pick up the destination of the block's          */
            /* destination                                     */
            /*                                                 */
            /* if the block's destination is free or already   */
            /*                                  processed then */
            /*                                                 */
            /*    the tail of a ripple insertion chain has     */
            /*    been found                                   */
            /*                                                 */
            /* endif                                           */
            /*                                                 */
            /* mark the block's destination as already         */
            /* processed                                       */
            /*                                                 */
            /***************************************************/

         if ( (blkno2 = blocktable[ blkno1 - inodesect ]) EQ ZERO )
         {
            break;
         }

         blocktable[ blkno1 - inodesect ] = ZERO;

            /***************************************************/
            /***************************************************/
            /*                                                 */
            /* read in (ripple) the block's destination and    */
            /*                                                 */
            /* write out (insert) the block to its destination */
            /*                                                 */
            /* resync for the next chain block                 */
            /*                                                 */
            /***************************************************/

         if ( NOT fsblockio( verbose, mesgtype, cmdname, fsname,
                                      mesgbuf, fsfd, logfp, bsize,
                                               READ, block2, blkno1 ) )
         {
            printmesg( TRUE, mesgtype, cmdname, fsname,
                              "blkreorder: ripple out block.", logfp );

            return( FALSE );
         }

         if ( NOT fsblockio( verbose, mesgtype, cmdname, fsname,
                                      mesgbuf, fsfd, logfp, bsize,
                                              WRITE, block1, blkno1 ) )
         {
            printmesg( TRUE, mesgtype, cmdname, fsname,
                                  "blkreorder: insert block.", logfp );

            return( FALSE );
         }

         blkno1 = blkno2;

         XCHG( block1, block2, tmpblock );
      }

         /******************************************************/
         /******************************************************/
         /*                                                    */
         /* end while the block's destination                  */
         /*                                                    */
         /* write out (insert) the chain tail block to its     */
         /* destination                                        */
         /*                                                    */
         /******************************************************/

      if ( NOT fsblockio( verbose, mesgtype, cmdname, fsname,
                                    mesgbuf, fsfd, logfp, bsize,
                                              WRITE, block1, blkno1 ) )
      {
         printmesg( TRUE, mesgtype, cmdname, fsname,
                       "blkreorder: insert chain tail block.", logfp );

         return( FALSE );
      }
   }

      /*********************************************************/
      /*********************************************************/
      /*                                                       */
      /* end for each entry in the block table                 */
      /*                                                       */
      /* return success                                        */
      /*                                                       */
      /*********************************************************/

   return( TRUE );

   /************************************************************/
   /************************************************************/
   /*                                                          */
   /* end reorder_data_and_indirect_blocks                     */
   /*                                                          */
   /************************************************************/
}
