#!/usr/bin/perl

#Checks for missing files and then resubmits individual missing dumps to the queue

#Enter runlist
print "\nEnter the run list: ";
$runlist = <STDIN>;
chomp($runlist);

#Enter runlist
print "\nEnter the data type (ba, bg or cf): ";
$datatype = <STDIN>;
chomp($datatype);

#resubmit?
#print "\nEnter the data type (ba, bg or cf): ";
#$datatype = <STDIN>;
#chomp($datatype);



# ==============================================================================

#open runlist file for reading
if ( open( RUNLIST, "runlists/$runlist" ) != 1 )
  {
    die "Couldn't open file submission/$runlist for reading!";
  }

# ==============================================================================

#reading all the lines in the runlist
while($line = <RUNLIST>) {
  chomp ($line);

  #put series number into correct format
  $series = substr($line, 0, 11);

  #get number of dumps
  $length = length($line)-12;
  $maxevents = substr($line, 12, $length);

  $dumpmod = $maxevents%500;
  if($dumpmod > 0)
  {
      $maxdumps = int($maxevents/500) + 1;
  }
  else
  {
      $maxdumps = int($maxevents/500);
  }

  print "\n\nChecking the series $series";
  print ", the number of dumps = $maxdumps";

  #Loop over the dumps and check if the file is there
  for($i = 1; $i <= $maxdumps; $i++)
  { 
      $nleadzeros = 4-length($i);
      $dump = "F";
      while( $nleadzeros >= 1 )
      {
	  $dump .= "0"; 
	  $nleadzeros = $nleadzeros - 1;
      }
      $dump .= $i;

      #look for missing files from dump 1 to dump max
      $outputdir = "/data1/BatRootTesting/R123/ROOT";
      $cmd = "ls $outputdir/$datatype/$series/grid_dec9\_$series\_$dump.root";
      $grepno = `$cmd 2>&1`;

      #if not found, then try to scp
      $searchstring = "No such file";
      if($grepno =~ m/$searchstring/)
      {
	  print "\nFound missing file for series $series, dump $dump\n";
	  system("scp llhsu\@fnpcsrv1:processing/br_output/R123/$datatype/$series/grid_dec9\_$series\_$dump.root $outputdir/$datatype/$series/.");
      }

  } #done looping over dumps

  print "\nDone! \n";

} #done looping over runs
