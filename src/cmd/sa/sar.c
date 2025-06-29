/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sa:sar.c	1.25"
/*	sar.c 1.25 of 11/27/85	*/
/*
	sar.c - It generates a report either
		from an input data file or
		by invoking sadc to read system activity counters 
		at the specified intervals.
	usage: sar [-ubdycwaqvmpr] [-o file] t [n]    or
	       sar [-ubdycwaqvmpr][-s hh:mm][-e hh:mm][-i ss][-f file]
*/
#include <stdio.h>
#include <sys/param.h>
#include <ctype.h>
#include <time.h>
#include <sys/types.h>
#include <sys/sysinfo.h>
#include <sys/utsname.h>
#include <sys/fcntl.h>
#include <sys/flock.h>
#include "sa.h"

struct sa nx,ox,ax,bx;
struct tm *localtime(), *curt,args,arge;
struct utsname name;
int	sflg, eflg, iflg, oflg, fflg;
int	dflg=0;
float	Isyscall, Isysread, Isyswrite, Isysexec, Ireadch, Iwritech;
float	Osyscall, Osysread, Osyswrite, Osysexec, Oreadch, Owritech;
float 	Lsyscall, Lsysread, Lsyswrite, Lsysexec, Lreadch, Lwritech;
int	realtime;
int	passno;
int	t=0;
int	n=0;
int	rcnt=0;
#if	u3b2 || u3b5 || m68k
double 	magic = 4.294967296e9;
#endif
#ifndef u370
int	recsz, tblmap[SINFO];
#endif
#ifdef u370
int recsz,tblmap[10];	/*tblmap is a kludge for the 370 - not really used */
#endif
int	j,i;
int	tabflg;
char	options[15],fopt[15];
char	cc;
float	tdiff;
float	stime, etime, isec;
int	fin, fout, childid;
int	pipedes[2];
char	arg1[10], arg2[10];
int	strlen(), strdmp();
char	*strcpy(),*strncat(), *strncpy(), *strchr();
extern  int optind;
extern  char *optarg;


main (argc,argv)
char	**argv;
int	argc;
{
	char    flnm[50], ofile[50];
	char	ccc;
	long    temp;
	int	jj=0;
	float	convtm();
	long	time(), lseek();

/*      process options with arguments and pack options 
	without arguments  */
	while ((i= getopt(argc,argv,"uybdvcwaqmprADSo:s:e:i:f:")) != EOF)
		switch(ccc = i){
		case 'D':
			dflg++;
			break;
		case 'o':
			oflg++;
			sprintf(ofile,"%s",optarg);
			break;
		case 's':
			if (sscanf(optarg,"%d:%d:%d",
			&args.tm_hour, &args.tm_min, &args.tm_sec) < 1)
				pillarg();
			else {
				sflg++,
				stime = args.tm_hour*3600.0 +
					args.tm_min*60.0 +
					args.tm_sec;
			}
			break;
		case 'e':
			if(sscanf(optarg,"%d:%d:%d",
			&arge.tm_hour, &arge.tm_min, &arge.tm_sec) < 1)
				pillarg();
			else {
				eflg++;
				etime = arge.tm_hour*3600.0 +
					arge.tm_min*60.0 +
					arge.tm_sec;
			}
			break;
		case 'i':
			if(sscanf(optarg,"%f",&isec) < 1)
				pillarg();
			else{
			if (isec > 0.0)
				iflg++;
			}
			break;
		case 'f':
			fflg++;
			sprintf(flnm,"%s",optarg);
			break;
		case '?':
			fprintf(stderr,"usage: sar [-ubdycwaqvmprADS][-o file] t [n]\n");
			fprintf(stderr,"       sar [-ubdycwaqvmprADS][-s hh:mm][-e hh:mm][-i ss][-f file]\n");
			exit(2);
			break;
		default:
			strncat (options,&ccc,1);
			break;
		}

	/*   Are starting and ending times consistent?  */
	if ((sflg) && (eflg) && (etime <= stime))
		pmsgexit("etime <= stime");

	/*   Determine if t and n arguments are given,
	and whether to run in real time or from a file   */
	switch(argc - optind) {
	case 0:		/*   Get input data from file   */
		if(fflg == 0) {
			temp = time((long *) 0);
			curt = localtime(&temp);
			sprintf(flnm,"/usr/adm/sa/sa%.2d", curt->tm_mday);
		}
		if((fin = open(flnm, 0)) == -1) {
			fprintf(stderr, "sar:Can't open %s\n", flnm);
			exit(1);
		}
		break;
	case 1:		/*   Real time data; one cycle   */
		realtime++;
		t = atoi(argv[optind]);
		n = 2;
		break;
	case 2:		/*   Real time data; specified cycles   */
	default:
		realtime++;
		t = atoi(argv[optind]);
		n = 1 + atoi(argv[optind+1]);
		break;
	}

	/*	"u" is default option to display cpu utilization   */
	if(strlen(options) == 0)
		strcpy(options, "u");
	/*    'A' means all data options   */
	if(strchr(options, 'A') != NULL) {
		strcpy(options, "udqbwcayvmprS");
		dflg++;
	}
	else if ( (dflg)
	     && (strchr(options,'u') == NULL)
	     && (strchr(options,'c') == NULL) ) strcat(options,"u");

	if(realtime) {
	/*	Get input data from sadc via pipe   */
		if((t <= 0) || (n < 2))
			pmsgexit("args t & n <= 0");
		sprintf(arg1,"%d", t);
		sprintf(arg2,"%d", n);
		if (pipe(pipedes) == -1)
			perrexit();
		if ((childid = fork()) == 0){	/*  child:   */
			close(1);       /*  shift pipedes[write] to stdout  */
			dup(pipedes[1]);
			if (execlp ("/usr/lib/sa/sadc","/usr/lib/sa/sadc",arg1,arg2,0) == -1)
				perrexit();
		}		/*   parent:   */
		fin = pipedes[0];
		close(pipedes[1]);	/*   Close unused output   */
	}

	if(oflg) {
		if(strcmp(ofile, flnm) == 0)
			pmsgexit("ofile same as ffile");
		fout = creat(ofile, 00644);
	}

/*      read the header record and compute record size */
	if (read(fin, tblmap, sizeof tblmap) < 0)
		perrexit ();
#ifndef u370
	for (i=0;i<SINFO;i++)
		recsz += tblmap[i];
	recsz = sizeof (struct sa) - sizeof nx.devio +
		recsz * sizeof nx.devio[0];
#endif
#ifdef u370
		recsz = sizeof (struct sa);
#endif

	if(oflg)	write(fout, tblmap, sizeof tblmap);

	if(realtime) {
		/*   Make single pass, processing all options   */
		strcpy(fopt, options);
		passno++;
		prpass();
		kill(childid, 2);
		wait((int *) 0);
	}
	else {
		/*   Make multiple passes, one for each option   */
		while(strlen(strncpy(fopt,&options[jj++],1)) > 0) {
			lseek(fin, (long)(sizeof tblmap), 0);
			passno++;
			prpass();
		}
	}
	exit(0);
}

