/*
 * Define system error numbers, including the ones related to networking.
 * Include the standard system error numbers.  Then make sure that the
 * network numbers are defined as we expect them to be.  (Their actual
 * values are significant because the ENP code knows about these numbers.)
 */

#include <sys/errno.h>			/* include standard errors */

/* non-blocking and interrupt i/o */
#ifdef	EWOULDBLOCK
# undef	EWOULDBLOCK
#endif	/* EWOULDBLOCK */
#define EWOULDBLOCK	55		/* Operation would block */

#ifdef	EINPROGRESS
# undef	EINPROGRESS
#endif	/* EINPROGRESS */
#define EINPROGRESS	56		/* Operation now in progress */

#ifdef	EALREADY
# undef	EALREADY
#endif	/* EALREADY */
#define EALREADY	57		/* Operation already in progress */


	/* argument errors */
#ifdef	ENOTSOCK
# undef	ENOTSOCK
#endif	/* ENOTSOCK */
#define ENOTSOCK	58		/* Socket operation on non-socket */

#ifdef	EDESTADDRREQ
# undef	EDESTADDRREQ
#endif	/* EDESTADDRREQ */
#define EDESTADDRREQ	59		/* Destination address required */

#ifdef	EMSGSIZE
# undef	EMSGSIZE
#endif	/* EMSGSIZE */
#define EMSGSIZE	60		/* Message too long */

#ifdef	EPROTOTYPE
# undef	EPROTOTYPE
#endif	/* EPROTOTYPE */
#define EPROTOTYPE	61		/* Protocol wrong type for socket */

#ifdef	ENOPROTOOPT
# undef	ENOPROTOOPT
#endif	/* ENOPROTOOPT */
#define ENOPROTOOPT	62		/* Protocol not available */

#ifdef	EPROTONOSUPPORT
# undef	EPROTONOSUPPORT
#endif	/* EPROTONOSUPPORT */
#define EPROTONOSUPPORT	63		/* Protocol not supported */

#ifdef	ESOCKTNOSUPPORT
# undef	ESOCKTNOSUPPORT
#endif	/* ESOCKTNOSUPPORT */
#define ESOCKTNOSUPPORT	64		/* Socket type not supported */

#ifdef	EOPNOTSUPP
# undef	EOPNOTSUPP
#endif	/* EOPNOTSUPP */
#define EOPNOTSUPP	65		/* Operation not supported on socket */

#ifdef	EPFNOSUPPORT
# undef	EPFNOSUPPORT
#endif	/* EPFNOSUPPORT */
#define EPFNOSUPPORT	66		/* Protocol family not supported */

#ifdef	EAFNOSUPPORT
# undef	EAFNOSUPPORT
#endif	/* EAFNOSUPPORT */
#define EAFNOSUPPORT	67		/* Proto fam doesn't support addr fam */

#ifdef	EADDRINUSE
# undef	EADDRINUSE
#endif	/* EADDRINUSE */
#define EADDRINUSE	68		/* Address already in use */

#ifdef	EADDRNOTAVAIL
# undef	EADDRNOTAVAIL
#endif	/* EADDRNOTAVAIL */
#define EADDRNOTAVAIL	69		/* Can't assign requested address */


	/* operational errors */
#ifdef	ENETDOWN
# undef	ENETDOWN
#endif	/* ENETDOWN */
#define ENETDOWN	70		/* Network is down */

#ifdef	ENETUNREACH
# undef	ENETUNREACH
#endif	/* ENETUNREACH */
#define ENETUNREACH	71		/* Network is unreachable */

#ifdef	ENETRESET
# undef	ENETRESET
#endif	/* ENETRESET */
#define ENETRESET	72		/* Network dropped connection on reset */

#ifdef	ECONNABORTED
# undef	ECONNABORTED
#endif	/* ECONNABORTED */
#define ECONNABORTED	73		/* Software caused connection abort */

#ifdef	ECONNRESET
# undef	ECONNRESET
#endif	/* ECONNRESET */
#define ECONNRESET	74		/* Connection reset by peer */

#ifdef	ENOBUFS
# undef	ENOBUFS
#endif	/* ENOBUFS */
#define ENOBUFS		75		/* No buffer space available */

#ifdef	EISCONN
# undef	EISCONN
#endif	/* EISCONN */
#define EISCONN		76		/* Socket is already connected */

#ifdef	ENOTCONN
# undef	ENOTCONN
#endif	/* ENOTCONN */
#define ENOTCONN	77		/* Socket is not connected */

#ifdef	ESHUTDOWN
# undef	ESHUTDOWN
#endif	/* ESHUTDOWN */
#define ESHUTDOWN	78		/* Can't send after socket shutdown */

#ifdef	ETOOMANYREFS
# undef	ETOOMANYREFS
#endif	/* ETOOMANYREFS */
#define ETOOMANYREFS	79		/* Too many references: can't splice */

#ifdef	ETIMEDOUT
# undef	ETIMEDOUT
#endif	/* ETIMEDOUT */
#define ETIMEDOUT	80		/* Connection timed out */

#ifdef	ECONNREFUSED
# undef	ECONNREFUSED
#endif	/* ECONNREFUSED */
#define ECONNREFUSED	81		/* Connection refused */

