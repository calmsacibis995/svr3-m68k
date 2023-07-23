#ifndef RELEASE
#ident crc -- a variant of 'check' that computes 16-bit crc's on files
#ident {crc handles two switches that check doesn't: -c and -l}
#ident {now also handles -d switch to include date on output}
#endif
/*********************************************************************/
/***                                                               ***/
/***  (C) Copyright  Motorola, Inc.  1985                          ***/
/***  All Rights Reserved                                          ***/
/***                                                               ***/
/***  PROGRAM:  CHECK - print 16-bit file crc's.                   ***/
/***                                                               ***/
/***  REVISION AUTHOR:  Norman L. Nealy                            ***/
/***                    Senior Software Systems Engineer           ***/
/***                    Motorola/Four-Phase Systems, Inc.          ***/
/***                    3013 South 52nd Street  MS-J3              ***/
/***                    Tempe, Arizona  85282                      ***/
/***                    (602) 438-5724/5600                        ***/
/***                                                               ***/
/***  DATE:  December 2, 1985                                      ***/
/***                                                               ***/
/***  REVISED BY:  David P. Schwartz                               ***/
/***  DATE:  December 2, 1985                                      ***/
/***                                                               ***/
/***               Added '-c' invocation option that adds a        ***/
/***               byte count to the output line.                  ***/
/***                                                               ***/
/***  REVISED BY:  David P. Schwartz                               ***/
/***  DATE:  February 21, 1986                                     ***/
/***                                                               ***/
/***               Added '-l' option that computes crc's for each  ***/
/***               line.  Outputs a decimal number for each line   ***/
/***               in the input stream.                            ***/
/***                                                               ***/
/***  REVISED BY:  David P. Schwartz                               ***/
/***  DATE:  April 29, 1986                                        ***/
/***                                                               ***/
/***               Added '-d' option that prints out the file's    ***/
/***               modification date in col 3.                     ***/
/***                                                               ***/
/***  SOURCE LOCATION:  scm!/usr/local/src/crc                     ***/
/***                                                               ***/
/***  OBJECT LOCATION:  scm!/usr/local                             ***/
/***                                                               ***/
/*********************************************************************/

#include <stdio.h>
#include <filehdr.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>

extern int errno;
extern int stat();

#define BLOCK   1024
#define FAILED  (-1)
#define READ    0

   /************************************************************/
   /*                                                          */
   /* Table used for generating 16-bit crc's on a byte stream. */
   /*                                                          */
   /************************************************************/

