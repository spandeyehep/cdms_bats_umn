#!/bin/bash -e

#this script prepares a working directory of a failed/interrupted job launched
#by processing_master.sh for resuming. 
#It takes 1 argument: the working directory path

if [ $# -ne 1 ] ; then
    echo "Incorrect number of arguments given!"
    echo "Usage: $0 <working directory>"
    exit 1
fi

WORKDIR=$1

[ -f jobsetup.sh ] && . jobsetup.sh

#make sure the work directory exists
if [ ! -d "$WORKDIR" ] ; then
    echo "ERROR: $WORKDIR is not a directory!"
    exit 2
fi

#we assume all previous child processes have been stopped
echo "Attempting to resume job $(basename $WORKDIR): "
echo "!!! All previous child processes must be halted first !!!"
echo "Are you sure you want to continue? [y/n]"
read go
if [[ "${go:0:1}" != [Yy] ]] ; then
    echo "Quitting..."
    exit 3
fi

#check to see if there was an abort error on the previous job
if [ -f $WORKDIR/$ABORTFILE ] ; then
    echo "The previous job was aborted with message $(cat $WORKDIR/$ABORTFILE)"
    /bin/mv $WORKDIR/$ABORTFILE $WORKDIR/$LOGDIR/
fi

#Clean up things left in an incoherent state
#backup the old logs
noldlogs=$(ls -1d $WORKDIR/$LOGDIR.bak.* 2>/dev/null | wc -l)
/bin/mv -f $WORKDIR/$LOGDIR $WORKDIR/$LOGDIR.bak.${noldlogs}
$MKDIR $WORKDIR/$LOGDIR

#clean up the scrath directory
if [ -n "$SCRATCHDIR" ] ; then
    /bin/rm -rf $WORKDIR/$SCRATCHDIR
fi
$MKDIR $WORKDIR/$SCRATCHDIR

#rename all of the .####.lock files to .job files
lockfiles=$(find $WORKDIR/stage[0-9] -name "*.lock")
for lockfile in $lockfiles ; do
    jobfile="$(echo $lockfile | sed 's/\.lock$/.job/')"
    /bin/mv $lockfile $jobfile
done
#rename all of the .fail files to .job files
failfiles=$(find $WORKDIR/stage[0-9] -name "*.fail")
for failfile in $failfiles ; do
    jobfile="$(echo $failfile | sed 's/\.fail$/.job/')"
    /bin/mv $failfile $jobfile
done

echo "$WORKDIR is now ready to resume child processing"
exit 0

