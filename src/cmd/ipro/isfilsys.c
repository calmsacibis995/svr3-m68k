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
/***  FUNCTION:  isfilsys - determine whether a file is a valid    ***/
/***                        UNIX filesystem.                       ***/
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
/***     begin is_filesystem                                       ***/
/***                                                               ***/
/***        return true and the filesystem block size or           ***/
/***                                                               ***/
/***        return false and an error                              ***/
/***                                                               ***/
/***     end is_filesystem                                         ***/
/***                                                               ***/
/***                                                               ***/
/***  RETURN VALUES:  true - success, no errors detected.          ***/
/***                                                               ***/
/***                  false - failure, error detected in this      ***/
/***                          routine or erroneous value returned  ***/
/***                          from an invoked routine.             ***/
/***                                                               ***/
/***                                                               ***/
/***  NOTES:  Besides the traditional checks of the "s_magic" and  ***/
/***          the "s_type" field values and their relationships,   ***/
/***          a check of the consistency of the "s_isize" and      ***/
/***          "s_fsize" fields is made.  A check is made on the    ***/
/***          "s_nfree" and "s_tfree" fields.  A range check on    ***/
/***          the "s_type" field is also made.  All of these       ***/
/***          additional checks ensure that the target file for    ***/
/***          the reorganization is truly a UNIX filesystem.       ***/
/***                                                               ***/
/***                                                               ***/
/***  SOURCE LOCATION:  /usr/src/cmd/ipro/isfilsys.c               ***/
/***                                                               ***/
/***                                                               ***/
/*********************************************************************/
/*********************************************************************/


#include "iprodefs.h"


   BOOLEAN
isfilsys( verbose, mesgtype, cmdname, fsname, mesgbuf,
                                           logfp, bsizeptr, supblkptr )

   BOOLEAN     verbose;
   CHARPTR     mesgtype;
   CHARPTR     cmdname;
   CHARPTR     fsname;
   CHARPTR     mesgbuf;
   FILEPTR     logfp;
   LONGPTR     bsizeptr;
   SUPERBLOCK *supblkptr;
{
   LONG        fstype;
   LONG        longpow();

   /************************************************************/
   /************************************************************/
   /*                                                          */
   /* begin is_filesystem                                      */
   /*                                                          */
   /************************************************************/

   printmesg( verbose, INFORMATIVE, cmdname, fsname,
                            "Filesystem Determination Check.", logfp );

   if ( ( bsizeptr EQ NULL )  OR  ( supblkptr EQ NULL ) )
   {
      printmesg( TRUE, mesgtype, cmdname, fsname,
                 "isfilsys: bad parameters/variables not initialized.",
                                                               logfp );

      return( FALSE );
   }

      /*********************************************************/
      /*********************************************************/
      /*                                                       */
      /* return true and the filesystem block size or          */
      /*                                                       */
      /* return false and an error                             */
      /*                                                       */
      /*********************************************************/

   fstype = supblkptr->s_type;

   if ( supblkptr->s_isize GE supblkptr->s_fsize )
   {
      sprintf( mesgbuf,
                "isfilsys: filesystem section size mismatch %ld %ld.",
                              supblkptr->s_isize, supblkptr->s_fsize );

      printmesg( TRUE, mesgtype, cmdname, fsname, mesgbuf, logfp );

      return( FALSE );
   }
   else if ( supblkptr->s_nfree NE
                               ( supblkptr->s_tfree % NICFREE + ONE ) )
   {
      sprintf( mesgbuf,
              "isfilsys: filesystem free block list mismatch %ld %ld.",
                              supblkptr->s_nfree, supblkptr->s_tfree );

      printmesg( TRUE, mesgtype, cmdname, fsname, mesgbuf, logfp );

      return( FALSE );
   }
   else if ( supblkptr->s_magic NE FsMAGIC )
   {
      *bsizeptr = OBSIZE;

      return( TRUE );
   }
   else
   {
      switch ( fstype )
      {
         case Fs1b:

            *bsizeptr = OBSIZE * longpow( TWO, fstype - ONE );
            break;

         case Fs2b:

            *bsizeptr = OBSIZE * longpow( TWO, fstype - ONE );
            break;

         case Fs4b:

            *bsizeptr = OBSIZE * longpow( TWO, fstype - ONE );
            break;

         case Fs8b:

            *bsizeptr = OBSIZE * longpow( TWO, fstype - ONE );
            break;

         case Fs16b:

            *bsizeptr = OBSIZE * longpow( TWO, fstype - ONE );
            break;

         default:

            sprintf( mesgbuf,
                "isfilsys: can't determine filesystem block size %ld.",
                                                              fstype );

            printmesg( TRUE, mesgtype, cmdname, fsname, mesgbuf,
                                                               logfp );
            return( FALSE );

      }

      return( TRUE );
   }

   /************************************************************/
   /************************************************************/
   /*                                                          */
   /* end is_filesystem                                        */
   /*                                                          */
   /************************************************************/
}
