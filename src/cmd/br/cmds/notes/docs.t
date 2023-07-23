Notes -

Major sections:
 - Background info about why backups are done, full vs. incremental, system
size and reliability tradeoffs, etc.  Be sure to define terms.
    archive
    archive volume

 - How to use this backup/restore package. Why things were done with simple
text menus and input, a basic overview (archive device description, files
saving times, all done with 'bru', etc.). Cover first-release tradeoffs, and
how to make the best of them.

 - Be sure that explanations are given in terms understandable to anyone.
Avoid "boot"; use "startup".


--- Add an item to the top menu called PREFERENCES.  It will allow changes
to all the little things like schedule time, confirmation options, and any
other things that can be saved away in files to make life easier for someone.

Possible other items for preferences:
    - verbose size estimates?
    - verbose incremental backups?
    - verbose full backups?
    - save verbose info in log files?
    - never ask for confirmation

Set a bunch of shell variables using:
    . GetPrefs   # executes in this shell

use a similar trick to read archive variables instead of the in-line code
used now.


--- Should the QUIT item actually be named CANCEL in the menus, so it can be
used for exiting menu levels, and also for cancelling multiple-choice menus?
This would be more consistent, but preclude use of any other keywords
starting with "C", unless ChooseQ detected this and substituted "QUIT" as
its output.

Note that CANCEL is not a good name in some circumstances, like when
changing archive items.  Here, QUIT means DONE.

Under ARCHIVE top item, allow archive descriptions to be selected from a
library file?  One line per description:
    streaming-tape      /dev/rmt0   ....
    disk-cartridge      /dev/rcart  ....

The item-name is first, with the description following on the line.

The INCREMENTAL NOW item should check the run-level and ask for confirmation
if != 1.  The FULL NOW should probably do the same.  The question could be:
    "Take system to run-level 1 now? [y/n]"

where 'no' would simply proceed, assuming the user knew what was going on.

INCR.UNAT should also check run-level, but simply issue a warning and
proceed.
---  Perhaps, the run-level should be checked upon entering top-level, and
the option to 'init 1' presented there.  If declined, incremental and full
backups simply give a warning and proceed.  If accepted, remember previous
run-level and return to it upon exit.

ChooseDir needs a QUIT item, which simply results in output DIR being the
same as input DIR.

