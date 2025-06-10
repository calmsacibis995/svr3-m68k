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
/***  SUBROUTINE:  procfilsys - process a UNIX filesystem.         ***/
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
/***     begin process_filesystem                                  ***/
/***                                                               ***/
/***        update all of the system disks                         ***/
/***                                                               ***/
/***        device file check                                      ***/
/***                                                               ***/
/***        open the filesystem for update purposes                ***/
/***                                                               ***/
/***        create the temporary super block                       ***/
/***                                                               ***/
/***        read in the temporary super block                      ***/
/***                                                               ***/
/***        check for valid UNIX filesystem at filesystem block    ***/
/***        0, offset 512 and pick up the filesystem block size.   ***/
/***        this is the standard MOTOROLA V/68 location for the    ***/
/***        super block record.                                    ***/
/***                                                               ***/
/***        check for old filesystem and raw device usage          ***/
/***                                                               ***/
/***        destroy the temporary super block                      ***/
/***                                                               ***/
/***        create the working block array                         ***/
/***                                                               ***/
/***        get two working blocks                                 ***/
/***                                                               ***/
/***        read in the boot block                                 ***/
/***                                                               ***/
/***        read in the super block                                ***/
/***                                                               ***/
/***        point to the super block record                        ***/
/***                                                               ***/
/***        determine current filesystem specific parameters       ***/
/***                                                               ***/
/***        determine new filesystem specific parameters           ***/
/***                                                               ***/
/***        test new filesystem specific parameters for sanity     ***/
/***                                                               ***/
/***        if the new filesystem size is larger than the current  ***/
/***                                         filesystem size then  ***/
/***                                                               ***/
/***           get one working block                               ***/
/***                                                               ***/
/***           read in the last block of the new, larger           ***/
/***           filesystem to test whether the end of the           ***/
/***           filesystem even exists on the disk                  ***/
/***                                                               ***/
/***           write out the last block of the new, larger         ***/
/***           filesystem to test whether the end of the           ***/
/***           filesystem even exists on the disk                  ***/
/***                                                               ***/
/***           destroy the last block                              ***/
/***                                                               ***/
/***        endif                                                  ***/
/***                                                               ***/
/***        create the test block table so that it is known that   ***/
/***        enough memory can be allocated before the filesystem   ***/
/***        is put into a corrupted state                          ***/
/***                                                               ***/
/***        destroy the test block table                           ***/
/***                                                               ***/
/***        a valid UNIX filesystem exists                         ***/
/***                                                               ***/
/***        load the free list array and the free inode array in   ***/
/***        the super block                                        ***/
/***                                                               ***/
/***           NOTE:  From this point on, any exit from IPRO is    ***/
/***                  FATAL and REQUIRES an immediate reload of    ***/
/***                  the filesystem in question.                  ***/
/***                                                               ***/
/***        write out the filesystem boot block                    ***/
/***                                                               ***/
/***        write out the filesystem super block                   ***/
/***                                                               ***/
/***        destroy the boot and super blocks                      ***/
/***                                                               ***/
/***        create the inode table                                 ***/
/***                                                               ***/
/***        create the directory-list table                        ***/
/***                                                               ***/
/***        load the inode table with inode reordering             ***/
/***        information                                            ***/
/***                                                               ***/
/***        reorder the inodes with respect to ascending file      ***/
/***        modification times                                     ***/
/***                                                               ***/
/***        create the directory-block table                       ***/
/***                                                               ***/
/***        create the directory table                             ***/
/***                                                               ***/
/***        compact the directories                                ***/
/***                                                               ***/
/***        the inodes and the directories have been processed     ***/
/***        successfully                                           ***/
/***                                                               ***/
/***        free up the now unnecessary inode and directory        ***/
/***        tables and continue processing                         ***/
/***                                                               ***/
/***        create the block table                                 ***/
/***                                                               ***/
/***        load the block table with block reordering             ***/
/***        information                                            ***/
/***                                                               ***/
/***        print estimated completion times                       ***/
/***                                                               ***/
/***        reorder the file data and indirect blocks              ***/
/***                                                               ***/
/***        load the filesystem free block list                    ***/
/***                                                               ***/
/***        zero out all new inode blocks, if any                  ***/
/***                                                               ***/
/***        zero out all old filesystem blocks, if any             ***/
/***                                                               ***/
/***        free up the working block array and the block table    ***/
/***        and close the filesystem                               ***/
/***                                                               ***/
/***        the filesystem has been successfully reorganized and   ***/
/***                                                               ***/
/***        return                                                 ***/
/***                                                               ***/
/***     end process_filesystem                                    ***/
/***                                                               ***/
/***                                                               ***/
/***  EXIT VALUES:  22 - variables not initialized.                ***/
/***                                                               ***/
/***                23 - file not a device file.                   ***/
/***                                                               ***/
/***                24 - can't open filesystem for update.         ***/
/***                                                               ***/
/***                25 - can't create temporary super block.       ***/
/***                                                               ***/
/***                26 - can't read in temporary super block.      ***/
/***                                                               ***/
/***                27 - no filesystem at block 0, offset 512.     ***/
/***                                                               ***/
/***                28 - old filesystem and raw device.            ***/
/***                                                               ***/
/***                29 - can't destroy temporary super block.      ***/
/***                                                               ***/
/***                30 - can't create the working block array.     ***/
/***                                                               ***/
/***                31 - can't get first working block.            ***/
/***                                                               ***/
/***                32 - can't get second working block.           ***/
/***                                                               ***/
/***                33 - can't read in the boot block.             ***/
/***                                                               ***/
/***                34 - can't read in the super block.            ***/
/***                                                               ***/
/***                35 - new inode block count too large.          ***/
/***                                                               ***/
/***                36 - new filesystem block count too large.     ***/
/***                                                               ***/
/***                37 - can't delete so many inode blocks.        ***/
/***                                                               ***/
/***                38 - can't delete so many free blocks.         ***/
/***                                                               ***/
/***                39 - can't get third working block.            ***/
/***                                                               ***/
/***                40 - can't read in last block.                 ***/
/***                                                               ***/
/***                41 - can't write out last block.               ***/
/***                                                               ***/
/***                42 - can't put third working block.            ***/
/***                                                               ***/
/***                43 - can't create the test block table.        ***/
/***                                                               ***/
/***                44 - can't destroy the test block table.       ***/
/***                                                               ***/
/***                45 - can't load super block arrays.            ***/
/***                                                               ***/
/***                46 - can't write out boot block.               ***/
/***                                                               ***/
/***                47 - can't write out super block.              ***/
/***                                                               ***/
/***                48 - can't put second working block.           ***/
/***                                                               ***/
/***                49 - can't put first working block.            ***/
/***                                                               ***/
/***                50 - can't create inode table.                 ***/
/***                                                               ***/
/***                51 - can't create directory-list table.        ***/
/***                                                               ***/
/***                52 - can't load inode table.                   ***/
/***                                                               ***/
/***                53 - can't reorder inodes.                     ***/
/***                                                               ***/
/***                54 - can't create directory-block table.       ***/
/***                                                               ***/
/***                55 - can't create directory table.             ***/
/***                                                               ***/
/***                56 - can't compact directories.                ***/
/***                                                               ***/
/***                57 - can't perform intermediate cleanup.       ***/
/***                                                               ***/
/***                58 - can't create block table.                 ***/
/***                                                               ***/
/***                59 - can't load block table.                   ***/
/***                                                               ***/
/***                60 - can't print completion times.             ***/
/***                                                               ***/
/***                61 - can't reorder file blocks.                ***/
/***                                                               ***/
/***                62 - can't load free list.                     ***/
/***                                                               ***/
/***                63 - can't zero out new inode blocks.          ***/
/***                                                               ***/
/***                64 - can't zero out old filesystem blocks.     ***/
/***                                                               ***/
/***                65 - can't perform final cleanup.              ***/
/***                                                               ***/
/***                                                               ***/
/***  ASSUMPTIONS:  fsname - points to the name of the filesystem  ***/
/***                         currently being reorganized.          ***/
/***                                                               ***/
/***                fsfd - is a valid, open-for-update file        ***/
/***                       descriptor of the filesystem.           ***/
/***                                                               ***/
/***                inodetable - points to an allocated memory     ***/
/***                             table capable of holding inode    ***/
/***                             reordering information for every  ***/
/***                             inode, initialized to zero,       ***/
/***                             valid entries start at the        ***/
/***                             SECOND position, proceed          ***/
/***                             sequentially without a break,     ***/
/***                             and are null terminated.          ***/
/***                                                               ***/
/***                dirlsttbl - points to an allocated memory      ***/
/***                            table capable of holding a list    ***/
/***                            of all directory inodes in the     ***/
/***                            filesystem, initialized to zero,   ***/
/***                            valid entries start at the FIRST   ***/
/***                            position, proceed sequentially     ***/
/***                            without a break, and are null      ***/
/***                            terminated.                        ***/
/***                                                               ***/
/***                dirblktbl - points to an allocated memory      ***/
/***                            table capable of holding a list    ***/
/***                            of directory block numbers for     ***/
/***                            the largest directory in the       ***/
/***                            filesystem, valid entries start    ***/
/***                            at the FIRST position and proceed  ***/
/***                            sequentially without a break.      ***/
/***                                                               ***/
/***                dirtable - points to an allocated memory       ***/
/***                           table capable of holding the        ***/
/***                           entire contents of the largest      ***/
/***                           directory in the filesystem, valid  ***/
/***                           entries start at the FIRST          ***/
/***                           position and proceed sequentially   ***/
/***                           without a break.                    ***/
/***                                                               ***/
/***                blocktable - points to an allocated memory     ***/
/***                             table capable of holding block    ***/
/***                             reordering information for every  ***/
/***                             non-inode block, initialized to   ***/
/***                             zero, valid entries start at the  ***/
/***                             FIRST position, and proceed       ***/
/***                             sequentially without a break.     ***/
/***                                                               ***/
/***                (n)inodesect - contains the (new) count of     ***/
/***                               the boot, super, and inode      ***/
/***                               blocks of the filesystem.       ***/
/***                                                               ***/
/***                inotblcnt - contains the number of entries in  ***/
/***                            the inode table.                   ***/
/***                                                               ***/
/***                dirlstcnt - contains the number of entries in  ***/
/***                            the directory-list table.          ***/
/***                                                               ***/
/***                blktblcnt - contains the number of entries in  ***/
/***                            the block table.                   ***/
/***                                                               ***/
/***                (n)fsblkcnt - contains the (new) count of all  ***/
/***                              the blocks in the filesystem.    ***/
/***                                                               ***/
/***                (n)inoblkcnt - contains the (new) count of     ***/
/***                               the inode blocks in the         ***/
/***                               filesystem.                     ***/
/***                                                               ***/
/***                (n)inodecnt - contains the (new) count of the  ***/
/***                              inodes in the filesystem.        ***/
/***                                                               ***/
/***                (n)interspersed - contains the (new) count of  ***/
/***                                  the data, indirect, and      ***/
/***                                  free blocks in the           ***/
/***                                  filesystem.                  ***/
/***                                                               ***/
/***                supblkptr - points to the super block of the   ***/
/***                            filesystem that has been read      ***/
/***                            into memory.                       ***/
/***                                                               ***/
/***                lrgdirblks - contains the running count of     ***/
/***                             data blocks for the largest       ***/
/***                             directory in the filesystem       ***/
/***                             found so far, and is initialized  ***/
/***                             to zero.                          ***/
/***                                                               ***/
/***                outofplace - contains the running count of     ***/
/***                             data and indirect blocks that     ***/
/***                             need to be exchanged.             ***/
/***                                                               ***/
/***                inotblsz - contains the byte count of the      ***/
/***                           size of the inode table.            ***/
/***                                                               ***/
/***                dirlstsz - contains the byte count of the      ***/
/***                           size of the directory list table.   ***/
/***                                                               ***/
/***                blktblsz - contains the byte count of the      ***/
/***                           size of the block table.            ***/
/***                                                               ***/
/***                (n)totfreblks - contains the (new) block       ***/
/***                                count of the entire            ***/
/***                                filesystem.                    ***/
/***                                                               ***/
/***                (n)totfresect - contains the (new) number of   ***/
/***                                free list sections.            ***/
/***                                                               ***/
/***                (n)presectcnt - contains the (new) number of   ***/
/***                                free list blocks preceding     ***/
/***                                the first free list section.   ***/
/***                                                               ***/
/***                (n)supfrecnt - contains the (new) number of    ***/
/***                               free list blocks in the super   ***/
/***                               block free list array.          ***/
/***                                                               ***/
/***                (n)totfreinos - contains the (new) number of   ***/
/***                                available inodes.              ***/
/***                                                               ***/
/***                (n)valinodes - contains the (new) number of    ***/
/***                               allocated inodes.               ***/
/***                                                               ***/
/***                (n)supinocnt - contains the (new) number of    ***/
/***                               available inodes listed in the  ***/
/***                               super block free inode array.   ***/
/***                                                               ***/
/***                deltainoblks - contains the inode block        ***/
/***                               difference between the present  ***/
/***                               and new filesystems.            ***/
/***                                                               ***/
/***                deltafreblks - contains the free block         ***/
/***                               difference between the present  ***/
/***                               and new filesystems.            ***/
/***                                                               ***/
/***                                                               ***/
/***  SOURCE LOCATION:  /usr/src/cmd/ipro/procfilsys.c             ***/
/***                                                               ***/
/***                                                               ***/
/*********************************************************************/
/*********************************************************************/


