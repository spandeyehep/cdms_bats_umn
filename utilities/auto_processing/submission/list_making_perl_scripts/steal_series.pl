#!/usr/bin/perl
#usage: ./steal_series.pl --file=[TAB DELIMITED FILE]
#optionally write out number of files, and don't write out 
#duplicate series [LLH]

eval '(exit $?0)' && eval 'exec perl -w -S $0 ${1+"$@"}'
                  && eval 'exec perl -w -S $0 $argv:q'
                  if 0;

use strict;
use Cwd;
use FileHandle;
use Getopt::Long;
use Pod::Usage;
use Env;

system("date");

Getopt::Long::Configure(qw(no_ignore_case bundling require_order));

my $option = {
	      file => "",
	     };

GetOptions( $option, "file|f=s", ) or pod2usage(-exitval => 2);

open(FILE,"$option->{file}");

my $outf = $option->{file};

$outf = $outf.".skim";

print "$outf\n";

my $out  = "";
my $line = "";
my $nfiles = 0;

#Enter runlist
#print "\nWrite out number of files (Y/N)?";
#my $writeFiles = <STDIN>;
#chomp($writeFiles);

#until($writeFiles =~ /^y\b/i || $writeFiles =~ /^n\b/i) 
#{
#  print "\nWrong input!  Choose Y or N:  ";
#  $writeFiles = <STDIN>;
#  chomp($writeFiles);
#}


my %seriestable = ();
#{
#   local @ARGV = ($file);
#   local $^I = '.bac';
#   while(<>){
#      $seriestable{$_}++;
#      next if $seriestable{$_} > 1;
#      print;
#   }
#}

while($line = <FILE>){
    chomp ($line);

  #strip out everything from the massive file except for series number
  if($line =~ m#(01\d{6}_\d{4})#) 
  {
      #count the number of raw data files on atto
      $nfiles = `ls /atto/data1/$1/*.gz | wc -l`;

      #store this for the output file
      print "Found series $1 with maxfilenumber = $nfiles";

      #store the series in the array
      $seriestable{$1}++;

      #skip rest of the lines in this loop if its already been found
      next if $seriestable{$1} > 1;

      #now write out the series
#      if($writeFiles =~ /^y\b/i)
#      {	  
	  #write it out with number of files
#	  $out .= "$1 $nfiles";
#      }
#      else{
	  #or don't write it out with number of files
	  $out .= "$1\n";
      #}

  }
}

my $fh = new FileHandle("> $outf") || halt("Bye!");

print $fh $out;
$fh->close();

1;
