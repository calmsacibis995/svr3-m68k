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
/***  FUNCTION:  loadblktbl - load the block table with the        ***/
/***                          destination block numbers of all     ***/
/***                          data and indirect blocks.            ***/
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
/***     begin load_block_table                                    ***/
/***                                                               ***/
/***        get a working block                                    ***/
/***                                                               ***/
/***        for each block of inodes do                            ***/
/***                                                               ***/
/***           read in the inode block and                         ***/
/***                                                               ***/
/***           for each inode in the inode block do                ***/
/***                                                               ***/
/***              if the last inode has been processed then        ***/
/***                                                               ***/
/***                 cease processing inodes                       ***/
/***                                                               ***/
/***              else                                             ***/
/***                                                               ***/
/***                 load the destination block information of     ***/
/***                 the current inode's blocks into the block     ***/
/***                 table                                         ***/
/***                                                               ***/
/***              endif                                            ***/
/***                                                               ***/
/***           end for each inode                                  ***/
/***                                                               ***/
/***           write out the current inode block                   ***/
/***                                                               ***/
/***        end for each inode block                               ***/
/***                                                               ***/
/***        if the current inode block is valid then               ***/
/***                                                               ***/
/***           write out the block                                 ***/
/***                                                               ***/
/***        endif                                                  ***/
/***                                                               ***/
/***        either return failure if all of the data and indirect  ***/
/***        blocks have not been accounted for, or                 ***/
/***                                                               ***/
/***        return success                                         ***/
/***                                                               ***/
/***     end load_block_table                                      ***/
/***                                                               ***/
/***                                                               ***/
/***  RETURN VALUES:  true - success, no errors detected.          ***/
/***                                                               ***/
/***                  false - failure, error detected in this      ***/
/***                          routine or erroneous value returned  ***/
/***                          from an invoked routine.             ***/
/***                                                               ***/
/***                                                               ***/
/***  SOURCE LOCATION:  /usr/src/cmd/ipro/loadblktbl.c             ***/
/***                                                               ***/
/***                                                               ***/
/*********************************************************************/
/*********************************************************************/


#include "iprodefs.h"


   BOOLEAN
