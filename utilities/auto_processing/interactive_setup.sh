#!/bin/bash -e

#This script is intended to be run on fnpcsrv1.fnal.gov, to launch cdms 
#processing jobs. This script will create a formatted set of lockable
#items for grid daemons to handle, then launch the requested number of daemons.

#it takes as optional argument the name of the series list file. If not 
#provided, the user can select from the list in the submission folder

#setup paths
source jobsetup.sh

##################  query the user for configuration  ##################

#determine the runlist to use
if [ -n "$1" ] ; then
    SERIESLIST="$1"
else
    runlists=$(find submission -type f | sort | grep -v CVS | grep -v '.sh' |\
               sed 's/^submission\///g')
    echo "Select the runlist filename:"
    PS3="runlist: "
    select SERIESLIST in $runlists ; do
	[ -n "$SERIESLIST" ] && break
	echo "You must enter a valid number"
    done
    SERIESLIST=submission/$SERIESLIST
fi
if [ ! -f "$SERIESLIST" ] ; then
    echo "ERROR: Can't find series list file $SERIESLIST; aborting"
    exit 1
fi

 
#test to see if the working directory already exists 
[ -z "$CDMS_PROCESSINGDIR" ] && \
    export CDMS_PROCESSINGDIR="`pwd -P`/activejobs/"

TEMPWORKDIR="${CDMS_PROCESSINGDIR}/$(basename $SERIESLIST .txt)"
echo ""
echo "Enter a working directory name (default $TEMPWORKDIR):"
read WORKDIR
[ -z "$WORKDIR" ] && WORKDIR=$TEMPWORKDIR

echo ""
echo "Enter the datatype (ba,bg, or cf):"
read DATATYPE

echo ""
echo "Enter the run (e.g. R124,R133):"
read RUNID

#look for cdmsbats
if [ -z "$CDMSBATSDIR" ] ; then
    echo ""
    echo "Enter the directory where cdmsbats is located (CDMSBATSDIR):"
    read batdir
    export CDMSBATSDIR=$batdir
fi

echo ""
echo "Run BatNoise on first or last dump?"
select NOISEDUMP in first last ; do
    [ -n "$NOISEDUMP" ] && break
    echo "You must select a valid integer"
done

#determine the appropriate configuration files
echo ""
echo "Select the BatRoot processing cfg file:"
PS3="BatRoot processing cfg file: "
batrootcfgdir=$CDMSBATSDIR/UserSettings/BatRootSettings/processing
procfiles=$(find $batrootcfgdir -maxdepth 1 -type f -exec basename {} \; \
    | sort)
select BATROOT_PROCCFG in $procfiles ; do
    [ -n "$BATROOT_PROCCFG" ] && break
    echo "You must enter a valid integer"
done

echo ""
echo "Select the BatRoot analysis cfg file:"
PS3="BatRoot analysis cfg file: "
batrootcfgdir2=$CDMSBATSDIR/UserSettings/BatRootSettings/analysis
procfiles=$(find $batrootcfgdir2 -maxdepth 1 -type f -exec basename {} \; \
    | sort)
select BATROOT_ANACFG in $procfiles ; do
    [ -n "$BATROOT_ANACFG" ] && break
    echo "You must enter a valid integer"
done

echo ""
echo "Select the BatCalib processing cfg file:"
PS3="BatCalib processing cfg file: "
batcalibcfgdir=$CDMSBATSDIR/UserSettings/BatCalibSettings/processing
optfiles=$(find $batcalibcfgdir -maxdepth 1 -type f -exec basename {} \; \
    | sort)
select BATCALIB_PROCCFG in $optfiles ; do
    [ -n "$BATCALIB_PROCCFG" ] && break
    echo "You must enter a valid integer"
done

echo ""
echo "Select the BatCalib calibration cfg file:"
PS3="BatCalib calibration cfg file: "
batcalibcfgdir2=$CDMSBATSDIR/UserSettings/BatCalibSettings/calibration
optfiles=$(find $batcalibcfgdir2 -maxdepth 1 -type f -exec basename {} \; \
    | sort)
select BATCALIB_CALIBCFG in $optfiles ; do
    [ -n "$BATCALIB_CALIBCFG" ] && break
    echo "You must enter a valid integer"
done

echo ""
echo "Enter the number of job processes to launch: <200>"
read NWORKERS
[ -z "$NWORKERS" ] && NWORKERS=200

echo ""
echo "Is this a grid job? [y/n]:"
read ISGRID
if   [[ "${ISGRID:0:1}" == [Yy] ]] ; then ISGRID="true" 
elif [[ "${ISGRID:0:1}" == [Nn] ]] ; then ISGRID="false"
else
    echo "You must enter y or n"
    exit 1
fi
    
echo ""
echo "Enter your email address for status notifications:"
read EMAIL
    


##prepare to actually launch
export WORKDIR
export SERIESLIST
export DATATYPE
export RUNID
export NOISEDUMP
export CDMSBATSDIR
export BATROOT_PROCCFG
export BATROOT_ANACFG
export BATCALIB_PROCCFG
export BATCALIB_CALIBCFG
export NWORKERS
export ISGRID
export EMAIL

launchcmd="./launch_cdms_processing.sh"

echo ""
echo "Enter a filename to save job setup to (<enter> to skip):"
read savefile
if [ -n "$savefile" ] ; then
    print_examplejobfile >$savefile
    launchcmd="$launchcmd $savefile"
    echo "To rerun this job in the future, call "
    echo ">  $launchcmd"
fi

echo "Launch the job now [y/N]?"
read launchnow
if [[ "${launchnow:0:1}" == [Yy] ]] ; then 
    $launchcmd
fi