/*****************************************************/

/*	Read records from input, classify, and decide on printing   */
prpass(){
	int	lines=0;
	int recno=0;
	float tnext=0;
	float trec;
	if(sflg)	tnext = stime;
	while(read(fin, &nx, (unsigned)recsz) > 0) {
		curt = localtime(&nx.ts);
		trec =    curt->tm_hour * 3600.0
			+ curt->tm_min * 60.0
			+ curt->tm_sec;
		if((recno == 0) && (trec < stime))
			continue;
		if((eflg) && (trec > etime))
			break;
		if((oflg) && (passno == 1))
			write(fout, &nx, (unsigned)recsz);
		if(recno == 0) {
			if(passno == 1) {
				uname(&name);
				printf("\n%s %s %s %s %s    %.2d/%.2d/%.2d\n",
					name.sysname,
					name.nodename,
					name.release,
					name.version,
					name.machine,
					curt->tm_mon + 1,
					curt->tm_mday,
					curt->tm_year);
			}
			prthdg();
			recno = 1;
			if((iflg) && (tnext == 0))
				tnext = trec;
		}
		if ((nx.si.cpu[0] + nx.si.cpu[1] + nx.si.cpu[2] + nx.si.cpu[3]) < 0) {
		/*  This dummy record signifies system restart
		    New initial values of counters follow in next record  */
			prttim();
			printf("\tsysV68 restarts\n");
			recno = 1;
			continue;
		}
		if((iflg) && (trec < tnext))
			continue;
		if(recno++ > 1) {
#ifdef u370
			tdiff = nx.elpstm - ox.elpstm;
#else
			tdiff =   nx.si.cpu[0] - ox.si.cpu[0]
				+ nx.si.cpu[1] - ox.si.cpu[1]
				+ nx.si.cpu[2] - ox.si.cpu[2]
				+ nx.si.cpu[3] - ox.si.cpu[3];
			if (nx.apstate)
				tdiff = tdiff/2;
#endif
			if(tdiff <= 0) 
				continue;
			
			prtopt();	/*  Print a line of data  */
			lines++;
		}
		ox = nx;		/*  Age the data	*/
		if(isec > 0)
			while(tnext <= trec)
				tnext += isec;
	}
	rcnt=recno;
	if(lines > 1)
		prtavg();
	ax = bx;		/*  Zero out the accumulators   */
}

/************************************************************/

/*      print time label routine	*/
prttim()
{
	curt = localtime(&nx.ts);
	printf("%.2d:%.2d:%.2d",
		curt->tm_hour,
		curt->tm_min,
		curt->tm_sec);
	tabflg = 1;
}

/***********************************************************/

/*      test if 8-spaces to be added routine    */
tsttab()
{
	if (tabflg == 0) 
		printf("        ");
	else
		tabflg = 0;
}

/************************************************************/

/*      print report heading routine    */
prthdg()
{
	int	jj=0;
	char	ccc;

	printf("\n");
	prttim();
	while((ccc = fopt[jj++]) != NULL)
	switch(ccc){
	case 'u':
		tsttab();
#ifndef u370
		if (dflg) {
			printf(" %7s %7s %7s %7s %7s\n",
				"%usr",
				"%sys",
				"%sys",
				"%wio",
				"%idle");
			tsttab();
			printf(" %7s %7s %7s\n",
				"",
				"local",
				"remote");
			break;
		}
		printf(" %7s %7s %7s %7s\n",
			"%usr",
			"%sys",
			"%wio",
			"%idle");
#endif
#ifdef u370
			printf(" %7s %7s %7s %7s\n",
				"%usr",
				"%usup",
				"%tss",
				"%idle");
#endif
		break;
	case 'y':
		tsttab();
#ifndef u370
		printf(" %7s %7s %7s %7s %7s %7s\n",
			"rawch/s",
			"canch/s",
			"outch/s",
			"rcvin/s",
			"xmtin/s",
			"mdmin/s");
#endif
#ifdef u370
			printf(" %7s %7s %7s %7s\n", 
				"rch/s", 
				"s1rch/s", 
				"wch/s", 
				"s1wch/s"); 
#endif 
		break;
	case 'b':
		tsttab();
		printf(" %7s %7s %7s %7s %7s %7s %7s %7s\n",
			"bread/s",
			"lread/s",
			"%rcache",
			"bwrit/s",
			"lwrit/s",
			"%wcache",
			"pread/s",
			"pwrit/s");
		break;
	case 'd':
		tsttab();
#ifndef u370
		printf(" %7s %7s %7s %7s %7s %7s %7s\n",
			"device",
			"%busy",
			"avque",
			"r+w/s",
			"blks/s",
			"avwait",
			"avserv");
#endif
#ifdef u370
			printf(" %7s %7s %7s %7s %7s %7s\n", 
				"device", 
				"sread/s", 
				"pread/s", 
				"swrit/s", 
				"pwrit/s",
				"total/s");
#endif 
		break;
	case 'v':
		tsttab();
		printf(" %s %s %s %s %s\n",
			"proc-sz ov",
			"inod-sz ov",
			"file-sz ov",
			"lock-sz   ",
			"fhdr-sz   ");
		break;
	case 'c':
		tsttab();
		printf(" %7s %7s %7s %7s %7s %7s %7s\n",
			"scall/s",
			"sread/s",
			"swrit/s",
			"fork/s",
			"exec/s",
			"rchar/s",
			"wchar/s");
		break;
	case 'w':
		tsttab();
#ifndef u370
		printf(" %7s %7s %7s %7s %7s\n",
			"swpin/s",
			"bswin/s",
			"swpot/s",
			"bswot/s",
			"pswch/s");
#endif
#ifdef u370
			printf(" %7s %7s %7s\n",
				"sched/s",
				"tsend/s",
				"disp/s");
#endif
		break;
	case 'a':
		tsttab();
		printf(" %7s %7s %7s\n",
			"iget/s",
			"namei/s",
			"dirbk/s");
		break;
	case 'q':
		tsttab();
#ifndef u370
		printf(" %7s %7s %7s %7s\n",
			"runq-sz",
			"%runocc",
			"swpq-sz",
			"%swpocc");
#endif
#ifdef u370
			printf(" %7s %7s %7s %7s %7s\n",
				"run-sz",
				"wtsm-sz",
				"semtm/p",
				"wtio-sz",
				"tmio/p");
#endif 
		break;
	case 'm':
		tsttab();
		printf(" %7s %7s\n",
			"msg/s",
			"sema/s");
		break;
#if	vax || u3b | u3b5 || u3b2 || m68k
	case 'r':
		tsttab();
		printf(" %7s %7s\n",
			"freemem",
			"freeswp");
		break;
#endif
#if	u3b || u3b5 || u3b2 || m68k
	case 'S':
		tsttab();
		printf("%11s %9s %9s %9s %9s\n",
			"serv/lo-hi",
			"request",
			"request",
			"server",
			"server");
		tsttab();
		printf("%6s %d %1s %d %9s %9s %9s %9s\n",
			"",
			nx.minserve,
			"-",
			nx.maxserve,
			"%busy",
			"avg lgth",
			"%avail",
			"avg avail");
		break;
#endif
#if	vax || u3b || u3b5 || u3b2 || m68k
	case 'p':
		tsttab();
		printf(" %7s %7s %7s %7s\n",
			"vflt/s",
			"pflt/s",
			"pgfil/s",
			"rclm/s");
		break;
#endif
	}
	if(jj > 2)	printf("\n");
}

