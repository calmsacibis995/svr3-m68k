#include <stdio.h>
#include <fcntl.h>
#include <math.h>

static char filename[1331][8];

main()

{
   double exp();

   int    fd;
   int    blks;
   int    bytes;
   char  *fn;

   static int filbytcnt[23]=
   {
         0, 1, 2, 117, 254, 255, 256, 257, 399, 436, 510, 511, 512, 513,
       677, 766, 767, 768, 769, 982, 1022, 1023, 1024
   };

   static int filblkcnt[37]=
   {
         0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 15, 101, 197, 264, 265,
       266, 267, 281, 342, 473, 520, 521, 522, 523, 562, 644, 701, 776,
       777, 778, 779, 783, 1017, 1022, 1023
   };

   char datablock[7923];
   char buf[80];
   char *gets();

   srand( 0 );

   gets(buf);

   initdatblk( datablock, 7923 );

   gets(buf);

   loadfilnam();

   gets(buf);

   while ( 1 )
   {
      fn = filename[ rand() % 1331 ];

      if ( (fd = creat( fn, O_WRONLY )) == -1 )
      {
         printf( "can't open %s\n", fn );
         continue;
      }

      blks = filblkcnt[ (int)( .69 * (exp(.0004 * (rand()%10000)) - 1)) % 37 ];
      bytes = filbytcnt[ rand() % 23 ];

      printf( "%d blks %d bytes\n", blks, bytes );

      while ( blks-- > 0 )
      {
         if ( write( fd, datablock[ rand() % 6899 ], 1024 ) != 1024 )
         {
            break;
         }
      }

      write( fd, datablock[ rand() % 6899 ], bytes );

      close( fd );

      sync();
      sync();
      sync();
   }

   exit( 0 );
}


loadfilnam()

{
   char i;
   char j;
   char k;

   for ( i = 0; i < 11; i++ )
   {
      for ( j = 0; j < 11; j++ )
      {
         for ( k = 0; k < 11; k++ )
         {
            sprintf( filename[ 121 * i + 11 * j + k ], "./%c/%c/%c",
                                           i + 'a', j + 'a', k + 'a' );
         }
      }
   }
}


initdatblk( datablock, n )

   char *datablock;
   int   n;
{

   while ( --n >= 0 )
   {
      datablock[ n ] = rand() % 128;
   }
}
