#!/usr/bin/perl

#For merging of multiple series (series by series merge)

#Enter runlist
print "\nEnter the run list: ";
$runlist = <STDIN>;
chomp($runlist);

#Enter datatype
print "\nEnter the data type (ba, bg or cf): ";
$datatype = <STDIN>;
chomp($datatype);

#Enter run
print "\nEnter the run (e.g. R123): ";
$runid = <STDIN>;
chomp($runid);

#Enter prefix
print "\nEnter the file prefix: ";
$prefix = <STDIN>;
chomp($prefix);

# ==============================================================================

#open runlist file for reading
if ( open( RUNLIST, "/micro/data3/cdmsbatsProd/$runid/masterRunLists/$runlist" ) != 1 )
  {
    die "Couldn't open file /micro/data3/cdmsbatsProd/$runid/masterRunLists/$runlist for reading!";
  }

# ==============================================================================

#reading all the lines in the runlist
$searchR130 = "R130";
$searchR132 = "R132";
$searchR133 = "R133";
while($line = <RUNLIST>) {
  chomp ($line);

 if($runid =~ m/$searchR130/||$runid =~ m/$searchR132/||$runid =~ m/$searchR133/)
 {
     #put series number into correct format
     $series = substr($line, 0, 13);
 
     #get number of dumps
     $length = length($line)-14;
     $maxdumps = substr($line, 14, $length);
 }
  else
  {
      #put series number into correct format
      $series = substr($line, 0, 11);
      
      #get number of dumps
      $length = length($line)-12;
      $maxdumps = substr($line, 12, $length);
      
  }

  #run merging scripts
  print "\nMerging the series $series";
  print ", the number of dumps = $maxdumps\n";

  system("root -b -q 'merge_all.C\(\"$prefix\",\"$series\",\"$datatype\",\"$runid\",$maxdumps\)' >logs/merge\_$runlist\_$series.log 2>logs/merge\_$runlist\_$series.err;");

  system("root -b -q 'merge_izip_calib.C\(\"$prefix\",\"$series\",\"$datatype\",\"$runid\",$maxdumps\)' >logs/calib\_$runlist\_$series.log 2>logs/calib\_$runlist\_$series.err;");

  print "Done, moving to next series!";

}
