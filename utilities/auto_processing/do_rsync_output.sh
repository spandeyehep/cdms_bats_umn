#!/bin/bash -e
#
# Arguments:
# $1 : runid
# $2 : tag
# $3 : datatype
# $4 : total number of series

if [ $# -ne 4 ] ; then
    echo "Incorrect number of arguments given" >&2
    echo "Usage: $0 <runid> <tag> <datatype> <nseries>" >&2
    exit 1
fi

runid=$1
tag=$2
datatype=$3
nseries=$4

#set up environemnt
[ -f paths.sh ] && . paths.sh

echo "$(tstamp): Starting data copy into release $tag..." 

#first, wait until all series have finished merging 
unmerged_tld=$(get_unmerged_dir $datatype)
allseries=$(ls $unmerged_tld)
if [ $nseries -ne $(echo "$allseries" | wc -w) ] ; then
    echo "ERROR: Incorrect number of series folders in $unmerged_tld" >&2
    echo "Aborting copy" >&2
    exit 2
fi

mergedir=$(get_merged_all_dir $datatype)
for series in $allseries ; do 
    while [ ! -f $mergedir/$(get_merged_rq_filename $tag $series) ] || \
	  [ ! -f $mergedir/$(get_merged_rrq_filename $tag $series) ]  ; do
	sleep 10
	if [ -f $ABORTFILE ] ; then
	    echo "$(tstamp): ABORT signalled by another process! Exiting" >&2
	    exit 10
	fi
    done
	
done

echo "$(tstamp): Creating 'byseries' links..."

newtype=$datatype
if [ "$datatype" == "bg" ] ; then
    newtype="bg_restricted"
    echo "Renaming $datatype to $newtype for data copy..."
    $typedirs=$(find $OUTPUTDIR -name "$datatype" -type d)
    for dir in $typedirs ; do 
	newdir=$(echo "$dir" | sed "s#/$datatype#$newtype#g")
	chmod 700 $dir
	mv $dir $newdir
    done
fi

mergedir=$(get_merged_all_dir $newtype)
for series in $allseries ; do
    ln -s ../../../../$mergedir/$(get_merged_rq_filenam $tag $series) \
	$(get_merged_byseries_dir $newtype $series)/
    ln -s ../../../../$mergedir/$(get_merged_rrq_filenam $tag $series) \
	$(get_merged_byseries_dir $newtype $series)/
done
dest="$CDMS_RQDATA/$runid/dataReleases/$tag"
echo "Rsyncing $OUTPUTDIR directory tree to $dest"
logfile="$LOGDIR/rsync_output.log"
$RSYNC $RSYNC_ARGS ${OUTPUTDIR}/ ${dest}/ &> $logfile

echo "$(tstamp): Done syncing processed data."
echo "$(tstamp): Returning all permissions to 775"
chmod -R 775 . 


    