#include "iprodefs.h"


   VOID
procfilsys( verbose, cmdname, fsname, mesgbuf, logfp,
                                       fp, deltainoblks, deltafreblks )

   BOOLEAN     verbose;
   CHARPTR     cmdname;
   CHARPTR     fsname;
   CHARPTR     mesgbuf;
   FILEPTR     logfp;
   FILEPTR     fp;
   LONG        deltainoblks;
   LONG        deltafreblks;
{
   INODEINFO  *inodetable    = NULL;
   INO_T      *dirlsttbl     = NULL;
   LONGPTR     dirblktbl     = NULL;
   DIRPTR      dirtable      = NULL;
   BLOCKINFO  *blocktable    = NULL;
   LONG        lrgdirblks    = ZERO;
   LONG        outofplace    = ZERO;
   INT         fsfd          = CLOSED;
   CHARPTR     lastblock     = NULL;
   CHARPTR     bootblock     = NULL;
   CHARPTR     superblock    = NULL;
   SUPERBLOCK *supblkptr     = NULL;
   LONG        inodesect;
   LONG        inoblkcnt;
   LONG        inodecnt;
   LONG        inotblcnt;
   LONG        inotblsz;
   LONG        dirlstcnt;
   LONG        dirlstsz;
   LONG        fsblkcnt;
   LONG        interspersed;
   LONG        blktblcnt;
   LONG        blktblsz;
   LONG        totfreblks;
   LONG        totfresect;
   LONG        presectcnt;
   LONG        supfrecnt;
   LONG        totfreinos;
   LONG        valinodes;
   LONG        supinocnt;
   LONG        ninodesect;
   LONG        ninoblkcnt;
   LONG        ninodecnt;
   LONG        nfsblkcnt;
   LONG        ninterspersed;
   LONG        ntotfreblks;
   LONG        ntotfresect;
   LONG        npresectcnt;
   LONG        nsupfrecnt;
   LONG        ntotfreinos;
   LONG        nvalinodes;
   LONG        nsupinocnt;
   LONG        bsize;
   LONG        inopb;
   LONG        nindir;
   LONG        dirpb;
   CHARPTR     wrkblks       = NULL;
   LONG        wrkblkidx     = ZERO;
   CHARPTR     tmpsupblk     = NULL;
   CHARPTR     tstblktbl     = NULL;
   LONG        printtime();

   /************************************************************/
   /************************************************************/
   /*                                                          */
   /* begin process_filesystem                                 */
   /*                                                          */
   /************************************************************/

   if ( ( inodetable NE NULL )  OR
          ( dirlsttbl NE NULL )   OR
            ( dirblktbl NE NULL )   OR
              ( dirtable NE NULL )    OR
                ( blocktable NE NULL )  OR
                  ( lrgdirblks NE ZERO )  OR
                    ( outofplace NE ZERO )  OR
                      ( fsfd NE CLOSED )      OR
                        ( lastblock NE NULL )   OR
                          ( bootblock NE NULL )   OR
                            ( superblock NE NULL )  OR
                              ( supblkptr NE NULL )   OR
                                ( wrkblks NE NULL )     OR
                                  ( wrkblkidx NE ZERO )   OR
                                    ( tmpsupblk NE NULL )   OR
                                                ( tstblktbl NE NULL ) )
   {
      printmesg( TRUE, PROCEDURAL, cmdname, fsname,
               "procfilsys: bad parameters/variables not initialized.",
                                                               logfp );

      terminate( TRUE, PROCEDURAL, cmdname, fsname, mesgbuf,
                                fsfd, logfp, fp, NULL, NULL, NULL,
                                               NULL, NULL, TWENTYTWO );
   }

      /*********************************************************/
      /*********************************************************/
      /*                                                       */
      /* update all of the system disks                        */
      /*                                                       */
      /*********************************************************/

   sync();
   sync();
   sync();

   printmesg( verbose, INFORMATIVE, cmdname, fsname,
                               "Begin Filesystem Processing.", logfp );

   printtime( verbose, INFORMATIVE, cmdname, fsname, mesgbuf, logfp );

      /*********************************************************/
      /*********************************************************/
      /*                                                       */
      /* device file check                                     */
      /*                                                       */
      /*********************************************************/

   if ( NOT isdevice( verbose, PROCEDURAL, cmdname, fsname, mesgbuf,
                                                              logfp ) )
   {
      printmesg( TRUE, PROCEDURAL, cmdname, fsname,
                             "procfilsys: device file check.", logfp );

      terminate( TRUE, PROCEDURAL, cmdname, fsname, mesgbuf,
                              fsfd, logfp, fp, NULL, NULL, NULL,
                                             NULL, NULL, TWENTYTHREE );
   }

      /*********************************************************/
      /*********************************************************/
      /*                                                       */
      /* open the filesystem for update purposes               */
      /*                                                       */
      /*********************************************************/

   if ( (fsfd = open( fsname, UPDATE )) EQ CLOSED )
   {
      printmesg( TRUE, PROCEDURAL, cmdname, fsname,
              "procfilsys: can't open filesystem for update purposes.",
                                                               logfp );

      terminate( TRUE, PROCEDURAL, cmdname, fsname, mesgbuf,
                               fsfd, logfp, fp, NULL, NULL, NULL,
                                              NULL, NULL, TWENTYFOUR );
   }

      /*********************************************************/
      /*********************************************************/
      /*                                                       */
      /* create the temporary super block                      */
      /*                                                       */
      /*********************************************************/

   if ( NOT memops( verbose, PROCEDURAL, cmdname, fsname, mesgbuf,
                               logfp, ALLOC_MEM, &tmpsupblk, NULL,
                                         NULL, NULL, TMPBLKSZ, NULL ) )
   {
      printmesg( TRUE, PROCEDURAL, cmdname, fsname,
                  "procfilsys: create temporary super block.", logfp );

      terminate( TRUE, PROCEDURAL, cmdname, fsname, mesgbuf,
                               fsfd, logfp, fp, NULL, NULL, NULL,
                                              NULL, NULL, TWENTYFIVE );
   }

      /*********************************************************/
      /*********************************************************/
      /*                                                       */
      /* read in the temporary super block                     */
      /*                                                       */
      /*********************************************************/

   if ( NOT fsblockio( verbose, PROCEDURAL, cmdname, fsname,
                                  mesgbuf, fsfd, logfp, TMPBLKSZ,
                                              READ, tmpsupblk, ZERO ) )
   {
      printmesg( TRUE, PROCEDURAL, cmdname, fsname,
                 "procfilsys: read in temporary super block.", logfp );

      terminate( TRUE, PROCEDURAL, cmdname, fsname, mesgbuf,
                            fsfd, logfp, fp, tmpsupblk, NULL,
                                         NULL, NULL, NULL, TWENTYSIX );
   }

      /*********************************************************/
      /*********************************************************/
      /*                                                       */
      /* check for valid UNIX filesystem at filesystem block   */
      /* 0, offset 512 and pick up the filesystem block size.  */
      /* this is the standard MOTOROLA V/68 location for the   */
      /* super block record.                                   */
      /*                                                       */
      /*********************************************************/

   if ( NOT isfilsys( verbose, PROCEDURAL, cmdname,
                         fsname, mesgbuf, logfp, &bsize,
                            (SUPERBLOCK *)( tmpsupblk + V68SUPOFF ) ) )
   {
      printmesg( TRUE, PROCEDURAL, cmdname, fsname,
                    "procfilsys: check if filesystem exists.", logfp );

      terminate( TRUE, PROCEDURAL, cmdname, fsname, mesgbuf,
                            fsfd, logfp, fp, tmpsupblk, NULL,
                                       NULL, NULL, NULL, TWENTYSEVEN );
   }

      /*********************************************************/
      /*********************************************************/
      /*                                                       */
      /* check for old filesystem and raw device usage         */
      /*                                                       */
      /*********************************************************/

   if ( israw( verbose, PROCEDURAL, cmdname, fsname, mesgbuf, logfp )
                           AND  ( MRIOPHYSBLKS GT ONE )  AND
                                                  ( bsize LE OBSIZE ) )
   {
      printmesg( TRUE, PROCEDURAL, cmdname, fsname,
                            "procfilsys: use non-raw device.", logfp );

      terminate( TRUE, PROCEDURAL, cmdname, fsname, mesgbuf,
                            fsfd, logfp, fp, tmpsupblk, NULL,
                                       NULL, NULL, NULL, TWENTYEIGHT );
   }

      /*********************************************************/
      /*********************************************************/
      /*                                                       */
      /* destroy the temporary super block                     */
      /*                                                       */
      /*********************************************************/

   if ( NOT cleanup( verbose, PROCEDURAL, cmdname, fsname,
                                mesgbuf, CLOSED, logfp, tmpsupblk,
                                             NULL, NULL, NULL, NULL ) )
   {
      printmesg( TRUE, PROCEDURAL, cmdname, fsname,
                 "procfilsys: destroy temporary super block.", logfp );

      terminate( TRUE, PROCEDURAL, cmdname, fsname, mesgbuf,
                             fsfd, logfp, fp, tmpsupblk, NULL,
                                        NULL, NULL, NULL, TWENTYNINE );
   }

      /*********************************************************/
      /*********************************************************/
      /*                                                       */
      /* create the working block array                        */
      /*                                                       */
      /*********************************************************/

   if ( NOT memops( verbose, PROCEDURAL, cmdname, fsname,
                           mesgbuf, logfp, ALLOC_MEM,
                                   &wrkblks, NULL, NULL, NULL,
                                            WRKBLKCNT * bsize, NULL ) )
   {
      sprintf( mesgbuf,
                  "procfilsys: create working block array %ld %ld.",
                                                    WRKBLKCNT, bsize );

      printmesg( TRUE, PROCEDURAL, cmdname, fsname, mesgbuf, logfp );

      terminate( TRUE, PROCEDURAL, cmdname, fsname, mesgbuf,
                               fsfd, logfp, fp, NULL, NULL,
                                            NULL, NULL, NULL, THIRTY );
   }

      /*********************************************************/
      /*********************************************************/
      /*                                                       */
      /* get two working blocks                                */
      /*                                                       */
      /*********************************************************/

   if ( (bootblock = GETWRKBLK( wrkblks, WRKBLKCNT,
                                          wrkblkidx, bsize )) EQ NULL )
   {
      sprintf( mesgbuf, 
           "procfilsys: can't get first working block %ld of %ld %ld.",
                                   wrkblkidx - ONE, WRKBLKCNT, bsize );

      printmesg( TRUE, PROCEDURAL, cmdname, fsname, mesgbuf, logfp );

      terminate( TRUE, PROCEDURAL, cmdname, fsname, mesgbuf,
                              fsfd, logfp, fp, wrkblks, NULL,
                                         NULL, NULL, NULL, THIRTYONE );
   }

   if ( (superblock = GETWRKBLK( wrkblks, WRKBLKCNT,
                                          wrkblkidx, bsize )) EQ NULL )
   {
      sprintf( mesgbuf, 
          "procfilsys: can't get second working block %ld of %ld %ld.",
                                   wrkblkidx - ONE, WRKBLKCNT, bsize );

      printmesg( TRUE, PROCEDURAL, cmdname, fsname, mesgbuf, logfp );

      terminate( TRUE, PROCEDURAL, cmdname, fsname, mesgbuf,
                              fsfd, logfp, fp, wrkblks, NULL,
                                         NULL, NULL, NULL, THIRTYTWO );
   }

      /*********************************************************/
      /*********************************************************/
      /*                                                       */
      /* read in the boot block                                */
      /*                                                       */
      /*********************************************************/

   if ( NOT fsblockio( verbose, PROCEDURAL, cmdname, fsname,
                                 mesgbuf, fsfd, logfp, bsize, READ,
                                              bootblock, BOOTBLKOFF ) )
   {
      printmesg( TRUE, PROCEDURAL, cmdname, fsname,
                            "procfilsys: read in boot block.", logfp );

      terminate( TRUE, PROCEDURAL, cmdname, fsname, mesgbuf,
                              fsfd, logfp, fp, wrkblks, NULL, NULL,
                                             NULL, NULL, THIRTYTHREE );
   }

      /*********************************************************/
      /*********************************************************/
      /*                                                       */
      /* read in the super block                               */
      /*                                                       */
      /*********************************************************/

   if ( NOT fsblockio( verbose, PROCEDURAL, cmdname, fsname,
                                 mesgbuf, fsfd, logfp, bsize, READ,
                                              superblock, SUPBLKOFF ) )
   {
      printmesg( TRUE, PROCEDURAL, cmdname, fsname,
                           "procfilsys: read in super block.", logfp );

      terminate( TRUE, PROCEDURAL, cmdname, fsname, mesgbuf,
                              fsfd, logfp, fp, wrkblks, NULL, NULL,
                                              NULL, NULL, THIRTYFOUR );
   }

      /*********************************************************/
      /*********************************************************/
      /*                                                       */
      /* point to the super block record                       */
      /*                                                       */
      /*********************************************************/

   if ( bsize EQ OBSIZE )
   {
      supblkptr = (SUPERBLOCK *)( superblock + ZERO );
   }
   else
   {
      supblkptr = (SUPERBLOCK *)( bootblock + V68SUPOFF );
   }

      /*********************************************************/
      /*********************************************************/
      /*                                                       */
      /* determine current filesystem specific parameters      */
      /*                                                       */
      /*********************************************************/

   inopb = bsize / sizeof( INODE );
   nindir = bsize / sizeof( LONG );
   dirpb = bsize / sizeof( DIRECTORY );

   inodesect = supblkptr->s_isize;
   inoblkcnt = inodesect - INOBLKOFF;
   inodecnt = inoblkcnt * inopb;
   inotblcnt = inodecnt + TWO;
   inotblsz = inotblcnt * sizeof( INODEINFO );
   dirlstcnt = inodecnt + ONE;
   dirlstsz = dirlstcnt * sizeof( INO_T );

   fsblkcnt = supblkptr->s_fsize;
   interspersed = fsblkcnt - inodesect;
   blktblcnt = interspersed;
   blktblsz = blktblcnt * sizeof( BLOCKINFO );

   totfreblks = supblkptr->s_tfree;
   totfresect = totfreblks / NICFREE;
   presectcnt = totfreblks % NICFREE;
   supfrecnt = presectcnt + ONE;

   totfreinos = supblkptr->s_tinode;
   valinodes = inodecnt - totfreinos;
   supinocnt = MIN2( totfreinos, NICINOD );

      /*********************************************************/
      /*********************************************************/
      /*                                                       */
      /* determine new filesystem specific parameters          */
      /*                                                       */
      /*********************************************************/

   ninodesect = inodesect + deltainoblks;
   ninoblkcnt = ninodesect - INOBLKOFF;
   ninodecnt = ninoblkcnt * inopb;

   nfsblkcnt = fsblkcnt + deltainoblks + deltafreblks;
   ninterspersed = nfsblkcnt - ninodesect;

   ntotfreblks = totfreblks + deltafreblks;
   ntotfresect = ntotfreblks / NICFREE;
   npresectcnt = ntotfreblks % NICFREE;
   nsupfrecnt = npresectcnt + ONE;

   ntotfreinos = totfreinos + deltainoblks * inopb;
   nvalinodes = ninodecnt - ntotfreinos;
   nsupinocnt = MIN2( ntotfreinos, NICINOD );

      /*********************************************************/
      /*********************************************************/
      /*                                                       */
      /* test new filesystem specific parameters for sanity    */
      /*                                                       */
      /*********************************************************/

   if ( ninoblkcnt GT MAXINOBLKS )
   {
      sprintf( mesgbuf,
               "procfilsys: new inode block count too large %ld %ld.",
                                              ninoblkcnt, MAXINOBLKS );

      printmesg( TRUE, PROCEDURAL, cmdname, fsname, mesgbuf, logfp );

      terminate( TRUE, PROCEDURAL, cmdname, fsname, mesgbuf,
                              fsfd, logfp, fp, wrkblks, NULL, NULL,
                                              NULL, NULL, THIRTYFIVE );
   }

   if ( nfsblkcnt GT MAXFSBLKS )
   {
      sprintf( mesgbuf,
           "procfilsys: new filesystem block count too large %ld %ld.",
                                                nfsblkcnt, MAXFSBLKS );

      printmesg( TRUE, PROCEDURAL, cmdname, fsname, mesgbuf, logfp );

      terminate( TRUE, PROCEDURAL, cmdname, fsname, mesgbuf,
                              fsfd, logfp, fp, wrkblks, NULL, NULL,
                                               NULL, NULL, THIRTYSIX );
   }

   if ( ntotfreinos LT ZERO )
   {
      sprintf( mesgbuf, "procfilsys: can't delete so many inodes %ld.",
                                                         ntotfreinos );

      printmesg( TRUE, PROCEDURAL, cmdname, fsname, mesgbuf, logfp );

      terminate( TRUE, PROCEDURAL, cmdname, fsname, mesgbuf,
                              fsfd, logfp, fp, wrkblks, NULL, NULL,
                                             NULL, NULL, THIRTYSEVEN );
   }

   if ( ntotfreblks LT ZERO )
   {
      sprintf( mesgbuf,
                  "procfilsys: can't delete so many free blocks %ld.",
                                                         ntotfreblks );

      printmesg( TRUE, PROCEDURAL, cmdname, fsname, mesgbuf, logfp );

      terminate( TRUE, PROCEDURAL, cmdname, fsname, mesgbuf,
                              fsfd, logfp, fp, wrkblks, NULL, NULL,
                                             NULL, NULL, THIRTYEIGHT );
   }

      /*********************************************************/
      /*********************************************************/
      /*                                                       */
      /* if the new filesystem size is larger than the current */
      /*                                  filesystem size then */
      /*                                                       */
      /*********************************************************/

   if ( nfsblkcnt GT fsblkcnt )
   {
         /******************************************************/
         /******************************************************/
         /*                                                    */
         /* get one working block                              */
         /*                                                    */
         /******************************************************/
   
      if ( (lastblock = GETWRKBLK( wrkblks, WRKBLKCNT,
                                          wrkblkidx, bsize )) EQ NULL )
      {
         sprintf( mesgbuf, 
           "procfilsys: can't get third working block %ld of %ld %ld.",
                                   wrkblkidx - ONE, WRKBLKCNT, bsize );

         printmesg( TRUE, PROCEDURAL, cmdname, fsname, mesgbuf, logfp );
   
         terminate( TRUE, PROCEDURAL, cmdname, fsname, mesgbuf,
                              fsfd, logfp, fp, wrkblks, NULL,
                                        NULL, NULL, NULL, THIRTYNINE );
      }
   
         /******************************************************/
         /******************************************************/
         /*                                                    */
         /* read in the last block of the new, larger          */
         /* filesystem to test whether the end of the          */
         /* filesystem even exists on the disk                 */
         /*                                                    */
         /******************************************************/
   
      if ( NOT fsblockio( verbose, PROCEDURAL, cmdname, fsname,
                                 mesgbuf, fsfd, logfp, bsize, READ,
                                         lastblock, nfsblkcnt - ONE ) )
      {
         printmesg( TRUE, PROCEDURAL, cmdname, fsname,
                            "procfilsys: read in last block.", logfp );
   
         terminate( TRUE, PROCEDURAL, cmdname, fsname, mesgbuf,
                                fsfd, logfp, fp, wrkblks, NULL, NULL,
                                                   NULL, NULL, FORTY );
      }
   
         /******************************************************/
         /******************************************************/
         /*                                                    */
         /* write out the last block of the new, larger        */
         /* filesystem to test whether the end of the          */
         /* filesystem even exists on the disk                 */
         /*                                                    */
         /******************************************************/
   
      if ( NOT fsblockio( verbose, PROCEDURAL, cmdname, fsname,
                                 mesgbuf, fsfd, logfp, bsize, WRITE,
                                         lastblock, nfsblkcnt - ONE ) )
      {
         printmesg( TRUE, PROCEDURAL, cmdname, fsname,
                          "procfilsys: write out last block.", logfp );
   
         terminate( TRUE, PROCEDURAL, cmdname, fsname, mesgbuf,
                                fsfd, logfp, fp, wrkblks, NULL, NULL,
                                                NULL, NULL, FORTYONE );
      }
   
         /******************************************************/
         /******************************************************/
         /*                                                    */
         /* destroy the last block                             */
         /*                                                    */
         /******************************************************/

      if ( (lastblock = PUTWRKBLK( wrkblks, WRKBLKCNT,
                                          wrkblkidx, bsize )) NE NULL )
      {
         sprintf( mesgbuf, 
           "procfilsys: can't put third working block %ld of %ld %ld.",
                                         wrkblkidx, WRKBLKCNT, bsize );

         printmesg( TRUE, PROCEDURAL, cmdname, fsname, mesgbuf,
                                                               logfp );

         terminate( TRUE, PROCEDURAL, cmdname, fsname, mesgbuf,
                            fsfd, logfp, fp, wrkblks, NULL,
                                          NULL, NULL, NULL, FORTYTWO );
      }
   }

      /*********************************************************/
      /*********************************************************/
      /*                                                       */
      /* endif                                                 */
      /*                                                       */
      /* create the test block table so that it is known that  */
      /* enough memory can be allocated before the filesystem  */
      /* is put into a corrupted state                         */
      /*                                                       */
      /*********************************************************/

   if ( NOT memops( verbose, PROCEDURAL, cmdname, fsname, mesgbuf,
                               logfp, ALLOC_MEM, &tstblktbl, NULL,
                                         NULL, NULL, blktblsz, NULL ) )
   {
      printmesg( TRUE, PROCEDURAL, cmdname, fsname,
                       "procfilsys: create test block table.", logfp );

      terminate( TRUE, PROCEDURAL, cmdname, fsname, mesgbuf,
                              fsfd, logfp, fp, wrkblks, NULL, NULL,
                                              NULL, NULL, FORTYTHREE );
   }

      /*********************************************************/
      /*********************************************************/
      /*                                                       */
      /* destroy the test block table                          */
      /*                                                       */
      /*********************************************************/

   if ( NOT cleanup( verbose, PROCEDURAL, cmdname, fsname,
                                mesgbuf, CLOSED, logfp, tstblktbl,
                                             NULL, NULL, NULL, NULL ) )
   {
      printmesg( TRUE, PROCEDURAL, cmdname, fsname,
                      "procfilsys: destroy test block table.", logfp );

      terminate( TRUE, PROCEDURAL, cmdname, fsname, mesgbuf,
                           fsfd, logfp, fp, wrkblks, tstblktbl,
                                         NULL, NULL, NULL, FORTYFOUR );
   }

      /*********************************************************/
      /*********************************************************/
      /*                                                       */
      /* a valid UNIX filesystem exists                        */
      /*                                                       */
      /* load the free list array and the free inode array in  */
      /* the super block                                       */
      /*                                                       */
      /*********************************************************/

   if ( NOT loadsupblk( verbose, PROCEDURAL, cmdname, fsname,
                             mesgbuf, logfp, ninodesect, nfsblkcnt,
                                  ntotfreblks, ntotfresect, nsupfrecnt,
                                        ntotfreinos, nvalinodes,
                                              nsupinocnt, supblkptr ) )
   {
      printmesg( TRUE, PROCEDURAL, cmdname, fsname,
                              "procfilsys: load super block.", logfp );

      terminate( TRUE, PROCEDURAL, cmdname, fsname, mesgbuf,
                            fsfd, logfp, fp, wrkblks, NULL,
                                         NULL, NULL, NULL, FORTYFIVE );
   }

      /*********************************************************/
      /*********************************************************/
      /*                                                       */
      /*    NOTE:  From this point on, any exit from IPRO is   */
      /*           FATAL and REQUIRES an immediate reload of   */
      /*           the filesystem in question.                 */
      /*                                                       */
      /* write out the filesystem boot block                   */
      /*                                                       */
      /*********************************************************/

   if ( NOT fsblockio( verbose, FATAL, cmdname, fsname,
                                mesgbuf, fsfd, logfp, bsize, WRITE,
                                              bootblock, BOOTBLKOFF ) )
   {
      printmesg( TRUE, FATAL, cmdname, fsname,
                              "procfilsys: write boot block.", logfp );

      terminate( TRUE, FATAL, cmdname, fsname, mesgbuf,
                            fsfd, logfp, fp, wrkblks, NULL,
                                          NULL, NULL, NULL, FORTYSIX );
   }

      /*********************************************************/
      /*********************************************************/
      /*                                                       */
      /* write out the filesystem super block                  */
      /*                                                       */
      /*********************************************************/

   if ( NOT fsblockio( verbose, FATAL, cmdname, fsname,
                                mesgbuf, fsfd, logfp, bsize, WRITE,
                                              superblock, SUPBLKOFF ) )
   {
      printmesg( TRUE, FATAL, cmdname, fsname,
                             "procfilsys: write super block.", logfp );

      terminate( TRUE, FATAL, cmdname, fsname, mesgbuf,
                            fsfd, logfp, fp, wrkblks, NULL,
                                        NULL, NULL, NULL, FORTYSEVEN );
   }

      /*********************************************************/
      /*********************************************************/
      /*                                                       */
      /* destroy the boot and super blocks                     */
      /*                                                       */
      /*********************************************************/

   if ( (superblock = PUTWRKBLK( wrkblks, WRKBLKCNT,
                                          wrkblkidx, bsize )) NE NULL )
   {
      sprintf( mesgbuf, 
          "procfilsys: can't put second working block %ld of %ld %ld.",
                                         wrkblkidx, WRKBLKCNT, bsize );

      printmesg( TRUE, FATAL, cmdname, fsname, mesgbuf, logfp );

      terminate( TRUE, FATAL, cmdname, fsname, mesgbuf,
                            fsfd, logfp, fp, wrkblks, NULL,
                                        NULL, NULL, NULL, FORTYEIGHT );
   }

   if ( (bootblock = PUTWRKBLK( wrkblks, WRKBLKCNT,
                                          wrkblkidx, bsize )) NE NULL )
   {
      sprintf( mesgbuf, 
           "procfilsys: can't put first working block %ld of %ld %ld.",
                                         wrkblkidx, WRKBLKCNT, bsize );

      printmesg( TRUE, FATAL, cmdname, fsname, mesgbuf, logfp );

      terminate( TRUE, FATAL, cmdname, fsname, mesgbuf,
                            fsfd, logfp, fp, wrkblks, NULL,
                                         NULL, NULL, NULL, FORTYNINE );
   }

      /*********************************************************/
      /*********************************************************/
      /*                                                       */
      /* create the inode table                                */
      /*                                                       */
      /*********************************************************/

   if ( NOT memops( verbose, FATAL, cmdname, fsname, mesgbuf,
                           logfp, ALLOC_INIT_MEM,
                                   (CHARPTR *)( &inodetable ),
                                            INOTBLNAME, NULL, NULL,
                                                     inotblsz, NULL ) )
   {
      printmesg( TRUE, FATAL, cmdname, fsname,
                            "procfilsys: create inode table.", logfp );

      terminate( TRUE, FATAL, cmdname, fsname, mesgbuf,
                             fsfd, logfp, fp, wrkblks, NULL,
                                             NULL, NULL, NULL, FIFTY );
   }

      /*********************************************************/
      /*********************************************************/
      /*                                                       */
      /* create the directory-list table                       */
      /*                                                       */
      /*********************************************************/

   if ( NOT memops( verbose, FATAL, cmdname, fsname, mesgbuf,
                           logfp, ALLOC_INIT_MEM,
                                   (CHARPTR *)( &dirlsttbl ),
                                            DIRLSTNAME, NULL, NULL,
                                                     dirlstsz, NULL ) )
   {
      printmesg( TRUE, FATAL, cmdname, fsname,
                   "procfilsys: create directory-list table.", logfp );

      terminate( TRUE, FATAL, cmdname, fsname, mesgbuf,
                            fsfd, logfp, fp, wrkblks, inodetable,
                                          NULL, NULL, NULL, FIFTYONE );
   }

      /*********************************************************/
      /*********************************************************/
      /*                                                       */
      /* load the inode table with inode reordering            */
      /* information                                           */
      /*                                                       */
      /*********************************************************/

   if ( NOT loadinotbl( verbose, FATAL, cmdname, fsname,
                              mesgbuf, fsfd, logfp, inodetable,
                                    dirlsttbl, bsize, inopb, inoblkcnt,
                                          valinodes, &lrgdirblks,
                                                 wrkblks, wrkblkidx ) )
   {
      printmesg( TRUE, FATAL, cmdname, fsname,
                              "procfilsys: load inode table.", logfp );

      terminate( TRUE, FATAL, cmdname, fsname, mesgbuf,
                          fsfd, logfp, fp, wrkblks, inodetable,
                                     dirlsttbl, NULL, NULL, FIFTYTWO );
   }

      /*********************************************************/
      /*********************************************************/
      /*                                                       */
      /* reorder the inodes with respect to ascending file     */
      /* modification times                                    */
      /*                                                       */
      /*********************************************************/

   if ( NOT inoreorder( verbose, FATAL, cmdname, fsname,
                             mesgbuf, fsfd, logfp, inodetable,
                                    bsize, inopb, inoblkcnt, valinodes,
                                                 wrkblks, wrkblkidx ) )
   {
      printmesg( TRUE, FATAL, cmdname, fsname,
                                "procfilsys: reorder inodes.", logfp );

      terminate( TRUE, FATAL, cmdname, fsname, mesgbuf,
                         fsfd, logfp, fp, wrkblks, inodetable,
                                   dirlsttbl, NULL, NULL, FIFTYTHREE );
   }

      /*********************************************************/
      /*********************************************************/
      /*                                                       */
      /* create the directory-block table                      */
      /*                                                       */
      /*********************************************************/

   if ( NOT memops( verbose, FATAL, cmdname, fsname,
                          mesgbuf, logfp, ALLOC_MEM,
                               (CHARPTR *)( &dirblktbl ),
                                     DIRBLKNAME, NULL, NULL,
                                           lrgdirblks * sizeof( LONG ),
                                                               NULL ) )
   {
      printmesg( TRUE, FATAL, cmdname, fsname,
                  "procfilsys: create directory-block table.", logfp );

      terminate( TRUE, FATAL, cmdname, fsname, mesgbuf,
                         fsfd, logfp, fp, wrkblks, inodetable,
                                    dirlsttbl, NULL, NULL, FIFTYFOUR );
   }

      /*********************************************************/
      /*********************************************************/
      /*                                                       */
      /* create the directory table                            */
      /*                                                       */
      /*********************************************************/

   if ( NOT memops( verbose, FATAL, cmdname, fsname, mesgbuf,
                         logfp, ALLOC_MEM,
                               (CHARPTR *)( &dirtable ),
                                     DIRTBLNAME, NULL, NULL,
                                           lrgdirblks * bsize, NULL ) )
   {
      printmesg( TRUE, FATAL, cmdname, fsname,
                        "procfilsys: create directory table.", logfp );

      terminate( TRUE, FATAL, cmdname, fsname, mesgbuf,
                       fsfd, logfp, fp, wrkblks, inodetable,
                               dirlsttbl, dirblktbl, NULL, FIFTYFIVE );
   }

      /*********************************************************/
      /*********************************************************/
      /*                                                       */
      /* compact the directories                               */
      /*                                                       */
      /*********************************************************/

   if ( NOT dircompact( verbose, FATAL, cmdname, fsname, mesgbuf,
                           fsfd, logfp, inodetable, dirlsttbl,
                              dirblktbl, dirtable, bsize, inopb,
                                  nindir, dirpb, wrkblks, wrkblkidx ) )
   {
      printmesg( TRUE, FATAL, cmdname, fsname,
                           "procfilsys: compact directories.", logfp );

      terminate( TRUE, FATAL, cmdname, fsname, mesgbuf,
                     fsfd, logfp, fp, wrkblks, inodetable,
                            dirlsttbl, dirblktbl, dirtable, FIFTYSIX );
   }

      /*********************************************************/
      /*********************************************************/
      /*                                                       */
      /* the inodes and the directories have been processed    */
      /* successfully                                          */
      /*                                                       */
      /* free up the now unnecessary inode and directory       */
      /* tables and continue processing                        */
      /*                                                       */
      /*********************************************************/

   if ( NOT cleanup( verbose, FATAL, cmdname, fsname,
                          mesgbuf, CLOSED, logfp, inodetable,
                               dirlsttbl, dirblktbl, dirtable, NULL ) )
   {
      printmesg( TRUE, FATAL, cmdname, fsname,
                          "procfilsys: intermediate cleanup.", logfp );

      terminate( TRUE, FATAL, cmdname, fsname, mesgbuf,
                     fsfd, logfp, fp, wrkblks, inodetable,
                          dirlsttbl, dirblktbl, dirtable, FIFTYSEVEN );
   }

      /*********************************************************/
      /*********************************************************/
      /*                                                       */
      /* create the block table                                */
      /*                                                       */
      /*********************************************************/

   if ( NOT memops( verbose, FATAL, cmdname, fsname, mesgbuf,
                           logfp, ALLOC_INIT_MEM,
                                   (CHARPTR *)( &blocktable ),
                                            BLKTBLNAME, NULL, NULL,
                                                     blktblsz, NULL ) )
   {
      printmesg( TRUE, FATAL, cmdname, fsname,
                            "procfilsys: create block table.", logfp );

      terminate( TRUE, FATAL, cmdname, fsname, mesgbuf,
                            fsfd, logfp, fp, wrkblks, NULL,
                                        NULL, NULL, NULL, FIFTYEIGHT );
   }

      /*********************************************************/
      /*********************************************************/
      /*                                                       */
      /* load the block table with block reordering            */
      /* information                                           */
      /*                                                       */
      /*********************************************************/

   if ( NOT loadblktbl( verbose, FATAL, cmdname, fsname,
                           mesgbuf, fsfd, logfp, blocktable,
                              bsize, inopb, nindir, inodesect,
                                  inoblkcnt, ninodesect, ninterspersed,
                                        ntotfreblks, &outofplace,
                                                 wrkblks, wrkblkidx ) )
   {
      printmesg( TRUE, FATAL, cmdname, fsname,
                              "procfilsys: load block table.", logfp );

      terminate( TRUE, FATAL, cmdname, fsname, mesgbuf,
                            fsfd, logfp, fp, wrkblks, blocktable,
                                         NULL, NULL, NULL, FIFTYNINE );
   }

      /*********************************************************/
      /*********************************************************/
      /*                                                       */
      /* print estimated completion times                      */
      /*                                                       */
      /*********************************************************/

   printtime( verbose, INFORMATIVE, cmdname, fsname, mesgbuf, logfp );

   if ( NOT compltimes( verbose, FATAL, cmdname, fsname,
                              mesgbuf, logfp, inodesect,
                                    fsblkcnt, ninodesect, nfsblkcnt,
                                            ntotfreblks, outofplace ) )
   {
      printmesg( TRUE, FATAL, cmdname, fsname,
                        "procfilsys: print completion times.", logfp );

      terminate( TRUE, FATAL, cmdname, fsname, mesgbuf,
                            fsfd, logfp, fp, wrkblks, blocktable,
                                             NULL, NULL, NULL, SIXTY );
   }

      /*********************************************************/
      /*********************************************************/
      /*                                                       */
      /* reorder the file data and indirect blocks             */
      /*                                                       */
      /*********************************************************/

   printtime( verbose, INFORMATIVE, cmdname, fsname, mesgbuf, logfp );

   if ( NOT blkreorder( verbose, FATAL, cmdname, fsname,
                            mesgbuf, fsfd, logfp, blocktable,
                                 bsize, inodesect, fsblkcnt,
                                      blktblcnt, wrkblks, wrkblkidx ) )
   {
      printmesg( TRUE, FATAL, cmdname, fsname,
                                "procfilsys: reorder blocks.", logfp );

      terminate( TRUE, FATAL, cmdname, fsname, mesgbuf,
                            fsfd, logfp, fp, wrkblks, blocktable,
                                          NULL, NULL, NULL, SIXTYONE );
   }

      /*********************************************************/
      /*********************************************************/
      /*                                                       */
      /* load the filesystem free block list                   */
      /*                                                       */
      /*********************************************************/

   printtime( verbose, INFORMATIVE, cmdname, fsname, mesgbuf, logfp );

   if ( NOT loadfrelst( verbose, FATAL, cmdname, fsname,
                            mesgbuf, fsfd, logfp, bsize,
                                nfsblkcnt, ntotfreblks, ntotfresect,
                                    npresectcnt, wrkblks, wrkblkidx ) )
   {
      printmesg( TRUE, FATAL, cmdname, fsname,
                                "procfilsys: load free list.", logfp );

      terminate( TRUE, FATAL, cmdname, fsname, mesgbuf,
                            fsfd, logfp, fp, wrkblks, blocktable,
                                          NULL, NULL, NULL, SIXTYTWO );
   }

      /*********************************************************/
      /*********************************************************/
      /*                                                       */
      /* zero out all new inode blocks, if any                 */
      /*                                                       */
      /*********************************************************/

   printtime( verbose, INFORMATIVE, cmdname, fsname, mesgbuf, logfp );

   printmesg( verbose, INFORMATIVE, cmdname, fsname,
                                  "Zeroing New Inode Blocks.", logfp );

   if ( NOT initblocks( verbose, FATAL, cmdname, fsname,
                              mesgbuf, fsfd, logfp, bsize,
                                    inodesect, ninodesect - ONE,
                                           NULL, wrkblks, wrkblkidx ) )
   {
      printmesg( TRUE, FATAL, cmdname, fsname,
                             "procfilsys: zero inode blocks.", logfp );

      terminate( TRUE, FATAL, cmdname, fsname, mesgbuf,
                            fsfd, logfp, fp, wrkblks, blocktable,
                                        NULL, NULL, NULL, SIXTYTHREE );
   }

      /*********************************************************/
      /*********************************************************/
      /*                                                       */
      /* zero out all old filesystem blocks, if any            */
      /*                                                       */
      /*********************************************************/

   printtime( verbose, INFORMATIVE, cmdname, fsname, mesgbuf, logfp );

   printmesg( verbose, INFORMATIVE, cmdname, fsname,
                             "Zeroing Old Filesystem Blocks.", logfp );

   if ( NOT initblocks( verbose, FATAL, cmdname, fsname,
                              mesgbuf, fsfd, logfp, bsize,
                                    nfsblkcnt, fsblkcnt - ONE,
                                           NULL, wrkblks, wrkblkidx ) )
   {
      printmesg( TRUE, FATAL, cmdname, fsname,
                        "procfilsys: zero filesystem blocks.", logfp );

      terminate( TRUE, FATAL, cmdname, fsname, mesgbuf,
                            fsfd, logfp, fp, wrkblks, blocktable,
                                         NULL, NULL, NULL, SIXTYFOUR );
   }

      /*********************************************************/
      /*********************************************************/
      /*                                                       */
      /* free up the working block array and the block table   */
      /* and close the filesystem                              */
      /*                                                       */
      /*********************************************************/

   if ( NOT cleanup( verbose, PROCEDURAL, cmdname, fsname,
                             mesgbuf, fsfd, logfp, wrkblks,
                                       blocktable, NULL, NULL, NULL ) )
   {
      printmesg( TRUE, PROCEDURAL, cmdname, fsname,
                                 "procfilsys: final cleanup.", logfp );

      terminate( TRUE, PROCEDURAL, cmdname, fsname, mesgbuf,
                            fsfd, logfp, fp, wrkblks, blocktable,
                                         NULL, NULL, NULL, SIXTYFIVE );
   }

      /*********************************************************/
      /*********************************************************/
      /*                                                       */
      /* the filesystem has been successfully reorganized and  */
      /*                                                       */
      /* return                                                */
      /*                                                       */
      /*********************************************************/

   printtime( verbose, INFORMATIVE, cmdname, fsname, mesgbuf, logfp );

   printmesg( verbose, INFORMATIVE, cmdname, fsname,
                                 "End Filesystem Processing.", logfp );

   fsfd = CLOSED;

   sync();
   sync();
   sync();

   return;

   /************************************************************/
   /************************************************************/
   /*                                                          */
   /* end process_filesystem                                   */
   /*                                                          */
   /************************************************************/
}
