#!/bin/bash

#for submission of noise file generation, eventually modify this to do multiple series per condor job
#usage: br_condor.sh series datatype(ba, cf or bg) run

### Check expected arguments ###

EXPECTED_ARGS=3

if [ $# -ne $EXPECTED_ARGS ]
then
  echo "Wrong number of arguments! usage: br_condor.sh series datatype(ba, cf or bg) run";
  exit;
fi


condor_submit -a "Arguments=$1 1 500 $2 $3" br_condor_noise 
echo "submitting for $1, dump 1" 
