/*
	defines for the mk68564 driver
	concurrent console/modem operation
*/

#define CONCURRENT	('C'<<8)
#define CONC		(CONCURRENT)
#define CONC_ON		(CONCURRENT|1)
#define CONC_OFF	(CONCURRENT|2)
#define CONC_DEBUG	(CONCURRENT|3)
