
/*	@(#)cp_gen.c	6.2		*/
/* MC68020 Disassembler - Generalized Coprocessor Support */

	static char SCCSID[] = "@(#)cp_gen.c	6.2		85/05/31";

#include	<stdio.h>
#include	"dis.h"
#include	"names.h"
#ifndef SDB
#	include	<setjmp.h>
#	include	<filehdr.h>
#	include	<ldfcn.h>
#	include	<sgs.h>		/* for definition of SGS */
#	include	<scnhdr.h>
#endif

#include	"disdefs.h"


void
cp_gen(idsp)
int	idsp;
{
    int		    cp_id, cp_cmd;
    unsigned long   val;
    char	    *conv_temp[100];


    cp_id = BITS11_9(curinstr);
    sprintf(mneu,"%x",cp_id);

    switch( cp_cmd = BITS8_6(curinstr) ) {

    case 0:			/* cpGEN */
	GET2BYTES;
	convert(cur2bytes,conv_temp,LEAD);
	sprintf(mneu,"%sGEN   CpCommand=%s,EA=%s", mneu,
		conv_temp, std_eff_add(NULL,NULL) );
	return;

    case 1:
	GET2BYTES;
	if( BITS5_3(curinstr)==1) {
	    sprintf(mneu,"%sDB%x", mneu, BITS5_0(cur2bytes) );
	    PAD(mneu);
	    sprintf(mneu,"%s%s,", mneu, reg_names[BITS3_0(curinstr)] );
	    GET2BYTES;
	    if (BIT15(cur2bytes)) strcat(mneu,"-");
	    convert(BIT15(cur2bytes) ? -(short) cur2bytes : 
		    cur2bytes,conv_temp,LEAD);
	    strcat(mneu,conv_temp);
	    compoff((BIT15(cur2bytes) ? ((long) (short) cur2bytes) :
		    (long) cur2bytes) -2, conv_temp);
		    /* the -2 above is needed because loc has been 
		       updated when getting the displacement, but 
		       for Motorola the pc is the address of the 
		       extension word */
	    return;
	};

	if( BITS5_3(curinstr)==7 ) {
	    sprintf(mneu,"%sTRAP%x", mneu, BITS5_0(cur2bytes) );
	    PAD(mneu);

	    switch( BITS2_0(curinstr) ) {
	    case 0:
	    case 1:
		sprintf(mneu,"ERROR in cpTRAP instruction");
	    case 4:
		return;
	    case 2:
		GET2BYTES;
		val = cur2bytes;
		break;
	    case 3:
		GET4BYTES;
		val = cur4bytes;
		break;
	    };

	    strcat(mneu, imm_string(val,UNSIGNED) );
	    return;

	};

	sprintf(mneu,"%sS%x", mneu, BITS5_0(cur2bytes) );
	PAD(mneu);
	strcat(mneu,std_eff_add(NULL,NULL) );
	return;

    case 2:
    case 3:				/* cpBcc.w or cpBcc.l */
	sprintf(mneu,"%sB%x%s", mneu, BITS5_0(cur2bytes) );
   
	if( cp_cmd == 2 )	/* word disp */
	{
		strcat(mneu,opt_size[WORD]);
		PAD(mneu);
		GET2BYTES ;
		convert(BIT15(cur2bytes) ? -((short) cur2bytes) :
			cur2bytes,conv_temp,LEAD);
		compoff((BIT15(cur2bytes) ? ((long) (short) cur2bytes) :
			(long) cur2bytes) -2, conv_temp);
				/* the -2 above is needed because loc has been 
				   updated when getting the displacement, but 
				   for Motorola the pc is the address of the 
				   extension word */
		if (BIT15(cur2bytes))
			strcat(mneu,"-");
	}
	else		/* long disp */
	{
		long	fourbytes;

		strcat(mneu,opt_size[LONG]);
		PAD(mneu);

		GET4BYTES;

		convert(  (HIOF32(cur4bytes) ? -cur4bytes : cur4bytes ),
				conv_temp,LEAD);
		compoff( (HIOF32(cur4bytes)?-cur4bytes:cur4bytes)-2, conv_temp);
				/* the -2 above is needed because loc has been 
				   updated when getting the displacement, but 
				   for Motorola the pc is the address of the 
				   extension word */
		if (HIOF32(cur4bytes))
			strcat(mneu,"-");
	}
	strcat(mneu,conv_temp);
	return;

    case 4:
	strcat(mneu,"SAVE  ");
	strcat(mneu,std_eff_add(NULL,NULL));
	return;
    
    case 5:
	strcat(mneu,"RESTORE  ");
	strcat(mneu,std_eff_add(NULL,NULL));
	return;

    };
};
