#!/usr/bin/perl

# ######################################################################################
#
# Script to merge data ("merge" or "supermerge")
# 
# INTRUCTIONS: 
# 
#  1) Modify hardcoded parameters:
#        - "execdir": path where this script is located
#        - "do_supermerge": set to 1 if "supermerge", 0 if "merge" (from _FXXXX files)
#        - "auto_set_path": set to 1 to automatically use hardcoded paths, otherwise 
#                             script will prompt for input  
#        - "auto_set_name": set to 1 to automatically construct output filename using 
#                           run list (no 4th argument should be given)
#        -  PATHS (used if "auto_set_path" set to 1, otherwise prompt for input): 
#               "runlist_path_default", "datadir_default", "merge_datadir_default"
#
#               NOTE 
#                A) for supermerge, a temporary directory will be created in  
#                  "merge_datadir_default" (can be deleted after processing done)
#                B) Besides temporary directory above, all other directories shoud exists!
#
#  2) Script can then be run with some optional arguments, if argument not given, 
#     script will prompt for input
#
#         Optional Argments:
#             (1) dataset list name
#             (2) data type (ba, cf_bg_permitted_Sept2013, ....)
#             (3) File prefix (Prodv5-3, ..)
#             (4) supermerge file name base 
#
#      Example: ./mergeByList.pl prodR134_z14lite_1405a_bg.txt bg_restricted Prodv5-3
#
#  
# ####################################################################################

my $file = "mergepaths.cfg";

if (my $cfg = do $file) {
    $execdir = $cfg{execdir};
    $runlist_path_default=$cfg{runlist_path_default};
    $datadir_default = $cfg{datadir_default};
    $merge_datadir_default = $cfg{merge_datadir_default};
    $mergename = $cfg{mergename};
    $auto_set_name=$cfg{auto_set_name};
}


# working directory
#$execdir = "/home/georgemm01/Bats/newcdmsbats/auto_processing";

# Merge or SuperMerge?
$do_supermerge=1;

# flag
$auto_set_path=1;
# $auto_set_name=0; # only apply for supermerge

# =============================
# PATHs
# =============================

# dataset list

#$runlist_path_default='/micro/data6/cdmsbatsProd/R133/scripts/supermerge_scripts/supermergelist/';
#$runlist_path_default='/micro/data6/cdmsbatsProd/R133/scripts/supermerge_scripts/supermergelist/R135';
#$runlist_path_default='/micro/data6/cdmsbatsProd/R133/scripts/supermerge_scripts/mergelist';
#$runlist_path_default='/fdata/hepx/store/user/georgemm01/Bats/simulation_v5/config/serieslists/LTPb';


# data path 
# --- R133 ---
#$datadir_default='/fdata/hepx/store/user/georgemm01/Bats/simulation_v5/activejobs/Pb206_surface/output/submerged/all';
#$merge_datadir_default='/fdata/hepx/store/user/georgemm01/Bats/simulation_v5/activejobs/Pb206_surface/output/merged/all';


# --- R134 ---
#$datadir_default='/data2/cdmsbatsProd/R134/dataReleases/Prodv5-3-6_cdmslite/submerged/all/';
#$merge_datadir_default='/data1/cdmsbatsProd/R134/dataReleases/Prodv5-3-6_cdmslite/merged/all/';

# --- R135 ---

# supermerge
#$datadir_default='/fdata/hepx/store/user/georgemm01/Bats/simulation_v5/activejobs/1502a_bg/output/submerged/all';
#datadir_default='/tera2/data1/cdmsbatsProd/R135/dataReleases/Prodv5-3-6_cdmslite/submerged/all';
#$merge_datadir_default='/fdata/hepx/store/user/georgemm01/Bats/simulation_v5/activejobs/1502a_bg/output/merged/all';
#$merge_datadir_default='/tera2/data1/cdmsbatsProd/R135/dataReleases/Prodv5-3-6_cdmslite/merged/all';

# merge
#$datadir_default='/tera2/data1/cdmsbatsProd/R135/dataReleases/Prodv5-3-6_cdmslite/unmerged';
#$merge_datadir_default='/tera2/data1/cdmsbatsProd/R135/dataReleases/Prodv5-3-6_cdmslite/submerged/all';


