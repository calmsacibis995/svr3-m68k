Pseudo-code description of backup procedure.

HowTo BACKUP...

Quick-check sanity of DiskInfo,
	exit if insane.

Tell when last full and last incremental was.
Tell about CURRENT-ARCHIVE device(s).

Choose:
	incremental backup (all things, since last full)
	full backup (all things)
	selected backup (specify selection criteria)
	restore from archive
	choose archive device
	quit

Choice was:

INCREMENTAL...
	Loop...
		Choose:
			estimate media needs
			schedule an unattended backup
			do the backup now
			verify the backup
			quit
		Choice was:
		ESTIMATE...
			Run bru -e -n on /
		SCHEDULE...
			default-time = today, at same hour as last incremental
			time correct?
				( ask user )
			will it fit on CURRENT-ARCHIVE media?
				( estimate backup size )
			is media currently in place?
				( try to open & write to it )
			if all above OK,
				schedule:
				DO-INCREMENTAL from / with CURRENT-ARCHIVE
		DOIT...
			DO-INCREMENTAL from / with CURRENT-ARCHIVE
		VERIFY...
			Run bru -i on archive
		QUIT...
			break the loop
	Loop-end


FULL...
	Loop...
		Choose:
			estimate media needs
			do the backup
			verify the backup
			quit
		Choice was:
		ESTIMATE...
			Run bru -e on /
		DOIT...
			DO-FULL from / with CURRENT-ARCHIVE
		VERIFY...
			Run bru -i on archive
		QUIT...
			break the loop
	Loop-end



SELECTED...
	Loop...
		Show current-criteria
		Choose:
			select individual
			change current-criteria
			do the backup
			quit
		Choice was:
		INDIVIDUAL...
			show list of current things
			criteria = choice of thing
		CHANGE...
			criteria = input changes to current-criteria
			( criteria consists of parts, each of which can be
			  independently modified.  Criteria parts are:
				root-point	( 'cd' here first )
				user-name	( -o user )
				since-date	( -n date )
				unconditional	( -u [bcdlpr]* )
				name-pattern	( shell meta-patterns )
				mount-keep-out	( -m )
				wait-confirm	( -w )
				I-become-owner	( remove the -C )
				keep-access-time	( -a )
		DOIT...
			DO-FULL from criteria with CURRENT-ARCHIVE
		QUIT...
			break the loop
	Loop-end


RESTORE...
	Loop...
		Show current selection criteria
		Choose:
			view entire contents of archive
			view selected portions of archive
			set new selection criteria
			restore selected portions of archive
			quit
		Choice was:
		VIEW-ALL...
			Run bru -t[vvvv] on archive
		VIEW-PART...
			Input selection criteria (save for possible restore)
			Run bru -t[vvvv] with selection criteria
		RESTORE...
			Choose:
				previous selection criteria
				new selection criteria
				quit
			Choice was:
			PREVIOUS...
				use current selection criteria
			NEW...
				input new selection criteria
			QUIT...
				exit restore function
			Run bru -x with selection criteria
		QUIT...
			break loop
	Loop-end
			

CHOOSE-ARCHIVE...
	Loop...
		Choose:
			known archive devices
			add new archive device
			quit
		Choice was:
		KNOWN...
			make known device into current device
		NEW...
			input parameters of new device
			(don't make it current)
		QUIT...
			break the loop
	Loop-end

QUIT...
	don't do anything

end of HowToBackup 


-----
HowTo  DO-INCREMENTAL...
{
If not root userID,
	say "Not SuperUser"
	exit
Change directory to /
Run bru -ci -n on "."
touch time-file for incremental-backup
}

-----
HowTo  DO-FULL...
{
If not root userID,
	say "Not SuperUser"
	exit
Change directory to /
Remember which file-systems are mounted
Run bru -ci on "."
touch time-file for full-backup
touch time-files for individual-backup of each mounted file-system
}
