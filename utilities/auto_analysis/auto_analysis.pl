#!/usr/bin/perl
# $Id$
# 20111115  Michael Kelsey -- Modify for CDMSBATS directory reorganization

###################### Get the arguments from the commandline (if any)
if ( $#ARGV == -1 )  # (no arguments)
{
    $mode = "generate";
}
elsif ( $#ARGV == 0 )  # (one argument)
{
    $mode = $ARGV[0];
}
else { die "Too many arguments"; }

###################### Make sure we are running in auto_analysis directory

use File::Basename;
$dir = dirname($0);
if ( $dir ne "." ) {
  chdir($dir) or die "Can't find script directory $dir";
}

################################################### printing a header

system("clear");
print "\# ====================================================================================\n";
print "\nAUTOGENERATION SCRIPT FOR NEW BATROOT ANALYSIS CLASSES:\n\n";  
print "This code will attempt to automatically generate a new analysis class according to your specifications.  In the process it will modify several files in your version of CDMSBATS so that your new analysis class will function as part of the data processing package.\n";
#print "Last Modified: ";
#system("date");

print "\n======================================================================================\n";


################################################### Query the user to setup the class

#Enter Name
print "\nPlease enter your name or initials: ";
$authorName = <STDIN>;
chomp($authorName);

#Class Name
print "\nNext we will need the name for the analysis class.";
print "\nTo conform with standard c++ naming conventions, please follow these rules:\n";
print "\n1. Use only alphanumeric characters (NOT \. \_ \# \$ \@ etc.)."; 
print "\n2. Capitalize the first letter of each word in the name, but not the others. Acronyms may be all capitals.\n";
print "\nExamples of GOOD names: MyAnalysisClass, OFChargeFilter, PipeFitTimeDomain";
print "\nExamples of BAD names: myanalysisclass, OF\_Charge_Filter, PipeFit.TimeDomain";

print "\n\nNow enter the name of your class (case sensitivity): ";
$className = <STDIN>;
chomp($className);

#Pulse Analysis by Location - currently only active for Soudan, let users follow the example
@placelist = (SOUDAN);
%placehash = ();

foreach $placename ( @placelist )
{ 
  #Soudan config defaults
  print "\n*************";
  print "\n\nIn the most likely scenario for $placename data, do you expect your algorithm be performed on: \n";
  print "\na) only phonon pulses";
  print "\nb) only charge pulses";
  print "\nc) only veto pulses";
  print "\nd) both charge and phonon pulses";
  print "\n\nWhat you specify here will be the default in the config file.  Choose from options above: ";
  $tempstring = <STDIN>;
  chomp($tempstring);

  until($tempstring =~ /^a\b/i || $tempstring =~ /^b\b/i || $tempstring =~ /^c\b/i || 
	$tempstring =~ /^d\b/i) 
  {
      print "\nWrong input!  Choose a,b,c or d:  ";
      $tempstring = <STDIN>;
      chomp($tempstring);
  }

  $placehash{$placename} = $tempstring;

  if($tempstring eq "a") {
    $channelChoice = "phonon";
  }

  if($tempstring eq "b") {
    $channelChoice = "charge";
  }

  if($tempstring eq "d") {
    $channelChoice = "either";
  }

} #end of channel type queries


print "\nDefault will be: $channelChoice";

#Using Minuit?
print "\n\n*************\n";

print "\nDo you need the Minuit fitter for your algorithm (Y/N)?  ";
$useMinuit = <STDIN>;
chomp($useMinuit);

until($useMinuit =~ /^y\b/i || $useMinuit =~ /^n\b/i) {
  print "\nWrong input!  Choose Y or N:  ";
  $useMinuit = <STDIN>;
  chomp($useMinuit);
}

#RQs
@rqlist = ();

print "\n*************";
print "\n\n(OPTIONAL) Now entering the RQ list.\n";
print "\nBatRoot will construct the RQ from the base name that you supply.  As an example, consider the RQs \"QIOFvolts\", \"QOOFvolts\".  The base name to supply for this case is \"OFvolts\".  BatRoot will append the channel names.  \n";
print "\nNow enter your first RQ name (hit enter with no input to exit this option):\n";
$testRQname = <STDIN>;
chomp($testRQname);
push(@rqlist, $testRQname);

while($testRQname ne "") {
  print "\nEnter the next RQ name (hit enter with no input if no additional RQs needed):\n";
  $testRQname = <STDIN>;
  chomp($testRQname);
  if($testRQname ne "") {
    push(@rqlist, $testRQname);
  }
}

print "\nThe RQ list:";
foreach $rqname ( @rqlist ) {
  print "\n$rqname";
}

#Done with inputs
print "\n\n*************";

################################################## Choosing template

if($useMinuit =~ m/y/i) {
$templateName = "TestAnalysisMinuit";
}
else {
$templateName = "TestAnalysis";
}

################################################## Writing analysis header

$fileheader = "$className.h";
print "\nWriting file \"$fileheader\"";

#open template file for reading
if ( open( TEMPLATE, "classtemplates/$templateName.h" ) != 1 )
  {
    die "Couldn't open file \"$templateName.h\" for reading!";
  }

#open new file for writing
if ( open( FILE, "> $fileheader" ) != 1 )
  {
    die "Couldn't open file \"$fileheader\" for writing";
  }

#search and replace the template name with the new class name
while($line = <TEMPLATE>) {
  chomp ($line);
  $line =~ s/$templateName/$className/gi;
  $line =~ s/LLH/$authorName/gi;
  
  print FILE "$line\n";

}

################################################## Writing analysis.cxx 

$file = "$className.cxx";
print "\nWriting file \"$file\"";

#open template file for reading
if ( open( TEMPLATE, "classtemplates/$templateName.cxx" ) != 1 )
  {
    die "Couldn't open file classtemplates\"$templateName.cxx\" for reading!";
  }


#open new file for writing
if ( open( FILE, "> $file" ) != 1 )
  {
    die "Couldn't open file \"$file\" for writing";
  }

#reading all the lines in the template
while($line = <TEMPLATE>) {
  chomp ($line);
  #search and replace the template name with the new class/author name
  $line =~ s/$templateName/$className/gi;
  $line =~ s/LLH/$authorName/gi;

  #printing the new script
  unless($line =~ m/fRQList/) {
    print FILE "$line\n";
  }

  #adding lines that generate RQ list
  $searchstring = "construct the RQ list here";
  if($line =~ m/$searchstring/) {
    foreach $rqname ( @rqlist ) {
      print FILE "   fRQList.insert(pair<string,double>(\"$rqname\", initVal))\;\n";
    }
  }

  #adding lines that store RQ's
  #$searchstring = "if(fStoreRQs)";
  $itr = 1.0;
  if($line =~ m/fStoreRQs/ && $line =~ m/if/) {
    foreach $rqname ( @rqlist ) {
      print FILE "      fRQList\[\"$rqname\"\] = $itr\;\n";
      $itr = $itr + 1.0;
    }
  }

}


################################################### Add new class to PulseData

$batmanFile = "BatOutputManager.cxx";
print "\nWriting file \"$batmanFile\"";

#open template file for reading
if ( open( TEMPLATE, "../../BatRoot/$batmanFile" ) != 1 )
  {
    die "Couldn't open file \"../../BatRoot/$batmanFile\" for reading!";
  }

#open template file for writing
if ( open( FILE, "> $batmanFile" ) != 1 )
  {
    die "Couldn't open file \"$batmanFile\" for writing";
  }

#reading all the lines in the template
while($line = <TEMPLATE>) {
  chomp ($line);

  print FILE "$line\n";

  #include new class header in BatOutputMan - this goes after the flag
  $searchstring = "//User Analysis Classes";
  if($line =~ m/$searchstring/) {
    print FILE "#include \"$className.h\"\n";
  }

  #adding lines that register the RQ list - this goes after the flag
  $searchstring = "additional analysis options go here";
  if($line =~ m/$searchstring/) {
      print FILE "\n   if(analysisName == \"$className\")
   \{
      $className emptyAnalysis;
      rqList = emptyAnalysis.GetRQList();
      found = true;
   \}\n\n";
  }

} #end while loop over file

################################################### Add new class to EventBuilder

## first for .cxx
$placestring = "SOUDAN";
$eventBuilderFile = "EventBuilder.cxx";

print "\nWriting file \"$eventBuilderFile\"";

#open template file for reading
if ( open( TEMPLATE, "../../BatRoot/$eventBuilderFile" ) != 1 )
  {
    die "Couldn't open file \"../../BatRoot/$eventBuilderFile\" for reading!";
  }

#open template file for writing
if ( open( FILE, "> $eventBuilderFile" ) != 1 )
  {
    die "Couldn't open file \"$eventBuilderFile\" for writing";
  }

#reading all the lines in the template
while($line = <TEMPLATE>) {
  chomp ($line);

  print FILE "$line\n";

  #include new class header - this goes after the flag
  $searchstring = "//User Analysis Classes";
  if($line =~ m/$searchstring/) {
    print FILE "#include \"$className.h\"\n";
  }

  #note there is no QT pulse right now - though that may change in the future
  if($placehash{$placestring} eq "a" || $placehash{$placestring} eq "d") 
  { 
      $extraPTcheck = "\n          //analysis flag for sum of phonon pulses\n          if(aPulseData->GetChannelName() == \"PT\" && !fUserData.DoAlgorithm(detNum,\"PT\",\"$className\")) \{ continue; \}\n";
      $extraPTcheck = $extraPTcheck."          if(aPulseData->GetChannelName() == \"PS1\" && !fUserData.DoAlgorithm(detNum,\"PSIDES\",\"$className\")) \{ continue; \}\n";
      $extraPTcheck = $extraPTcheck."          if(aPulseData->GetChannelName() == \"PS2\" && !fUserData.DoAlgorithm(detNum,\"PSIDES\",\"$className\")) \{ continue; \}\n";
  }
  else
  {
      $extraPTcheck = "";
  }

  #adding an empty function call to DoCalc
  $searchstring = "Interfaces to User Analysis Classes Go Here";
  if($line =~ m/$searchstring/) {
     
      #if not veto  
      if($placehash{$placestring} ne "c") {
   
	  print FILE "\n//Modify this as needed to correctly call your class";
	  print FILE "\nvoid EventBuilder::Do$className(int detNum, const string& sensorType)";
	  print FILE "\n\{\n
    //retrive the vector of pulses for this zip
    vector<PulseData>* zipPulseList;
    map< int, vector<PulseData> >::iterator mapItr = fMapOfZipPulses.find(detNum);
    if(mapItr != fMapOfZipPulses.end())
    \{
       //Get the pulse list for this zip
       zipPulseList = &(mapItr->second);

       //loop over zip pulse collection
       for(uint pulseItr = 0; pulseItr < zipPulseList->size(); pulseItr++)
       {	  
	  PulseData* aPulseData = &((*zipPulseList)[pulseItr]);
	  
          //check whether processing is desired for this sensor type
	  if(aPulseData->GetChannelType() != sensorType) { continue; }
	  $extraPTcheck
          //Get the pulse
          //other options are: GetRawPulse, GetBaselineSubtractedNormPulse (norm = 1 when isr is not read)
	  vector<double> aPulse = aPulseData->GetBaselineSubPulse();  
	  
	  $className temp$className;

	  temp$className.DoCalc(aPulse); 
	  
	  //store instance of this class so RQs can be read out a little later
	  aPulseData->StorePulseAnalysis(temp$className);
       \}

   \} //end if detNum found in map

    return; \n\}\n\n";
      }  else {
	  print FILE "\n//Modify this as needed to correctly call your class";
	  print FILE "\nvoid EventBuilder::Do$className()";
	  print FILE "\n\{\n
    //loop over entire pulse collection
    for(uint pulseItr = 0; pulseItr < fVectorOfVetoPulses.size(); pulseItr++)
    \{
       PulseData*     aPulseData = &fVectorOfVetoPulses[pulseItr]; 
       int            detNum = aPulseData->GetDetectorNum();
       $className     temp$className;

       //optional function for setting parameters         
       //temp$className.InitializeParameters();\n

       //only option for veto pulses is RawPulse (other pulses in PulseData are not filled for veto)
       temp$className.DoCalc(aPulseData->GetRawPulse());

	//store instance of this class so RQs can be read out a little later
        aPulseData->StorePulseAnalysis(temp$className);
     \}

    return; \n\}\n\n";
      }
         
  } #end adding code to EventBuilder

} #end while

