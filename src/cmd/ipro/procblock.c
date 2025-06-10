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
/***  FUNCTION:  procblock - recursively process data and          ***/
/***                         indirect blocks for a file block      ***/
/***                         tree.                                 ***/
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
/***     begin process_block                                       ***/
/***                                                               ***/
/***        get a working block                                    ***/
/***                                                               ***/
/***        if the current block number is legitimate then         ***/
/***                                                               ***/
/***           pick up the next block table index                  ***/
/***                                                               ***/
/***           if necessary, renumber the block in the sequence    ***/
/***           in which it was found and keep a running count of   ***/
/***           the number of out-of-place blocks for timing        ***/
/***           purposes                                            ***/
/***                                                               ***/
/***           keep destination location information of the        ***/
/***           current block for block movement purposes           ***/
/***                                                               ***/
/***           if the current block is an indirect block then      ***/
/***                                                               ***/
/***              read in the indirect block and                   ***/
/***                                                               ***/
/***              pick up the number of valid entries in the       ***/
/***              indirect block (this is a bit tricky.  if this   ***/
/***              indirect block is the very last indirect block   ***/
/***              at this level in this file block tree, then      ***/
/***              process only the valid entries in it)            ***/
/***              (key algorithm)                                  ***/
/***                                                               ***/
/***              for each valid entry this indirect block do      ***/
/***                                                               ***/
/***                 recursively process the next lower level of   ***/
/***                 this part of the current file block tree      ***/
/***                                                               ***/
/***              end for each valid entry                         ***/
/***                                                               ***/
/***              zero out the remainder of this indirect block    ***/
/***                                                               ***/
/***              write out the modified indirect block            ***/
/***                                                               ***/
/***           endif                                               ***/
/***                                                               ***/
/***        endif                                                  ***/
/***                                                               ***/
/***        return success                                         ***/
/***                                                               ***/
/***     end process_block                                         ***/
/***                                                               ***/
/***                                                               ***/
/***  RETURN VALUES:  true - success, no errors detected.          ***/
/***                                                               ***/
/***                  false - failure, error detected in this      ***/
/***                          routine or erroneous value returned  ***/
/***                          from an invoked routine.             ***/
/***                                                               ***/
/***                                                               ***/
/***  NOTES:  This is a recursive routine.                         ***/
/***                                                               ***/
/***                                                               ***/
/***  SOURCE LOCATION:  /usr/src/cmd/ipro/procblock.c              ***/
/***                                                               ***/
/***                                                               ***/
/*********************************************************************/
/*********************************************************************/


#include "iprodefs.h"


   BOOLEAN