int Mval[] =
{
   0x0000, 0xC0C1, 0xC181, 0x0140, 0xC301, 0x03C0, 0x0280, 0xC241,
   0xC601, 0x06C0, 0x0780, 0xC741, 0x0500, 0xC5C1, 0xC481, 0x0440,
   0xCC01, 0x0CC0, 0x0D80, 0xCD41, 0x0F00, 0xCFC1, 0xCE81, 0x0E40,
   0x0A00, 0xCAC1, 0xCB81, 0x0B40, 0xC901, 0x09C0, 0x0880, 0xC841,
   0xD801, 0x18C0, 0x1980, 0xD941, 0x1B00, 0xDBC1, 0xDA81, 0x1A40,
   0x1E00, 0xDEC1, 0xDF81, 0x1F40, 0xDD01, 0x1DC0, 0x1C80, 0xDC41,
   0x1400, 0xD4C1, 0xD581, 0x1540, 0xD701, 0x17C0, 0x1680, 0xD641,
   0xD201, 0x12C0, 0x1380, 0xD341, 0x1100, 0xD1C1, 0xD081, 0x1040,
   0xF001, 0x30C0, 0x3180, 0xF141, 0x3300, 0xF3C1, 0xF281, 0x3240,
   0x3600, 0xF6C1, 0xF781, 0x3740, 0xF501, 0x35C0, 0x3480, 0xF441,
   0x3C00, 0xFCC1, 0xFD81, 0x3D40, 0xFF01, 0x3FC0, 0x3E80, 0xFE41,
   0xFA01, 0x3AC0, 0x3B80, 0xFB41, 0x3900, 0xF9C1, 0xF881, 0x3840,
   0x2800, 0xE8C1, 0xE981, 0x2940, 0xEB01, 0x2BC0, 0x2A80, 0xEA41,
   0xEE01, 0x2EC0, 0x2F80, 0xEF41, 0x2D00, 0xEDC1, 0xEC81, 0x2C40,
   0xE401, 0x24C0, 0x2580, 0xE541, 0x2700, 0xE7C1, 0xE681, 0x2640,
   0x2200, 0xE2C1, 0xE381, 0x2340, 0xE101, 0x21C0, 0x2080, 0xE041,
   0xA001, 0x60C0, 0x6180, 0xA141, 0x6300, 0xA3C1, 0xA281, 0x6240,
   0x6600, 0xA6C1, 0xA781, 0x6740, 0xA501, 0x65C0, 0x6480, 0xA441,
   0x6C00, 0xACC1, 0xAD81, 0x6D40, 0xAF01, 0x6FC0, 0x6E80, 0xAE41,
   0xAA01, 0x6AC0, 0x6B80, 0xAB41, 0x6900, 0xA9C1, 0xA881, 0x6840,
   0x7800, 0xB8C1, 0xB981, 0x7940, 0xBB01, 0x7BC0, 0x7A80, 0xBA41,
   0xBE01, 0x7EC0, 0x7F80, 0xBF41, 0x7D00, 0xBDC1, 0xBC81, 0x7C40,
   0xB401, 0x74C0, 0x7580, 0xB541, 0x7700, 0xB7C1, 0xB681, 0x7640,
   0x7200, 0xB2C1, 0xB381, 0x7340, 0xB101, 0x71C0, 0x7080, 0xB041,
   0x5000, 0x90C1, 0x9181, 0x5140, 0x9301, 0x53C0, 0x5280, 0x9241,
   0x9601, 0x56C0, 0x5780, 0x9741, 0x5500, 0x95C1, 0x9481, 0x5440,
   0x9C01, 0x5CC0, 0x5D80, 0x9D41, 0x5F00, 0x9FC1, 0x9E81, 0x5E40,
   0x5A00, 0x9AC1, 0x9B81, 0x5B40, 0x9901, 0x59C0, 0x5880, 0x9841,
   0x8801, 0x48C0, 0x4980, 0x8941, 0x4B00, 0x8BC1, 0x8A81, 0x4A40,
   0x4E00, 0x8EC1, 0x8F81, 0x4F40, 0x8D01, 0x4DC0, 0x4C80, 0x8C41,
   0x4400, 0x84C1, 0x8581, 0x4540, 0x8701, 0x47C0, 0x4680, 0x8641,
   0x8201, 0x42C0, 0x4380, 0x8341, 0x4100, 0x81C1, 0x8081, 0x4040
};

void do_lines();	/* fwd ref */

   int