## now the header
$eventBuilderHeaderFile = "EventBuilder.h";

print "\nWriting file \"$eventBuilderHeaderFile\"";

#open template file for reading
if ( open( TEMPLATE, "../../BatRoot/$eventBuilderHeaderFile" ) != 1 )
  {
    die "Couldn't open file \"../../BatRoot/$eventBuilderHeaderFile\" for reading!";
  }

#open template file for writing
if ( open( FILE, "> $eventBuilderHeaderFile" ) != 1 )
  {
    die "Couldn't open file \"$eventBuilderHeaderFile\" for writing";
  }

#reading all the lines in the template
while($line = <TEMPLATE>) {
  chomp ($line);

  print FILE "$line\n";

  #include new class header - this goes after the flag
  $searchstring = "//User Analysis Classes";
  if($line =~ m/$searchstring/) {

    #if not veto  
      if($placehash{$placestring} ne "c") {
	print FILE "      void Do$className(int detNum, const string& sensorType);\n";
      } 
      else
      {
	  print FILE "      void Do$className();\n";
      }
  }
  
}

################################################### Add new class to Main

$mainFile = "BatRoot.cxx";

print "\nWriting file \"$mainFile\"";

#open template file for reading
if ( open( TEMPLATE, "../../BatRoot/$mainFile" ) != 1 )
  {
    die "Couldn't open file \"../../BatRoot/$mainFile\" for reading!";
  }

