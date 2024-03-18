#!/usr/bin/perl

#Enter data type
print "\nEnter the data type (ba, bg or cf): ";
$datatype = <STDIN>;
chomp($datatype);

# ==============================================================================

#open runlist file for reading
$cmd = "ls /grid/data/cdms/processing/br_output/R123/$datatype";
open(PIPE, "$cmd |"); 

#reading all the series in the directory
while($series = <PIPE>) {
  chomp ($series);

  print "\nTar of series: $series";

  system("cd /grid/data/cdms/processing/br_output/R123/$datatype; tar -cvf $series.tar $series");

  print "Done with tar, now scp...";

  system("scp /grid/data/cdms/processing/br_output/R123/$datatype/$series.tar llhsu\@cdmsnano:/data1/BatRootTesting/R123/ROOT/$datatype/.");
 # system("scp $series.tar llhsu\@cdmsnano:/data1/BatRootTesting/R123/ROOT/$datatype/.");

  print "Done with scp, now storing tarball";
  system("mv *.tar backuptar/.");

} #done looping over runs