loadblktbl( verbose, mesgtype, cmdname, fsname, mesgbuf,
                 fsfd, logfp, blocktable, bsize, inopb, nindir,
                      inodesect, inoblkcnt, ninodesect, ninterspersed,
                           ntotfreblks, outplcptr, wrkblks, wrkblkidx )

   BOOLEAN    verbose;
   CHARPTR    mesgtype;
   CHARPTR    cmdname;
   CHARPTR    fsname;
   CHARPTR    mesgbuf;
   INT        fsfd;
   FILEPTR    logfp;
   BLOCKINFO *blocktable;
   LONG       bsize;
   LONG       inopb;
   LONG       nindir;
   LONG       inodesect;
   LONG       inoblkcnt;
   LONG       ninodesect;
   LONG       ninterspersed;
   LONG       ntotfreblks;
   LONGPTR    outplcptr;
   CHARPTR    wrkblks;
   LONG       wrkblkidx;
{
   LONG       inoblkno;
   LONG       curblkno      = INVALID;
   INODEPTR   inodeblock;
   LONG       inoblkidx;
   LONG       inuseblkcnt   = ZERO;
   INT        mode;

   /************************************************************/
   /************************************************************/
   /*                                                          */
   /* begin load_block_table                                   */
   /*                                                          */
   /************************************************************/

   sprintf( mesgbuf, "Loading The %s Table.", BLKTBLNAME );

   printmesg( verbose, INFORMATIVE, cmdname, fsname, mesgbuf, logfp );

   if ( ( fsfd EQ CLOSED )         OR
          ( blocktable EQ NULL )     OR
            ( bsize LE ZERO )          OR
              ( inopb LE ZERO )          OR
                ( nindir LE ZERO )         OR
                  ( inodesect LE ZERO )      OR
                    ( inoblkcnt LE ZERO )      OR
                      ( ninodesect LE ZERO )     OR
                        ( ninterspersed LE ZERO )  OR
                          ( ntotfreblks LT ZERO )    OR
                            ( outplcptr EQ NULL )      OR
                              ( wrkblks EQ NULL )        OR
                                ( wrkblkidx LT ZERO )      OR
                                  ( curblkno NE INVALID )    OR
                                              ( inuseblkcnt NE ZERO ) )
   {
      printmesg( TRUE, mesgtype, cmdname, fsname,
               "loadblktbl: bad parameters/variables not initialized.",
                                                               logfp );

      return( FALSE );
   }

      /*********************************************************/
      /*********************************************************/
      /*                                                       */
      /* get a working block                                   */
      /*                                                       */
      /*********************************************************/

   if ( (inodeblock = (INODEPTR)( GETWRKBLK( wrkblks, WRKBLKCNT,
                                        wrkblkidx, bsize ) )) EQ NULL )
   {
      sprintf( mesgbuf, 
           "loadblktbl: can't get first working block %ld of %ld %ld.",
                                   wrkblkidx - ONE, WRKBLKCNT, bsize );

      printmesg( TRUE, mesgtype, cmdname, fsname, mesgbuf, logfp );

      return( FALSE );
   }

      /*********************************************************/
      /*********************************************************/
      /*                                                       */
      /* for each block of inodes do                           */
      /*                                                       */
      /*********************************************************/

   for ( inoblkno = ZERO; inoblkno LT inoblkcnt; inoblkno++ )
   {
         /******************************************************/
         /******************************************************/
         /*                                                    */
         /* read in the inode block and                        */
         /*                                                    */
         /* for each inode in the inode block do               */
         /*                                                    */
         /******************************************************/

      curblkno = inoblkno;

      if ( NOT fsblockio( verbose, mesgtype, cmdname, fsname,
                                 mesgbuf, fsfd, logfp, bsize,
                                        READ, (CHARPTR)inodeblock,
                                               INOBLKOFF + curblkno ) )
      {
         printmesg( TRUE, mesgtype, cmdname, fsname,
                              "loadblktbl: read inode block.", logfp );

         return( FALSE );
      }

      for ( inoblkidx = ZERO; inoblkidx LT inopb; inoblkidx++ )
      {
            /***************************************************/
            /***************************************************/
            /*                                                 */
            /* if the last inode has been processed then       */
            /*                                                 */
            /*    cease processing inodes                      */
            /*                                                 */
            /* else                                            */
            /*                                                 */
            /*    load the destination block information of    */
            /*    the current inode's blocks into the block    */
            /*    table                                        */
            /*                                                 */
            /* endif                                           */
            /*                                                 */
            /***************************************************/

         if ( (mode = inodeblock[ inoblkidx ].di_mode) EQ ZERO )
         {
            goto nomoreinodes;
         }

         if ( NOT procinode( verbose, mesgtype, cmdname, fsname,
                              mesgbuf, fsfd, logfp, blocktable,
                               bsize, nindir, inodesect, ninodesect,
                                &inuseblkcnt, outplcptr,
                                  inodeblock + inoblkidx,
                                    curblkno * inopb + inoblkidx + ONE,
                                           mode, wrkblks, wrkblkidx ) )
         {
            sprintf( mesgbuf, "loadblktbl: process inode %ld %ld %ld.",
                                    curblkno * inopb + inoblkidx + ONE,
                                             inuseblkcnt, *outplcptr );

            printmesg( TRUE, mesgtype, cmdname, fsname, mesgbuf,
                                                               logfp );

            return( FALSE );
         }
      }

         /******************************************************/
         /******************************************************/
         /*                                                    */
         /* end for each inode                                 */
         /*                                                    */
         /* write out the current inode block                  */
         /*                                                    */
         /******************************************************/

      if ( NOT fsblockio( verbose, mesgtype, cmdname, fsname,
                                 mesgbuf, fsfd, logfp, bsize,
                                        WRITE, (CHARPTR)inodeblock,
                                               INOBLKOFF + curblkno ) )
      {
         printmesg( TRUE, mesgtype, cmdname, fsname,
                             "loadblktbl: write inode block.", logfp );

         return( FALSE );
      }

      curblkno = INVALID;
   }

      /*********************************************************/
      /*********************************************************/
      /*                                                       */
      /* end for each inode block                              */
      /*                                                       */
      /* if the current inode block is valid then              */
      /*                                                       */
      /*    write out the block                                */
      /*                                                       */
      /* endif                                                 */
      /*                                                       */
      /* either return failure if all of the data and indirect */
      /* blocks have not been accounted for, or                */
      /*                                                       */
      /* return success                                        */
      /*                                                       */
      /*********************************************************/

nomoreinodes:

   if ( curblkno NE INVALID )
   {
      if ( NOT fsblockio( verbose, mesgtype, cmdname, fsname,
                                 mesgbuf, fsfd, logfp, bsize,
                                        WRITE, (CHARPTR)inodeblock,
                                               INOBLKOFF + curblkno ) )
      {
         printmesg( TRUE, mesgtype, cmdname, fsname,
                       "loadblktbl: write valid inode block.", logfp );

         return( FALSE );
      }
   }

   if ( ( inuseblkcnt + ntotfreblks ) NE ninterspersed )
   {
      sprintf( mesgbuf, 
               "loadblktbl: filesystem sections mismatch %ld %ld %ld.",
                             inuseblkcnt, ntotfreblks, ninterspersed );

      printmesg( TRUE, mesgtype, cmdname, fsname, mesgbuf, logfp );

      return( FALSE );
   }

   return( TRUE );

   /************************************************************/
   /************************************************************/
   /*                                                          */
   /* end load_block_table                                     */
   /*                                                          */
   /************************************************************/
}
