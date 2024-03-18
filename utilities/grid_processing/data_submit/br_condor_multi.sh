#!/bin/bash

#for submission of multipled files (nchunk files) to be processed per condor job.  This is more efficient for fast processing, but not useful for cleanup.
#usage: br_condor.sh series maxdumps nchunk datatype(ba, cf or bg) run

### Check expected arguments ###

EXPECTED_ARGS=5

if [ $# -ne $EXPECTED_ARGS ]
then
  echo "Wrong number of arguments! usage: br_condor.sh series maxdumps nchunk datatype(ba, cf or bg) run";
  exit;
fi

### Compute some needed values ###

MAXITER=$(( $2 / $3 ));
MAXITER=$(($MAXITER-1));
#echo $MAXITER;

MODMAX=$(( $2 % $3 ));
#echo $MODMAX;

### submit the jobs ###

#submit jobs in groups of nchunk
for i in `seq 0 $MAXITER` ; do STARTDUMP=$(( ($i * $3) + 1 )); condor_submit -a "Arguments=$1 500 $STARTDUMP $3 $4 $5" br_condor_multi; echo "submitting dumps $STARTDUMP to $(( $STARTDUMP + ($3 - 1) ))"; done

#submit the remaining jobs as the last chunk
if [ $MODMAX -ne 0 ]
then
STARTDUMP=$(( (($MAXITER+1) * $3) + 1 ));
condor_submit -a "Arguments=$1 500 $STARTDUMP $MODMAX $4 $5" br_condor_multi;
echo "submitting dumps $STARTDUMP to $(($STARTDUMP + MODMAX - 1))"; 
fi


