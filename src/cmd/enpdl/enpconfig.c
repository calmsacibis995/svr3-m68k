/*
 * Build system-specific ENP parameter file.
 *
 * Usage:	enpconfig [-d] parameter_file_name
 *
 * Input is text file, one parameter per line.
 * Output (to standard out) is text file in S-record format,
 *	suitable for downloading via enpload.
 * If -d is specified, debugging output (also to stdout) dumps the
 *	table in a more readable format.
 *
 * Supported input lines are:
 *	enp	10|20|2020|30|40|44
 *	host	iaddr [eaddr]
 *	flags	number
 *	gateway	iaddr
 *	arp	iaddr eaddr
 *	laf	long_number
 *	mcast	eaddr
 *	# rest of line is comment; can appear anywhere on line
 * where
 *	iaddr ::= "host" | number
 *	eaddr ::= 1-12 hex digits (leading 0x optional)
 *	long_number ::= 1-16 hex digits (leading 0x optional)
 * and	numbers can be decimal, octal, or hex.
 *
 * The input file must have exactly one entry for each of "enp" and "host".
 * Many "arp" entries may be specified; they must have unique iaddrs.
 * "Laf" and "multicast" are supported as parameters, but the enpcode does
 *	not use them.
 *
 * The size and format of the output structure must correspond exactly to
 * that expected by the ENP code.  Expectations include:
 *	- all pointers are 4 bytes
 *	- ints and longs are 4 bytes
 *	- left-to-right bytes in a character string correspond to
 *	  increasing memory locations
 *	- the highest byte of an int or long corresponds to the
 *	  lowest address memory location
 * The subroutine which generates S-records assumes that bytes have 8
 *	bits.
 * The routines get_eaddr and get_laf store the decoded numbers in static
 *	character arrays; subsequent calls overwrite previous numbers.
 *	This will need to be changed if any commands are added which take
 *	multiple Ethernet addresses or LAF's in one command.
 *
 * 21 May 1985 - Inital coding.		Deb Brown, CMC
 */
/**/
# include <CMC/longnames.h>
# include <ctype.h>
# include <stdio.h>
# include <CMC/types.h>
# include <CMC/netdb.h>

# define EA_LTH		6	/* num bytes in ethernet address	*/
# define IA_LTH		4	/* num bytes in internet address	*/
# define LAF_LTH	8	/* num bytes in logical addr filter	*/
# define HEAPSIZE	1024
# define MAX_ARP	51
# define BSZ		25
# define LSZ		100

# define asn_eaddr(a,b)		(memcpy((a), (b), EA_LTH))
# define asn_iaddr(a,b)		(memcpy((a), (b), IA_LTH))
# define asn_laf(a,b)		(memcpy((a), (b), LAF_LTH))

# define match(s1,s2)		(!(strcmp((s1),(s2))))
# define matchn(s1,s2,n)	(!(memcmp((s1),(s2),(n))))
# define min(a,b)		((a) < (b) ? (a) : (b))

typedef	char *	IADDR;
typedef	char *	EADDR;
typedef	char *	LAF;
typedef	int	INT;

/*	ARP table structure						*/

typedef
struct arptab {
	u_char	at_iaddr[4];		/* internet address		*/
	u_char	at_enaddr[6];		/* Ethernet address		*/
	u_char	at_timer;		/* minutes since last ref	*/
	u_char	at_flags;		/* flags			*/
} ARPTAB;

/*	System Parameters Structure					*/

typedef
struct	sysparam {
	EADDR		sys_esa;	/* Node's Ethernet address	*/
	IADDR		sys_isa;	/* and Internet address		*/
	INT		sys_flags;	/* Flag bits  -  below		*/
	IADDR		sys_gateway;	/* My gateway's Internet addr	*/
	char *		sys_resv;	/* reserved			*/
	ARPTAB		*sys_arp;	/* Pre-defined ARP entries	*/
	LAF		sys_laf;	/* Logical Address Filter	*/
	EADDR		sys_mcast;	/* Multicast			*/
} SYSPARAM;

/* -------------------- G L O B A L S --------------------------------- */

