/**
  *	comm tracker routine
  *
  *	entry:	pointer to array of trace structures 
  *		pointer to trace header structure
  *
  *     exit:	pointer to current trace structure	
  *
  *	init:	thdptr->start = 0 
  *		thdptr->end   = number of trace elements 
  *
  */
#include "sys/track.h"
#ifndef NULL
#define NULL 0
#endif

struct track *tracker(tptr,thdptr)
struct track *tptr;
struct track_hd *thdptr;
{
	struct track *current, *trace;
	register x;

	x = spl6();
	trace = &tptr[thdptr->start];		/* pointer into buffer */
	
	/* check end and add end of trace indicators */
	if(++thdptr->start >= thdptr->end)
	{
		thdptr->start = 0;
	}

	/* zero trk structure */
	trace->trc_id = (int) NULL;
	trace->trc_f1 = (int) NULL;
	trace->trc_f2 = (int) NULL;
	trace->trc_f3 = (int) NULL;
	trace->trc_f4 = (int) NULL;
	trace->trc_f5 = (int) NULL;
	trace->trc_f6 = (int) NULL;
	trace->trc_f7 = (int) NULL;

	current = &tptr[thdptr->start];		/* pointer into buffer */
	current->trc_id = 0x3c2d2d2d;		/* <--- */

	splx(x);
	return(trace);
}

