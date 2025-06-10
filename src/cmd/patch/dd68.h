
	/* Contains data for UNIX */

char    msgcpr[] = "Copyright (c) 1984 Motorola",
	msgpr1[] = "Options\n",
	msgpr2[] = "  o = open file\n",
	msgpr3[] = "  s = show contents\n",
	msgpr4[] = "  c = change bytes\n",
	msgpr5[] = "  d = display status\n",
	msgpr6[] = "  q = quit\n",
	msgopn[] = " Open file error %d \n",
	msgnof[] = " No file open\n",
	msginv[] = " Invalid input\n",
	msgexc[] = " You have read only permission for this file\n",
	msgskk[] = " Seek error no. %d\n",
	msgrdd[] = " Read error no. %d\n",
	msgwrt[] = " Write error no. %d\n",
msgttl[] = " Address    0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F\n";

char    inbuf[128],             /* Input from terminal */
	otbuf[79],              /* Ouput to terminal */
	dkbuf[16];              /* Disk buffer */