main( argc, argv )

   int                argc;
   char              *argv[];
{
   char              *invoked;
   int                c;
   int                i;
   int                infile;
   extern int         optind;
   extern char       *optarg;
   int                filemode  = 0;          /*  -f  */
   int                rawmode   = 0;          /*  -r  */
   int                inputmode = 0;          /*  '-'  */
   int                countmode = 0;          /*  -c  */
   int                linemode  = 0;          /*  -l  */
   int                showdate	= 0;          /*  -d  */
   FILE              *ifp, *fp;
   FILE              *fopen();
   char               filename[ 512 ];


   invoked = argv[ 0 ];

   while ( (c = getopt( argc, argv, "frcld" )) != EOF )
   {
      switch ( c )
      {
         case 'f':

            filemode++;
            break;

         case 'r':

            rawmode++;
            break;

		 case 'c':

			countmode++;
			break;

		 case 'l':

			linemode++;
			break;

		 case 'd':

			showdate++;
			break;

         case '?':

            usage( invoked, "bad option letter." );
            exit( 1 );
      }
   }

      /*********************************************************/
      /*                                                       */
      /* if no file names appear after the command line        */
      /* options then                                          */
      /*    exit                                               */
      /* else if the first file name is a hyphen then          */
      /*    read the standard input                            */
      /* endif                                                 */
      /*                                                       */
      /*********************************************************/

   if ( optind >= argc )
   {
      usage( invoked, "argument count." );
      exit( 2 );
   }
   else if ( strcmp( argv[ optind ], "-" ) == 0 )
   {
      inputmode++;
   }

      /*********************************************************/
      /*                                                       */
      /* if file mode has been selected then                   */
      /*                                                       */
      /*    if reading the standard input then                 */
      /*       for each file name read do                      */
      /*          open the file for reading,                   */
      /*          crc the file, and                            */
      /*          close the file                               */
      /*       endfor                                          */
      /*                                                       */
      /*    else                                               */
      /*       for each file name on the command line do       */
      /*          open the file for reading and                */
      /*          for each file name read do                   */
      /*             open the file for reading,                */
      /*             crc the file, and                         */
      /*             close the file                            */
      /*          endfor                                       */
      /*       endfor                                          */
      /*                                                       */
      /*    endif                                              */
      /*                                                       */
      /*********************************************************/

   if ( filemode )
   {
      if ( inputmode )
      {
         while ( scanf( "%s", filename ) != EOF )
         {
		    if (!linemode)
	    	{
               	if ( (infile = open( filename, READ )) == FAILED )
               	{
               	fprintf( stderr, 
							"%s: can't open %s for reading.\n",
                        	invoked, filename );
               	continue;
                }
                doit( filename, infile, rawmode, invoked, countmode, showdate );
               	close( infile );
            }
			else
				do_lines( filename, NULL, invoked, showdate );
		 }
      }
      else
      {
         for ( i = optind; i < argc; i++ )
         {
            if ( (ifp = fopen( argv[ i ], "r" )) == NULL )
            {
               fprintf( stderr, 
						"%s: can't open %s for reading.\n",
                        invoked, argv[ i ] );
               continue;
            }

            while ( fscanf( ifp, "%s", filename ) != EOF )
            {
			   if (!linemode)
			   {
                  if ( (infile = open( filename, READ )) == FAILED )
                  {
                     fprintf(  stderr, 
							   "%s: can't open %s for reading.\n",
                               invoked, filename );
                     continue;
                  }
                  doit( filename, infile, rawmode, invoked, countmode, showdate );
                  close( infile );
			   }
			   else
                  do_lines( filename, NULL, invoked, showdate );
            }
            fclose( ifp );
         }
      }
   }

      /*********************************************************/
      /*                                                       */
      /* else                                                  */
      /*                                                       */
      /*    if reading the standard input then                 */
      /*          crc the standard input                       */
      /*    else                                               */
      /*       for each file name on the command line do       */
      /*          open the file for reading,                   */
      /*          crc the file, and                            */
      /*          close the file                               */
      /*       endfor                                          */
      /*    endif                                              */
      /*                                                       */
      /* endif                                                 */
      /*                                                       */
      /* exit normally                                         */
      /*                                                       */
      /*********************************************************/

   else
   {
      if ( inputmode )
      {
		 if (!linemode)
            doit( "Standard Input", 0, rawmode, invoked, countmode, showdate );
         else
            do_lines( "Standard Input", stdin, invoked, showdate );
      }
      else
      {
         for ( i = optind; i < argc; i++ )
         {
            if (!linemode)
            {
               if ( (infile = open( argv[ i ], READ )) == FAILED )
               {
                  fprintf( stderr, 
                           "%s: can't open %s for reading.\n",
                           invoked, argv[ i ] );
                  continue;
               }
               doit( argv[ i ], infile, rawmode, invoked, countmode, showdate );
               close( infile );
            }
            else
               do_lines( argv[i], NULL, invoked, showdate );

		 } /* endfor */
      } /* end if (inputmode) */
   }  /* endfor */
   exit( 0 );
}


   int
usage( invoked, s )

   char *invoked;
   char *s;
{

   fprintf( stderr, "%s: %s\n", invoked, s );
   fprintf( stderr, "Usage: %s [ -frcld ] file_name_list\n", invoked );
   fprintf( stderr, "       %s [ -frcld ] -\n", invoked );

   return;
}


#ifdef  mc6809
#define LOBYTE(c)       (c)

char
   crchi = 0,
   crclo = 0;

makecrc( c )
char    c;
{
   union {
      unsigned un;
      char bytes[2];
   } i;          /* holds word from table */

   i.un = Mval[ (crchi ^= c) ];
   crchi = i.bytes[1] ^ crclo;        /* use low byte */
   crclo = i.bytes[0];           /* use high byte */
}

#else
#define LOBYTE(c)       ((c)&0xFF)

unsigned
   crchi = 0,
   crclo = 0;

makecrc(c)
register int c;
{
   register unsigned d;

   d = (crchi ^= c) & 0xFF;
   crchi =  Mval[ d ] & 0x00FF ^ crclo;
   crclo = (Mval[ d ] & 0xFF00) >> 8 & 0x00FF;
   return(d);
}

#endif

initcrc()
{
   crchi = crclo = 0;
}


getcrc()
{
   return ( (LOBYTE(crchi) << 8)  |  LOBYTE(crclo) );
}


gencrc( buf, nbytes )
char    *buf;
unsigned nbytes;
{
   /*
   **  Thru every character in the buffer.
   */

   while ( nbytes )
   {
      makecrc( *buf++ );
      --nbytes;
   }
}

char
*getdate( filename, fp )		/* returns a ptr to a 26-byte ASCII datebuf */