#
#$merge_datadir_default='/data1/cdmsbatsProd/R133-4_samples/merged';


# paths prompts 

if ($auto_set_path != 1) {
    
    #Enter root directory
    print "\nEnter unmerged data directory [$datadir_default]: ";
    $datadir = <STDIN>;
    chomp($datadir);
    $datadir=$datadir_default if $datadir eq '';
   
    #Enter root directory
    print "\nEnter merged data directory [$merge_datadir_default]:  ";
    $merge_datadir = <STDIN>;
    chomp($merge_datadir);
    $merge_datadir=$merge_datadir_default if $merge_datadir eq '';
    
    #Enter runlist
    print "\nEnter the dataset lists path [$runlist_path_default]: ";
    $runlist_path = <STDIN>;
    chomp($runlist_path);
    $runlist_path= $runlist_path_default if $runlist_path eq  '';
    
} else {
    $datadir =   $datadir_default;
    $merge_datadir =  $merge_datadir_default;
    $runlist_path = $runlist_path_default;
}

    

#  =============================
#  (Optinal) arguments
#  =============================

# number of arguments
$num_args = $#ARGV + 1;

# merge file basename
if ($num_args <4) {
    if ($auto_set_name!= 1) {
	print "\nEnter output file base (from config): ";
	$mergefile= $mergename;
	print $mergename."\n";
	chomp($mergefile);
    }
} else {
    $mergefile=$ARGV[3];
}

# Data prefix
if ($num_args <3) {
    print "\nEnter the file prefix: ";
    $prefix = <STDIN>;
    chomp($prefix);
} else {
    $prefix= $ARGV[2];
}

#datatype
if ($num_args <2) {
    print "\nEnter the data type (ba, bg_permitted, bg_restricted, cf): ";
    $datatype = <STDIN>;
    chomp($datatype);
} else {
    $datatype = $ARGV[1];
}

#Runlist
if ($num_args <1) {
    print "\nEnter the run list: ";
    $runlist = <STDIN>;
    chomp($runlist);
} else {
    $runlist = $ARGV[0];
}


#FIXME  currently saved as submerged bg
$datatypename=$datatype;
if ($datatype eq 'bg_restricted' or $datatype eq 'bg_permitted' or $datatype eq 'bg_permitted_Sept2013') {
    $datatypename="bg";

}




# ================================
# SUPERMERGE (submerged->merged) 
# ================================

