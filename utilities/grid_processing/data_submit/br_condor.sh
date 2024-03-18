#!/bin/bash

#for submission of single files to be processed per condor job.  This is not the most efficient way, but useful for cleanup
#usage: br_condor.sh series maxdumps datatype(ba, cf or bg) run

### Check expected arguments ###

EXPECTED_ARGS=4

if [ $# -ne $EXPECTED_ARGS ]
then
  echo "Wrong number of arguments! usage: br_condor.sh series maxdumps datatype(ba, cf or bg) run";
  exit;
fi

### Compute some needed values ###

#usage: br_condor.sh series maxdumps datatype(ba, cf or bg)

for i in `seq 1 $2`; do condor_submit -a "Arguments=$1 $i 500 $3 $4" br_condor ; echo "submitting for $1, dump $i"; done
