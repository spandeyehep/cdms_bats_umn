#functions.sh
#This file contains functions common to CDMS processing scripts
#it should only be included from jobsetup.sh

MKDIR="`which mkdir` -p -m 775"
RSYNC="`which rsync`"
export RSYNC_PASSWORD="SoupR"
RSYNC_ARGS="-ah --copy-unsafe-links"

CPCMD="$RSYNC $RSYNC_ARGS"
if [ -z "$(echo $CDMS_RAWDATA | grep ':')" ] ; then
    #the raw data is on the same host as us, so just simlink everything
    CPCMD="/bin/ln -s"
fi

tstamp(){
    /bin/date "+%d %b %H:%M"
}

copy_atomically(){
    if [ ! -f $1 ] ; then
	echo "Error copying $1: file doesn't exist!" >&2
	return $ERROR
    fi
    dest="$2"
    if [ "${dest: -1}" == "/" ] ; then 
	dest="$2/$(basename $1)"
    fi
    /bin/cp -f $1 ${dest}.lock
    chmod ug+rw ${dest}.lock
    /bin/mv ${dest}.lock ${dest}
}

rsync_atomically(){
    if [ ! -f $1 ] ; then
        echo "Error copying $1: file doesn't exist!" >&2
        return $ERROR
    fi
    dest="$2"
    if [ "${dest: -1}" == "/" ] ; then
        dest="$2/$(basename $1)"
    fi
    rate="$3"
    /usr/bin/rsync --bwlimit=$3 $1 ${dest}.lock
    chmod ug+rw ${dest}.lock
    /bin/mv ${dest}.lock ${dest}
}

