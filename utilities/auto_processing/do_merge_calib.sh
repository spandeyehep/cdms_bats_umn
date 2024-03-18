#!/bin/bash -e
#
# Arguments:
# $1 : runid
# $2 : tag
# $3 : datatype
# $4 : series
# $5 : total number of dumps
# $6 : BatCalib cfg file

if [ $# -ne 6 ] ; then
    echo "Incorrect number of arguments given"
    echo "Usage: $0 <runid> <tag> <datatype> <series> <ndumps> <BatCalib cfg>"
    exit 1
fi

runid=$1
tag=$2
datatype=$3
series=$4
ndumps=$5
batcalibcfg=$6

###### Configuration Setup #######
[ -f paths.sh ] && . paths.sh

echo "$(tstamp): Starting merge of series $series"
echo "Hostname: $HOSTNAME"

#make sure we don't share scratch space
$MKDIR $SCRATCHDIR/$series

#wait until all dumps are present before merging
echo "$(tstamp): Waiting for all dumps to finish processing (ctrl-c to abort)"
rqdir=$(get_unmerged_dir $datatype $series)
#simlinks may need to be absolute paths
if [ "${rqdir:0:1}" != "/" ] ; then
    rqdir=`pwd -P`/$rqdir
fi

for dumpnum in `seq 1 $ndumps` ; do
    rqfile=$(get_rq_filename $tag $series $dumpnum)
    while [ ! -f $rqdir/$rqfile ] ; do 
	sleep 60 
    done
    /bin/ln -s $rqdir/$rqfile $SCRATCHDIR/$series/$rqfile
done

#do the merging
echo "$(tstamp): Merging series $series..."
root -b -q "merge_all.C(\"$SCRATCHDIR/$series\",\"$tag\",\"$series\",\"runid\",\"$datatype\",$ndumps)" 

#run batcalib on the merged data
echo "$(tstamp): Done merging series $series. Calibrating..."
export BATROOT_AUXFILES=$DATADIR/$series
export BATCALIB_RQFILES=$SCRATCHDIR/$series
export BATCALIB_RRQFILES=$SCRATCHDIR/$series
logfile="$LOGDIR/$series.bc.log"
$CDMSBATSDIR/BUILD/bin/BatCalib $series merged 1001000 $batcalibcfg >& $logfile

echo "$(tstamp): Finished merging and calibrating series $series, copying"
copy_atomically $SCRATCHDIR/$series/$(get_merged_rq_filename $tag $series) $(get_merged_all_dir $datatype)/
copy_atomically $SCRATCHDIR/$series/$(get_merged_rrq_filename $tag $series) $(get_merged_all_dir $datatype)/

#cleanup
#rm -rf is dangerous, so take some precautions here
if [ -n "$SCRATCHDIR" ] && [ -d "$SCRATCHDIR/$series" ] ; then
    /bin/rm -rf $SCRATCHDIR/$series
fi

echo "$(tstamp) Finished with merge_calib on series $series"
exit 0