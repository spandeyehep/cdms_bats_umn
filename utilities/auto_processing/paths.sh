#!/bin/bash

#exit codes
export ABORTCALLED=-1
export SUCCESS=0
export ERROR=1
export FAIL=2
export ABORT=3
export ARGERROR=4
export CONFIGERROR=5

#grid-specific setup
[ -f /usr/local/etc/setups.sh ] && . /usr/local/etc/setups.sh
[ -f /local/ups/grid/setup.sh ] && . /local/ups/grid/setup.sh

export HOSTNAME=`hostname`

#CDMS data repositories
if [ -z "$CDMS_RAWDATA" ] ; then
    export CDMS_RAWDATA="rsync://cdms@cdmsmicro.fnal.gov"
    if [ "$HOSTNAME" == "cdmsmicro.fnal.gov" ] ; then
	export CDMS_RAWDATA="/localhome/cdms/rawdata"
    fi
fi
#if [ -z "$CDMS_RQDATA" ] ; then
#    export CDMS_RQDATA="rsync://processing@cdmsmicro.fnal.gov:2222/"
#fi

LOGDIR="logs"
DATADIR="raw"
SCRATCHDIR="scratch"
OUTPUTDIR="output"
ABORTFILE="ABORT"

#  HTCondor in brazos, combined with SLURM doesn't have 
#  a CONDOR_SCRATCH DIR we need to parse the TMPdir
if [ "${HOSTNAME: -15}" == "brazos.tamu.edu" ] ; then
    [ -z "$_CONDOR_SCRATCH_DIR" ] && [ -n "$TMPDIR" ] && export _CONDOR_SCRATCH_DIR=$TMPDIR   
fi

#scratch directories for grid processing
if [ -n "$_CONDOR_SCRATCH_DIR" ] ; then
    SCRATCHDIR="$_CONDOR_SCRATCH_DIR"
fi


get_unmerged_dir(){
    #args are type, series
    echo "$OUTPUTDIR/unmerged/$1/$2"
}

get_merged_all_dir(){
    #args are type
    echo "$OUTPUTDIR/submerged/all/$1"
}

get_merged_byseries_dir(){
    #args are type, series
    echo "$OUTPUTDIR/submerged/byseries/$1/$2"
}

get_noise_dir(){
    #no args
    echo "$OUTPUTDIR/noise/ROOT"
}

get_dump_suffix(){
    #arg is dumpnum
    suffix=$1
    while [ ${#suffix} -lt 4 ] ; do suffix="0$suffix" ; done
    echo "F$suffix"
}

get_raw_filename(){
    #args are series, dumpnum
    echo "${1}_$(get_dump_suffix $2).gz"
}

get_rq_filename(){
    #args are tag, series, dumpnum
    echo "${1}_${2}_$(get_dump_suffix $3).root"
}

get_rrq_filename(){
    #args are tag, series, dumpnum
    echo "calib_$(get_rq_filename $@)"
}

get_filter_filename(){
    #args are tag, series
    echo "${1}_Filter_${2}.root"
}
get_merged_rq_filename(){
    #args are tag, series
    echo "merge_${1}_${2}.root"
}

get_merged_rrq_filename(){
    #args are tag, series
    echo "calib_${1}_${2}.root"
}


#Working directories
#export CDMS_SCRIPTDIR="`pwd`"
export CDMS_STAGE1SCRIPT="do_noise.sh"
export CDMS_STAGE2SCRIPT="do_processing.sh"
export CDMS_STAGE3SCRIPT="do_merge.sh"
export CDMS_STAGE4SCRIPT="do_rsync_output.sh"


#ROOT
FERMIGRID5SYS="/grid/fermiapp/cdms/processing_source/root_v5.34.05"
FERMIGRID6SYS="/grid/fermiapp/cdms/processing_source/root_v5.34.05_slf6"

if [ -z "$ROOTSYS" ] ; then
    #try common locations
    if [ "$HOSTNAME" == "cdmsmicro.fnal.gov" ] ; then
	export ROOTSYS="/usr/local/cern/root_v5.27_source"
    elif [[ "${HOSTNAME:0:4}" == "fnpc" || "${HOSTNAME:0:4}" == "fcdf" ]] && \
	 [ "${HOSTNAME: -9}" == ".fnal.gov" ] ; then
	export ROOTSYS=$FERMIGRID5SYS
    elif [ "${HOSTNAME: -13}" == ".stanford.edu" ] ; then
	export ROOTSYS="/soft/root"
    fi
fi

if [ "$ROOTSYS" == "$FERMIGRID5SYS" ] && [ -z "`/sbin/ldconfig -p | grep mysqlclient.so.15`" ] ; then export ROOTSYS=$FERMIGRID6SYS ; fi


[ -f $ROOTSYS/bin/thisroot.sh ] && . $ROOTSYS/bin/thisroot.sh

if [ "${HOSTNAME: -15}" == "brazos.tamu.edu" ] ; then
    # ROOT with miniconda doesn't need ROOTSYS (cdmsbats needs to be compiled accordingly)
    export PATH=/home/hepxadmin/miniconda/bin:$PATH
    export LD_LIBRARY_PATH=/home/hepxadmin/miniconda/lib:$LD_LIBRARY_PATH
else
    export PATH=$ROOTSYS/bin:$PATH
    export LD_LIBRARY_PATH=$ROOTSYS/lib:$LD_LIBRARY_PATH
fi

######   CDMSBATS specific stuff  $$$$$$$$
#first look for a simlink in the current directory
if [ -z "$CDMSBATSDIR" ] ; then
    if [ -d "cdmsbats" ] ; then
	export CDMSBATSDIR=`pwd -P`/cdmsbats
    elif [ -f ../../BUILD/bin/BatRoot ] ; then
	export CDMSBATSDIR=`pwd -P`/../..
    fi
fi
#only reset some variables if they're not already defined
if [ -z "$BATNOISE_TEMPLATES" ] ; then
    export BATNOISE_TEMPLATES=$CDMSBATSDIR/PulseTemplates
fi
#gpib will always be copied/linked to DATADIR
export BATROOT_GPIBFILES=$DATADIR
#aux files will always be copied/linked to DATADIR/series, so can't export!

#raw data will always be copied/linked to scratch
export BATROOT_RAWDATA=$SCRATCHDIR
#output will usually be placed in scratch
export BATROOT_RQDATA=$SCRATCHDIR
export BATCALIB_RQDATA=$SCRATCHDIR
export BATCALIB_RRQDATA=$SCRATCHDIR
#noisefiles will usually be in the output directory
export BATROOT_NOISEFILES=$(get_noise_dir)


# Create environment variables for simulation [AJA]
if [ -z "$BATROOT_ENERGYINPUTDIR" ] ; then
    # directory should be created in launch_cdms_processing.sh if
    # pulse simulation is enabled
    if [ -e "$WORKDIR/energy_input_files" ] ; then
        export BATROOT_ENERGYINPUTDIR=$WORKDIR/energy_input_files

	if [ ! -d $SCRATCHDIR/pulsesim_input_files ] ; then
	    mkdir $SCRATCHDIR/pulsesim_input_files
	fi
    fi
fi