procblock( verbose, mesgtype, cmdname, fsname, mesgbuf,
                  fsfd, logfp, blocktable, bsize, nindir,
                         inodesect, ninodesect, level, longptr,
                                inuseptr, outplcptr, blksbelow,
                                        lastindex, wrkblks, wrkblkidx )

   BOOLEAN    verbose;
   CHARPTR    mesgtype;
   CHARPTR    cmdname;
   CHARPTR    fsname;
   CHARPTR    mesgbuf;
   INT        fsfd;
   FILEPTR    logfp;
   BLOCKINFO *blocktable;
   LONG       bsize;
   LONG       nindir;
   LONG       inodesect;
   LONG       ninodesect;
   LONG       level;
   LONGPTR    longptr;
   LONGPTR    inuseptr;
   LONGPTR    outplcptr;
   LONG       blksbelow;
   BOOLEAN    lastindex;
   CHARPTR    wrkblks;
   LONG       wrkblkidx;
{
   LONG       thisblkno;
   LONG       blktblseq;
   INDPTR     indirect;
   DOUBLE     divisor;
   LONG       longpow();
   LONG       lvlcnt;
   DOUBLE     ceil();
   LONG       indindex;
   LONG       blkno;

   /************************************************************/
   /************************************************************/
   /*                                                          */
   /* begin process_block                                      */
   /*                                                          */
   /************************************************************/

   if ( ( fsfd EQ CLOSED )      OR
           ( blocktable EQ NULL )  OR
              ( bsize LE ZERO )       OR
                 ( nindir LE ZERO )      OR
                    ( inodesect LE ZERO )   OR
                       ( ninodesect LE ZERO )  OR
                          ( level LT ZERO )       OR
                             ( longptr EQ NULL )     OR
                                ( inuseptr EQ NULL )    OR
                                   ( outplcptr EQ NULL )   OR
                                      ( blksbelow LE ZERO )   OR
                                         ( lastindex LT ZERO )   OR
                                            ( wrkblks EQ NULL )     OR
                                                ( wrkblkidx LT ZERO ) )
   {
      printmesg( TRUE, mesgtype, cmdname, fsname,
                "procblock: bad parameters/variables not initialized.",
                                                               logfp );

      return( FALSE );
   }

      /*********************************************************/
      /*********************************************************/
      /*                                                       */
      /* get a working block                                   */
      /*                                                       */
      /*********************************************************/

   if ( (indirect = (INDPTR)( GETWRKBLK( wrkblks, WRKBLKCNT,
                                        wrkblkidx, bsize ) )) EQ NULL )
   {
      sprintf( mesgbuf, 
            "procblock: can't get first working block %ld of %ld %ld.",
                                   wrkblkidx - ONE, WRKBLKCNT, bsize );

      printmesg( TRUE, mesgtype, cmdname, fsname, mesgbuf, logfp );

      return( FALSE );
   }

      /*********************************************************/
      /*********************************************************/
      /*                                                       */
      /* if the current block number is legitimate then        */
      /*                                                       */
      /*    pick up the next block table index                 */
      /*                                                       */
      /*    if necessary, renumber the block in the sequence   */
      /*    in which it was found and keep a running count of  */
      /*    the number of out-of-place blocks for timing       */
      /*    purposes                                           */
      /*                                                       */
      /*    keep destination location information of the       */
      /*    current block for block movement purposes          */
      /*                                                       */
      /*********************************************************/

   if ( (thisblkno = *longptr) GT ZERO )  /*** fsck(1) will return ***/ 
   {                                      /*** a POSSIBLE FILE     ***/
      blktblseq = ( *inuseptr )++;        /*** SIZE ERROR if zero  ***/

      blkno = ninodesect + blktblseq;

      if ( thisblkno NE blkno )
      {
         *longptr = blkno;

         ( *outplcptr )++;
      }

      blocktable[ thisblkno - inodesect ] = blkno;

         /******************************************************/
         /******************************************************/
         /*                                                    */
         /* if the current block is an indirect block then     */
         /*                                                    */
         /*    read in the indirect block and                  */
         /*                                                    */
         /*    pick up the number of valid entries in the      */
         /*    indirect block (this is a bit tricky.  if this  */
         /*    indirect block is the very last indirect block  */
         /*    at this level in this file block tree, then     */
         /*    process only the valid entries in it)           */
         /*    (key algorithm)                                 */
         /*                                                    */
         /******************************************************/

      if ( level GT ZERO )
      {
         if ( NOT fsblockio( verbose, mesgtype, cmdname, fsname,
                                  mesgbuf, fsfd, logfp, bsize, READ, 
                                       (CHARPTR)indirect, thisblkno ) )
         {
            printmesg( TRUE, mesgtype, cmdname, fsname,
                                   "procblock: read indirect block.",
                                                               logfp );

            return( FALSE );
         }

         if ( lastindex )
         {
            divisor = (DOUBLE)longpow( nindir, level - ONE );

            lvlcnt = ( ( (LONG)ceil( blksbelow / divisor ) - ONE ) %
                                                        nindir ) + ONE;
         }
         else
         {
            lvlcnt = nindir;
         }

            /***************************************************/
            /***************************************************/
            /*                                                 */
            /* for each valid entry this indirect block do     */
            /*                                                 */
            /*    recursively process the next lower level of  */
            /*    this part of the current file block tree     */
            /*                                                 */
            /* end for each valid entry                        */
            /*                                                 */
            /* zero out the remainder of this indirect block   */
            /*                                                 */
            /* write out the modified indirect block           */
            /*                                                 */
            /***************************************************/

         for ( indindex = ZERO; indindex LT lvlcnt; indindex++ )
         {
            if ( NOT procblock( verbose, mesgtype, cmdname, fsname,
                                 mesgbuf, fsfd, logfp, blocktable,
                                  bsize, nindir, inodesect, ninodesect,
                                    level - ONE, indirect + indindex,
                                       inuseptr, outplcptr, blksbelow,
                                           indindex EQ lvlcnt - ONE,
                                                 wrkblks, wrkblkidx ) )
            {
               sprintf( mesgbuf,
               "procblock: process block %ld %ld %ld %ld %ld %ld %ld.",
                              level - ONE, indindex, lvlcnt,
                                     *inuseptr, *outplcptr,
                                              indindex EQ lvlcnt - ONE,
                                                           blksbelow );

               printmesg( TRUE, mesgtype, cmdname, fsname, mesgbuf,
                                                               logfp );

               return( FALSE );
            }
         }

         for ( indindex = lvlcnt; indindex LT nindir; indindex++ )
         {
            indirect[ indindex ] = ZERO;
         }

         if ( NOT fsblockio( verbose, mesgtype, cmdname, fsname,
                                  mesgbuf, fsfd, logfp, bsize, WRITE, 
                                       (CHARPTR)indirect, thisblkno ) )
         {
            printmesg( TRUE, mesgtype, cmdname, fsname,
                                  "procblock: write indirect block.",
                                                               logfp );

            return( FALSE );
         }
      }
   }

      /*********************************************************/
      /*********************************************************/
      /*                                                       */
      /*    endif                                              */
      /*                                                       */
      /* endif                                                 */
      /*                                                       */
      /* return success                                        */
      /*                                                       */
      /*********************************************************/

   return( TRUE );

   /************************************************************/
   /************************************************************/
   /*                                                          */
   /* end process_block                                        */
   /*                                                          */
   /************************************************************/
}
