/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sa:sadc.c	1.25"
/*	sadc.c 1.25 of 11/27/85	*/
/*
	sadc.c - writes system activity binary data from /dev/kmem to a
		file or stdout.
	Usage: sadc [t n] [file]
		if t and n are not specified, it writes
		a dummy record to data file. This usage is
		particularly used at system booting.
		If t and n are specified, it writes system data n times to
		file every t seconds.
		In both cases, if file is not specified, it writes
		data to stdout.
*/


#include <sys/types.h>
#include <sys/param.h>
#include <sys/sysmacros.h>
#ifdef m68k
#include <sys/immu.h>
#include <sys/region.h>
#endif
#ifdef u3b
#include <sys/page.h>
#include <sys/region.h>
#include <sys/seg.h>
#endif
#include <sys/var.h>
#include <sys/iobuf.h>
#include <sys/stat.h>
#include <sys/elog.h>
#include <sys/inode.h>
#include <sys/file.h>
#ifdef u3b2
#include <sys/sbd.h>
#include <sys/immu.h>
#include <sys/id.h>
#include <sys/region.h>
#endif
#include <sys/proc.h>
#include <sys/sysinfo.h>
#include <sys/fcntl.h>
#include <sys/flock.h>
#include "sa.h"
#ifdef u3b
#include <sys/mtc.h>
#include <sys/dma.h>
#include <sys/vtoc.h>
#include <sys/dfc.h>
#define ctob(x) ptob(x)
int dskcnt, mtccnt;
char *taploc,*dskloc;
int apstate;
#endif

#ifdef u3b2
int idcnt;
struct idstatstruct idstatus[IDNDRV];
struct iotime idstat[IDNDRV];
struct iotime ifstat;
#endif /* u3b2 */

#ifdef u3b5
#include <sys/dfdrv.h>
int dfcnt;
char *dfloc;
#endif /*u3b5 */

#ifdef u370
#include <sys/times.h>
#include <sst.h>
time_t times();
#endif

#ifdef m68k
struct iotime ia[NDEVS];
int devcnt[SINFO];
int base;
#endif

struct stat ubuf,syb;
struct var tbl;
struct syserr err;
#if pdp11 || vax
struct iotime ia[NCTRA][NDRA];
struct iotime ib[NCTRB][NDRB];
struct iotime ic[NCTRC][NDRC];
struct iotime id[NCTRC][NDRD];
int rlcnt;
int gdcnt;
#endif

struct sa d;
struct flckinfo flckinfo;
char *flnm = "/tmp/sa.adrfl";
char *loc;
static int fa = 0;
unsigned tblloc;
char *malloc();
#ifndef u370
static int tblmap[SINFO];
#endif
#ifdef u370
static int tblmap[10];	/* A kludge for the 370 -not really used */
#endif
static int recsz,tmpsz;
long time();
int i,j,k;
long lseek();
int f;

