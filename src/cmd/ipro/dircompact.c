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
/***  FUNCTION:  dircompact - compact and sort all directories.    ***/
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
/***     begin compact_directories                                 ***/
/***                                                               ***/
/***        get a working block                                    ***/
/***                                                               ***/
/***        for each valid directory inode number found in the     ***/
/***                                      directory-list table do  ***/
/***                                                               ***/
/***           translate the inode number to its new position      ***/
/***           since the filesystem inodes have now been sorted    ***/
/***                                                               ***/
/***           pick up the number of the inode block containing    ***/
/***           the inode and the index of the inode within that    ***/
/***           inode block                                         ***/
/***                                                               ***/
/***           if the inode cache does not contain the new inode   ***/
/***                                                         then  ***/
/***              an inode cache fault has occurred and            ***/
/***                                                               ***/
/***              read in the inode block that contains the new    ***/
/***              inode into the inode cache                       ***/
/***                                                               ***/
/***           endif                                               ***/
/***                                                               ***/
/***           pick up the byte and block counts of the directory  ***/
/***                                                               ***/
/***           translate the inode block addresses into full       ***/
/***           block numbers                                       ***/
/***                                                               ***/
/***           for each block in the directory do                  ***/
/***                                                               ***/
/***              pick up the number of the block, load that       ***/
/***              number into the directory-block table, and read  ***/
/***              in that block into the directory table           ***/
/***                                                               ***/
/***           end for each block in the directory                 ***/
/***                                                               ***/
/***           pick up the number of directory links and           ***/
/***                                                               ***/
/***           for each link in the directory do                   ***/
/***                                                               ***/
/***              either zero the entire entry if the inode        ***/
/***              number is zero or                                ***/
/***                                                               ***/
/***              translate each inode number to its new position  ***/
/***              since the filesystem inodes have now been        ***/
/***              sorted                                           ***/
/***                                                               ***/
/***           end for each link in the directory                  ***/
/***                                                               ***/
/***           sort the directory entries on descending inode      ***/
/***           last modification times (the most recently          ***/
/***           modified files will be found the fastest)           ***/
/***           (skipping the first two entries since they are the  ***/
/***           current and parent directories, respectively, and   ***/
/***           must not be disturbed)                              ***/
/***                                                               ***/
/***           for each block in the directory do                  ***/
/***                                                               ***/
/***              write out that block in directory block table    ***/
/***              sequence                                         ***/
/***                                                               ***/
/***           end for each block in the directory                 ***/
/***                                                               ***/
/***        end for each valid directory inode number              ***/
/***                                                               ***/
/***        return success                                         ***/
/***                                                               ***/
/***     end compact_directories                                   ***/
/***                                                               ***/
/***                                                               ***/
/***  RETURN VALUES:  true - success, no errors detected.          ***/
/***                                                               ***/
/***                  false - failure, error detected in this      ***/
/***                          routine or erroneous value returned  ***/
/***                          from an invoked routine.             ***/
/***                                                               ***/
/***                                                               ***/
/***  NOTES:  This routine does NOT free excess directory blocks.  ***/
/***                                                               ***/
/***                                                               ***/
/***  ASSUMPTIONS:  the inodes have been reordered.                ***/
/***                                                               ***/
/***                                                               ***/
/***  SOURCE LOCATION:  /usr/src/cmd/ipro/dircompact.c             ***/
/***                                                               ***/
/***                                                               ***/
/*********************************************************************/
/*********************************************************************/


#include "iprodefs.h"


   BOOLEAN
