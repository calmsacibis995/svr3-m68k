/*********************************************************************/
/*********************************************************************/
/***                                                               ***/
/***                                                               ***/
/***  (C) Copyright  Mototola, Inc.  1985, 1986, 1987              ***/
/***  All Rights Reserved                                          ***/
/***  Motorola Confidential/Proprietary                            ***/
/***                                                               ***/
/***                                                               ***/
/***  SOFTWARE PRODUCT:  IPRO                                      ***/
/***                                                               ***/
/***                                                               ***/
/***  FUNCTION:  memops - allocated memory operations.             ***/
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
/***     begin memory_operations                                   ***/
/***                                                               ***/
/***        allocate memory operation found                        ***/
/***                                                               ***/
/***           print informational message                         ***/
/***                                                               ***/
/***           check the validity of the memory pointer and the    ***/
/***           memory byte count,                                  ***/
/***                                                               ***/
/***           allocate the memory, and                            ***/
/***                                                               ***/
/***           return successful memory allocation                 ***/
/***                                                               ***/
/***        initialize memory operation found                      ***/
/***                                                               ***/
/***           print informational message                         ***/
/***                                                               ***/
/***           check the validity of the memory pointer and the    ***/
/***           memory byte count,                                  ***/
/***                                                               ***/
/***           initialize the memory, and                          ***/
/***                                                               ***/
/***           return successful memory initialization             ***/
/***                                                               ***/
/***        allocate and initialize memory operation found         ***/
/***                                                               ***/
/***           allocate the memory,                                ***/
/***                                                               ***/
/***           initialize the memory, and                          ***/
/***                                                               ***/
/***           return successful memory allocation and             ***/
/***           initialization                                      ***/
/***                                                               ***/
/***        deallocate memory operation found                      ***/
/***                                                               ***/
/***           check the validity of the memory pointer,           ***/
/***                                                               ***/
/***           deallocate the memory, and                          ***/
/***                                                               ***/
/***           return successful memory deallocation               ***/
/***                                                               ***/
/***        copy memory operation found                            ***/
/***                                                               ***/
/***           check the validity of the memory pointers and       ***/
/***           the memory byte count,                              ***/
/***                                                               ***/
/***           copy the memory, and                                ***/
/***                                                               ***/
/***           return successful memory copy                       ***/
/***                                                               ***/
/***        copy and initialize memory operation found             ***/
/***                                                               ***/
/***           copy the memory,                                    ***/
/***                                                               ***/
/***           initialize the source memory, and                   ***/
/***                                                               ***/
/***           return successful memory copy and source memory     ***/
/***           initialization                                      ***/
/***                                                               ***/
/***        exchange memory operation found                        ***/
/***                                                               ***/
/***           exchange the memory character for character and     ***/
/***                                                               ***/
/***           return successful memory exchange                   ***/
/***                                                               ***/
/***        operation not listed above                             ***/
/***                                                               ***/
/***           return failure on bad operation type                ***/
/***                                                               ***/
/***        end memory operation search                            ***/
/***                                                               ***/
/***     end memory_operations                                     ***/
/***                                                               ***/
/***                                                               ***/
/***  RETURN VALUES:  true - success, no errors detected.          ***/
/***                                                               ***/
/***                  false - failure, error detected in this      ***/
/***                          routine or erroneous value returned  ***/
/***                          from an invoked routine.             ***/
/***                                                               ***/
/***                                                               ***/
/***  NOTES:  This is a recursive routine for the combinative      ***/
/***          operations.                                          ***/
/***                                                               ***/
/***                                                               ***/
/***  ASSUMPTIONS:  the allocate operation requires that           ***/
/***                membufptr be a pointer to a pointer to a       ***/
/***                character since the newly allocated memory     ***/
/***                must be attached to a variable outside of      ***/
/***                this routine.                                  ***/
/***                                                               ***/
/***                                                               ***/
/***  SOURCE LOCATION:  /usr/src/cmd/ipro/memops.c                 ***/
/***                                                               ***/
/***                                                               ***/
/*********************************************************************/
/*********************************************************************/


#include "iprodefs.h"


   BOOLEAN
