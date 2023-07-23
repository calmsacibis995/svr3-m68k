Notes on implementation of various features

An unattended incremental is never verbose, even if the flag for it is set
when the incrmenetal is scheduled.  This cuts down on the mail sent
regarding the unattended incremental.  An alternative would be to save the
output in a file somewhere, but this could easily suck up lots of disk
space.  It is safer to simply do a WHOLE VIEW on the archive, perhaps
directing the output to the line printer.

Perhaps several stand-alone versions of programs to view and display header
info, for use when B/R is not necessary.
	- VIEW entire archive contents  (bru -tvv)
	- HEADER display  (bru -g)
