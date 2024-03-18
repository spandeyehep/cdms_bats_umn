#!/bin/bash
#
# Arguments:
# $1 : tag
# $2 : datatype
# $3 : series
# $4 : total number of dumps

###### Configuration Setup #######
[ -f jobsetup.sh ] && . jobsetup.sh

if [ $# -ne 4 ] ; then
    echo "Incorrect number of arguments given:"
    echo "Usage: $0 <tag> <datatype> <series> <ndumps>"
    exit $ARGERROR
fi

tag=$1
datatype=$2
series=$3
ndumps=$4


echo "$(tstamp): Starting merge of series $series"
echo "Hostname: $HOSTNAME"

#make sure we don't share scratch space
$MKDIR $SCRATCHDIR/$series

rqdir=$(get_unmerged_dir $datatype $series)
#simlinks may need to be absolute paths
if [ "${rqdir:0:1}" != "/" ] ; then
    rqdir=`pwd -P`/$rqdir
fi

rqfiles=""
rrqfiles=""

for dumpnum in `seq 1 $ndumps` ; do
    rqfile=$(get_rq_filename $tag $series $dumpnum)
    rrqfile=$(get_rrq_filename $tag $series $dumpnum)
    LN="/bin/ln -s"
    CP="/bin/cp -f"
    #don't do the merging from an NFS mount...
    if [ -n "$_CONDOR_SCRATCH_DIR" ] ; then 
	LN="/bin/cp"
    fi
    $LN $rqdir/$rqfile $SCRATCHDIR/$series/$rqfile
    $LN $rqdir/$rrqfile $SCRATCHDIR/$series/$rrqfile
    rqfiles="$rqfiles $SCRATCHDIR/$series/$rqfile"
    rrqfiles="$rrqfiles $SCRATCHDIR/$series/$rrqfile"
done

#create the merge executable
#merging requires the merge executable
if [ "${HOSTNAME: -15}" == "brazos.tamu.edu" ] ; then
    ROOTCONFIG=root-config
else
    ROOTCONFIG=$ROOTSYS/bin/root-config
fi

MERGE=$SCRATCHDIR/$series/merge_filelist
g++ -o $MERGE merge_filelist.C `$ROOTCONFIG --cflags --libs`
if [ $? -ne 0 ] || [ ! -x $MERGE ] ; then 
    echo "There was an error compiling merge_filelist.C! Exiting" >&2
    exit $FAIL
fi

#do the merging
echo "$(tstamp): Merging RQ files..."
merge_rqfile=$SCRATCHDIR/$series/$(get_merged_rq_filename $tag $series)
$MERGE $merge_rqfile monobasket $rqfiles 
if [ $? -ne 0 ] || [ ! -f $merge_rqfile ] ; then
    echo "An error occurred with the root command. See log for errors." >&2
    exit $FAIL
fi

echo "$(tstamp): Merging RRQ files..."
merge_rrqfile=$SCRATCHDIR/$series/$(get_merged_rrq_filename $tag $series)
$MERGE $merge_rrqfile monobasket $rrqfiles 
if [ $? -ne 0 ] || [ ! -f $merge_rrqfile ] ; then
    echo "An error occurred with the root command. See log for errors." >&2
    exit $FAIL
fi

#copy the output 
echo "$(tstamp): Done merging. Copying back to $OUTDIR ..."
copy_atomically $merge_rqfile $(get_merged_all_dir $datatype)/ || exit $FAIL 
copy_atomically $merge_rrqfile $(get_merged_all_dir $datatype)/ || exit $FAIL

#create the byseries links
byseriesdir=$(get_merged_byseries_dir $datatype $series)
[ -d $byseriesdir ] || $MKDIR $byseriesdir
/bin/ln -s ../../../all/$datatype/$(get_merged_rq_filename $tag $series) $byseriesdir/
/bin/ln -s ../../../all/$datatype/$(get_merged_rrq_filename $tag $series) $byseriesdir/

#clean up
echo "$(tstamp): Done copying, cleaning up scratch directory..."
rm -rf $SCRATCHDIR/$series

echo "$(tstamp): Finished merging series $series"
exit $SUCCESS