memops( verbose, mesgtype, cmdname, fsname, mesgbuf,
                  logfp, operation, membufptr, membufname,
                               membuf1, membuf2, membufsz, meminitchr )

   BOOLEAN  verbose;
   CHARPTR  mesgtype;
   CHARPTR  cmdname;
   CHARPTR  fsname;
   CHARPTR  mesgbuf;
   FILEPTR  logfp;
   INT      operation;
   CHARPTR *membufptr;
   CHARPTR  membufname;
   CHARPTR  membuf1;
   CHARPTR  membuf2;
   LONG     membufsz;
   INT      meminitchr;
{
   CHARPTR  malloc();
   CHARPTR  memset();
   CHARPTR  memcpy();
   INT      tempchr;

   /************************************************************/
   /************************************************************/
   /*                                                          */
   /* begin memory_operations                                  */
   /*                                                          */
   /************************************************************/

      /*********************************************************/
      /*********************************************************/
      /*                                                       */
      /* allocate memory operation found                       */
      /*                                                       */
      /*    print informational message                        */
      /*                                                       */
      /*    check the validity of the memory pointer and the   */
      /*    memory byte count,                                 */
      /*                                                       */
      /*    allocate the memory, and                           */
      /*                                                       */
      /*    return successful memory allocation                */
      /*                                                       */
      /*********************************************************/

   if ( operation EQ ALLOC_MEM )
   {
      if ( membufname NE NULL )
      {
         sprintf( mesgbuf, "Creating The %s Table.", membufname );

         printmesg( verbose, INFORMATIVE, cmdname, fsname, mesgbuf,
                                                               logfp );
      }

      if ( membufptr EQ NULL )
      {
         printmesg( TRUE, mesgtype, cmdname, fsname,
                           "memops: alloc_mem: null memory pointer.",
                                                               logfp );

         return( FALSE );
      }

      if ( membufsz LE ZERO )
      {
         sprintf( mesgbuf,
                      "memops: alloc_mem: byte count %ld is <= 0.",
                                                            membufsz );

         printmesg( TRUE, mesgtype, cmdname, fsname, mesgbuf, logfp );

         return( FALSE );
      }

      if ( (*membufptr = malloc( membufsz )) EQ NULL )
      {
         sprintf( mesgbuf, "memops: alloc_mem: can't get %ld bytes.",
                                                            membufsz );

         printmesg( TRUE, mesgtype, cmdname, fsname, mesgbuf, logfp );

         return( FALSE );
      }

      return( TRUE );
   }

      /*********************************************************/
      /*********************************************************/
      /*                                                       */
      /* initialize memory operation found                     */
      /*                                                       */
      /*    print informational message                        */
      /*                                                       */
      /*    check the validity of the memory pointer and the   */
      /*    memory byte count,                                 */
      /*                                                       */
      /*    initialize the memory, and                         */
      /*                                                       */
      /*    return successful memory initialization            */
      /*                                                       */
      /*********************************************************/

   else if ( operation EQ INIT_MEM )
   {
      if ( membufname NE NULL )
      {
         sprintf( mesgbuf, "Initializing The %s Table.", membufname );

         printmesg( verbose, INFORMATIVE, cmdname, fsname, mesgbuf,
                                                               logfp );
      }

      if ( membuf1 EQ NULL )
      {
         printmesg( TRUE, mesgtype, cmdname, fsname,
                            "memops: init_mem: null memory pointer.",
                                                               logfp );

         return( FALSE );
      }

      if ( membufsz LE ZERO )
      {
         sprintf( mesgbuf,
                      "memops: init_mem: byte count %ld is <= 0.",
                                                            membufsz );

         printmesg( TRUE, mesgtype, cmdname, fsname, mesgbuf, logfp );

         return( FALSE );
      }

      memset( membuf1, meminitchr, membufsz );

      return( TRUE );
   }

      /*********************************************************/
      /*********************************************************/
      /*                                                       */
      /* allocate and initialize memory operation found        */
      /*                                                       */
      /*    allocate the memory,                               */
      /*                                                       */
      /*    initialize the memory, and                         */
      /*                                                       */
      /*    return successful memory allocation and            */
      /*    initialization                                     */
      /*                                                       */
      /*********************************************************/

   else if ( operation EQ ALLOC_INIT_MEM )
   {
      if ( NOT memops( verbose, mesgtype, cmdname, fsname,
                          mesgbuf, logfp, ALLOC_MEM, membufptr,
                             membufname, NULL, NULL, membufsz, NULL ) )
      {
         printmesg( TRUE, mesgtype, cmdname, fsname,
                    "memops: alloc_init_mem: can't allocate memory.",
                                                               logfp );

         return( FALSE );
      }

      if ( NOT memops( verbose, mesgtype, cmdname, fsname,
                          mesgbuf, logfp, INIT_MEM, NULL, membufname,
                             *membufptr, NULL, membufsz, meminitchr ) )
      {
         printmesg( TRUE, mesgtype, cmdname, fsname,
                    "memops: alloc_init_mem: can't initialize memory.",
                                                               logfp );

         return( FALSE );
      }

      return( TRUE );
   }

      /*********************************************************/
      /*********************************************************/
      /*                                                       */
      /* deallocate memory operation found                     */
      /*                                                       */
      /*    check the validity of the memory pointer,          */
      /*                                                       */
      /*    deallocate the memory, and                         */
      /*                                                       */
      /*    return successful memory deallocation              */
      /*                                                       */
      /*********************************************************/

   else if ( operation EQ DEALLOC_MEM )
   {
      if ( membuf1 EQ NULL )
      {
         printmesg( TRUE, mesgtype, cmdname, fsname,
                         "memops: dealloc_mem: null memory pointer.",
                                                               logfp );

         return( FALSE );
      }

      free( membuf1 );

      return( TRUE );
   }

      /*********************************************************/
      /*********************************************************/
      /*                                                       */
      /* copy memory operation found                           */
      /*                                                       */
      /*    check the validity of the memory pointers and      */
      /*    the memory byte count,                             */
      /*                                                       */
      /*    copy the memory, and                               */
      /*                                                       */
      /*    return successful memory copy                      */
      /*                                                       */
      /*********************************************************/

   else if ( operation EQ COPY_MEM )
   {
      if ( membuf1 EQ NULL )
      {
         printmesg( TRUE, mesgtype, cmdname, fsname,
                         "memops: copy_mem: null #1 memory pointer.",
                                                               logfp );

         return( FALSE );
      }

      if ( membuf2 EQ NULL )
      {
         printmesg( TRUE, mesgtype, cmdname, fsname,
                         "memops: copy_mem: null #2 memory pointer.",
                                                               logfp );

         return( FALSE );
      }

      if ( membufsz LE ZERO )
      {
         sprintf( mesgbuf,
                      "memops: copy_mem: byte count %ld is <= 0.",
                                                            membufsz );

         printmesg( TRUE, mesgtype, cmdname, fsname, mesgbuf, logfp );

         return( FALSE );
      }

      memcpy( membuf2, membuf1, membufsz );

      return( TRUE );
   }

      /*********************************************************/
      /*********************************************************/
      /*                                                       */
      /* copy and initialize memory operation found            */
      /*                                                       */
      /*    copy the memory,                                   */
      /*                                                       */
      /*    initialize the source memory, and                  */
      /*                                                       */
      /*    return successful memory copy and source memory    */
      /*    initialization                                     */
      /*                                                       */
      /*********************************************************/

   else if ( operation EQ COPY_INIT_MEM )
   {
      if ( NOT memops( verbose, mesgtype, cmdname, fsname,
                             mesgbuf, logfp, COPY_MEM, NULL, NULL,
                                   membuf1, membuf2, membufsz, NULL ) )
      {
         printmesg( TRUE, mesgtype, cmdname, fsname,
                     "memops: copy_init_mem: can't allocate memory.",
                                                               logfp );

         return( FALSE );
      }

      if ( NOT memops( verbose, mesgtype, cmdname, fsname,
                           mesgbuf, logfp, INIT_MEM, NULL, NULL,
                                membuf1, NULL, membufsz, meminitchr ) )
      {
         printmesg( TRUE, mesgtype, cmdname, fsname,
                   "memops: copy_init_mem: can't initialize memory.",
                                                               logfp );

         return( FALSE );
      }

      return( TRUE );
   }

      /*********************************************************/
      /*********************************************************/
      /*                                                       */
      /* exchange memory operation found                       */
      /*                                                       */
      /*    exchange the memory character for character and    */
      /*                                                       */
      /*    return successful memory exchange                  */
      /*                                                       */
      /*********************************************************/

   else if ( operation EQ XCHG_MEM )
   {
      if ( membuf1 EQ NULL )
      {
         printmesg( TRUE, mesgtype, cmdname, fsname,
                         "memops: xchg_mem: null #1 memory pointer.",
                                                               logfp );

         return( FALSE );
      }

      if ( membuf2 EQ NULL )
      {
         printmesg( TRUE, mesgtype, cmdname, fsname,
                         "memops: xchg_mem: null #2 memory pointer.",
                                                               logfp );

         return( FALSE );
      }

      if ( membufsz LE ZERO )
      {
         sprintf( mesgbuf,
                      "memops: xchg_mem: byte count %ld is <= 0.",
                                                            membufsz );

         printmesg( TRUE, mesgtype, cmdname, fsname, mesgbuf, logfp );

         return( FALSE );
      }

      while ( membufsz-- GT ZERO )
      {
         XCHG( *membuf1, *membuf2, tempchr );

         membuf1++;

         membuf2++;
      }

      return( TRUE );
   }

      /*********************************************************/
      /*********************************************************/
      /*                                                       */
      /* operation not listed above                            */
      /*                                                       */
      /*    return failure on bad operation type               */
      /*                                                       */
      /*********************************************************/

   else
   {
      sprintf( mesgbuf, "memops: default: bad operation type, %d.",
                                                           operation );

      printmesg( TRUE, mesgtype, cmdname, fsname, mesgbuf, logfp );

      return( FALSE );
   }

      /*********************************************************/
      /*********************************************************/
      /*                                                       */
      /* end memory operation search                           */
      /*                                                       */
      /*********************************************************/

   /************************************************************/
   /************************************************************/
   /*                                                          */
   /* end memory_operations                                    */
   /*                                                          */
   /************************************************************/
}
