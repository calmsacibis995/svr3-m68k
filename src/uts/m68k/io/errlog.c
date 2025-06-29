/* @(#)errlog.c 6.3 */
#include "sys/param.h"
#include "sys/types.h"
#include "sys/sysmacros.h"
#include "sys/systm.h"
#include "sys/buf.h"
#include "sys/conf.h"
#include "sys/map.h"
#include "sys/utsname.h"
#include "sys/elog.h"
#include "sys/erec.h"
#include "sys/err.h"
#include "sys/iobuf.h"
#include "sys/var.h"                                                           

int     blkacty;

errinit()
{
        if (err.e_nslot) {
                mapinit(err.e_map, (err.e_nslot+3)/2);
                mfree(err.e_map, err.e_nslot, 1);
        }
        err.e_org = err.e_ptrs;
        err.e_nxt = err.e_ptrs;
}
struct errhdr *
geteslot(size)
{
        register ns, *p;
        register struct errhdr *ep;
        int n, sps;

        ns = (size+sizeof(struct errhdr)+sizeof(struct errslot)-1)
                /sizeof(struct errslot);
        sps = splhi();
        n = malloc(err.e_map, ns);
        splx(sps);
        if (n == 0)
                return(NULL);
        ep = (struct errhdr *)(&err.e_slot[--n]);
        ns *= sizeof(struct errslot)/sizeof(int);
        p = (int *)ep;
        do {
                *p++ = 0;
        } while(--ns);
        ep->e_len = size + sizeof(struct errhdr);
        return(++ep);
}

freeslot(ep)
register struct errhdr *ep;
{
        register ns, sps;

        ns = (ep->e_len+sizeof(struct errslot)-1)/sizeof(struct errslot);
        sps = splhi();
        mfree(err.e_map, ns, (((struct errslot *)ep)-err.e_slot)+1);
        splx(sps);
}

struct errhdr *
geterec()
{
        register sps;
        register struct errhdr *ep;

        sps = splhi();
        while(*err.e_org == NULL) {
                err.e_flag |= E_SLP;
                sleep(&err.e_org, PZERO+1);
        }
        ep = *err.e_org;
        *err.e_org++ = NULL;
        if (err.e_org >= &err.e_ptrs[err.e_nslot])
                err.e_org = err.e_ptrs;
        splx(sps);
        return(ep);
}

puterec(ep, type)
register struct errhdr *ep;
{
        register sps;

        (--ep)->e_type = type;
        ep->e_time = time;
        sps = splhi();
        *err.e_nxt++ = ep;
        if (err.e_nxt >= &err.e_ptrs[err.e_nslot])
                err.e_nxt = err.e_ptrs;
        splx(sps);
        if (err.e_flag&E_SLP) {
                err.e_flag &= ~E_SLP;
                wakeup(&err.e_org);
        }
}

logstart()
{
        register sps;
        register struct estart *ep;

        sps = splhi();
        for(err.e_org = &err.e_ptrs[err.e_nslot-1]; err.e_org >= err.e_ptrs; err.e_org--)
                if (*err.e_org != NULL) {
                        freeslot(*err.e_org);
                        *err.e_org = NULL;
                }
        err.e_org = err.e_ptrs;
        err.e_nxt = err.e_ptrs;
        ep = (struct estart *)geteslot(sizeof(struct estart));
        splx(sps);
        if (ep == NULL)
                return;
        ep->e_cpu = cputype;
        ep->e_name = utsname;
        puterec(ep, E_GOTS);
}

logtchg(nt)
time_t nt;
{
        register struct etimchg *ep;

        if ((ep = (struct etimchg *)geteslot(sizeof(struct etimchg))) != NULL) {
                ep->e_ntime = nt;
                puterec(ep, E_TCHG);
        }
}

logstray(addr)
physadr addr;
{
        register struct estray *ep;

        if ((ep = (struct estray *)geteslot(sizeof(struct estray))) != NULL) {
                ep->e_saddr = addr;
                ep->e_sbacty = blkacty;
                puterec(ep, E_STRAY);
        }
}

