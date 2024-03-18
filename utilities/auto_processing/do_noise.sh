#!/bin/bash -e
#
# Arguments:
# $1 : series number
# $2 : dump number
# $3 : max events to process
# $4 : datatype (ba, bg or cf)
# $5 : run (e.g. R123, R124,...)
# $6 : tag (e.g. Prodv5-2)
# $7 : processing config file
# $8 : analysis config file

###### Configuration Setup #######
[ -f jobsetup.sh ] && . jobsetup.sh

if [ $# -ne 8 ] ; then
    echo "Incorrect number of arguments given"
    echo "Usage: $0 <series> <dump> <max events> <datatype> <run> <tag>"
    exit $ARGERROR
fi

series=$1
dumpnum=$2
maxevents=$3
datatype=$4
runid=$5
tag=$6
proccfgfile=$7
anacfgfile=$8 

echo "Hostname: $HOSTNAME"

#get all the auxiliary files from the rsync source
#do we actually need to do this?
echo "$(tstamp): Copying/linking auxiliary files for series $series..."
seriesdir="$DATADIR/$series"
$MKDIR $seriesdir
auxsuffixes="dmm hv isr veto.scalers zip.scalers"
for suffix in $auxsuffixes ; do
    auxfile="$CDMS_RAWDATA/$runid/$series/${series}.${suffix}"
    try_copy $auxfile $seriesdir/ || exit $FAIL
done

echo "$(tstamp): Starting noise processing for series $series..."
[ -d $SCRATCHDIR ] || $MKDIR $SCRATCHDIR
#copy/link the raw data from the main repository to scratch
echo "$(tstamp): Copying/Linking raw data to scratch..."
rawfile=$(get_raw_filename $series $dumpnum)
try_realcopy $CDMS_RAWDATA/$runid/$series/$rawfile $SCRATCHDIR || exit $FAIL

######### Run br #################
export BATROOT_AUXFILES=$DATADIR/$series
export BATROOT_NOISEFILES=$SCRATCHDIR

echo "$(tstamp): Starting BatNoise, logging to $logfile"
$CDMSBATSDIR/BUILD/bin/BatNoise $series $dumpnum $maxevents \
    $proccfgfile $anacfgfile || \
    exit $FAIL
echo "$(tstamp): BatNoise finished"

####### copy the resulting filter file to the output directory ###### 
echo "$(tstamp): Copying to $OUTPUTDIR"
filterfile=$(get_filter_filename $tag $series)
copy_atomically $SCRATCHDIR/$filterfile $(get_noise_dir)/ || exit $FAIL

#clean up
echo "$(tstamp): Cleaning up scratch directory"
rm -f $SCRATCHDIR/$rawfile
rm -f $SCRATCHDIR/$filterfile

echo "$(tstamp): Finished noies for series $series"

exit $SUCCESS