#open template file for writing
if ( open( FILE, "> $mainFile" ) != 1 )
  {
    die "Couldn't open file \"$mainFile\" for writing";
  }

#reading all the lines in the template
while($line = <TEMPLATE>) {
  chomp ($line);

  print FILE "$line\n";

  #add rule for new class
  $searchstring = "additional user analysis classes";

  #add call to DoMyAnalysis for phonon channels
  if($channelChoice eq "charge" || $channelChoice eq "either") {
    if($line =~ m/$searchstring/) {
      print FILE "       if( myUserData.DoAlgorithm(detNum, \"charge\", \"$className\") ) 
          eventBuilder.Do$className(detNum, \"charge\");\n"
    }
  }

  #add call to DoMyAnalysis for charge channels
  if($channelChoice eq "phonon" || $channelChoice eq "either") {
    if($line =~ m/$searchstring/) {
      print FILE "       if( myUserData.DoAlgorithm(detNum, \"phonon\", \"$className\") ) 
          eventBuilder.Do$className(detNum, \"phonon\");\n"
    }
  }

}

################################################### Add new class to default Soudan config

#deactivated for veto at the moment
$placestring = "SOUDAN";
if($placehash{$placestring} ne "c") {

    $configFile = "processingSoudanData.Default";
    print "\nWriting file \"$configFile\"";

    #open template file for reading
    if ( open( TEMPLATE, "../../UserSettings/BatRootSettings/processing/$configFile" ) != 1 )
    {
	die "Couldn't open file \"../../UserSettings/BatRootSettings/processing/$configFile\" for reading!";
    }

    #open template file for writing
    if ( open( FILE, "> $configFile" ) != 1 )
    {
	die "Couldn't open file \"$configFile\" for writing";
    }

    #reading all the lines in the template
    while($line = <TEMPLATE>) {
	chomp ($line);

	print FILE "$line\n";

	#add flag for new phonon class
	if($placehash{$placestring} eq "a" || $placehash{$placestring} eq "d") 
	  { 
	    $searchstring = "phonon analysis classes";
	    if($line =~ m/$searchstring/) {
		#print this for phonons  
		print FILE "\nDO_PHONON_ALGORITHM     $className\t\tDETECTOR 1-30   =       1"
	  }
	} #end if option a or d
	
	#add flag for new charge class
	if($placehash{$placestring} eq "b" || $placehash{$placestring} eq "d") 
          {
	    $searchstring = "charge analysis classes";
	    if($line =~ m/$searchstring/) { 
		#print this for phonons  
		print FILE "\nDO_CHARGE_ALGORITHM     $className\t\tDETECTOR 1-30   =       1"
	    }
	} #end if option b or d
	
    } #end while loop over file

} #end if not veto


################################################### make a short script for moving files
$cleanupScript = "cleanup.scr";

print "\nWriting file \"$cleanupScript\"";

#open file for writing
if ( open( FILE, "> $cleanupScript" ) != 1 )
  {
    die "Couldn't open file \"$batmanFile\" for writing";
  }

print FILE "cd $dir\n" if ($dir ne ".");

print FILE "mv BatRoot.cxx ../../BatRoot/\n";
if($placehash{$placestring} ne "c") { 
    print FILE "mv processingSoudanData.Default ../../UserSettings/BatRootSettings/processing/\n";
}
print FILE "mv EventBuilder.* ../../BatRoot/\n";
print FILE "mv BatOutputManager.cxx ../../BatRoot/\n";
print FILE "mv $className.* ../../BatCommon/analysis\n";

system("chmod +x cleanup.scr");

################################################### DONE! now followup...

print "\nDone!\n";
print"\n\nNote that this script only produces a processing configuration file for the default Soudan setup.  If you want to run BatRoot on data taken at a different setup (e.g. test facilities, monte carlo, etc.) follow the example in the processingSoudanData.Default to add to the appropriate lines to your desired processing config file.\n";
print "\n*****************************\n";
print "\nIf you have run this script correctly then you should now run the cleanup script \(cleanup.scr\)."; 
print "\n\nWARNING: DO NOT RUN THE cleanup script UNTIL you are satisfied with the autogenerated code you have just made!\n\n";  
print "The cleanup script copies the generated files into the working areas of BatRoot and your class will then be completely merged. At that point you cannot easily remove the class or changes its name, but instead must manually go into each file and modify the lines of code related to your class (not hard but it is tedious).";  
print "\n\nIf you don't like the outcome of this auto_analysis session (i.e. you want to change the name of your class or one of the options you chose), then simply run the script again.  \n\n";
print "\nCongratulations, you are now ready to merge your code with BatRoot!\n\n\n";