struct	arptab	atab[MAX_ARP];	/* pre-defined ARP equivalences		*/
int	debug;			/* generate debugging output		*/
char	*enp_addr;		/* addr of RAM in ENP memory space	*/
char	*enp_tab;		/* where in ENP RAM to put table	*/
int	enp_set;		/* flag whether ENP has been specified	*/
FILE	*osid = stdout;		/* stream to write S-records to		*/
int	num_arp = 0;		/* num entries in arp table		*/
struct {			/* place where we build the parm table	*/
	struct sysparam	parm;
	char   heap [HEAPSIZE];
	} ptab;
char	*hp = ptab.heap;	/* ptr to next free space in heap	*/
int	num_tok;		/* number tokens on a line		*/
char	line [LSZ];		/* holds one parameter line		*/
char	cmd [BSZ],		/* command from the line		*/
	parm1 [BSZ],		/* first parameter for command		*/
	parm2 [BSZ];		/* second parameter (if any) for cmd	*/

char	*strchr ();
char	*strrchr ();
/* */
/* --------------------- C H K _ N U M _ T O K ------------------------ */
/*
 * Make sure user specified right number of fields on a parameter line
 * Usage:	chk_num_tok ((int) num);
 */

chk_num_tok (num)
int	num;
{
	if (num_tok != num)
		fatal ("Improper number of fields: %s\n", line);
}


/* -------------------- C M D _ A R P --------------------------------- */
/*
 * Add predefined ARP entry.
 * Usage:	cmd_arp (iaddr, eaddr);
 */

cmd_arp (p1, p2)
IADDR	p1;
EADDR	p2;
{
register int i;

	for (i = 0; i < num_arp; ++i)
		if (matchn (p1, atab[i].at_iaddr, IA_LTH))
			fatal ("Duplicate ARP entry for %X\n",
				*((u_long *) p1));
	asn_iaddr (atab[num_arp].at_iaddr, p1);
	asn_eaddr (atab[num_arp].at_enaddr, p2);
	if (++num_arp > MAX_ARP - 1)
		fatal ("Too many ARP entries specified\n");
}

/* -------------------- C M D _ E N P --------------------------------- */
/*
 * Set address variables based on type of ENP
 * Usage:	cmd_enp (num);
 */

cmd_enp (p1)
int	p1;
{
	if (enp_set)
		fatal ("Multiple specifications: enp\n");
	switch (p1)
	{
	case 10:	enp_addr = (char *)0xF01000;  break;
	case 20:	enp_addr = (char *)0xF01000;  break;
	case 2020:	enp_addr = (char *)0x001000;  break;
	case 30:	enp_addr = (char *)0xF01000;  break;
	case 40:	enp_addr = (char *)0x001000;  break;
	case 44:	enp_addr = (char *)0x001000;  break;
	default:
		fatal ("Unknown ENP number %d\n", p1);
	}
	enp_tab = enp_addr + 0x100;
	enp_set = 1;
}

/* -------------------- C M D _ F L A G S ----------------------------- */
/*
 * Set system flag parameter
 * Usage:	cmd_flags ((int) num);
 */

cmd_flags (p1)
int	p1;
{
	if (ptab.parm.sys_flags)
		fatal ("Multiple specifications: flags\n");
	ptab.parm.sys_flags = p1;
}

/* -------------------- C M D _ G W A Y ------------------------------- */
/*
 * Set system gateway parameter.
 * Usage:	cmd_gway (iaddr);
 */

cmd_gway (p1)
IADDR p1;
{
	if (ptab.parm.sys_gateway)
		fatal ("Multiple specifications: gateway\n");
	ptab.parm.sys_gateway = hp;
	asn_iaddr (hp, p1);
	hp += IA_LTH;
}

/* -------------------- C M D _ H O S T ------------------------------- */
/*
 * Set system Internet and (optionally) ETHERNET addresses.
 * Usage:	cmd_host (iaddr, eaddr);
 */

