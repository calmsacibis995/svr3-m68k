/*
 *	Ethernet protocol types
 */

# define ETHPROTO_RAW	255

# define ETHPROTO_MAX	256

extern struct domain ethdomain;
extern struct protosw ethsw[];