main(argc, argv)
char **argv;
int argc;
{
#ifdef u370
	long tmstmp;
	struct pxs *pxs;
	struct stv *stv;
	int sst;
	int n;
	DEC dp;
	struct tms buff;
#endif	
	int ct;
	unsigned ti;
	int fp;
	long min;
	long rem;
	struct stat buf;
	char *fname;

	ct = argc >= 3? atoi(argv[2]): 0;
	min = time((long *) 0);
	ti = argc >= 3? atoi(argv[1]): 0;

/*	check if the address file is there and not older
	than /sysV68, if so, read the offsets in.
	Otherwise, search /sysV68 name list to get the offsets
	and create an address file for later usage.
	*/
	stat ("/sysV68",&syb);
	if ((stat(flnm,&ubuf) == -1) ||
		 (ubuf.st_mtime <= syb.st_ctime))
			goto creafl;
	fa = open(flnm,2);
	if (read(fa,setup,sizeof setup) != sizeof setup){
		close(fa);
		unlink(flnm);
creafl:
		i = umask(0);
		fa = open(flnm, O_RDWR|O_CREAT|O_TRUNC, 0664);
		umask(i);
		chown(flnm, 0, getegid());

/*		search name list to get offsets	*/
		nlist("/sysV68",setup);
#ifdef vax
		for (i=0;i<SERR +1;i++)
			setup[i].n_value &= ~(0x80000000);
#endif
#ifdef u370
/*		open /dev/mem - get system table addresses	*/
		if((f = open("/dev/mem", 0)) == -1)
			exit(2);
		for (i = INO; i < V; i++){
			lseek(f, setup[i].n_value, 0);
			read(f, &setup[i].n_value, 4);
		}
/*		close /dev/mem	*/
		close(f);
#endif

/*		write offsets to address file	*/
		write(fa,setup,sizeof setup);
		close (fa);
	}
#ifndef u370
/*	open /dev/kmem	*/
	if((f = open("/dev/kmem", 0)) == -1)
		perrexit();
#endif
#ifdef u370
/*	open /dev/mem	*/
	if((f = open("/dev/mem", 0)) == -1)
		exit(2);
/*	open tss system statistics table	*/
	if((sst = open("/dev/sst",0)) == -1)
		exit(2);
#endif

#if pdp11 || vax
/*	get number of disk drives on rl, ra, and general disk drivers.	*/
	if (setup[RLCNT].n_value != 0){
		lseek(f,(long)setup[RLCNT].n_value, 0);
		if (read(f,&rlcnt,sizeof rlcnt) == -1)
			perrexit();
	}
	if (setup[GDCNT].n_value != 0){
		lseek(f,(long)setup[GDCNT].n_value, 0);
		if (read(f,&gdcnt,sizeof gdcnt) == -1)
			perrexit();
	}
#endif
#ifdef u3b
/*	get numbers of tape controller and disk drives for 3B	*/
	if (setup[MTCCNT].n_value !=0){
		lseek(f,(long)setup[MTCCNT].n_value, 0);
		if (read(f,&mtccnt,sizeof mtccnt) == -1)
			perrexit();
		taploc = malloc(sizeof (struct mtc)*mtccnt*4);
		if (taploc == NULL)
			perrexit();
	}
	if (setup[DSKCNT].n_value !=0){
		lseek(f,(long)setup[DSKCNT].n_value, 0);
		if (read(f,&dskcnt,sizeof dskcnt) == -1)
			perrexit();
		dskloc = malloc(sizeof (struct dskinfo)*dskcnt);
		if (dskloc == NULL)
			perrexit();
	}
#endif

#ifdef m68k
	/* 
	 * get count value for each configured block device driver and
	 * convert count to number of physical devices for the driver
	 */
	for(i=0; i<SINFO; i++){
		if (setup[DEVCNT+i].n_value != 0){
			lseek(f,(long)setup[DEVCNT+i].n_value, 0);
			if (read(f,&devcnt[i],sizeof devcnt[0]) == -1)
				perrexit();
			devcnt[i] = CNTtoPHYS(devcnt[i]);
		}
	}
#endif

#ifdef u3b5
/*	get numbers of disk drives for 3B5	*/
	if (setup[DFCNT].n_value !=0){
		lseek(f,(long)setup[DFCNT].n_value, 0);
		if (read(f,&dfcnt,sizeof dfcnt) == -1)
			perrexit();
		dfloc = malloc(sizeof (struct dfc)*dfcnt);
		if (dfloc == NULL)
			perrexit();
	}
#endif

#ifdef u3b2
/*	get number of hard disk drives for 3B2	*/
	  if (setup[IDEQ].n_value !=0){
		lseek(f,(long)setup[IDEQ].n_value, 0);
		if (read(f, idstatus,sizeof idstatus) != sizeof idstatus)
			perrexit();
		for (i = 0; i < IDNDRV; i++)
			if (idstatus[i].equipped)
				idcnt++;
	  }
#endif /* u3b2 */

#ifndef u370
/*	construct tblmap and compute record size	*/
	for (i=0;i<SINFO;i++){
#if pdp11 || vax
		if (setup[i].n_value != 0){
			if (i == GDS)
				tblmap[i] = gdcnt * 8;
			else {
				if (i == RLS)
					tblmap[i] = rlcnt;
				else
					tblmap[i] =iotbsz[i];
			}
#endif
#ifdef u3b
		if (setup[i].n_value != 0){
			if (i == MTC)
				tblmap[i] = mtccnt * 4;
			else {
				if (i == DSKINFO)
					tblmap[i] = dskcnt;
				else
					tblmap[i] = iotbsz[i];
			}
#endif

#ifdef  m68k
		if (setup[i].n_value != 0){
			if (devcnt[i])
				tblmap[i] = devcnt[i];	/* actual count */
			else
				tblmap[i] =iotbsz[i];	/* default size */
#endif

#ifdef u3b5
		if(setup[i].n_value != 0) {
			if(i == DFDFC)
				tblmap[i] = dfcnt * NDRV;
			else
				tblmap[i] = iotbsz[i];
#endif /* u3b5 */

#ifdef u3b2
		if(setup[i].n_value != 0) {
			if(i == ID)
				tblmap[i] = idcnt;
			else
				tblmap[i] = iotbsz[i];
#endif /* u3b2 */
			recsz += tblmap[i];
		}
		else
			tblmap[i] = 0;
	}
	recsz = sizeof (struct sa) - sizeof d.devio + recsz * sizeof d.devio[0];
#endif
#ifdef u370
/*	assign record size	*/
	recsz = sizeof (struct sa);
#endif
	if (argc == 3 || argc == 1){

/*	no data file is specified, direct data to stdout	*/
		fp = 1;
		/*	write header record	*/
		write(fp,tblmap,sizeof tblmap);
	}
	else {
		fname = argc==2? argv[1]: argv[3];
		/*	check if the data file is there	*/
		/*	check if data file is too old	*/
		if ((stat(fname,&buf) == -1) ||
		 ((min - buf.st_mtime) > 86400))
			goto credfl;
		if ((fp = open(fname,2)) == -1){
credfl:

/*			data file does not exist:
			create one and write the header record.	*/
			if ((fp = creat(fname,00644)) == -1)
				perrexit();
			close(fp);
			fp = open (fname,2);
			lseek(fp,0L,0);
			write (fp,tblmap,sizeof tblmap);
		}
		else{

/*			data file exist:
			position the write pointer to the last good record.  */
			lseek(fp,-(long)((buf.st_size - sizeof tblmap) % recsz),2);
		}
	}

/*	if n =0 , write the additional dummy record	*/
	if (ct == 0){
		for (i=0;i<4;i++)
			d.si.cpu[i] = -300;
		d.ts = min;
		write(fp,&d,recsz);
	}

/*	get memory for tables	*/
	if(lseek(f,(long)setup[V].n_value,0) == -1)
		perrexit();
	if(read(f,&tbl,sizeof tbl) == -1)
		perrexit();
	if (tblloc < sizeof(struct inode)*tbl.v_inode)
		tblloc =sizeof(struct inode)*tbl.v_inode;
	if (tblloc < sizeof(struct file)*tbl.v_file)
		tblloc = sizeof (struct file)*tbl.v_file;
	if (tblloc < sizeof (struct proc)*tbl.v_proc)
		tblloc = sizeof (struct proc)*tbl.v_proc;
	loc = malloc(tblloc);
	if (loc == NULL){
		perrexit();
	}
#ifdef u370
	pxs = (struct pxs *)malloc(sizeof(*pxs));
	stv = (struct stv *)malloc(sizeof(*stv));
#endif

	for(;;) {


/*		read data from /dev/kmem to structure d	*/
		lseek(f,(long)setup[SINFO].n_value,0);
		if (read(f, &d.si, sizeof d.si) == -1)
			perrexit();

/*	Distributed Unix info	*/
		if (setup[DINFO].n_value != 0)  {
			lseek( f, (long)setup[DINFO].n_value, 0);
			if (read( f, &d.di, sizeof d.di) == -1)
				perrexit();
		}
		if (setup[MINSERVE].n_value != 0)	{
			lseek( f, (long)setup[MINSERVE].n_value, 0);
			if (read( f, &d.minserve, sizeof d.minserve) == -1)
				perrexit();
		}
		if (setup[MAXSERVE].n_value != 0)	{
			lseek( f, (long)setup[MAXSERVE].n_value, 0);
			if (read( f, &d.maxserve, sizeof d.maxserve) == -1)
				perrexit();
		}

#if	vax || u3b || u3b5 || u3b2 || m68k
/*	virtual memory	*/
		if (setup[MINFO].n_value != 0)	{
			lseek( f, (long)setup[MINFO].n_value, 0);
			if (read( f, &d.mi, sizeof d.mi) == -1)
				perrexit();
		}
#endif

#if pdp11 || vax 
		d.si.bswapin = ctod(d.si.bswapin);
		d.si.bswapout = ctod(d.si.bswapout);
		for (i=HPS;i<SINFO;i++)
			if (setup[i].n_value != 0){
				lseek(f,(long)setup[i].n_value,0);
				if (i<RLS){
				if (read(f,ia[i],sizeof ia[i]) == -1)
					perrexit();
				}
				else{
				if (i == TSS){
				if (read(f,id[i-TSS],sizeof id[i-TSS]) == -1)
					perrexit();
				}
				else {
				if (i == GDS){
				if (read(f,ic[i-GDS],(sizeof (struct iotime)*gdcnt*8)) == -1)
					perrexit();
				}
				else
				if (read(f,ib[i-RLS],sizeof ib[i-RLS]) == -1)
					perrexit();
				}
				}
			}
#endif /* pdp11 || vax */

#ifdef m68k
		/* these were  calls to ctod(), which we don't have */
		d.si.bswapin = ptod(d.si.bswapin);
		d.si.bswapout = ptod(d.si.bswapout);

		j = 0;
		/* for each controller type */
		for (i=0; i<SINFO; i++)
			if (setup[i].n_value != 0){
				lseek(f,(long)setup[i].n_value,0);
				if (read(f, &ia[j], IOSZ(tblmap[i])) == -1)
					perrexit();
				j += tblmap[i];
			}
		/* end for each controller type */
#endif

#ifdef u3b
		d.si.bswapin=ctob(d.si.bswapin)/NBPSCTR; /* BSIZE to NBPSCTR */
		d.si.bswapout=ctob(d.si.bswapout)/NBPSCTR; /* BSIZE to NBPSCTR */
		tapetbl(taploc);
		dsktbl(dskloc);
#endif /* u3b */

#ifdef u3b5
		dftbl(dfloc);
#endif /* u3b5 */

#ifdef u3b2
		/* translate from clicks to disk blocks */
		d.si.bswapin=ctob(d.si.bswapin)/NBPSCTR; /* BSUZE to NBPSCTR */
		d.si.bswapout=ctob(d.si.bswapout)/NBPSCTR; /* BSIZE to NBPSCTR */
		if ( setup[ID].n_value != 0 ) {
			lseek( f, (long) setup[ID].n_value, 0 );
			if ( read( f, idstat, sizeof( struct iotime ) * idcnt) == -1) {
				perrexit();
			}
		}
		if ( setup[IF].n_value != 0 ) {
			lseek( f, (long) setup[IF].n_value, 0 );
			if ( read( f, &ifstat, sizeof( struct iotime ) ) == -1) {
				perrexit();
			}
		}
#endif /* u3b2 */

#ifdef u370
		if (read(sst, pxs, sizeof(*pxs)) == -1)
			exit(2);
		lseek(sst, 4096L, 0);
		if (read(sst, stv, sizeof(*stv)) == -1)
			exit(2);
		lseek(sst, 0, 0);
/*	Get performance statistics from tss sst tables	*/
/*	Get cpu usage info	*/
		d.nap = pxs->nap;
		d.vmtm = stv->vmb;
		d.usrtm = stv->nvmb;
		d.usuptm = d.vmtm - d.usrtm;
		d.idletm = stv->wtm1 + stv->wtm2;
		d.ccv = stv->ccv;
/*	Get scheduling info	*/
		dp = stv->tsend.alt;
		n = cvb(dp);
		d.tsend = n;
		d.intsched = stv->sched.kie;
		d.mkdisp = stv->sched.kid;
/*	Get drum and disk info	*/
		for (i = 0; i < NDEV;i++) {
			n = cvb(stv->dd[i].ddrs);
			d.io[i].io_sread = n;
			n = cvb(stv->dd[i].ddrp);
			d.io[i].io_pread = n;
			n = cvb(stv->dd[i].ddws);
			d.io[i].io_swrite = n;
			n = cvb(stv->dd[i].ddwp);
			d.io[i].io_pwrite = n;
			d.io[i].io_total = d.io[i].io_sread +
					d.io[i].io_pread +
					d.io[i].io_swrite +
					d.io[i].io_pwrite;
		}
		d.elpstm = times(&buff);
		d.curtm = time((long *) 0);
#endif
/*		compute size of system table	*/
		d.szinode = inodetbl(loc);
		d.szfile = filetbl(loc);
#ifndef u370
		d.szproc = proctbl(loc);
#endif

#ifdef u370
		proctbl(loc);
		d.szproc = d.pi.sz;
#endif

#if vax || u3b || u3b5 || u3b2 || m68k
		if ( setup[FLCK].n_value != 0 ) {
			lseek( f, (long) setup[FLCK].n_value, 0 );
			if (read(f, &flckinfo, sizeof(struct flckinfo)) == -1) {
				perrexit();
			}
		}
		/* record system table sizes */
		d.szlckr = flckinfo.reccnt;
		/* record maximum sizes of system tables */
		d.mszlckr = flckinfo.recs;
#endif
/*		record maximum sizes of system tables	*/
		d.mszinode = tbl.v_inode;
		d.mszfile = tbl.v_file;
		d.mszproc = tbl.v_proc;

/*		record system tables overflows	*/
		lseek(f,(long)setup[SERR].n_value,0);
		if (read(f,&err,sizeof (struct syserr)) == -1)
			perrexit();
		d.inodeovf = err.inodeovf;
		d.fileovf = err.fileovf;
		/*	get time stamp	*/
		d.ts = time ((long *) 0);
#ifdef u3b
/* Ascertain whether the AP is online or not */
	if (setup[APSTATE].n_value !=0){
		lseek(f,(long)setup[APSTATE].n_value, 0);
		if (read(f,&apstate,sizeof apstate) == -1)
			perrexit();
		d.apstate = apstate;
	}
#endif
		i=0;
#ifndef u370
#ifdef m68k
		base = 0;
#endif
		for (j=0;j<SINFO;j++){
#if pdp11 || vax
			if (setup[j].n_value != 0){
				for (k=0;k<tblmap[j];k++){
					if (j == TSS){
						d.devio[i][0] = id[j-TSS][k].io_cnt;
						d.devio[i][1] = ctod(id[j-TSS][k].io_bcnt);
						d.devio[i][2] = id[j-TSS][k].io_act;
						d.devio[i][3] = id[j-TSS][k].io_resp;
					}
					else {
					if (j == GDS){
						d.devio[i][0] = ic[j-GDS][k].io_cnt;
						d.devio[i][1] = ctod(ic[j-GDS][k].io_bcnt);
						d.devio[i][2] = ic[j-GDS][k].io_act;
						d.devio[i][3] = ic[j-GDS][k].io_resp;
					}
					else {
					if (j >= RLS && j < GDS){
						d.devio[i][0] = ib[j-RLS][k].io_cnt;
						d.devio[i][1] = ctod(ib[j-RLS][k].io_bcnt);
						d.devio[i][2] = ib[j-RLS][k].io_act;
						d.devio[i][3] = ib[j-RLS][k].io_resp;
					}

					else{
						d.devio[i][0] = ia[j][k].io_cnt;
						d.devio[i][1] = ctod(ia[j][k].io_bcnt);
						d.devio[i][2] = ia[j][k].io_act;
						d.devio[i][3] = ia[j][k].io_resp;
					}
					}
					}
					i++;
				}
		}
#endif /* vax || pdp11 */

#ifdef m68k
			if (setup[j].n_value != 0){
				for (k=base;k<base+tblmap[j];k++){
					d.devio[k][0] = ia[k].io_cnt;
					/* this was a call to ctod(),
					which we don't have */
					d.devio[k][1] = ptod(ia[k].io_bcnt);
					d.devio[k][2] = ia[k].io_act;
					d.devio[k][3] = ia[k].io_resp;
				}
				base += tblmap[j];
			}
#endif

#ifdef u3b
			if (setup[j].n_value != 0)
					if (j == MTC)
						tapemv(taploc);
					else
						dskmv(dskloc);
#endif /* u3b */

#ifdef u3b5
				dfmv(dfloc);
#endif /* u3b5 */

#ifdef u3b2
		for (j=0; j<SINFO; j++){
			if (setup[j].n_value != 0){
				for (k=0;k<tblmap[j];k++){
					if (j == ID) {
						d.devio[i][0] = idstat[k].io_cnt;
						d.devio[i][1] = idstat[k].io_bcnt;
						d.devio[i][2] = idstat[k].io_act;
						d.devio[i][3] = idstat[k].io_resp;
					} else if (j == IF) {
						d.devio[i][0] = ifstat.io_cnt;
						d.devio[i][1] = ifstat.io_bcnt;
						d.devio[i][2] = ifstat.io_act;
						d.devio[i][3] = ifstat.io_resp;
					}
					i++;
				}
			}
		}
#endif /* u3b2 */
	}
#endif

/*	write data to data file from structure d	*/
		write(fp,&d,recsz);
		if(--ct > 0)
			sleep(ti);
		else {
			close(fp);
			exit(0);
		}
	}
}

inodetbl(x)
register struct inode *x;
{
	register i,n;
	lseek(f,(long)setup[INO].n_value,0);
	read(f,x,tbl.v_inode*sizeof(struct inode));

	for (i=n=0;i<tbl.v_inode;i++,x++)

		if (x->i_count != 0)
			n++;

	return(n);
}
filetbl(x)
register struct file *x;
{
	register i,n;
	lseek(f,(long)setup[FLE].n_value,0);
	read(f,x,tbl.v_file*sizeof(struct file));
	for (i=n=0;i<tbl.v_file; i++,x++)
		if (x->f_count != 0)
			n++;
	return(n);
}
#ifndef u370
proctbl(x)
register struct proc *x;
{
	register i,n;
	lseek(f,(long)setup[PRO].n_value,0);
	read (f,x,tbl.v_proc*sizeof(struct proc));
	for (i=n=0;i<tbl.v_proc;i++,x++)
		if(x->p_stat !=NULL)
			n++;
	return(n);
}
#endif

#ifdef u370
proctbl(x)
register struct proc *x;
{
	register i;
/*	reinitialize procinfo structure	*/
	d.pi.sz = 0;
	d.pi.run = 0;
	d.pi.wtsem = 0;
	d.pi.wtio = 0;
	d.pi.wtsemtm = 0;
	d.pi.wtiotm = 0;

	lseek(f,(long)setup[PRO].n_value, 0);
	read(f, x, tbl.v_proc*sizeof(struct proc));
	for (i=0; i < tbl.v_proc; i++, x++){
		if (x->p_stat == NULL)
			continue;
		d.pi.sz++;
		if (x->p_stat == SRUN){
			d.pi.run++;
			continue;
		}
		else
			if (x->p_stat == SSLEEP && x->p_ppid !=1)
				if (x->p_wchan == 0) {
					d.pi.wtio++;
					d.pi.wtiotm += d.curtm - x->p_start;
				}
				else {
					d.pi.wtsem++;
					d.pi.wtsemtm += d.curtm - x->p_start;
					}
	}
/*	The following prevents a 0 divide in sar -q	*/
	if (d.pi.wtio == 0)
		d.pi.wtio=1;
	if (d.pi.wtsem == 0)
		d.pi.wtsem=1;
	return;
}
#endif


#ifdef u3b
tapetbl(x)
register struct mtc *x;
{
	lseek (f,(long)setup[MTC].n_value,0);
	read (f,x,mtccnt*sizeof (struct mtc)*4);
	return;
}
dsktbl(x)
register struct dskinfo *x;
{
	register n;
	lseek (f,(long)setup[DSKINFO].n_value,0);
	read (f,x,dskcnt*sizeof (struct dskinfo));
	return;
}
long tapemv(x)
register struct mtc *x;
{
	register p;
	for (p=0;p<mtccnt*4;p++,x++,i++){
		d.devio[i][0] = x->iotime.ios.io_ops;
		d.devio[i][1] = x->iotime.io_bcnt/NBPSCTR; /* BSIZE to NBPSCTR */
	}
	return ;
}
long dskmv(x)
register struct dskinfo *x;
{
	register p;
	for(p=0;p<dskcnt;p++,x++,i++){
		d.devio[i][0] = x->devinfo.io_cnt;
		d.devio[i][1] = x->devinfo.io_bcnt;
		d.devio[i][2] = x->devinfo.io_act;
		d.devio[i][3] = x->devinfo.io_resp;
	}
	return;
}

#endif /* u3b */

#ifdef u3b5
dftbl(x)
register struct dfc *x;
{
	register n;
	lseek (f,(long)setup[DFDFC].n_value,0);
	read (f,x,dfcnt*sizeof (struct dfc));
	return;
}

dfmv(x)
register struct dfc *x;
{
	register p;
	int q;

	for (q=0;q<dfcnt;q++) {
		for (p=0;p<NDRV;p++,i++) {
			d.devio[i][0] = x->df_stat[p].io_cnt;
			d.devio[i][1] = x->df_stat[p].io_bcnt;
			/* job response time is in millisecond units
			   convert drive active time from milliseconds to HZ
			*/
			/*d.devio[i][2] = (x->df_stat[p].io_act * HZ) / 1000; */
			d.devio[i][2] = x->df_stat[p].io_act;
			d.devio[i][3] = x->df_stat[p].io_resp;
		}
		x++;
	}
	return;
}

#endif /* u3b */
perrexit()
{
	perror("sadc");
	exit(2);
}