cmd_host (p1, p2)
IADDR	p1;
EADDR	p2;
{
	if (ptab.parm.sys_isa)
		fatal ("Multiple specifications: host\n");
	ptab.parm.sys_isa = hp;
	asn_iaddr (hp, p1);
	hp += IA_LTH;
	if (p2)
	{
		ptab.parm.sys_esa = hp;
		asn_eaddr (hp, p2);
		hp += EA_LTH;
	}
}

/* -------------------- C M D _ L A F --------------------------------- */
/*
 * Set logical address filter
 * Usage:	cmd_laf (laf);
 */

cmd_laf (p1)
LAF	p1;
{
	if (ptab.parm.sys_laf)
		fatal ("Multiple specifications: laf\n");
	ptab.parm.sys_laf = hp;
	asn_laf (hp, p1);
	hp += LAF_LTH;
}

/* -------------------- C M D _ M C A S T ----------------------------- */
/*
 * Set multicast address
 * Usage:	cmd_mcast (eaddr);
 */

cmd_mcast (p1)
EADDR	p1;
{
	if (ptab.parm.sys_mcast)
		fatal ("Multiple specifications: mcast\n");
	ptab.parm.sys_mcast = hp;
	asn_eaddr (hp, p1);
	hp += EA_LTH;
}

/* -------------------- D C D ----------------------------------------- */
/*
 * Convert a hex char to 4 binary bits
 * Usage:	num = dcd (c);
 * Returns:	-1 for invalid character
 */

dcd (c)
register char c;
{
	if (!isascii(c) || !isxdigit(c))
		return (-1);
	if (isdigit(c))
		return (c - '0');
	if (islower(c))
		return (c - 'a' + 10);
	if (isupper(c))
		return (c - 'A' + 10);
	return (-1);
}

/* -------------------- G E T _ E A D D R ------------------------------ */
/*
 * Parse a 6 digit ETHERNET address.  Number is always taken as hex; a
 * leading 0x is optional.
 *
 * Usage:	peaddr = get_eaddr (ip);
 */

EADDR
get_eaddr (ip)
char	*ip;
{
static	char obuf [EA_LTH];	/* storage for one decoded e'net addr	*/
int	lth;
EADDR	p;
EADDR	lnum ();

	lth = strlen (ip);
	p = lnum (ip, lth, obuf, EA_LTH);
	if (p == 0)
		fatal ("Too many digits in Ethernet addr: %s\n", ip);
	if (p == (char *) -1)
		fatal ("Invalid digit in Ethernet addr: %s\n", ip);
	return (p);
}

/* -------------------- G E T _ I A D D R ----------------------------- */
/*
 * Considers an input string to be a general Internet address, either
 * host name or in . notation.  Calls appropriate routine to decode,
 * depending on type.
 *
 * Usage:	iaddr = get_iaddr ((char *) s);
 */

IADDR
get_iaddr (s)
char	*s;
{
static	char obuf [IA_LTH];	/* buf for one decoded internet addr	*/
IADDR	iaddr;
IADDR	iaddr_host ();
u_long	inet_addr ();
u_long	uaddr;
register int	i;
register char	*p;

	if (*s == '"')		/* quote indicates host name		*/
	{
		if ((p = strrchr (s, '"')) == NULL)
			fatal ("Missing ending quote on Internet address: %s\n",
				s);
		*p = 0;
		return (iaddr_host (++s));
	}
	else
	{
		if ((uaddr = inet_addr (s)) == -1)
			fatal ("Invalid Internet address: %s\n", s);
		for (i = 0; i < IA_LTH; ++i)
		{
			obuf [IA_LTH - 1 - i] = uaddr & 0xFF;
			uaddr >>= 8;
		}
		return (obuf);
	}
}

/* -------------------- G E T _ L A F --------------------------------- */
/*
 * Parse a 8 digit logical address filter.  Number is always taken as hex;
 * a leading 0x is optional.
 *
 * Usage:	laf_num = get_laf (ip);
 */

LAF
get_laf (ip)
char	*ip;
{
static	obuf [LAF_LTH];		/* storage for one decoded laf number	*/
int	lth;
char	*p;
char	*lnum();

	lth = strlen (ip);
	p = lnum (ip, lth, obuf, LAF_LTH);
	if (p == 0)
		fatal ("Too many digits in logical address filter: %s\n", ip);
	if (p == (char *) -1)
		fatal ("Invalid digit in logical address filter: %s\n", ip);
	return (p);
}

