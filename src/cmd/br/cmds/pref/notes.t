Notes on items which can change via preferences...

--- Add an item to the top menu called PREFERENCES.  It will allow changes
to all the little things like schedule time, confirmation options, and any
other things that can be saved away in files to make life easier for
someone.

Definite items under preferences:
	- schedule time
	- verbose ESTIMATE
	- verbose INCREMENTAL
	- verbose FULL
	- verbose VERIFY
RESTORE and VIEW are already verbose.

Possible other items for preferences:
    - save verbose info in log files?
    - never ask for confirmation

Implement verbosity switches as words in a file (...files/Verbosity).  If
the word is present the switch is verbose (one-level: '-v'), if the word is
missing the switch is off.  The words are basically the names of the actions
which are verbose.  That is, any combination of:

	ESTIMATE  INCREMENTAL  FULL  VERIFY

or even the first letters are enough.