dircompact( verbose, mesgtype, cmdname, fsname, mesgbuf,
                   fsfd, logfp, inodetable, dirlsttbl,
                           dirblktbl, dirtable, bsize, inopb,
                                    nindir, dirpb, wrkblks, wrkblkidx )

   BOOLEAN     verbose;
   CHARPTR     mesgtype;
   CHARPTR     cmdname;
   CHARPTR     fsname;
   CHARPTR     mesgbuf;
   INT         fsfd;
   FILEPTR     logfp;
   INODEINFO  *inodetable;
   INO_T      *dirlsttbl;
   LONGPTR     dirblktbl;
   DIRPTR      dirtable;
   LONG        bsize;
   LONG        inopb;
   LONG        nindir;
   LONG        dirpb;
   CHARPTR     wrkblks;
   LONG        wrkblkidx;
{
   LONG        odirinum;
   LONG        ndirinum;
   LONG        dirlstidx   = ZERO;
   LONG        newblkno;
   LONG        newblkidx;
   LONG        curblkno    = INVALID;
   INODEPTR    inodeblock;
   LONG        dirsz;
   LONG        dirblkcnt;
   DOUBLE      ceil();
   INOADDRBUF  longbuf;
   LONG        blkseqno;
   LONG        getblkno();
   LONG        dirlnkcnt;
   LONG        linkseqno;
   LONG        inum;
   INT         dircmp();
   LONG        dirtblseqno;
   LONG        dirblkno;

   /************************************************************/
   /************************************************************/
   /*                                                          */
   /* begin compact_directories                                */
   /*                                                          */
   /************************************************************/

   printmesg( verbose, INFORMATIVE, cmdname, fsname,
                                "Compacting The Directories.", logfp );

   if ( ( fsfd EQ CLOSED )      OR
           ( inodetable EQ NULL )  OR
              ( dirlsttbl EQ NULL )   OR
                 ( dirblktbl EQ NULL )   OR
                    ( dirtable EQ NULL )    OR
                       ( bsize LE ZERO )       OR
                          ( inopb LE ZERO )       OR
                             ( nindir LE ZERO )      OR
                                ( dirpb LE ZERO )       OR
                                   ( wrkblks EQ NULL )     OR
                                      ( wrkblkidx LT ZERO )   OR
                                         ( dirlstidx NE ZERO )   OR
                                              ( curblkno NE INVALID ) )
   {
      printmesg( TRUE, mesgtype, cmdname, fsname,
              "dircompact: bad parameters/variables not initialized.",
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
           "dircompact: can't get first working block %ld of %ld %ld.",
                                   wrkblkidx - ONE, WRKBLKCNT, bsize );

      printmesg( TRUE, mesgtype, cmdname, fsname, mesgbuf, logfp );

      return( FALSE );
   }

      /*********************************************************/
      /*********************************************************/
      /*                                                       */
      /* for each valid directory inode number found in the    */
      /*                               directory-list table do */
      /*                                                       */
      /*    translate the inode number to its new position     */
      /*    since the filesystem inodes have now been sorted   */
      /*                                                       */
      /*    pick up the number of the inode block containing   */
      /*    the inode and the index of the inode within that   */
      /*    inode block                                        */
      /*                                                       */
      /*********************************************************/

   while ( (odirinum = dirlsttbl[ dirlstidx++ ]) NE ZERO )
   {
      ndirinum = inodetable[ odirinum ].dirdest;

      newblkno = ( ndirinum - ONE ) / inopb;

      newblkidx = ( ndirinum - ONE ) % inopb;

         /******************************************************/
         /******************************************************/
         /*                                                    */
         /* if the inode cache does not contain the new inode  */
         /*                                               then */
         /*    an inode cache fault has occurred and           */
         /*                                                    */
         /*    read in the inode block that contains the new   */
         /*    inode into the inode cache                      */
         /*                                                    */
         /* endif                                              */
         /*                                                    */
         /******************************************************/

      if ( newblkno NE curblkno )
      {
         if ( NOT fsblockio( verbose, mesgtype, cmdname,
                                 fsname, mesgbuf, fsfd,
                                     logfp, bsize, READ,
                                          (CHARPTR)inodeblock,
                                               INOBLKOFF + newblkno ) )
         {
            printmesg( TRUE, mesgtype, cmdname, fsname,
                             "dircompact: inode cache fault.", logfp );

            return( FALSE );
         }

         curblkno = newblkno;
      }

         /******************************************************/
         /******************************************************/
         /*                                                    */
         /* pick up the byte and block counts of the directory */
         /*                                                    */
         /* translate the inode block addresses into full      */
         /* block numbers                                      */
         /*                                                    */
         /******************************************************/

      if ( (dirsz = inodeblock[ newblkidx ].di_size) EQ ZERO )
      {
         continue;
      }

      dirblkcnt = (LONG)ceil( (DOUBLE)dirsz / bsize );

      l3tol( longbuf, inodeblock[ newblkidx ].di_addr, NADDR );

         /******************************************************/
         /******************************************************/
         /*                                                    */
         /* for each block in the directory do                 */
         /*                                                    */
         /*    pick up the number of the block, load that      */
         /*    number into the directory-block table, and read */
         /*    in that block into the directory table          */
         /*                                                    */
         /* end for each block in the directory                */
         /*                                                    */
         /******************************************************/

      dirtblseqno = ZERO;

      for ( blkseqno = ZERO; blkseqno LT dirblkcnt; blkseqno++ )
      {
         if ( (dirblkno = getblkno( verbose, mesgtype, cmdname,
                                      fsname, mesgbuf, fsfd, logfp,
                                        bsize, nindir, longbuf,
                                          dirblkcnt, blkseqno, wrkblks,
                                                 wrkblkidx )) LT ZERO )
         {
            sprintf( mesgbuf, 
             "dircompact: can't get block number %ld %ld %ld %ld %ld.",
                   dirblkno, blkseqno, dirblkcnt, odirinum, ndirinum );

            printmesg( TRUE, mesgtype, cmdname, fsname, mesgbuf,
                                                               logfp );

            return( FALSE );
         }
         else if ( dirblkno EQ ZERO ) /*** fsck(1) will report a   ***/
         {                            /*** POSSIBLE FILE SIZE      ***/
            continue;                 /*** ERROR on this condition ***/
         }

         dirblktbl[ dirtblseqno ] = dirblkno;

         if ( NOT fsblockio( verbose, mesgtype, cmdname, fsname,
                                  mesgbuf, fsfd, logfp, bsize, READ,
                           (CHARPTR)( dirtable + dirtblseqno * dirpb ),
                                                           dirblkno ) )
         {
            printmesg( TRUE, mesgtype, cmdname, fsname,
                          "dircompact: read directory block.", logfp );

            return( FALSE );
         }

         dirtblseqno++;
      }

         /******************************************************/
         /******************************************************/
         /*                                                    */
         /* pick up the number of directory links and          */
         /*                                                    */
         /* for each link in the directory do                  */
         /*                                                    */
         /*    either zero the entire entry if the inode       */
         /*    number is zero or                               */
         /*                                                    */
         /*    translate each inode number to its new position */
         /*    since the filesystem inodes have now been       */
         /*    sorted                                          */
         /*                                                    */
         /* end for each link in the directory                 */
         /*                                                    */
         /******************************************************/

      dirlnkcnt = dirtblseqno * dirpb;

      for ( linkseqno = ZERO; linkseqno LT dirlnkcnt; linkseqno++ )
      {
         if ( (inum = dirtable[ linkseqno ].d_ino) EQ ZERO )
         {
            strncpy( dirtable[ linkseqno ].d_name, NULSTR, DIRSIZ );
         }
         else
         {
            dirtable[ linkseqno ].d_ino = inodetable[ inum ].dirdest;
         }
      }

         /******************************************************/
         /******************************************************/
         /*                                                    */
         /* sort the directory entries on descending inode     */
         /* last modification times (the most recently         */
         /* modified files will be found the fastest)          */
         /* (skipping the first two entries since they are the */
         /* current and parent directories, respectively, and  */
         /* must not be disturbed)                             */
         /*                                                    */
         /******************************************************/

      qsort( (CHARPTR)( dirtable + TWO ), dirlnkcnt - TWO, DIRSIZE,
                                                              dircmp );

         /******************************************************/
         /******************************************************/
         /*                                                    */
         /* for each block in the directory do                 */
         /*                                                    */
         /*    write out that block in directory block table   */
         /*    sequence                                        */
         /*                                                    */
         /******************************************************/

      for ( blkseqno = ZERO; blkseqno LT dirtblseqno; blkseqno++ )
      {
         if ( NOT fsblockio( verbose, mesgtype, cmdname, fsname,
                                mesgbuf, fsfd, logfp, bsize, WRITE,
                             (CHARPTR)( dirtable + blkseqno * dirpb ),
                                              dirblktbl[ blkseqno ] ) )
         {
            printmesg( TRUE, mesgtype, cmdname, fsname,
                         "dircompact: write directory block.", logfp );

            return( FALSE );
         }
      }
   }

      /*********************************************************/
      /*********************************************************/
      /*                                                       */
      /*    end for each block in the directory                */
      /*                                                       */
      /* end for each valid directory inode number             */
      /*                                                       */
      /* return success                                        */
      /*                                                       */
      /*********************************************************/

   return( TRUE );

   /************************************************************/
   /************************************************************/
   /*                                                          */
   /* end compact_directories                                  */
   /*                                                          */
   /************************************************************/
}
