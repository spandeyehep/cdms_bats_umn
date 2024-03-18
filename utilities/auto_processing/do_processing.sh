#!/bin/bash -e
#
# Arguments:
# $1 : series number
# $2 : dump number
# $3 : max events to process
# $4 : datatype (ba, bg or cf)
# $5 : run (e.g. R123, R124,...)
# $6 : tag (e.g. Prodv5-2)
# $7 : BatRoot processing configuration file
# $8 : BatRoot analysis configuration file
# $9 : BatCalib processing configuration file
# $10: BatCalib calibration configuration file 

#setup environement variables and define useful functions
[ -f jobsetup.sh ] && . jobsetup.sh

#parse command args
if [ $# -ne 10 ] ; then
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
batrootproccfg=$7
batrootanacfg=$8
batcalibproccfg=$9
batcalibanacfg=${10}


#define the files we'll be working with
rawfile=$(get_raw_filename $series $dumpnum)
rqfile=$(get_rq_filename $tag $series $dumpnum)
rrqfile=$(get_rrq_filename $tag $series $dumpnum)

echo "$(tstamp): Begin processing $rawfile..."
echo "Hostname: $HOSTNAME"

filterfile=$(get_noise_dir)/$(get_filter_filename $tag $series)

#copy/link the raw data from the main repository to scratch
[ -d $SCRATCHDIR ] || $MKDIR $SCRATCHDIR
try_realcopy $CDMS_RAWDATA/$runid/$series/$rawfile $SCRATCHDIR || exit $FAIL

######### Run br #################
export BATROOT_AUXFILES=$DATADIR/$series
#change the BATROOT_NOISEFILES to make do_processing read the output file of stage 1 from tmp directory instead of from output dir
export BATROOT_NOISEFILES=$SCRATCHDIR 
/bin/cp -f $filterfile $BATROOT_NOISEFILES

echo "$(tstamp): Starting BatRoot on file $rawfile"
batrootcmd="$CDMSBATSDIR/BUILD/bin/BatRoot $series $dumpnum $maxevents $batrootproccfg $batrootanacfg"
echo "BatRoot command is $batrootcmd"
$batrootcmd || exit $FAIL

echo "$(tstamp): BatRoot finished; starting BatCalib on $rqfile"
if [[ "${batcalibproccfg}" =~ "LibrarySim" ]] ; then 
	maxevents=$(grep MAX_EVENTS ${CDMSBATSDIR}/UserSettings/BatCalibSettings/processing/${batcalibproccfg} | awk '{print $4}');
fi
batcalibcmd="$CDMSBATSDIR/BUILD/bin/BatCalib $series $dumpnum $maxevents $batcalibproccfg $batcalibanacfg"
echo "BatCalib command is $batcalibcmd"
$batcalibcmd || exit $FAIL


############# copy and clean up #####################
echo "$(tstamp): BatCalib finished; copying to $DATADIR"
destdir="$(get_unmerged_dir $datatype $series)/"
copy_atomically $SCRATCHDIR/$rqfile $destdir || exit $FAIL
copy_atomically $SCRATCHDIR/$rrqfile $destdir || exit $FAIL

echo "$(tstamp): Cleaning up scratch directory"
rm -f $SCRATCHDIR/$rawfile
rm -f $SCRATCHDIR/$rqfile
rm -f $SCRATCHDIR/$rrqfile
rm -f $SCRATCHDIR/$filterfile

##################################

echo "$(tstamp): Finished processing $rawfile"

exit $SUCCESS



