#!/bin/bash -e

#This script is intended to be run on fnpcsrv1.fnal.gov, to launch cdms 
#processing jobs. This script will create a formatted set of lockable
#items for grid daemons to handle, then launch the requested number of daemons.

#it takes as optional input a job submission file that defines all of the
#necessary attributes as environment variables. If any are not defined,
#an example will be printed to the terminal

#setup paths
source jobsetup.sh

#function to launch child jobs
launch_child_jobs(){
    #the function expects 1 argument: the working directory
    if [ $# -eq 0 ] ; then
	echo "ERROR: launch_child_jobs() requires the working directory!"
	exit 1
    fi
    WORKDIR=$1
    njobs=$(find $WORKDIR -name "*.job" | wc -w)
    echo "There are $njobs total jobs to process in $WORKDIR"

    #Partition option is likely only applicable to Texas A&M Brazos
    PARTITION=$2
    if [ "${HOSTNAME: -15}" == "brazos.tamu.edu" ] ; then
	echo "Resource manager is ce01.brazos.tamu.edu"
	if [[ "$PARTITION" != "" ]] ; then 
	    echo "     Selected partition to run jobs: ${PARTITION}"
	else
	    PARTITION=stakeholder
	    echo "     Default partition to run jobs: ${PARTITION}"
	fi
	UNIVERSE=grid
	GRIDRESOURCE="condor ce01.brazos.tamu.edu ce01.brazos.tamu.edu:9619"
	GLOBUSRSL="+remote_queue = \"$PARTITION\""
	GLOBUSRSL2='+remote_JobName = "CDMSBats"'
	#+maxMemory = 600
	#+maxWallTime = 15 
	USEPROXY="use_x509userproxy = true"
    else
	echo "Resource manager is gt2.fnpcosg1.fnal.gov"
	UNIVERSE=globus
	GRIDRESOURCE="gt2 fnpcosg1.fnal.gov/jobmanager-condor "
	GLOBUSRSL=" (jobtype=single)(maxwalltime=999)"
	USEPROXY=" "
    fi
    
    if [ "$ISGRID" == "true" ] ; then
	#create the condor submission file
	$MKDIR $WORKDIR/$LOGDIR/grid
	condor_jobfile=$WORKDIR/submit_job.condor
	cat <<-EOF >$condor_jobfile
	#Condor submission description file for CDMS processing job
	#automatically generated on $(date) by launch_cdms_processing.sh

	universe = $UNIVERSE
	GridResource = $GRIDRESOURCE 

	#getenv = true
	transfer_output = true
	transfer_error = true
	transfer_executable = true	
	notification = NEVER

	log = $WORKDIR/$LOGDIR/grid/\$(Cluster).\$(Process).log
	output = $WORKDIR/$LOGDIR/grid/\$(Cluster).\$(Process).out
	error = $WORKDIR/$LOGDIR/grid/\$(Cluster).\$(Process).err

	# Pass names of template libraries for pulse simulation as
	# environment variable to condor [AJA]
	environment = "BATROOT_TEMPLATENAMES=$BATROOT_TEMPLATENAMES CDMS_RAWDATA=$CDMS_RAWDATA"

	executable = $(pwd -P)/processing_child.sh
	initialdir = $WORKDIR
	arguments = "\$(Process) $WORKDIR $EMAIL"

	$GLOBUSRSL
	$GLOBUSRSL2

	$USEPROXY

	queue $NWORKERS
	EOF
        echo "Submitting condor job $condor_jobfile..."
	condor_submit $condor_jobfile
	echo "Done submitting jobs"
    else
	echo "Launching worker threads..."
	$MKDIR $WORKDIR/$LOGDIR
	for child_id in `seq 1 $NWORKERS` ; do
	    size=${#NWORKERS}
	    while [ ${#child_id} -lt $size ] ; do child_id="0$child_id" ; done
	    LOGFILE=${WORKDIR}/${LOGDIR}/${child_id}.log
	    ERRFILE=${WORKDIR}/${LOGDIR}/${child_id}.err
	    nohup ./processing_child.sh $child_id $WORKDIR $EMAIL \
		>$LOGFILE 2>$ERRFILE &
	done
    fi

}

function print_usage()
{
    cat<<-EOF

	Script usage: launch_cdms_processing.sh [<jobfile>]
	The jobfile defines all of the necessary information to launch 
	a cdmsbats job that will process a set of series. ALL of the varibles
	below are required 
	(though they can be defined as environemnt variables),
	except EMAIL, though it is recommended to get status notifications.
	All directory/file names can be relative or absolute paths

	Example input file:

	EOF
    print_examplejobfile
    exit $ARGERROR
}


##################  Make sure variables are properly defined  ##################

[ $# -gt 0 ] && source $1

PARTITION=$2

[ -n "$WORKDIR" ]            || print_usage
[ -n "$SERIESLIST" ]         || print_usage
[ -n "$DATATYPE" ]           || print_usage
[ -n "$RUNID" ]              || print_usage
[ -n "$CDMSBATSDIR" ]        || print_usage
[ -n "$BATROOT_PROCCFG" ]    || print_usage
[ -n "$BATROOT_ANACFG" ]     || print_usage
[ -n "$BATCALIB_PROCCFG" ]   || print_usage
[ -n "$BATCALIB_CALIBCFG" ]  || print_usage
[ -n "$NWORKERS" ]           || print_usage
[ -n "$ISGRID" ]             || print_usage
[ -n "$NOISEDUMP" ]          || print_usage

if [ "$ISGRID" != "true" ] && [ "$ISGRID" != "false" ] ; then
    echo "ERROR: ISGRID should be either 'true' or 'false'" >&2
    print_usage
fi

if [ "$NOISEDUMP" != "first" ] && [ "$NOISEDUMP" != "last" ] ; then
    echo "ERROR: NOISEDUMP must be either 'first' or 'last'" >&2
    print_usage
fi

if [ ! $NWORKERS -gt 0 ] ; then
    echo "ERROR: NWORKERS must be an integer >0" >&2
    print_usage
fi

if [ ! -f "$SERIESLIST" ] ; then
    echo "ERROR: Unable to find series list file $SERIESLIST" >&2
    print_usage
fi

if [ "$DATATYPE" != "ba" ] && [ "$DATATYPE" != "bg" ] \
    && [ "$DATATYPE" != "cf" ] ; then 
    echo "ERROR: DATATYPE must be one of 'ba','bg',or 'cf'" >&2
    print_usage
fi

########## check the cdmsbats directory ##########
if [ ! -d "$CDMSBATSDIR" ] ; then
    echo "ERROR: $CDMSBATSDIR is not a readable directory" >&2
    print_usage
fi
#make sure the executables we need are there
if  [ ! -x "$CDMSBATSDIR/BUILD/bin/BatRoot" ] || \
    [ ! -x "$CDMSBATSDIR/BUILD/bin/BatNoise" ] || \
    [ ! -x "$CDMSBATSDIR/BUILD/bin/BatCalib" ] ; then
    echo "ERROR: Executables are missing from your cdmsbats directory" >&2
    print_usage
fi
#check all the configuration files
batrootcfgdir=$CDMSBATSDIR/UserSettings/BatRootSettings/processing
batrootcfgdir2=$CDMSBATSDIR/UserSettings/BatRootSettings/analysis
batcalibcfgdir=$CDMSBATSDIR/UserSettings/BatCalibSettings/processing
batcalibcfgdir2=$CDMSBATSDIR/UserSettings/BatCalibSettings/calibration
if [ ! -f $batrootcfgdir/$BATROOT_PROCCFG ] ; then
    echo "ERROR: BatRoot processing options file $BATROOT_PROCCFG doesn't exist" >&2
    print_usage
fi
if [ ! -f $batrootcfgdir2/$BATROOT_ANACFG ] ; then
    echo "ERROR: BatRoot analysis options file $BATROOT_ANACFG doesn't exist" >&2
    print_usage
fi
if [ ! -f $batcalibcfgdir/$BATCALIB_PROCCFG ] ; then
    echo "ERROR: BatCalib processing options file $BATCALIB_PROCCFG doesn't exist" >&2
    print_usage
fi
if [ ! -f $batcalibcfgdir2/$BATCALIB_CALIBCFG ] ; then
    echo "ERROR: BatCalib calibraiton options file $BATCALIB_CALIBCFG doesn't exist" >&2
    print_usage
fi

#make sure the filename settings agree
prefix=$(grep -w "RQ_DATA_PREFIX" $batrootcfgdir/$BATROOT_PROCCFG | \
    awk '{print $4}')
filterprefix=$(grep -w "NOISE_PREFIX" $batrootcfgdir/$BATROOT_PROCCFG | \
    awk '{print $4}')
calibprefix=$(grep -w "RQ_DATA_PREFIX" $batcalibcfgdir/$BATCALIB_PROCCFG | \
    awk '{print $4}')
calibrrqprefix=$(grep -w "RRQ_DATA_PREFIX" $batcalibcfgdir/$BATCALIB_PROCCFG | \
    awk '{print $4}')
#for some stupid reason, the calibprefix does NOT end in '_' ...
if [ "${prefix: -1}" != "_" ] ; then 
    echo "ERROR: RQ_DATA_PREFIX in $BATROOT_PROCCFG must end in '_' " >&2
    print_usage
fi
if [ "$filterprefix" != "${prefix}Filter_" ] ; then
    echo "ERROR: NOISE_PREFIX must be equal to \$\{RQ_DATA_PREFIX\}Filter_" >&2
    print_usage
fi
if [ "${calibprefix}_" != "$prefix" ] ; then 
    echo "ERROR: RQ_DATA_PREFIX in $BATCALIB_PROCCFG should match "\
	"$BATROOT_PROCCFG (minus the trailing '_')" >&2
    print_usage
fi
if [ "$calibrrqprefix" != "calib_$calibprefix" ] ; then
    "ERROR: RRQ_DATA_PREFIX must be calib_\$RQ_DATA_PREFIX" >&2
    print_usage
fi



######################### Set up the working directory and launch #####################
tag="$calibprefix"
echo "The tag for this batch is $tag"

#make workdir an absolute path
[ "${WORKDIR:0:1}" != "/" ] && WORKDIR=$(pwd -P)/$WORKDIR

if [ -d "$WORKDIR" ] ; then
    echo ""
    echo "$WORKDIR already exists! Make sure that no previous job is running!"
    echo "If the previous job was aborted, you can try to resume it, or "
    echo "you can start the job over from scratch (ALL PROGRESS WILL BE LOST!)"
    PS3="how to proceed? "
    select choice in "Attempt to resume" "Restart from scratch" "Quit" ; do
	case $choice in 
	    "Attempt to resume" )
		./prepare_resume.sh $WORKDIR
		launch_child_jobs $WORKDIR $PARTITION
		echo "Done setting up, exiting..."
		exit 0 ;;
	    "Restart from scratch" )
		echo "Removing old working directory $WORKDIR..."
		rm -rf $WORKDIR
		break ;;
	    "Quit" )
		echo "Exiting..."
		exit $ARGERROR ;;
	    *)
		echo "You must select a valid number!";;
	esac
    done
fi

#create an output directory to hold everything
echo "All job scripts, logs, and output will be placed in $WORKDIR"
echo "Are you sure this location is correct? [y/N]"
read confirm
if [ "${confirm:0:1}" != "Y" ] && [ "${confirm:0:1}" != "y" ] ; then
    echo "Fix the working dir and start again"
    exit $ARGERROR
fi  
echo "Preparing working directory..."

$MKDIR $WORKDIR
cp paths.sh functions.sh jobsetup.sh jobstatus.py $WORKDIR/
/bin/ln -s $CDMSBATSDIR $WORKDIR/cdmsbats

# symlink directory with energies for pulse simulation,
# using environment variable BATROOT_ENERGYINPUTDIR [AJA]
if [ -n "$BATROOT_ENERGYINPUTDIR" ] ; then
    /bin/ln -s $BATROOT_ENERGYINPUTDIR $WORKDIR/energy_input_files
fi

# symlink directory with ROOT template files for pulse simulation
# using environment variable BATROOT_TEMPLATEINPUTDIR [AJA]
if [ -n "$BATROOT_TEMPLATEINPUTDIR" ] ; then
    /bin/ln -s $BATROOT_TEMPLATEINPUTDIR $WORKDIR/pulsesim_input_files
    #write sorted series list (for matching the input pulses with the dumps in order)
    dumpmap=$WORKDIR/sorted_dumpmap.txt
    sort -n $SERIESLIST > $dumpmap
    serlist=( `awk '{print $1}' ${dumpmap}` 'TOTAL' )
    dumplist=( 0 `awk '{print $2}' ${dumpmap}` ) 
    rm $dumpmap
    dumpsum=0
    for ii in `seq 0 $((${#serlist[@]}-1))` ; do 
	dumpsum=$((dumpsum + dumplist[$ii]))
	echo "${serlist[$ii]}    $dumpsum" >> $dumpmap 
    done
    echo "SAVED DUMPMAP for ordering DMC Pulse files with dumps ${dumpmap}"
    echo "creating pulsesim input filemap"
    # now write the dmc pulsefiles list which will be read by 
    # the dmc_input function to determine which pulse file to use (at runtime)
    targetfile=${WORKDIR}/pulsesim_input_filemap.txt
    ls ${WORKDIR}/pulsesim_input_files/libinput_*.root | sort -V > $targetfile
    echo "${targetfile}"

    
fi


$MKDIR $WORKDIR/$LOGDIR
$MKDIR $WORKDIR/$DATADIR
$MKDIR $WORKDIR/$SCRATCHDIR
$MKDIR $WORKDIR/$OUTPUTDIR
$MKDIR $WORKDIR/$(get_noise_dir)
$MKDIR $WORKDIR/$(get_merged_all_dir $DATATYPE)

echo "Copying/linking gpib_states_changed.log file to working directory..."
try_copy $CDMS_RAWDATA/$RUNID/gpib_states_changed.log $WORKDIR/$DATADIR/ || \
    exit $ABORT

#create directories to populate the "job" scripts with
$MKDIR $WORKDIR/stage1
cp $CDMS_STAGE1SCRIPT $WORKDIR/
$MKDIR $WORKDIR/stage2
cp $CDMS_STAGE2SCRIPT $WORKDIR/
$MKDIR $WORKDIR/stage3
cp $CDMS_STAGE3SCRIPT $WORKDIR/
#$MKDIR $WORKDIR/stage4
#cp $CDMS_STAGE4SCRIPT $WORKDIR/

cp merge_filelist.C $WORKDIR/

########## read the series line by line from the runlist and set up jobs ##########
while read series ndumps ; do
    #prevent errors with empty lines
    if [ -z "$series" ] ; then break; fi
    if [ -z "$ndumps" ] || [ $ndumps -eq 0 ] ; then
	echo "No dump files found for series $series. Aborting" >&2
	exit $ABORT
    fi
    echo "$ndumps dump files will be processed for series $series"
    #populate the output directory hierarchy
    $MKDIR $WORKDIR/$(get_unmerged_dir $DATATYPE $series)
    $MKDIR $WORKDIR/$(get_merged_byseries_dir $DATATYPE $series)
   
    #the "job" files are sub-scripts called by the child processes
    #each one should wait on the previous ones first
    waitonjobs=""
    #stage 1 is downloading dump 1 and aux files, then running noise generation
    jobfile1="stage1/$series.job"
    
    nf=1
    if [ "$NOISEDUMP" == "last" ] ; then
	nf=$ndumps
    fi

    echo -e "./$CDMS_STAGE1SCRIPT $series $nf 500 $DATATYPE $RUNID $tag "\
	"$BATROOT_PROCCFG $BATROOT_ANACFG" \
	>$WORKDIR/$jobfile1
    chmod 775 $WORKDIR/$jobfile1
        
    #job3 has to wait on all of job 2
    jobfile3="stage3/$series.job"
    echo "source jobsetup.sh" >$WORKDIR/$jobfile3

    #stage 2 is downloading each dump file then processing it
    if [ -n "$BATROOT_TEMPLATEINPUTDIR" ] ; then
	MAXEVENTS=$(grep MAX_EVENTS ${CDMSBATSDIR}/UserSettings/BatRootSettings/processing/${BATROOT_PROCCFG} | awk '{print $4}');
    else
    MAXEVENTS=10000 #normally 10000
    fi
    for dumpnum in `seq 1 $ndumps` ; do
	jobfile2="stage2/${series}_$(get_dump_suffix $dumpnum).job"
	echo "source jobsetup.sh" >$WORKDIR/$jobfile2
	
	echo "wait_for_job $jobfile1 || exit $FAIL" >>$WORKDIR/$jobfile2
	echo -e "./$CDMS_STAGE2SCRIPT $series $dumpnum $MAXEVENTS $DATATYPE $RUNID $tag "\
	    "$BATROOT_PROCCFG  $BATROOT_ANACFG $BATCALIB_PROCCFG $BATCALIB_CALIBCFG" \
	    >>$WORKDIR/$jobfile2
	chmod 775 $WORKDIR/$jobfile2
	echo "wait_for_job $jobfile2 || exit $FAIL" >>$WORKDIR/$jobfile3
    done
    
    #stage 3 is merging the output
    echo -e "./$CDMS_STAGE3SCRIPT $tag $DATATYPE $series $ndumps" >>$WORKDIR/$jobfile3
    chmod 775 $WORKDIR/$jobfile3
done < $SERIESLIST

#stage 4 is rsyncing the processed output back to the master location
#jobfile="$WORKDIR/stage4/rsync.job"
#nseries=$(echo "$allseries" | wc -w )
#echo -e "$RUNID $tag $DATATYPE $nseries" >$jobfile
#chmod 775 $jobfile    

#make sure all permissions are set with g+w in case of grid job
chmod -R 775 $WORKDIR

#the working directory is all set up, now launch the child processes
launch_child_jobs $WORKDIR $PARTITION
echo "Done with setup and thread launching! Exiting..."
exit 0