/*--- if fp is zero, then filename is used for the stat call ---*/

char *filename;	
FILE *fp;
{
	char *datebuf;
	int stat_flag;
	char junk[80];
	struct stat buf;

	errno = 0;
	if (fp == 0)
		stat_flag = stat( filename, &buf );
	else
		stat_flag = fstat( fp, &buf );

	if (stat_flag == 0)
	{
		datebuf = ctime( &buf.st_mtime );
		datebuf[24] = 0;
		return( &datebuf[4] );
	}

	return( NULL );
}

	void
do_lines( filename, fp, invoked, showdate )
char *filename, *invoked;
FILE *fp;
int showdate;
{
	char buf[128];

      /*********************************************************/
      /*                                                       */
      /*       fopen the file                                  */
      /*       for each line in input stream/file do           */
      /*                                                       */
      /*           get the crc for the line and print it       */
      /*                                                       */
      /*       fclose the file                                 */
      /*                                                       */
      /*********************************************************/


	if (fp == NULL)
		if ((fp = fopen( filename, "r" )) == NULL )
		{
			fprintf( stderr, 
					"%s: can't open %s for reading.\n",
					invoked, filename );
			return;
		}

	while ( fgets( buf, sizeof(buf), fp ) != NULL )
	{
		initcrc();
		gencrc( buf, strlen(buf) );
		if ( ! showdate )
			printf( "%d\n", getcrc() );
		else
			printf( "%d\t%s\n", getcrc(), getdate( filename, fp ) );
	}
	fclose( fp );
	return;
}


int
doit( filename, infile, rawmode, invoked, countmode, showdate )

   char           *filename;
   int             infile;
   int             rawmode;
   char           *invoked;
   int             countmode;
   int             showdate;
{
   int             nbytes;
   register unsigned filesize;         /*  counter for countmode */
   char            buf[ BLOCK ];
   FILE            *fp;
   struct filehdr *filhdrptr    = (struct filehdr *)buf;

      /*********************************************************/
      /*********************************************************/
      /*                                                       */
      /* if not in raw mode then                               */
      /*                                                       */
      /*    process the first file block and                   */
      /*                                                       */
      /*    if an a.out file then                              */
      /*                                                       */
      /*       ignore the time stamp field in the first file   */
      /*       block                                           */
      /*                                                       */
      /*    endif                                              */
      /*                                                       */
      /* endif                                                 */
      /*                                                       */
      /*********************************************************/

   initcrc();
   filesize = 0;

   if ( ! rawmode )
   {
      if ( (nbytes = read( infile, buf, BLOCK )) == FAILED )
      {
         fprintf( stderr, "%s: can't read %s.\n", invoked, filename );
         return;
      }
      else if ( nbytes == 0 )
      {
		 if ( ! countmode )
		 {
			if ( ! showdate )
            	printf( "$%04X for %s\n", getcrc(), filename );
			else
            	printf( "$%04X for %s (%s)\n", 
					getcrc(), filename, getdate( filename, 0 ) );
            return;
		 }
		 else
		 {
			if ( ! showdate )
            	printf( "$%04X\t%d\t%s\n", getcrc(), filesize, filename );
			else
            	printf( "$%04X\t%d\t%s\t%s\n", 
						getcrc(), filesize, getdate( filename, 0 ), filename );
            return;
		 }
      }
      else if ( filhdrptr->f_magic == MC68MAGIC )
      {
         filhdrptr->f_timdat = (long)0;
      }

      gencrc( buf, nbytes );
	  filesize += nbytes;
   }


      /*********************************************************/
      /*********************************************************/
      /*                                                       */
      /* process all remaining file blocks normally            */
      /*                                                       */
      /*********************************************************/

   while ( (nbytes = read( infile, buf, BLOCK )) > 0 )
   {
      gencrc( buf, nbytes );
	  filesize += nbytes;
   }

   if ( nbytes == FAILED )
   {
      fprintf( stderr, "%s: can't read %s.\n", invoked, filename );
   }
   else
   {
	  if ( ! countmode )
		 if ( ! showdate )
         	printf( "$%04X for %s\n", getcrc(), filename );
		 else
         	printf( "$%04X for %s (%s)\n", 
					getcrc(), filename, getdate( filename, 0 ) );
	  else
		 if ( ! showdate )
         	printf( "$%04X\t%d\t%s\n", getcrc(), filesize, filename );
		 else
         	printf( "$%04X\t%d\t%s\t%s\n", 
					getcrc(), filesize, getdate( filename, 0 ), filename );
		 
   }

   return;
}
