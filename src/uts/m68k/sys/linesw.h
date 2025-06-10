/* Line Discipline Switch Table */

extern int nulldev();
extern int ttopen(),ttclose(),ttread(),ttwrite(),ttioctl(),ttin(),ttout();
extern int sxtin(),sxtout();

/*   order: open close read write ioctl rxint txint modemint */
struct linesw linesw[]
	={
/* tty ------------- */
	   ttopen,
	   ttclose,
	   ttread,
	   ttwrite,
	   ttioctl,
	   ttin,
	   ttout,
	   nulldev,
/* sxt ------------- */
	   nulldev,
	   nulldev,
	   nulldev,
	   nulldev,
	   nulldev,
	   sxtin,
	   sxtout,
	   nulldev,
	   };
int linecnt ={2};
