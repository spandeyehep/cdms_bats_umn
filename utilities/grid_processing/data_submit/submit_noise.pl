#!/usr/bin/perl

#For submission of noise files

#Did you cleanup?
print "\nDid you cleanup old files? ";
$yesno = <STDIN>;
chomp($yesno);

until($yesno =~ /^y\b/i || $yesno =~ /^n\b/i) {
  print "\nWrong input!  Choose Y or N:  ";
  $yesno = <STDIN>;
  chomp($yesno);
}


if ($yesno =~ m/n/i)
{
    print "Cleanup old directories first!\n";
    exit;
}


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

# ==============================================================================

#open runlist file for reading
if ( open( RUNLIST, "submission/$runlist" ) != 1 )
  {
    die "Couldn't open file submission/$runlist for reading!";
  }

# ==============================================================================

#reading all the lines in the runlist
while($line = <RUNLIST>) {
  chomp ($line);

  #put series number into correct format
  $series = substr($line, 0, 11);

  print "\nSubmitting the series $series";

  system("./br_condor_noise.sh $series $datatype $runid");

}
