h00490
s 00031/00000/00000
d D 1.1 86/07/31 12:00:22 fnf 1 0
c Initial copy from 5.2 Generic M68000 distribution.
e
u
U
t
T
I 1
# @(#)massage.sed	6.1	
s/addmag\([^%]\)/addmag%%\1/g
s/fladd\([^%]\)/fladd%%\1/g
s/flcmp\([^%]\)/flcmp%%\1/g
s/fldiv\([^%]\)/fldiv%%\1/g
s/flmul\([^%]\)/flmul%%\1/g
s/flneg\([^%]\)/flneg%%\1/g
s/flsub\([^%]\)/flsub%%\1/g
s/fltst\([^%]\)/fltst%%\1/g
s/ftol\([^%]\)/ftol%%\1/g
s/itof\([^%]\)/itof%%\1/g
s/ltof\([^%]\)/ltof%%\1/g
s/ultof\([^%]\)/ultof%%\1/g
s/uldiv\([^%]\)/uldiv%%\1/g
s/ulrem\([^%]\)/ulrem%%\1/g
s/submag\([^%]\)/submag%%\1/g
s/addmag$/addmag%%/g
s/fladd$/fladd%%/g
s/flcmp$/flcmp%%/g
s/fldiv$/fldiv%%/g
s/flmul$/flmul%%/g
s/flneg$/flneg%%/g
s/flsub$/flsub%%/g
s/fltst$/fltst%%/g
s/ftol$/ftol%%/g
s/itof$/itof%%/g
s/ltof$/ltof%%/g
s/ultof$/ultof%%/g
s/uldiv$/uldiv%%/g
s/ulrem$/ulrem%%/g
s/submag$/submag%%/g
E 1