try_realcopy(){
#allow multiple tries in case something happens with the copy
#args are source,dest,maxtries
    MAXTRIES=50
    if [ $# -gt 2 ] ; then
        MAXTRIES=$3
    fi
    retval=0
    tries=0
    while [ $tries -lt $MAXTRIES ] ; do
        echo "$(tstamp): Attempting to copy $1 to $2 ..."
        /bin/cp -f $1 $2
        retval=$?
        if [ $retval -eq $SUCCESS ] ; then break ; fi;
        echo "$(tstamp): There was a problem with copying $1" >&2
        (( tries++ ))
        if [ $tries -lt $MAXTRIES ] ; then
            let "waittime = 60 + ($RANDOM % 60)"
            echo "Will try again in $waittime seconds"
            sleep $waittime
        fi

    done
    if [ $tries -ge $MAXTRIES ] ; then
        echo "$(tstamp): Unable to copy $1 ; giving up" >&2
    fi
    return $retval
}
try_copy(){
#allow multiple tries in case something happens with the copy
#args are source,dest,maxtries
    MAXTRIES=50
    if [ $# -gt 2 ] ; then
	MAXTRIES=$3
    fi
    retval=0
    tries=0
    while [ $tries -lt $MAXTRIES ] ; do
	echo "$(tstamp): Attempting to copy $1 to $2 ..."
	$CPCMD $1 $2
	retval=$?
	if [ $retval -eq $SUCCESS ] ; then break ; fi;
	echo "$(tstamp): There was a problem with copying $1" >&2
	(( tries++ ))
	if [ $tries -lt $MAXTRIES ] ; then
	    let "waittime = 60 + ($RANDOM % 60)"
	    echo "Will try again in $waittime seconds"
	    sleep $waittime
	fi
	
    done
    if [ $tries -ge $MAXTRIES ] ; then 
	echo "$(tstamp): Unable to copy $1 ; giving up" >&2
    fi
    return $retval
}

get_lockfilename(){
    #args are jobfile
    echo $1 | sed "s/\.job$/.lock/"
}

get_donefilename(){
    #args are jobfile
    echo $1 | sed 's/\.job$/.done/'
}

get_failfilename(){
    #args are jobfile
    echo $1 | sed 's/\.job/.fail/'
}

claim_job(){
    #try to lock a job file, since mv is always atomic
    #args are jobfile
    lockfile=$(get_lockfilename $1)
    mv $1 $lockfile 2>/dev/null
}

wait_for_job(){
    #wait for a job from an earlier stage to finish
    #single arg is filename
    jobfile=$1
    donefile=$(get_donefilename $jobfile)
    failfile=$(get_failfilename $jobfile)
    #first, check to see if the job is done
    while [ ! -f $donefile ] ; do
	#make sure there wasn't an abort
	if [ -f $ABORTFILE ] ; then
	    echo "$(tstamp): ABORT signalled by another process; Exiting" >&2
	    exit $ABORTCALLED
	fi
	#make sure the job didn't fail
	if [ -f $failfile ] ; then
	    echo "$(tstamp): Previous job (${jobfile}) failed; Exiting" >&2
	    exit $FAIL
	fi
	echo "$(tstamp): Waiting for job $jobfile to finish..."
	sleep 60
    done
    echo "$(tstamp): $jobfile has finished; we can continue now."
}


dmc_input(){
    # split environment variables on colons and break into arrays
    unset BATROOT_PULSELIBS
    IFS=':' read -a templatenames_array <<< "$BATROOT_TEMPLATENAMES"
    for templatename in "${templatenames_array[@]}"
    do
      IFS2=" " read -a targetfiles <<< $(cat ${WORKDIR}/pulsesim_input_filemap.txt | grep $templatename )
      lastnum=${#targetfiles[@]}
      # build filenames
      # check for integrity of selected pulse file
      # make a maximum of 100 attempts
      MAXTRIES=100
      THTRY=0
      NOFILES=1
      # minimum files size set to about 4.5MB
      MINSIZE=5000 
      while [ $THTRY -lt $MAXTRIES ] ; do
	  # get series and dump number from execution file
	  thser=`grep do_processing $lockfile | awk '{print $2}'`
	  thdump=`grep do_processing $lockfile | awk '{print $3}'`
	  # get total dump number for this sample
	  predump=`grep $thser "${WORKDIR}/sorted_dumpmap.txt" | awk '{print $2}'`
	  filenum=$((thdump + predump + THTRY -1))
	  fnum=$(expr $filenum % $lastnum)
	  file=${targetfiles[fnum]}
          if [ -e "$file" ] ; then
	      filesize=$(du $file | awk '{print $1;}')
	      if [ $filesize -gt $MINSIZE ] ; then
		  echo "Good libinput pulse file found: ${file}   |  size: $filesize"
		  scratch_target=$SCRATCHDIR"/pulsesim_input_files/libinput_"${templatename}"_"${fnum}".root"
		  NOFILES=0
		  break
	      else 
		  echo "Broken libinput pulse file found: ${file} | size $filesize"
		  NOFILES=1
	      fi
	  else
	      echo "Missing libinput pulse file: ${file}"
	      NOFILES=1
	  fi
	  THTRY=$[$THTRY+1]
      done

      if [ $NOFILES -eq 1 ] ; then
	  echo "ERROR! No libinput pulses found!"
	  exit $FAIL
      fi
      
      if [ -e $file ];
          then
          echo "Copying $file to $scratch_target"
            # use rsync with bandwidth limit of 3000KB/s. Should be okay for 500 jobs on fermigrid [AJA]
          rsync_atomically $file $scratch_target 3000 || exit $FAIL
      fi

      # add to pulse libs environment variable
      export BATROOT_PULSELIBS=$BATROOT_PULSELIBS:$scratch_target
    done
}

print_examplejobfile(){
    cat <<EOF
######### CDMS Auto Processing Job Configuration ##############

#email address for status notifications
EMAIL=$EMAIL

#ISGRID should be 'true' if jobs run under condor or 'false' otherwise
ISGRID=$ISGRID

#Number of worker processes to launch
NWORKERS=$NWORKERS

#Working directory (must be unique way to identify this job)
WORKDIR=$WORKDIR

#List of series and number of dumps to process
SERIESLIST=$SERIESLIST

#datatype (ba, bg, cf)
DATATYPE=$DATATYPE

#run label (e.g. R133)
RUNID=$RUNID

#cdmsbats installation directory
CDMSBATSDIR=$CDMSBATSDIR

#which dump file to run BatNoise on? must be 'first' or 'last'
NOISEDUMP=$NOISEDUMP

#cdmsbats configuration files
BATROOT_PROCCFG=$BATROOT_PROCCFG
BATROOT_ANACFG=$BATROOT_ANACFG
BATCALIB_PROCCFG=$BATCALIB_PROCCFG
BATCALIB_CALIBCFG=$BATCALIB_CALIBCFG
EOF

}