/* -------------------- F A T A L ------------------------------------- */

fatal (fmt, p1)
char *fmt;
{
	fprintf (stderr, fmt, p1);
	exit (1);
}

/* -------------------- I A D D R _ H O S T --------------------------- */
/*
 * Decode internet address given a host name.
 * Usage:	iaddr = iaddr_host (p);
 * Returns binary address.
 */

IADDR
iaddr_host (p)
char *p;
{
struct hostent *gethostbyname();
struct hostent *hep;
u_long		addr = 0;
register int	i;

	if ((hep = gethostbyname (p)) == 0)
		fatal ("Unknown host name %s\n", p);
	return (hep -> h_addr);
}

/* -------------------- L N U M --------------------------------------- */
/*
 * Decode a long hex number.
 * Usage:	ptr = lnum ((char *)ip, (int)lth, (char *)obuf, (int)olth);
 * Returns:	0 - Too many digits in input string
 *		-1 - Non-hex digit in input string
 * On return, obuf contains decoded string, two hex digits per byte, right
 *	justified in each byte.
 */

char *
lnum (ip, lth, obuf, olth)
char	*ip;			/* input ptr				*/
int	lth;			/* length of input string		*/
char	*obuf;			/* ptr to where to put results		*/
int	olth;			/* size of output buffer		*/
{
register int	shift = 0;	/* indicates left or right nibble	*/
register int	i;
register char	*op = obuf;

	if (ip[0] == '0' && (ip[1] == 'x' || ip[1] == 'X'))
	{
		ip += 2;
		lth -= 2;
	}
	if (lth > olth*2)		/* check for too long string	*/
		return ((char *)0);
	for (i = 0; i < olth; ++i)	/* zero fill decoded addr	*/
		*op++ = 0;
	--op;
	for (ip += lth; lth > 0; --lth)	/* scan back for each digit	*/
	{
		if ((i = dcd (*(--ip))) == -1)
			return ((char *)-1);
		*op += i << shift;
		shift = shift ? 0 : 4;
		if (!shift)
			--op;
	}
	return (obuf);
}

/* -------------------- M K _ P A R M --------------------------------- */
/*
 * Make complete parameter table.  Add specified ARP entries to heap.
 * Change pointers in sysparam table to point to locations in ENP
 * memory space.  Write whole thing out as S-records.
 *
 * Usage:	(void) mk_parm ();
 */

void
mk_parm ()
{
int	offset;

/* Verify user specified everything required				*/

	if (ptab.parm.sys_isa == 0)
		fatal ("ERROR: Must specify host Internet address\n");
	if (enp_set == 0)
		fatal ("ERROR: Must specify ENP model number\n");

/* If any ARP entried specified, copy table to heap			*/

	if (num_arp)
	{
		++num_arp;		/* null terminate list		*/
		ptab.parm.sys_arp = (ARPTAB *) hp;
		memcpy (hp, &(atab[0]), num_arp * sizeof(struct arptab));
		hp +=  num_arp * sizeof(struct arptab);
	}

/* Update pointer values to be in ENP memory space			*/

	offset = enp_tab - (char *)(&ptab);
	if (ptab.parm.sys_esa)
		ptab.parm.sys_esa += offset;
	if (ptab.parm.sys_isa)
		ptab.parm.sys_isa += offset;
	if (ptab.parm.sys_gateway)
		ptab.parm.sys_gateway += offset;
	if (ptab.parm.sys_arp)
		ptab.parm.sys_arp =
			(ARPTAB *) ((int) ptab.parm.sys_arp + offset);
	if (ptab.parm.sys_laf)
		ptab.parm.sys_laf += offset;
	if (ptab.parm.sys_mcast)
		ptab.parm.sys_mcast += offset;

/* Output parameter table in S-record format				*/

	if (debug)
		DUMP_PTAB ();
	mk_srec (enp_tab, &ptab, hp - (char *)(&ptab));
}

