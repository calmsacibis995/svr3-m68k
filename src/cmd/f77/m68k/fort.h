/*	@(#)fort.h	7.1		*/
/*	machine dependent file  */

label( n ){
#if TARGET==M68K
	printf( "L%%%d:\n", n );
#else
  	printf( ".L%d:\n", n );
#endif
	}

tlabel(){
	cerror("code generator asked to make label via tlabel\n");
	}