if ($do_supermerge == 1) {


    # ----------------
    # Read file list
    # ----------------

    #open runlist file for reading
    if ( open( RUNLIST, "$runlist_path/$runlist" ) != 1 )
    {
	die "Couldn't open file $runlist for reading!";
    }


    # fill array with filenames
    my @rqdata = ();
    my @rrqdata = ();
    
    while($line = <RUNLIST>) {
	chomp ($line);
	
	#put series number into correct format
	# FIXME (length hard coded)
	$series = substr($line, 0, 13);
	
	# file name
	$rqfile = "$datadir/$datatypename/merge\_$prefix\_$series.root";
	
	
	unless (-e "$rqfile"){
	    print "\nFile \'$rqfile\' not found!\n";
	    
	} else {
	    push(@rqdata,$rqfile);
	}
	
	
	$rrqfile = "$datadir/$datatypename/calib\_$prefix\_$series.root";
	
	unless (-e "$rrqfile"){
	    print "\nFile \'$rrqfile\' not found!\n";
	    
	} else {
	    
	    # fill array with files
	    push(@rrqdata,$rrqfile);
	    
	}
    }
    
    # number of files
    $nums_rq = @rqdata;
    $nums_rrq = @rrqdata;
    
    
    # ----------------
    # File name
    # ----------------
  

    # name defined using run list
    if (($num_args <4) &&  ($auto_set_name== 1)) {
	
	# start position check (either prodRXX or testRXX)
	my $position1 = index($runlist,"prodR");
	
	if ($position1 == -1) {
	    $position1 = index($runlist,"testR"); 
	}
	
	$mergefile = substr($runlist,$position1+9);
	
	
        # end position (either EORRand.txt or .txt)
	my $position2 = rindex($mergefile,"EORRand.txt");
	
	if ($position2 == -1) {
	    $position2 = rindex($mergefile,"txt");
	}
	
	$mergefile = substr($mergefile,0,$position2-1);
	
    }
    
    
    # temporary directory
    $tempdir = "$merge_datadir/tempMerge";
    
    if (! -d $tempdir) {
	mkdir  $tempdir }

    $rqmergefile_temp = "$tempdir/merge\_$prefix\_$mergefile\_temp.root";
    $rrqmergefile_temp = "$tempdir/calib\_$prefix\_$mergefile\_temp.root";
    


    $rqmergefile = "$merge_datadir/$datatype/merge\_$prefix\_$mergefile\.root";
    $rrqmergefile = "$merge_datadir/$datatype/calib\_$prefix\_$mergefile\.root";
    
    
    # ----------------
    # Merge
    # ----------------
    
    print "\nMerging $nums_rq RQ files";
    
    system("$execdir/merge_filelist $rqmergefile_temp  polybasket @rqdata >$execdir/logs/supermerge_rq_$mergefile\_0.log 2>$execdir/logs/supermerge_rq_$mergefile\_0.err;");
    system("$execdir/merge_filelist $rqmergefile fast  $rqmergefile_temp  >$execdir/logs/supermerge_rq_$mergefile\_1.log 2>$execdir/logs/supermerge_rq_$mergefile\_1.err;");
    
    # merge RRQ files
    print "\nMerging $nums_rrq RRQ files\n";
    system("$execdir/merge_filelist $rrqmergefile polybasket @rrqdata >$execdir/logs/supermerge_rrq_$mergefile\_0.log >$execdir/logs/supermerge_rrq_$mergefile\_0.err;");
    system("$execdir/merge_filelist $rrqmergefile fast  $rrqmergefile_temp  >$execdir/logs/supermerge_rq_$mergefile\_1.log 2>$execdir/logs/supermerge_rrq_$mergefile\_1.err;");
    
    
    print "\nDone!\n";

} 



# ================================
# MERGE (unmerged->submerged)
# ================================

if ($do_supermerge == 0) {

    # ----------------
    # Read file list
    # fill array
    # ----------------

    #open runlist file for reading
    if ( open( RUNLIST, "$runlist_path/$runlist" ) != 1 )
    {
	die "Couldn't open file $runlist for reading!";
    }
    

    # LOOP series and merge
    while($line = <RUNLIST>) {
	chomp ($line);
	
	#put series number into correct format
	# FIXME (length hard coded)
	$series = substr($line, 0, 13);

	# fill array with filenames
	my @rqdata = ();
	my @rrqdata = ();
    
	
	# fill data
	chdir("$datadir/$datatypename/$series") or die "can't find directory $datadir/$datatypename/$series";
	@rqdata=<$prefix\_*.root>;
	@rrqdata=<calib\_$prefix\_*.root>;
	
	# number of files
	$nums_rq = @rqdata;
	$nums_rrq = @rrqdata;
	
	
	# ----------------
	# File name
	# ----------------
	
	$rqmergefile = "$merge_datadir/$datatype/merge\_$prefix\_$series\.root";
	$rrqmergefile = "$merge_datadir/$datatype/calib\_$prefix\_$series\.root";
	
	# ----------------
	# Merge
	# ----------------
	
	# merge RQ files
	print "\nMerging $nums_rq RQ files for $series \n";
	system("$execdir/merge_filelist $rqmergefile  monobasket @rqdata >$execdir/logs/merge_rq_$series\_0.log 2>$execdir/logs/merge_rq_$series\_0.err;");
	
	# merge RRQ files
	print "\nMerging $nums_rrq RRQ files for $series\n";
	system("$execdir/merge_filelist $rrqmergefile  monobasket @rrqdata >$execdir/logs/calib_rq_$series\_0.log 2>$execdir/logs/calib_rq_$series\_0.err;");
	
	print "\nDone, moving to next series!\n";
    }
}


