#
#	@(#)MAKE.RES	7.1	
#
# '***********************************************************************'
# '*                                                                     *'
# '*   Function :         MAKE.RESIDENT [<processor>][macro definitions] *'
# '*                                                                     *'
# '*                      This shell script forces the compilation of    *'
# '*                      c[01] in the do68, do10, or do20 environment    *'
# '*                      depanding on the target processor.             *'
# '*                                                                     *'
# '*                      processor:    specifies the target processor   *'
# '*                                                                     *'
# '*                      macro definitions                              *'
# '*                                    Argument lists for make, possibly*'
# '*                                    null.                            *'
# '*                                                                     *'
# '*   Source file :      /port/port/src/work/cc/pcc2/m68k/MAKE.RESIDENT *'
# '*                                                                     *'
# '*   Imported key words:                                               *'
# '*                      $DEFPROC     Default target processor          *'
# '*                      $DESTM68000  Installed destination             *'
# '*                      $DESTM68010  depending on the specified        *'
# '*                      $DESTM68020  processor.                        *'
# '*                                                                     *'
# '*   Exported key words:                                               *'
# '*                      none                                           *'
# '*                                                                     *'
# '*   Shellscript/makefile called :                                     *'
# '*                      make         executes the makefile             *'
# '*                                                                     *'
# '*   Shellscript/makefile which call this shell :                      *'
# '*                      mk.res       this shell controls the creation  *'
# '*                                   of a log file.                    *'
# '*                                                                     *'
# '*   Operation :        This shell script can be run by executing this *'
# '*                      source file or under the control of command    *'
# '*                      file :                                         *'
# '*                      "/port/port/src/work/cc/pcc2/m68k/mk.res"      *'
# '*                                                                     *'
# '*   Associated files : All the result will be saved in a log file     *'
# '*			  whose name is the month, date, hour, and minute*'
# '*			  of the time of invocation.			 *'
# '*                                                                     *'
# '*   Errors :           The keyword parameters DEFPROC, DESTM68000,    *'
# '*                      DESTM68010, and DESTM68020 must be defined     *'
# '*                      and exported in user"s .profile.               *'
# '*                                                                     *'
# '***********************************************************************'
#

echo "************************* pcc2 MAKE.RESIDENT *******************"
echo "\nStarting MAKE.RESIDENT for pcc2. `date` \n"
echo "command line: mk.res $*\n"
#
#	Scan the first positional parameter. If it is a PROCESSOR specification
#	then remember it and shift it off.
#
case $1 in
   68000 | m68000 | M68000 | 00 |\
   68010 | m68010 | M68010 | 10 |\
   68020 | m68020 | M68020 | 20) 
	    PROCESSOR=$1
	    shift;;
   *) 	PROCESSOR="";;
esac


#	Verify that the user specified a PROCESSOR.  If one was not specified
#	then request that the user input a legal PROCESSOR. If the job is not
#	interactive, assign the default PROCESSOR.
#
#	Once a PROCESSOR has been determined, set up the ".do" 
#	environment, and initialize SUFFIX and DESTINATION.
#
#
	  SUFFIX=""
      while [ "$SUFFIX" = "" ] 
      do
		  case $PROCESSOR in
			68000 | m68000 | M68000 | 00) SUFFIX=00
					where | grep 68000 >&-	# find current environment
					if [ $? = 1 ]	# current environment is not 68000
      				then
						 :
						 . do68
                    fi
			        if [ "$DESTM68000" = "" ]
			           then
			           echo "**** ERROR **** Please define and export DESTM68000 in your .profile" 
					   
			           exit 255
			        else
			           DESTINATION="$DESTM68000"  
			        fi;;
			68010 | m68010 | M68010 | 10) SUFFIX=10
					where | grep 68010 >&-	# find current environment
					if [ $? = 1 ]	# current environment is not 68010
		            then
                         . do10
                    fi
			        if [ "$DESTM68010" = "" ]
			           then
			           echo "**** ERROR **** Please define and export DESTM68010 in your .profile" 
					   
			           exit 255
			        else
			           DESTINATION="$DESTM68010"  
			        fi;;
			68020 | m68020 | M68020 | 20) SUFFIX=20
					where | grep 68020 >&-	# find current environment
					if [ $? = 1 ]	# current environment is not 68020
                    then
						 . do20
                    fi
			        if [ "$DESTM68020" = "" ]
			           then
			           echo "**** ERROR **** Please define and export DESTM68020 in your .profile" 
					    
			           exit 255
			        else
			           DESTINATION="$DESTM68020"  
			        fi;;
			*) 
		       if [ -t 0 ]
		       then
		         echo "\nDesignate the target processor: M68000,M68010,orM68020"
                 PROCESSOR=`line`
               else
				 case $DEFPROC in
					  68000 | m68000 | M68000 | 00 | \
					  68010 | m68010 | M68010 | 10 | \
					  68020 | m68020 | M68020 | 20 ) PROCESSOR="$DEFPROC";;
					  *)
	                    echo "**** ERROR **** Please define and export DEFPROC"
	                    exit 255
				 esac
			   fi
         esac
       done
   
#     Start processing

      echo "Make command line: make $*\n"

      make clean                                 # remove all object files
      rm -f c[01]                                # from current directory

      mv resident$SUFFIX/*.o .                  # move all the previous object
												# file into current directory
      make build $*                               # generate c[01]
	  ERROR=$?


      cp c0 resident$SUFFIX/c0              # save a copy of c0           
      cp c1 resident$SUFFIX/c1              # save a copy of c1           
      mv *.o resident$SUFFIX                    # move all object to resident
												# object directory
	  echo "Executable and object files saved in `pwd`/resident$SUFFIX"


	  if [ $ERROR = 0 ]
	  then

          strip c0 c1
          chmod 775 c0 c1
#         Install c0 c1 onto $DESTINATION

         for i in $DESTINATION
         do
              rm -f $i/lib/c[01]
              cvout c0 $i/lib/c0
              cvout c1 $i/lib/c1
              chown port $i/lib/c[01]
		      echo "Install c[01] onto " $i
         done
       fi
#
#     Clean current directory
#
      rm c[01]