fmtberr(dp, cyl)
register struct iobuf *dp;
{
        register struct eblock *ep;
        register struct buf *bp;
        register n;
        register short *p;
        struct br {
                struct eblock   eb;
                short           cregs[1];
        };
        struct  iostat  *iosp;

        if (dp->io_erec != NULL) {
                dp->io_erec->e_rtry++;
                return;
        }
	if(dp->io_stp)							 
                iosp = dp->io_stp;                                             
        if (dp->io_addr == NULL ||
		(ep = (struct eblock *)geteslot(sizeof(struct eblock)+
                (dp->io_nreg*sizeof(short)))) == NULL)
	{
                if(iosp)                                                       
                        iosp->io_unlog++;                                      
                return;
        }
        n = major(dp->b_dev);
        bp = dp->b_actf;
        ep->e_dev = makedev(n, (bp==NULL)?minor(dp->b_dev):minor(bp->b_dev));
        ep->e_regloc = dp->io_addr;
        ep->e_bacty = blkacty;
        if(dp->io_stp)                                                         
        {                                                                      
                ep->e_stats.io_ops = iosp->io_ops;                             
                ep->e_stats.io_misc = iosp->io_misc;                           
                ep->e_stats.io_unlog = iosp->io_unlog;                         
        }                                                                      
        if (bp != NULL) {
                ep->e_bflags = (bp->b_flags&B_READ) ? E_READ : E_WRITE;
                if (bp->b_flags & B_PHYS)
                        ep->e_bflags |= E_PHYS;
                if (bp->b_flags & B_MAP)
                        ep->e_bflags |= E_MAP;
                ep->e_bnum = bp->b_blkno;
                ep->e_bytes = bp->b_bcount;
                ep->e_memadd = paddr(bp);
        }
        else
                ep->e_bflags = E_NOIO;
        ep->e_cyloff = cyl;
        ep->e_nreg = dp->io_nreg;
        p = (short *)(&((struct br *)ep)->cregs[0]);
        n = dp->io_nreg;
	{
                register short *rp;

                rp = (short *)dp->io_addr;
                while(--n >= 0)
                         *p++ = *rp++;
	}
        dp->io_erec = ep;
}

fmtraerr(dp)
register struct iobuf *dp;
{
        register struct eblock *ep;
        register n, *p;
        register long *rp;
        struct br {
                struct eblock   eb;
                long            cregs[1];
        };
        struct  iostat  *iosp;

                                                                               
        if(dp->io_stp)                                                         
                iosp = dp->io_stp;                                             
        if ( (ep = (struct eblock *)geteslot(sizeof(struct eblock)+
                (dp->io_nreg*sizeof(long)))) == NULL) {
                if(iosp)                                                       
                        iosp->io_unlog++;                                      
                return;
        }
        ep->e_dev = dp->b_dev;
        if(iosp)                                                               
        {                                                                      
                ep->e_stats.io_ops = iosp->io_ops;                             
                ep->e_stats.io_misc = iosp->io_misc;                           
                ep->e_stats.io_unlog = iosp->io_unlog;                         
        }                                                                      
        ep->e_nreg = dp->io_nreg;
#ifdef DEBUG
        printf("ep->e_dev = %x(hex)\n",ep->e_dev);
        printf("ep->e_stats.io_ops = %x(hex)\n",ep->e_stats.io_ops);
        printf("ep->e_stats.io_misc = %x(hex)\n",ep->e_stats.io_misc);
        printf("ep->e_stats.io_unlog = %x(hex)\n",ep->e_stats.io_unlog);
        printf("ep->e_nreg = %x(hex)\n",ep->e_nreg);
#endif
        p = (long *)(&((struct br *)ep)->cregs[0]);

        /* now copy the contents of struct mlg into the error record */
        n = dp->io_nreg;
        rp = (long *)dp->io_addr;
#ifdef DEBUG
        while(--n >= 0) {
                *p = *rp++;
                printf("mlgreg = %x(hex)\n",*p);
                p++;
        }
#else
        while(--n >= 0)
                 *p++ = *rp++;
#endif

        dp->io_erec = ep;
}

logberr(dp, erf)
register struct iobuf *dp;
{
        register struct eblock *ep;

        if ((ep = dp->io_erec) == NULL)
                return;
        if (erf)
                ep->e_bflags |= E_ERROR;
        puterec(ep, E_BLK);
        dp->io_erec = NULL;
}



#ifdef	m68k
void logmemory(syn, csr, typ)
int syn, typ;
char *csr;
{
	register struct ememory *ep;

	if ((ep = (struct ememory *)geteslot(sizeof(struct ememory))) != NULL) {
		ep->e_memsyn = syn;
		ep->e_memcsr = csr;
		ep->e_memtyp = typ;
		puterec(ep, E_PRTY);
	}
	return;
}
#else	m68k
logmemory(er, ad)
{
	register struct ememory *ep;

	if ((ep = (struct ememory *)geteslot(sizeof(struct ememory)))!=NULL) {
		ep->e_sbier = er;
		ep->e_memcad = ad;
		puterec(ep, E_PRTY);
	}
}
#endif	/* m68k */
