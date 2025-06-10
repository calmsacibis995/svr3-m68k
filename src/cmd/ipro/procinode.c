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
/***  FUNCTION:  procinode - process an inode's data and indirect  ***/
/***                         blocks.                               ***/
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
/***     begin process_inode                                       ***/
/***                                                               ***/
/***        get a working block                                    ***/
/***                                                               ***/
/***        translate the inode block addresses into full block    ***/
/***        numbers and determine the data block count of the      ***/
/***        file                                                   ***/
/***                                                               ***/
/***        determine the number of tree groups this inode, the    ***/
/***        number of inode block numbers used, the number of      ***/
/***        trees for each tree group, and the number of data      ***/
/***        blocks in each tree of each tree group                 ***/
/***                                                               ***/
/***        zero out the last block breakage of this file if the   ***/
/***        file size is not zero and the file is a regular        ***/
/***        file.  this has to be done before the block numbers    ***/
/***        in the inode and indirect blocks are renumbered.       ***/
/***                                                               ***/
/***        for each file block tree group do                      ***/
/***                                                               ***/
/***           pick up the number of trees contained in this tree  ***/
/***           group and the number of data blocks contained in    ***/
/***           each tree this tree group                           ***/
/***                                                               ***/
/***           for each file block tree this tree group do         ***/
/***                                                               ***/
/***              recursively process the data and indirect        ***/
/***              blocks in this file block tree                   ***/
/***                                                               ***/
/***           end for each file block tree                        ***/
/***                                                               ***/
/***        end for each file block tree group                     ***/
/***                                                               ***/
/***        zero out the remainder of the inode block numbers      ***/
/***        (for regular files only),                              ***/
/***                                                               ***/
/***        translate the full inode block numbers back into       ***/
/***        their shortened form, and                              ***/
/***                                                               ***/
/***        return success                                         ***/
/***                                                               ***/
/***     end process_inode                                         ***/
/***                                                               ***/
/***                                                               ***/
/***  RETURN VALUES:  true - success, no errors detected.          ***/
/***                                                               ***/
/***                  false - failure, error detected in this      ***/
/***                          routine or erroneous value returned  ***/
/***                          from an invoked routine.             ***/
/***                                                               ***/
/***                                                               ***/
/***  SOURCE LOCATION:  /usr/src/cmd/ipro/procinode.c              ***/
/***                                                               ***/
/***                                                               ***/
/*********************************************************************/
/*********************************************************************/


#include "iprodefs.h"


   BOOLEAN