/**********************************************************/

/*      print options routine   */
prtopt()
{
	register int ii,kk,mm;
	int jj=0;
	char	ccc;

	if(strcmp(fopt, "d") == 0)   printf("\n");
	prttim();
#ifndef u370
	for(ii=0;ii<4;ii++)
		ax.si.cpu[ii] += nx.si.cpu[ii] - ox.si.cpu[ii];
	if (dflg) ax.di.serve += nx.di.serve   - ox.di.serve;
#endif
#ifdef u370
	ax.elpstm += nx.elpstm - ox.elpstm;
	ax.tmelps += nx.ccv - ox.ccv;	/*get elapsed time from tss table - u370*/
	ax.nap = nx.nap;
#endif 

	while((ccc = fopt[jj++]) != NULL)
	switch(ccc){
	case 'u':
		tsttab();
#ifndef u370
	if (dflg) {
		if (nx.apstate) {
			printf(" %7.0f %7.0f %7.0f %7.0f %7.0f\n",
			(float)(nx.si.cpu[1] - ox.si.cpu[1])/(2*tdiff) * 100.0,

			(float)(nx.si.cpu[2] - ox.si.cpu[2] -
			       (nx.di.serve - ox.di.serve) )/(2*tdiff) * 100.0,
			(float)(nx.di.serve - ox.di.serve)/(2*tdiff) * 100.0,

			(float)(nx.si.cpu[3] - ox.si.cpu[3])/(2*tdiff) * 100.0,
			(float)(nx.si.cpu[0] - ox.si.cpu[0])/(2*tdiff) * 100.0);
		} else {
			printf(" %7.0f %7.0f %7.0f %7.0f %7.0f\n",
			(float)(nx.si.cpu[1] - ox.si.cpu[1])/tdiff * 100.0,

			(float)(nx.si.cpu[2] - ox.si.cpu[2] -
			       (nx.di.serve - ox.di.serve) )/tdiff * 100.0,
			(float)(nx.di.serve - ox.di.serve)/tdiff * 100.0,

			(float)(nx.si.cpu[3] - ox.si.cpu[3])/tdiff * 100.0,
			(float)(nx.si.cpu[0] - ox.si.cpu[0])/tdiff * 100.0);
		}
		break;
	}
		if (nx.apstate)
			printf(" %7.0f %7.0f %7.0f %7.0f\n",
			(float)(nx.si.cpu[1] - ox.si.cpu[1])/(2*tdiff) * 100.0,
			(float)(nx.si.cpu[2] - ox.si.cpu[2])/(2*tdiff) * 100.0,
			(float)(nx.si.cpu[3] - ox.si.cpu[3])/(2*tdiff) * 100.0,
			(float)(nx.si.cpu[0] - ox.si.cpu[0])/(2*tdiff) * 100.0);
		else
		printf(" %7.0f %7.0f %7.0f %7.0f\n",
		(float)(nx.si.cpu[1] - ox.si.cpu[1])/tdiff * 100.0,
		(float)(nx.si.cpu[2] - ox.si.cpu[2])/tdiff * 100.0,
		(float)(nx.si.cpu[3] - ox.si.cpu[3])/tdiff * 100.0,
		(float)(nx.si.cpu[0] - ox.si.cpu[0])/tdiff * 100.0);
#endif
#ifdef u370
		printf(" %7.0f %7.0f %7.0f %7.0f\n",
		(float)(nx.usrtm - ox.usrtm)/ 
		((nx.ccv - ox.ccv)*(double)nx.nap)*100.0, 
		(float)(nx.usuptm - ox.usuptm)/ 
		((nx.ccv - ox.ccv)*(double)nx.nap)*100.0, 
		(float)((nx.ccv - ox.ccv)*(double)nx.nap - 
			(nx.idletm - ox.idletm) - 
			(nx.vmtm - ox.vmtm))/ 
			((nx.ccv - ox.ccv)*(double)nx.nap)*100.0, 
		(float)(nx.idletm - ox.idletm)/ 
			((nx.ccv - ox.ccv)*(double)nx.nap)*100.0); 

		ax.idletm += nx.idletm - ox.idletm;
		ax.usrtm += nx.usrtm - ox.usrtm;
		ax.usuptm  += nx.usuptm - ox.usuptm;
		ax.vmtm += nx.vmtm - ox.vmtm;
#endif

		break;
	case 'y':
		tsttab();
#ifndef u370
		printf(" %7.0f %7.0f %7.0f %7.0f %7.0f %7.0f\n",
		(float)(nx.si.rawch - ox.si.rawch)/tdiff * HZ,
		(float)(nx.si.canch - ox.si.canch)/tdiff * HZ,
		(float)(nx.si.outch - ox.si.outch)/tdiff * HZ,
		(float)(nx.si.rcvint - ox.si.rcvint)/tdiff * HZ,
		(float)(nx.si.xmtint - ox.si.xmtint)/tdiff * HZ,
		(float)(nx.si.mdmint - ox.si.mdmint)/tdiff * HZ);

		ax.si.rawch += nx.si.rawch - ox.si.rawch;
		ax.si.canch += nx.si.canch - ox.si.canch;
		ax.si.outch += nx.si.outch - ox.si.outch;
		ax.si.rcvint += nx.si.rcvint - ox.si.rcvint;
		ax.si.xmtint += nx.si.xmtint - ox.si.xmtint;
		ax.si.mdmint += nx.si.mdmint - ox.si.mdmint;
#endif
#ifdef u370
		printf(" %7.0f %7.0f %7.0f %7.0f\n", 
		(float)(nx.si.termin - ox.si.termin)/tdiff * HZ, 
		(float)(nx.si.s1in - ox.si.s1in)/tdiff * HZ, 
		(float)(nx.si.termout - ox.si.termout)/tdiff * HZ, 
		(float)(nx.si.s1out - ox.si.s1out)/tdiff * HZ); 
  
		ax.si.termin += nx.si.termin - ox.si.termin; 
		ax.si.s1in += nx.si.s1in - ox.si.s1in; 
		ax.si.termout += nx.si.termout - ox.si.termout; 
		ax.si.s1out += nx.si.s1out - ox.si.s1out; 
#endif
		break;
	case 'b':
		tsttab();
		printf(" %7.0f %7.0f %7.0f %7.0f %7.0f %7.0f %7.0f %7.0f\n",
			(float)(nx.si.bread - ox.si.bread)/tdiff * HZ,
			(float)(nx.si.lread - ox.si.lread)/tdiff * HZ,
			(((float)(nx.si.lread - ox.si.lread) -
			  (float)(nx.si.bread - ox.si.bread))/
			  (float)(nx.si.lread - ox.si.lread) * 100.0),
			(float)(nx.si.bwrite - ox.si.bwrite)/tdiff * HZ,
			(float)(nx.si.lwrite - ox.si.lwrite)/tdiff * HZ,
			(((float)(nx.si.lwrite - ox.si.lwrite) -
			  (float)(nx.si.bwrite - ox.si.bwrite))/
			  (float)(nx.si.lwrite - ox.si.lwrite) * 100.0),
			(float)(nx.si.phread - ox.si.phread)/tdiff * HZ,
			(float)(nx.si.phwrite - ox.si.phwrite)/tdiff * HZ);

		ax.si.bread += nx.si.bread - ox.si.bread;
		ax.si.bwrite += nx.si.bwrite - ox.si.bwrite;
		ax.si.lread += nx.si.lread - ox.si.lread;
		ax.si.lwrite += nx.si.lwrite - ox.si.lwrite;
		ax.si.phread += nx.si.phread - ox.si.phread;
		ax.si.phwrite += nx.si.phwrite - ox.si.phwrite;
		break;
	case 'd':
#ifndef u370
		ii = 0;
		for (j=0;j<SINFO;j++){
			for (kk=0;kk<tblmap[j];kk++){
				if (((nx.devio[ii][0] - ox.devio[ii][0]) > 0) 
				&& ((nx.devio[ii][2] - ox.devio[ii][2]) > 0)){
				tsttab();
#ifdef u3b
		if (j == DSKINFO)  
#else
#ifdef u3b5
		if (j == DFDFC) 
#else
#ifdef u3b2
		if (j == ID || j == IF)
#else
#ifndef m68k
		if (j == GDS) 
#endif /* m68k */
#endif /* u3b2 */
#endif /* u3b5 */
#endif /* u3b */
#ifndef m68k
			printf(" %4s%-3d", devnm[j], kk);
		 else
#endif /* m68k */
			printf(" %5s%-2d", devnm[j], kk);
		printf(" %7.0f %7.1f %7.0f %7.0f %7.1f %7.1f\n",
			(float)(nx.devio[ii][2] - ox.devio[ii][2])/tdiff *100.0,
			(float)(nx.devio[ii][3] - ox.devio[ii][3])/
				(float)(nx.devio[ii][2] - ox.devio[ii][2]),
			(float)(nx.devio[ii][0] - ox.devio[ii][0])/tdiff* HZ,
			(float)(nx.devio[ii][1] - ox.devio[ii][1])/tdiff *HZ,
			((float)(nx.devio[ii][3] - ox.devio[ii][3]) -
				(float)(nx.devio[ii][2] - ox.devio[ii][2]))/
				(float)(nx.devio[ii][0] - ox.devio[ii][0])/
				HZ * 1000.,
			(float)(nx.devio[ii][2] - ox.devio[ii][2]) /
				(float)(nx.devio[ii][0] - ox.devio[ii][0] )/HZ * 1000.);
				for(mm=0;mm<4;mm++)
				ax.devio[ii][mm] +=
				nx.devio[ii][mm] - ox.devio[ii][mm];
				}
				ii++;
			}
		}
#endif
#ifdef u370
		for (ii = 0; ii < NDEV; ii++){ 
			if ((nx.io[ii].io_total - ox.io[ii].io_total)/HZ == 0)
				continue;
			tsttab(); 
			if (ii < NDRUM) 
				printf(" drm%-4d", ii+1); 
			else 
				printf(" dsk%-4d", ii - NDRUM + 1); 
			printf(" %7.1f %7.1f %7.1f %7.1f %7.1f\n", 
			(float)(nx.io[ii].io_sread - ox.io[ii].io_sread)/tdiff * HZ, 
			(float)(nx.io[ii].io_pread - ox.io[ii].io_pread)/tdiff * HZ, 
			(float)(nx.io[ii].io_swrite - ox.io[ii].io_swrite)/tdiff * HZ, 
			(float)(nx.io[ii].io_pwrite - ox.io[ii].io_pwrite)/tdiff * HZ, 
			(float)(nx.io[ii].io_total - ox.io[ii].io_total)/tdiff * HZ); 
		ax.io[ii].io_sread += nx.io[ii].io_sread - ox.io[ii].io_sread; 
		ax.io[ii].io_pread += nx.io[ii].io_pread - ox.io[ii].io_pread; 
		ax.io[ii].io_swrite += nx.io[ii].io_swrite - ox.io[ii].io_swrite; 
		ax.io[ii].io_pwrite += nx.io[ii].io_pwrite - ox.io[ii].io_pwrite; 
		ax.io[ii].io_total += nx.io[ii].io_total - ox.io[ii].io_total;
	} 
#endif 
		break;
	case 'v':
		tsttab();
		printf(" %3d/%3d%3ld %3d/%3d%3ld %3d/%3d%3ld %3d/%3d    %3d/%3d\n",
			nx.szproc, nx.mszproc, (nx.procovf - ox.procovf),
			nx.szinode, nx.mszinode, (nx.inodeovf - ox.inodeovf),
			nx.szfile, nx.mszfile, (nx.fileovf - ox.fileovf),
			nx.szlckr, nx.mszlckr,
			nx.szlckf, nx.mszlckf);
		break;
	case 'c':
		tsttab();
	if (dflg) {
		Isyscall = (float)(nx.di.isyscall - ox.di.isyscall )/tdiff * HZ;
		Osyscall = (float)(nx.di.osyscall - ox.di.osyscall )/tdiff * HZ;
		Lsyscall = (float)(nx.si.syscall - ox.si.syscall )/tdiff * HZ -
				(Isyscall + Osyscall);

		Isysread = (float)(nx.di.isysread - ox.di.isysread )/tdiff * HZ;
		Osysread = (float)(nx.di.osysread - ox.di.osysread )/tdiff * HZ;
		Lsysread = (float)(nx.si.sysread - ox.si.sysread )/tdiff * HZ -
				(Isysread + Osysread);

		Isyswrite = (float)(nx.di.isyswrite - ox.di.isyswrite )/tdiff * HZ;
		Osyswrite = (float)(nx.di.osyswrite - ox.di.osyswrite )/tdiff * HZ;
		Lsyswrite = (float)(nx.si.syswrite - ox.si.syswrite )/tdiff * HZ -
				(Isyswrite + Osyswrite);

		Isysexec = (float)(nx.di.isysexec - ox.di.isysexec )/tdiff * HZ;
		Osysexec = (float)(nx.di.osysexec - ox.di.osysexec )/tdiff * HZ;
		Lsysexec = (float)(nx.si.sysexec - ox.si.sysexec )/tdiff * HZ -
				  (Isysexec + Osysexec);

		Ireadch = (float)(nx.di.ireadch - ox.di.ireadch )/tdiff * HZ;
		Oreadch = (float)(nx.di.oreadch - ox.di.oreadch )/tdiff * HZ;
		Lreadch = (float)(nx.si.readch - ox.si.readch )/tdiff * HZ -
				(Ireadch + Oreadch);

		Iwritech = (float)(nx.di.iwritech - ox.di.iwritech )/tdiff * HZ;
		Owritech = (float)(nx.di.owritech - ox.di.owritech )/tdiff * HZ;
		Lwritech = (float)(nx.si.writech - ox.si.writech )/tdiff * HZ -
				(Iwritech + Owritech);

		printf("\n%-8s %7.0f %7.0f %7.0f %7s %7.2f %7.0f %7.0f\n",
			"   in", Isyscall,Isysread,Isyswrite,"",Isysexec,
			Ireadch,Iwritech);
		printf("%-8s %7.0f %7.0f %7.0f %7s %7.2f %7.0f %7.0f\n",
			"   out",Osyscall,Osysread,Osyswrite,"",Osysexec,
			Oreadch,Owritech);
		printf("%-8s %7.0f %7.0f %7.0f %7.2f %7.2f %7.0f %7.0f\n",
			"   local", Lsyscall, Lsysread, Lsyswrite,
			(float)(nx.si.sysfork - ox.si.sysfork)/tdiff * HZ,
			Lsysexec, Lreadch, Lwritech);
 
		ax.di.isyscall += nx.di.isyscall - ox.di.isyscall;
		ax.di.osyscall += nx.di.osyscall - ox.di.osyscall;
		ax.si.syscall += nx.si.syscall - ox.si.syscall;
 
		ax.di.isysread += nx.di.isysread - ox.di.isysread;
		ax.di.osysread += nx.di.osysread - ox.di.osysread;
		ax.si.sysread += nx.si.sysread - ox.si.sysread;
 
		ax.di.isyswrite += nx.di.isyswrite - ox.di.isyswrite;
		ax.di.osyswrite += nx.di.osyswrite - ox.di.osyswrite;
		ax.si.syswrite += nx.si.syswrite - ox.si.syswrite;
 
		ax.di.isysexec += nx.di.isysexec - ox.di.isysexec;
		ax.di.osysexec += nx.di.osysexec - ox.di.osysexec;
		ax.si.sysexec += nx.si.sysexec - ox.si.sysexec;
 
		ax.di.ireadch += nx.di.ireadch - ox.di.ireadch;
		ax.di.oreadch += nx.di.oreadch - ox.di.oreadch;
		ax.si.readch += nx.si.readch - ox.si.readch;
	 
		ax.di.iwritech += nx.di.iwritech - ox.di.iwritech;
		ax.di.owritech += nx.di.owritech - ox.di.owritech;
		ax.si.writech += nx.si.writech - ox.si.writech;

		ax.si.sysfork += nx.si.sysfork - ox.si.sysfork;

		break;
	}
		printf(" %7.0f %7.0f %7.0f %7.2f %7.2f %7.0f %7.0f\n",
			(float)(nx.si.syscall - ox.si.syscall)/tdiff *HZ,
			(float)(nx.si.sysread - ox.si.sysread)/tdiff *HZ,
			(float)(nx.si.syswrite - ox.si.syswrite)/tdiff *HZ,
			(float)(nx.si.sysfork - ox.si.sysfork)/tdiff *HZ,
			(float)(nx.si.sysexec - ox.si.sysexec)/tdiff *HZ,
			(float)(nx.si.readch - ox.si.readch)/tdiff * HZ,
			(float)(nx.si.writech - ox.si.writech)/tdiff * HZ);

		ax.si.syscall += nx.si.syscall - ox.si.syscall;
		ax.si.sysread += nx.si.sysread - ox.si.sysread;
		ax.si.syswrite += nx.si.syswrite - ox.si.syswrite;
		ax.si.sysfork += nx.si.sysfork - ox.si.sysfork;
		ax.si.sysexec += nx.si.sysexec - ox.si.sysexec;
		ax.si.readch += nx.si.readch - ox.si.readch;
		ax.si.writech += nx.si.writech - ox.si.writech;
		break;
	case 'w':
		tsttab();
#ifndef u370
		printf(" %7.2f %7.1f %7.2f %7.1f %7.0f\n",
			(float)(nx.si.swapin - ox.si.swapin)/tdiff * HZ,
			(float)(nx.si.bswapin -ox.si.bswapin)/tdiff * HZ,
			(float)(nx.si.swapout - ox.si.swapout)/tdiff * HZ,
			(float)(nx.si.bswapout - ox.si.bswapout)/tdiff * HZ,
			(float)(nx.si.pswitch - ox.si.pswitch)/tdiff * HZ);

		ax.si.swapin += nx.si.swapin - ox.si.swapin;
		ax.si.swapout += nx.si.swapout - ox.si.swapout;
		ax.si.bswapin += nx.si.bswapin - ox.si.bswapin;
		ax.si.bswapout += nx.si.bswapout - ox.si.bswapout;
		ax.si.pswitch += nx.si.pswitch - ox.si.pswitch;
#endif
#ifdef u370
		printf(" %7.0f %7.0f %7.0f\n",
			(float)(nx.intsched - ox.intsched)/(nx.ccv - ox.ccv)*1e6,
			(float)(nx.tsend - ox.tsend)/(nx.ccv - ox.ccv)*1e6,
			(float)(nx.mkdisp - ox.mkdisp)/(nx.ccv - ox.ccv)*1e6);
		ax.intsched += nx.intsched - ox.intsched;
		ax.tsend += nx.tsend - ox.tsend;
		ax.mkdisp += nx.mkdisp - ox.mkdisp;
#endif
		break;
	case 'a':
		tsttab();
		printf(" %7.0f %7.0f %7.0f\n",
			(float)(nx.si.iget - ox.si.iget)/tdiff * HZ,
			(float)(nx.si.namei - ox.si.namei)/tdiff * HZ,
			(float)(nx.si.dirblk - ox.si.dirblk)/tdiff * HZ);

		ax.si.iget += nx.si.iget - ox.si.iget;
		ax.si.namei += nx.si.namei - ox.si.namei;
		ax.si.dirblk += nx.si.dirblk - ox.si.dirblk;
		break;
	case 'S':
		tsttab();
		printf("%11.0f %9.1f %9.0f %9.1f %9.0f\n",
			(float)(nx.di.nservers - ox.di.nservers)/tdiff * HZ,
			(((nx.di.rcv_occ - ox.di.rcv_occ)/tdiff * HZ * 100.0) > 100) ? 100.0 :
				(float)(nx.di.rcv_occ - ox.di.rcv_occ)/tdiff * HZ * 100.0,
			(nx.di.rcv_occ - ox.di.rcv_occ <= 0) ? 0.0 :
				((nx.di.rcv_que - ox.di.rcv_que <= 0) ? 0.0 :
				(float)(nx.di.rcv_que - ox.di.rcv_que)/
					(float)(nx.di.rcv_occ - ox.di.rcv_occ)),
			(((nx.di.srv_occ - ox.di.srv_occ)/tdiff * HZ * 100.0) > 100) ? 100.0 :
				(float)(nx.di.srv_occ - ox.di.srv_occ)/tdiff * HZ * 100.0,
			(nx.di.srv_occ - ox.di.srv_occ == 0) ? 0.0 :
				(float)(nx.di.srv_que - ox.di.srv_que)/
					(float)(nx.di.srv_occ - ox.di.srv_occ));
 


		ax.di.nservers += nx.di.nservers - ox.di.nservers;
		ax.di.rcv_occ += nx.di.rcv_occ - ox.di.rcv_occ;
		ax.di.rcv_que += nx.di.rcv_que - ox.di.rcv_que;
		ax.di.srv_occ += nx.di.srv_occ - ox.di.srv_occ;
		ax.di.srv_que += nx.di.srv_que - ox.di.srv_que;
		break;
	
	case 'q':
#ifndef u370
		tsttab();
		if ((nx.si.runocc - ox.si.runocc) == 0)
			printf(" %7s %7s", "  ", "  ");
		else {
			printf(" %7.1f %7.0f",
			(float)(nx.si.runque -ox.si.runque)/
				(float)(nx.si.runocc - ox.si.runocc),
			(float)(nx.si.runocc -ox.si.runocc)/tdiff *HZ *100.0);
			ax.si.runque += nx.si.runque - ox.si.runque;
			ax.si.runocc += nx.si.runocc - ox.si.runocc;
		}
		if ((nx.si.swpocc - ox.si.swpocc) == 0)
			printf(" %7s %7s\n","  ","  ");
		else {
			printf(" %7.1f %7.0f\n",
			(float)(nx.si.swpque -ox.si.swpque)/
				(float)(nx.si.swpocc - ox.si.swpocc),
			(float)(nx.si.swpocc -ox.si.swpocc)/tdiff *HZ *100.0);
			ax.si.swpque += nx.si.swpque - ox.si.swpque;
			ax.si.swpocc += nx.si.swpocc - ox.si.swpocc;

		}
#endif
#ifdef u370
		printf(" %7.0f %7.0f %7.0f %7.0f %7.0f\n",
		(float)nx.pi.run,
		(float)nx.pi.wtsem,
		(float)(nx.pi.wtsemtm/((nx.pi.wtsem)*HZ)),
		(float)nx.pi.wtio,
		(float)(nx.pi.wtiotm/((nx.pi.wtio)*HZ)));

		ax.pi.run += nx.pi.run;
		ax.pi.wtsem += nx.pi.wtsem;
		ax.pi.wtsemtm += nx.pi.wtsemtm;
		ax.pi.wtio += nx.pi.wtio;
		ax.pi.wtiotm += nx.pi.wtiotm;
		ax.lines += 1;
#endif
		break;
	case 'm':
		tsttab();
		printf(" %7.2f %7.2f\n",
			(float)(nx.si.msg - ox.si.msg)/tdiff * HZ,
			(float)(nx.si.sema - ox.si.sema)/tdiff * HZ);

		ax.si.msg += nx.si.msg - ox.si.msg;
		ax.si.sema += nx.si.sema - ox.si.sema;
		break;
#if	vax || u3b 
	case 'r':
		tsttab();
		printf(" %7.0f %7.0f\n",
			(float)nx.mi.freemem,
			(float)nx.mi.freeswap);
		ax.mi.freemem += nx.mi.freemem;
		ax.mi.freeswap += nx.mi.freeswap;
		break;
#endif

/* new freemem for 3b2 */

#if	u3b2 || u3b5 || m68k

 	{
	unsigned long k0, k1, x;
	case 'r':
		tsttab();
		k1 = (nx.mi.freemem[1] - ox.mi.freemem[1]);
		if (nx.mi.freemem[0] >= ox.mi.freemem[0]) {
			k0 = nx.mi.freemem[0] - ox.mi.freemem[0]; 
		}
		else
		{ 	k0 = 1 + (~(ox.mi.freemem[0] - nx.mi.freemem[0]));
			k1--; 
		}
		printf(" %7.0f %7.0f\n",
			(double)(k0 + magic * k1)/tdiff,
			(float)nx.mi.freeswap);

			x = ax.mi.freemem[0];
			ax.mi.freemem[0] += k0;
			ax.mi.freemem[1] += k1;
			if ( x > ax.mi.freemem[0])
				ax.mi.freemem[1]++;
			ax.mi.freeswap += nx.mi.freeswap;
		break;
 	}
#endif


	 
#if	vax || u3b || u3b5 || u3b2 || m68k
	case 'p':
		tsttab();
		printf(" %7.2f %7.2f %7.2f %7.2f\n",
			(float)(nx.mi.vfault - ox.mi.vfault) / tdiff * HZ,
			(float)(nx.mi.pfault - ox.mi.pfault) / tdiff * HZ,
			(float)(nx.mi.file - ox.mi.file) / tdiff * HZ,
			(float)(nx.mi.freedpgs - ox.mi.freedpgs) / tdiff * HZ);
		ax.mi.vfault += nx.mi.vfault - ox.mi.vfault;
		ax.mi.pfault += nx.mi.pfault - ox.mi.pfault;
		ax.mi.file += nx.mi.file - ox.mi.file;
		ax.mi.freedpgs += nx.mi.freedpgs - ox.mi.freedpgs;
#endif
	}
	if(jj > 2)	printf("\n");
}