/* -------------------- M K _ S R E C --------------------------------- */
/*
 * Generate S-record output for a data buffer.
 * Usage:	(void) mk_srec (addr, buf, lth);
 *
 * Format of output:
 *	S 2 count addr data data ... csum <newline>
 * All numbers are hex.  Addr is 6 digits (3 bytes).  Count, data, and
 * csum are 2 digits each.  Count is number of bytes in record; counts
 * addr, csum, and all data.  Csum is one's complement of sum of bytes
 * in addr and all data.
 */

# define MX_SREC	16
# define MX_CLTH	MX_SREC - 4

mk_srec (addr, buf, lth)
u_long	addr;			/* download addr for first s-record	*/
char	*buf;			/* ptr to data to write			*/
int	lth;			/* number bytes to write		*/
{
u_char	csum;			/* calculated checksum			*/
int	clth;			/* length of current chunk		*/
u_long	saddr;			/* addr for current s-record		*/
int	i;

	for (; lth > 0; lth -= clth)
	{
		clth = min (MX_CLTH, lth);
		csum = clth + 4;
		fprintf (osid, "S2%02X%06X", clth+4, addr);
		for (saddr = addr, i = 0; i < 3; ++i, saddr >>= 8)
			csum += (saddr & 0xFF);
		addr += clth;
		for (i = 0; i < clth; ++i)
		{
			fprintf (osid, "%02X", (*buf) & 0xFF);
			csum += *buf++;
		}
		fprintf (osid, "%02X\n", ((~csum) & 0xFF));
	}
}
/* */
/* -------------------- M A I N --------------------------------------- */

main (argc, argv)
int	argc;
char	*argv[];
{
FILE	*isid;			/* input stream for parameter file	*/
char	*p1, *p2;
char	*file_name;
char	*prog_name;
char	c;
register int	i;

EADDR	get_eaddr ();
IADDR	get_iaddr ();

/* Check calling sequence, open input file, etc.			*/

	prog_name = argv[0];
	while ((argc > 1) && (argv[1][0] == '-'))
	{
		if (argv[1][1] == 'd')
			debug = 1;
		else
			fatal ("Unrecognized option: %s\n", argv[1]);
		--argc;
		++argv;
	}
	if (argc < 2)
		fatal ("Usage: %s [-d] filename\n", prog_name);
	file_name = argv[1];
	if ((isid = fopen (file_name, "r")) == NULL)
		fatal ("Cannot open input file %s\n", file_name);

/* Read each line.  Break it into tokens.  Process depending on command	*/

	while (1)
	{
		fgets (line, LSZ, isid);	/* read a line		*/
		if (ferror (isid))
			fatal ("Error reading file %s\n", file_name);
		if (feof (isid))
			break;
		if ((p1 = strchr (line, '#')))	/* check for comments	*/
			*p1 = '\0';
		if ((p1 = strchr (line, '\n')))	/* strip newline	*/
			*p1 = '\0';
		num_tok = sscanf (line, " %25s %25s %25s %c ",
			cmd, parm1, parm2, &c);
		if (num_tok <= 0)
			continue;

		if (match (cmd, "arp"))
		{
			chk_num_tok (3);
			cmd_arp (get_iaddr (parm1), get_eaddr (parm2));
			continue;
		}
		if (match (cmd, "enp"))
		{
			chk_num_tok (2);
			cmd_enp (atoi (parm1));
			continue;
		}
		if (match (cmd, "flags"))
		{
			chk_num_tok (2);
			cmd_flags (strtol (parm1, 0, 0));
			continue;
		}
		if (match (cmd, "gateway"))
		{
			chk_num_tok (2);
			cmd_gway (get_iaddr (parm1));
			continue;
		}
		if (match (cmd, "host"))
		{
			if (num_tok < 2 || num_tok > 3)
				fatal ("Improper number of fields: %s\n", line);
			p2 = (num_tok == 3) ? get_eaddr (parm2) : 0;
			cmd_host (get_iaddr (parm1), p2);
			continue;
		}
		if (match (cmd, "laf"))
		{
			chk_num_tok (2);
			cmd_laf (get_laf (parm1));
			continue;
		}
		if (match (cmd, "mcast"))
		{
			chk_num_tok (2);
			cmd_mcast (get_eaddr (parm1));
			continue;
		}
		fatal ("Unrecognized parameter: %s\n", line);
	}

/* Read all user's input.  Finish building table and write it out	*/

	mk_parm ();
}

