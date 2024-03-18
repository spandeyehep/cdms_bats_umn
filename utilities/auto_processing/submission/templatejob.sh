######### CDMS Auto Processing Job Configuration ##############

#email address for status notifications
EMAIL=

#ISGRID should be 'true' if jobs run under condor or 'false' otherwise
ISGRID=true

#Number of worker processes to launch
NWORKERS=200

#Working directory (must be unique way to identify this job)
WORKDIR=

#List of series and number of dumps to process
SERIESLIST=

#datatype (ba, bg, cf)
DATATYPE=

#run label (e.g. R133)
RUNID=

#cdmsbats installation directory
CDMSBATSDIR=/grid/fermiapp/cdms/processing_source/cdmsbats_tempbloer

#cdmsbats configuration files
BATROOT_PROCCFG=
BATROOT_ANACFG=
BATCALIB_PROCCFG=
BATCALIB_CALIBCFG=