/**********************************************************/

/*      print average routine  */
prtavg()
{
	register int ii,kk;
	int	jj=0;
	char	ccc;
#ifdef m68k
	int mm;
#endif
#ifndef u370
	tdiff = ax.si.cpu[0] + ax.si.cpu[1] + ax.si.cpu[2] + ax.si.cpu[3];
	if (nx.apstate)
		tdiff = tdiff/2;
#endif
#ifdef u370
	tdiff = ax.elpstm;
#endif
	if (tdiff <= 0.0)
		return;
	printf("\n");

	while((ccc = fopt[jj++]) != NULL)
	switch(ccc){
	case 'u':
#ifndef u370
		if (dflg) {
			if(nx.apstate)
				printf("Average  %7.0f %7.0f %7.0f %7.0f %7.0f\n",
				(float)ax.si.cpu[1]/(2*tdiff) * 100.0,
				(float)(ax.si.cpu[2] - ax.di.serve)/(2*tdiff) * 100.0,
				(float)(ax.di.serve)/(2*tdiff) * 100.0,
				(float)ax.si.cpu[3]/(2*tdiff) * 100.0,
				(float)ax.si.cpu[0]/(2*tdiff) * 100.0);
			else
				printf("Average  %7.0f %7.0f %7.0f %7.0f %7.0f\n",
				(float)ax.si.cpu[1]/tdiff * 100.0,
				(float)(ax.si.cpu[2] - ax.di.serve)/tdiff * 100.0,
				(float)(ax.di.serve)/tdiff * 100.0,
				(float)ax.si.cpu[3]/tdiff * 100.0,
				(float)ax.si.cpu[0]/tdiff * 100.0);
		break;
		}
		if(nx.apstate)
			printf("Average  %7.0f %7.0f %7.0f %7.0f\n",
			(float)ax.si.cpu[1]/(2*tdiff) * 100.0,
			(float)ax.si.cpu[2]/(2*tdiff) * 100.0,
			(float)ax.si.cpu[3]/(2*tdiff) * 100.0,
			(float)ax.si.cpu[0]/(2*tdiff) * 100.0);
		else
			printf("Average  %7.0f %7.0f %7.0f %7.0f\n",
			(float)ax.si.cpu[1]/tdiff * 100.0,
			(float)ax.si.cpu[2]/tdiff * 100.0,
			(float)ax.si.cpu[3]/tdiff * 100.0,
			(float)ax.si.cpu[0]/tdiff * 100.0);
#endif
#ifdef u370
		printf("Average  %7.0f %7.0f %7.0f %7.0f\n",
			(float) (ax.usrtm)/(ax.tmelps * (double)nx.nap) * 100.0,
			(float) (ax.usuptm)/(ax.tmelps * (double)ax.nap) * 100.0,
			(float) ((ax.tmelps)  * (double) ax.nap - ax.idletm - ax.vmtm )/
				((ax.tmelps)  * (double) ax.nap) * 100.0,
			(float) (ax.idletm)/(ax.tmelps * (double)nx.nap) * 100.0);
#endif
		break;
	case 'y':
#ifndef u370
		printf("Average  %7.0f %7.0f %7.0f %7.0f %7.0f %7.0f\n",
			(float)ax.si.rawch/tdiff *HZ,
			(float)ax.si.canch/tdiff *HZ,
			(float)ax.si.outch/tdiff *HZ,
			(float)ax.si.rcvint/tdiff *HZ,
			(float)ax.si.xmtint/tdiff *HZ,
			(float)ax.si.mdmint/tdiff *HZ);
#endif
#ifdef u370
		printf("Average  %7.0f %7.0f %7.0f %7.0f\n\n",
			(float)ax.si.termin/tdiff * HZ,
			(float)ax.si.s1in/tdiff * HZ,
			(float)ax.si.termout/tdiff * HZ,
			(float)ax.si.s1out/tdiff * HZ);
#endif
		break;
	case 'b':
		printf("Average  %7.0f %7.0f %7.0f %7.0f %7.0f %7.0f %7.0f %7.0f\n",
			(float)ax.si.bread/tdiff *HZ,
			(float)ax.si.lread/tdiff *HZ,
			(float)(ax.si.lread - ax.si.bread)/
				(float)(ax.si.lread) * 100.0,
			(float)ax.si.bwrite/tdiff *HZ,
			(float)ax.si.lwrite/tdiff *HZ,
			(float)(ax.si.lwrite - ax.si.bwrite)/
				(float)(ax.si.lwrite) * 100.0,
			(float)ax.si.phread/tdiff *HZ,
			(float)ax.si.phwrite/tdiff *HZ);
		break;
	case 'd':
		ii = 0;
		printf("Average ");
		tabflg = 1;
#ifndef u370
		for (j=0;j<SINFO;j++){
		    for (kk=0;kk<tblmap[j];kk++){
			if ((ax.devio[ii][0] > 0)  &&
			   (ax.devio[ii][2] > 0)){
				tsttab();
#if u3b
				if (j == DSKINFO)
#else
#if u3b5
				if (j == DFDFC)
#else
#if u3b2
				if (j == ID || j == IF)
#else
#ifndef m68k
				if (j == GDS)
#endif /* m68k */
#endif /* u3b2 */
#endif /* u3b5 */
#endif /* u3b */
#ifndef m68k
					printf(" %4s%-3d",devnm[j],kk);
				else
#endif /* m68k */
					printf(" %5s%-2d",devnm[j],kk);
				printf(" %7.0f %7.1f %7.0f %7.0f %7.1f %7.1f\n",
					(float)ax.devio[ii][2]/tdiff * 100.0,
					(float)ax.devio[ii][3]/(float)ax.devio[ii][2],
					(float)ax.devio[ii][0]/tdiff *HZ,
					(float)ax.devio[ii][1]/tdiff *HZ,
					(float)(ax.devio[ii][3] - ax.devio[ii][2]) /
						(float)ax.devio[ii][0] /HZ * 1000.,
					(float)ax.devio[ii][2] /(float)ax.devio[ii][0] /HZ *1000.);
			}
			ii++;
		    }
		}
#endif
#ifdef u370
		for (i = 0; i < NDEV; i++){ 
			if (ax.io[i].io_total/HZ == 0)
				continue;
		tsttab();
			if (i < NDRUM){ 
				printf(" drm%-4d", i + 1);
			} 
			else { 
				printf(" dsk%-4d", i - NDRUM + 1);
			} 
			printf(" %7.1f %7.1f %7.1f %7.1f %7.1f\n", 
			(float)ax.io[i].io_sread/tdiff * HZ, 
			(float)ax.io[i].io_pread/tdiff * HZ, 
			(float)ax.io[i].io_swrite/tdiff * HZ, 
			(float)ax.io[i].io_pwrite/tdiff * HZ, 
			(float)ax.io[i].io_total/tdiff * HZ); 
		} 
#endif 
		break;
	case 'v':
		break;
	case 'c':
		if (dflg) {
			Isyscall = (float)(ax.di.isyscall/tdiff * HZ);
			Osyscall = (float)(ax.di.osyscall/tdiff * HZ);
			Lsyscall = (float)(ax.si.syscall/tdiff * HZ) -
					(Isyscall + Osyscall);

			Isysread = (float)(ax.di.isysread/tdiff * HZ);
			Osysread = (float)(ax.di.osysread/tdiff * HZ);
			Lsysread = (float)(ax.si.sysread/tdiff * HZ) -
					(Isysread + Osysread);

			Isyswrite = (float)(ax.di.isyswrite/tdiff * HZ);
			Osyswrite = (float)(ax.di.osyswrite/tdiff * HZ);
			Lsyswrite = (float)(ax.si.syswrite/tdiff * HZ) -
					(Isyswrite + Osyswrite);

			Isysexec = (float)(ax.di.isysexec/tdiff * HZ);
			Osysexec = (float)(ax.di.osysexec/tdiff * HZ);
			Lsysexec = (float)(ax.si.sysexec/tdiff * HZ) -
					(Isysexec + Osysexec);

			Ireadch = (float)(ax.di.ireadch/tdiff * HZ);
			Oreadch = (float)(ax.di.oreadch/tdiff * HZ);
			Lreadch = (float)(ax.si.readch/tdiff * HZ) -
					(Ireadch + Oreadch);

			Iwritech = (float)(ax.di.iwritech/tdiff * HZ);
			Owritech = (float)(ax.di.owritech/tdiff * HZ);
			Lwritech = (float)(ax.si.writech/tdiff * HZ) -
					(Iwritech + Owritech);

			printf("Average\n%-8s %7.0f %7.0f %7.0f %7s %7.2f %7.0f %7.0f\n",
				"   in", Isyscall,Isysread,Isyswrite,"",Isysexec,
				Ireadch,Iwritech);
			printf("%-8s %7.0f %7.0f %7.0f %7s %7.2f %7.0f %7.0f\n",
				"   out",Osyscall,Osysread,Osyswrite,"",Osysexec,
				Oreadch,Owritech);
			printf("%-8s %7.0f %7.0f %7.0f %7.2f %7.2f %7.0f %7.0f\n",
				"   local", Lsyscall, Lsysread, Lsyswrite,
				(float)ax.si.sysfork/tdiff *HZ,
				Lsysexec, Lreadch, Lwritech);
			break;
		}

		printf("Average  %7.0f %7.0f %7.0f %7.2f %7.2f %7.0f %7.0f\n",
			(float)ax.si.syscall/tdiff *HZ,
			(float)ax.si.sysread/tdiff *HZ,
			(float)ax.si.syswrite/tdiff *HZ,
			(float)ax.si.sysfork/tdiff *HZ,
			(float)ax.si.sysexec/tdiff *HZ,
			(float)ax.si.readch/tdiff * HZ,
			(float)ax.si.writech/tdiff * HZ);
		break;
	case 'w':
#ifndef u370
		printf("Average  %7.2f %7.1f %7.2f %7.1f %7.0f\n",
			(float)ax.si.swapin/tdiff * HZ,
			(float)ax.si.bswapin /tdiff * HZ,
			(float)ax.si.swapout/tdiff * HZ,
			(float)ax.si.bswapout/tdiff * HZ,
			(float)ax.si.pswitch/tdiff * HZ);
#endif
#ifdef u370
		printf("Average  %7.1f %7.1f %7.1f\n",
		(float)ax.intsched/ax.tmelps*1e6,
		(float)ax.tsend/ax.tmelps*1e6,
		(float)ax.mkdisp/ax.tmelps*1e6);
#endif 
		break;
	case 'a':
		printf("Average  %7.0f %7.0f %7.0f\n",
			(float)ax.si.iget/tdiff * HZ,
			(float)ax.si.namei/tdiff * HZ,
			(float)ax.si.dirblk/tdiff * HZ);
		break;
	case 'S':
		printf("Average %11.0f %9.0f %9.0f %9.0f %9.0f\n",
			(float)ax.di.nservers / tdiff * HZ,
			((ax.di.rcv_occ / tdiff * HZ * 100.0) > 100) ? 100.0 :
				(float)ax.di.rcv_occ / tdiff * HZ * 100.0,
			(ax.di.rcv_occ == 0) ? 0.0 :
				(float)ax.di.rcv_que / (float)ax.di.rcv_occ,
			((ax.di.srv_occ / tdiff * HZ * 100.0) > 100) ? 100.0 :
				(float)ax.di.srv_occ / tdiff * HZ * 100.0,
			(ax.di.srv_occ == 0 ) ? 0.0 :
				(float)ax.di.srv_que / (float)ax.di.srv_occ );
		break;
	case 'q':
#ifndef u370
		if (ax.si.runocc == 0)
			printf("Average  %7s %7s ","  ","  ");
		else {
			printf("Average  %7.1f %7.0f ",
			(float)ax.si.runque /
				(float)ax.si.runocc,
			(float)ax.si.runocc /tdiff *HZ *100.0);
		}
		if (ax.si.swpocc == 0)
			printf("%7s %7s\n","  ","  ");
		else {
			printf("%7.1f %7.0f\n",
			(float)ax.si.swpque/
				(float)ax.si.swpocc,
			(float)ax.si.swpocc/tdiff *HZ *100.0);

		}
#endif
#ifdef u370
		printf("Average  %7.0f %7.0f %7.0f %7.0f %7.0f\n",
				(float)(ax.pi.run/ax.lines),
				(float)(ax.pi.wtsem/ax.lines),
				(float)(ax.pi.wtsemtm/((ax.pi.wtsem)*HZ)),
				(float)(ax.pi.wtio/ax.lines),
				(float)(ax.pi.wtiotm/((ax.pi.wtio)*HZ)));

#endif
		break;
	case 'm':
		printf("Average  %7.2f %7.2f\n",
			(float)ax.si.msg/tdiff * HZ,
			(float)ax.si.sema/tdiff * HZ);
		break;
#if	vax || u3b
	case 'r':
		printf("Average  %7.0f %7.0f\n",
			(float)(ax.mi.freemem / (rcnt-2)),
			(float)(ax.mi.freeswap) / (rcnt-2));
		break;
#endif

#if  u3b2 || u3b5 || m68k

	case 'r':
		printf("Average  %7.0f",
			(double)(ax.mi.freemem[0] + magic * ax.mi.freemem[1])/tdiff);
		if(rcnt > 2)
		  printf("%7.0f\n",(float)(ax.mi.freeswap) / (rcnt-2));
		else printf("\n");
		break;
#endif

#if	vax || u3b || u3b5 || u3b2 || m68k
	case 'p':
		printf("Average  %7.2f %7.2f %7.2f %7.2f\n",
			(float)(ax.mi.vfault / tdiff * HZ),
			(float)(ax.mi.pfault / tdiff * HZ),
			(float)(ax.mi.file / tdiff * HZ),
			(float)(ax.mi.freedpgs / tdiff * HZ));
#endif
	}
}

/**********************************************************/

/*      error exit routines  */
pillarg()
{
	fprintf(stderr,"%s -- illegal argument for option  %c\n",
		optarg,cc);
	exit(1);
}

perrexit()
{
	perror("sar");
	exit(1);
}

pmsgexit(s)
char	*s;
{
	fprintf(stderr, "%s\n", s);
	exit(1);
}