procinode( verbose, mesgtype, cmdname, fsname, mesgbuf,
                fsfd, logfp, blocktable, bsize, nindir,
                      inodesect, ninodesect, inuseptr, outplcptr,
                             inodeptr, inum, mode, wrkblks, wrkblkidx )

   BOOLEAN     verbose;
   CHARPTR     mesgtype;
   CHARPTR     cmdname;
   CHARPTR     fsname;
   CHARPTR     mesgbuf;
   INT         fsfd;
   FILEPTR     logfp;
   BLOCKINFO  *blocktable;
   LONG        bsize;
   LONG        nindir;
   LONG        inodesect;
   LONG        ninodesect;
   LONGPTR     inuseptr;
   LONGPTR     outplcptr;
   INODEPTR    inodeptr;
   LONG        inum;
   INT         mode;
   CHARPTR     wrkblks;
   LONG        wrkblkidx;
{
   INOADDRBUF  longbuf;
   LONG        datblkcnt;
   DOUBLE      ceil();
   LONG        dbc;
   LONG        treegrpcnt;
   LONG        usedaddrcnt;
   LONG        treespergrp[ FOUR ];
   LONG        datblksbelow[ FOUR ];
   LONG        lastblkno;
   LONG        getblkno();
   CHARPTR     lastblock;
   LONG        legitcnt;
   LONG        nullcnt;
   LONG        treegroup;
   LONG        treecnt;
   LONG        blksbelow;
   LONG        treeno;
   LONG        inoaddridx           = ZERO;
   LONG        nullindex;

   /************************************************************/
   /************************************************************/
   /*                                                          */
   /* begin process_inode                                      */
   /*                                                          */
   /************************************************************/

   if ( ( fsfd EQ CLOSED )      OR
           ( blocktable EQ NULL )  OR
              ( bsize LE ZERO )       OR
                 ( nindir LE ZERO )      OR
                    ( inodesect LE ZERO )   OR
                       ( ninodesect LE ZERO )  OR
                          ( inuseptr EQ NULL )    OR
                             ( outplcptr EQ NULL )   OR
                                ( inodeptr EQ NULL )    OR
                                   ( inum LE ZERO )        OR
                                      ( mode LE ZERO )        OR
                                         ( wrkblks EQ NULL )     OR
                                            ( wrkblkidx LT ZERO )   OR
                                               ( inoaddridx NE ZERO ) )
   {
      printmesg( TRUE, mesgtype, cmdname, fsname,
              "procinode: bad parameters/variables not initialized.",
                                                               logfp );

      return( FALSE );
   }

      /*********************************************************/
      /*********************************************************/
      /*                                                       */
      /* get a working block                                   */
      /*                                                       */
      /*********************************************************/

   if ( (lastblock = GETWRKBLK( wrkblks, WRKBLKCNT,
                                          wrkblkidx, bsize )) EQ NULL )
   {
      sprintf( mesgbuf, 
            "procinode: can't get first working block %ld of %ld %ld.",
                                   wrkblkidx - ONE, WRKBLKCNT, bsize );

      printmesg( TRUE, mesgtype, cmdname, fsname, mesgbuf, logfp );

      return( FALSE );
   }

      /*********************************************************/
      /*********************************************************/
      /*                                                       */
      /* translate the inode block addresses into full block   */
      /* numbers and determine the data block count of the     */
      /* file                                                  */
      /*                                                       */
      /*********************************************************/

   l3tol( longbuf, inodeptr->di_addr, NADDR );

   datblkcnt = (LONG)ceil( (DOUBLE)inodeptr->di_size / bsize );

   if ( (dbc = datblkcnt) LT ZERO )
   {
      sprintf( mesgbuf,
             "procinode: data blk cnt, %ld, is < 0 for inode no, %ld.",
                                                     datblkcnt, inum );

      printmesg( TRUE, mesgtype, cmdname, fsname, mesgbuf, logfp );

      return( FALSE );
   }

      /*********************************************************/
      /*********************************************************/
      /*                                                       */
      /* determine the number of tree groups this inode, the   */
      /* number of inode block numbers used, the number of     */
      /* trees for each tree group, and the number of data     */
      /* blocks in each tree of each tree group                */
      /*                                                       */
      /*********************************************************/

   else if ( dbc EQ ZERO )
   {
      treegrpcnt = ZERO;
      usedaddrcnt = ZERO;
   }
   else if ( dbc LE DIRECT )
   {
      treegrpcnt = ONE;
      usedaddrcnt = dbc;

      treespergrp[ ZERO ] = dbc;

      datblksbelow[ ZERO ] = ONE;
   }
   else if ( (dbc -= DIRECT) LE nindir )
   {
      treegrpcnt = TWO;
      usedaddrcnt = DIRECT + ONE;

      treespergrp[ ZERO ] = DIRECT;
      treespergrp[ ONE ] = ONE;

      datblksbelow[ ZERO ] = ONE;
      datblksbelow[ ONE ] = dbc;
   }
   else if ( (dbc -= nindir) LE SQR( nindir ) )
   {
      treegrpcnt = THREE;
      usedaddrcnt = DIRECT + TWO;

      treespergrp[ ZERO ] = DIRECT;
      treespergrp[ ONE ] = ONE;
      treespergrp[ TWO ] = ONE;

      datblksbelow[ ZERO ] = ONE;
      datblksbelow[ ONE ] = nindir;
      datblksbelow[ TWO ] = dbc;
   }
   else if ( (dbc -= SQR( nindir )) LE CUBE( nindir ) )
   {
      treegrpcnt = FOUR;
      usedaddrcnt = DIRECT + THREE;

      treespergrp[ ZERO ] = DIRECT;
      treespergrp[ ONE ] = ONE;
      treespergrp[ TWO ] = ONE;
      treespergrp[ THREE ] = ONE;

      datblksbelow[ ZERO ] = ONE;
      datblksbelow[ ONE ] = nindir;
      datblksbelow[ TWO ] = SQR( nindir );
      datblksbelow[ THREE ] = dbc;
   }
   else
   {
      sprintf( mesgbuf,
       "procinode: data blk cnt, %ld, out-of-range for inode no, %ld.",
                                                     datblkcnt, inum );

      printmesg( TRUE, mesgtype, cmdname, fsname, mesgbuf, logfp );

      return( FALSE );
   }

      /*********************************************************/
      /*********************************************************/
      /*                                                       */
      /* zero out the last block breakage of this file if the  */
      /* file size is not zero and the file is a regular       */
      /* file.  this has to be done before the block numbers   */
      /* in the inode and indirect blocks are renumbered.      */
      /*                                                       */
      /*********************************************************/

   if ( datblkcnt GT ZERO )
   {
      if ( isreg( mode ) )
      {
         if ( (lastblkno = getblkno( verbose, mesgtype, cmdname,
                                       fsname, mesgbuf, fsfd,
                                         logfp, bsize, nindir,
                                           longbuf, datblkcnt,
                                              datblkcnt - ONE, wrkblks,
                                                 wrkblkidx )) GT ZERO )
         {
            legitcnt = ( inodeptr->di_size - ONE ) % bsize + ONE;

            if ( (nullcnt = bsize - legitcnt) GT ZERO )
            {
               if ( NOT fsblockio( verbose, mesgtype, cmdname, fsname,
                                      mesgbuf, fsfd, logfp, bsize,
                                         READ, lastblock, lastblkno ) )
               {
                  printmesg( TRUE, mesgtype, cmdname, fsname,
                                       "procinode: read last block.",
                                                               logfp );

                  return( FALSE );
               }

               if ( NOT memops( verbose, mesgtype, cmdname, fsname,
                                     mesgbuf, logfp, INIT_MEM, NULL,
                                          NULL, lastblock + legitcnt,
                                                NULL, nullcnt, NULL ) )
               {
                  sprintf( mesgbuf, 
         "procinode: zero %ld bytes of last block breakage, inum %ld.",
                                                       nullcnt, inum );

                  printmesg( TRUE, mesgtype, cmdname, fsname, mesgbuf,
                                                               logfp );

                  return( FALSE );
               }

               if ( NOT fsblockio( verbose, mesgtype, cmdname,
                                       fsname, mesgbuf, fsfd,
                                           logfp, bsize, WRITE,
                                               lastblock, lastblkno ) )
               {
                  printmesg( TRUE, mesgtype, cmdname, fsname,
                                      "procinode: write last block.",
                                                               logfp );

                  return( FALSE );
               }
            }
         }
         else
         {
            sprintf( mesgbuf,
     "procinode: can't get blk no %ld of %ld blocks for inode no %ld.", 
                                    datblkcnt - ONE, datblkcnt, inum );

            printmesg( TRUE, mesgtype, cmdname, fsname, mesgbuf,
                                                               logfp );

            return( FALSE );
         }
      }
   }

      /*********************************************************/
      /*********************************************************/
      /*                                                       */
      /* for each file block tree group do                     */
      /*                                                       */
      /*********************************************************/

   for ( treegroup = ZERO; treegroup LT treegrpcnt; treegroup++ )
   {
         /******************************************************/
         /******************************************************/
         /*                                                    */
         /* pick up the number of trees contained in this tree */
         /* group and the number of data blocks contained in   */
         /* each tree this tree group                          */
         /*                                                    */
         /* for each file block tree this tree group do        */
         /*                                                    */
         /******************************************************/

      treecnt = treespergrp[ treegroup ];

      blksbelow = datblksbelow[ treegroup ];

      for ( treeno = ZERO; treeno LT treecnt; treeno++ )
      {
            /***************************************************/
            /***************************************************/
            /*                                                 */
            /* recursively process the data and indirect       */
            /* blocks in this file block tree                  */
            /*                                                 */
            /***************************************************/

         if ( NOT procblock( verbose, mesgtype, cmdname, fsname,
                               mesgbuf, fsfd, logfp, blocktable,
                                 bsize, nindir, inodesect, ninodesect,
                                    treegroup, longbuf + inoaddridx,
                                       inuseptr, outplcptr, blksbelow,
                                           TRUE, wrkblks, wrkblkidx ) )
         {
            sprintf( mesgbuf, 
         "procinode: start file blk tree %ld %ld %ld %ld %ld %ld %ld.",
                          treeno, treegroup, inoaddridx,
                              *inuseptr, *outplcptr, blksbelow, inum );

            printmesg( TRUE, mesgtype, cmdname, fsname, mesgbuf,
                                                               logfp );

            return( FALSE );
         }

         inoaddridx++;
      }

         /******************************************************/
         /******************************************************/
         /*                                                    */
         /* end for each file block tree                       */
         /*                                                    */
         /******************************************************/
   }

      /*********************************************************/
      /*********************************************************/
      /*                                                       */
      /* end for each file block tree group                    */
      /*                                                       */
      /*********************************************************/

      /*********************************************************/
      /*********************************************************/
      /*                                                       */
      /* zero out the remainder of the inode block numbers     */
      /* (for regular files only),                             */
      /*                                                       */
      /* translate the full inode block numbers back into      */
      /* their shortened form, and                             */
      /*                                                       */
      /* return success                                        */
      /*                                                       */
      /*********************************************************/

   if ( isreg( mode ) )
   {
      for ( nullindex = usedaddrcnt; nullindex LT NADDR; nullindex++ )
      {
         longbuf[ nullindex ] = ZERO;
      }
   }

   ltol3( inodeptr->di_addr, longbuf, NADDR );

   inodeptr->di_addr[ THIRTYNINE ] = ZERO;

   return( TRUE );

   /************************************************************/
   /************************************************************/
   /*                                                          */
   /* end process_inode                                        */
   /*                                                          */
   /************************************************************/
}