/* -------------------- D U M P _ P T A B ----------------------------- */
/*
 * DUMP_* routines will dump the parameter tables.  Useful for debugging
 * when moving to new systems (with possibly different data sizes or byte
 * orders).
 */

DUMP_PTAB()
{

	printf ("sys_esa:	0x%06x\n", ptab.parm.sys_esa);
	printf ("sys_isa:	0x%06x\n", ptab.parm.sys_isa);
	printf ("sys_flags:	0x%06x\n", ptab.parm.sys_flags);
	printf ("sys_gateway:	0x%06x\n", ptab.parm.sys_gateway);
	printf ("sys_arp:	0x%06x\n", ptab.parm.sys_arp);
	printf ("sys_laf:	0x%06x\n", ptab.parm.sys_laf);
	printf ("sys_mcast:	0x%06x\n", ptab.parm.sys_mcast);

	if (ptab.parm.sys_esa) DUMP_bytes ("esa", ptab.parm.sys_esa, 6);
	if (ptab.parm.sys_isa) DUMP_long ("isa", ptab.parm.sys_isa);
	if (ptab.parm.sys_flags) DUMP_int ("flags", ptab.parm.sys_flags);
	if (ptab.parm.sys_gateway) DUMP_long ("gateway", ptab.parm.sys_gateway);
	if (ptab.parm.sys_arp) DUMP_arp ("arp", ptab.parm.sys_arp);
	if (ptab.parm.sys_laf) DUMP_bytes ("laf", ptab.parm.sys_laf, 8);
	if (ptab.parm.sys_mcast) DUMP_bytes ("mcast", ptab.parm.sys_mcast, 6);

	if (hp != ptab.heap)
		DUMP_mem (&ptab, hp-((char *) &ptab));

	printf ("\n");
}

DUMP_bytes (name, ptr, lth)
char *name;
char *ptr;
int	lth;
{
	ptr -= (enp_tab - (char *)(&ptab));
	printf ("%s:	", name);
	while (lth-- > 0)
		printf ("%02x", *(ptr++) & 0xFF);
	printf ("\n");
}

DUMP_long (name, ptr)
char *name;
u_long *ptr;
{
register char *p;

	p = (char *) ptr;
	p -= (enp_tab - (char *)(&ptab));
	ptr = (u_long *) p;
	printf ("%s:	%08X\n", name, *ptr);
}

DUMP_int (name, num)
char *name;
int num;
{
	printf ("%s:	%x\n", name, num);
}

DUMP_arp (name, ptr)
char *name;
struct arptab *ptr;
{
register int i;
register char *p;

	p = (char *) ptr;
	p -= (enp_tab - (char *)(&ptab));
	ptr = (struct arptab *) p;
	printf ("%s:", name);
	for (; *((u_long *)ptr -> at_iaddr); ++ptr)
	{
		printf ("	");
		for (i = 0; i < IA_LTH; ++i)
			printf ("%02X", (ptr -> at_iaddr [i]) & 0xFF);
		printf (" ");
		for (i = 0; i < EA_LTH; ++i)
			printf ("%02X", (ptr -> at_enaddr [i]) & 0xFF);
		printf ("\n");
	}
}

DUMP_mem (ptr, lth)
char	*ptr;
int	lth;
{
register int	i, j;
char	*addr;

	addr = (ptr + (enp_tab - (char *)(&ptab)));
	for (i = 0; i < lth; i += 16)
	{
		printf ("%06X: ", addr);
		addr += 16;
		for (j = 0; (j < 16) && (i + j < lth); ++j)
			printf ("%02X ", (ptr [i+j]) & 0xFF);
		printf ("\n");
	}
}
