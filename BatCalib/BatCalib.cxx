///////////////////////////////////////////////////////////////////////////////// 
//main()
//Author:  L. Hsu, B. Serfass, Mark Kos
//Description: The central routine for running BatCalib (heavily copied from BatRoot main)
//
//File Import By: L. Hsu
//Creation Date: Dec. 23, 2009
//
//Modifications:
//  20111028  M. Kelsey / B. Serfass -- Add BATCALIB_CONST and BATCALIB_PROC, with
//		default values.  Provide default value for CDMSBATSDIR
////////////////////////////////////////////////////////////////////////////////// 

//Standard Libaries
#include <iostream>
//#include "time.h"
#include <regex.h>


//ROOT Libraries
#include "TFile.h"
#include "TTree.h"

//CDMS Libraries

#include "UserDataManager.h"
#include "BatCalibIOManager.h"
#include "BatCalibTypes.h"
#include "GenRRQDataEvent.h"
#include "GenRRQDataCDMSII.h"
#include "GenRRQDatamZIP.h"
#include "GenRRQDataEndcap.h"
#include "GenRRQDataiZIPSoudan.h"
#include "GenRRQDataCDMSliteI.h"

using namespace std;

/////////////////// BEGIN MAIN //////////////////////////////

int main(int argc, char* argv[]){

   //print versions explicitly, in lieu of a -V or --version option 
   cout << "cdmsbats version: " << __CB_GIT_VERSION << endl;
   cout << "BatCommon version: " << __BC_GIT_VERSION << endl;

   // ===============
   // assignments
   // ===============

   //reading inputs to main
   if(argc < 3)  
   {
      cout <<"ERROR running BatCalib!"
	   <<"\nThe command line is: BatCalib series# dump# nevents#(optional) processingOptions(optional) calibration(optional) "
	   << endl;
      exit(1);
   }

   string inputSeries = argv[1]; 

   
   //cdmsbats directory
   string cdmsbatsdir =
     getenv("CDMSBATSDIR") ? getenv("CDMSBATSDIR") : "./";


   //batcalib directory
   string batcalibdir = cdmsbatsdir + "/BatCalib";
    
   // processing/calibration root directory
   string batcalib_settings = cdmsbatsdir + "/UserSettings/BatCalibSettings";
  

   //processing files directory
   string batcalib_proc_default = batcalib_settings + "/processing/";
   string batcalib_proc =
     getenv("BATCALIB_PROC") ? getenv("BATCALIB_PROC")
     : batcalib_proc_default;


   //analysis files directory
   string batcalib_const_default = batcalib_settings + "/calibration/";
   string batcalib_const =
     getenv("BATCALIB_CONST") ? getenv("BATCALIB_CONST")
     : batcalib_const_default;





   // ===============================================
   // Read time/date and BatCalib git tag
   // ===============================================

   //  time/data

   char date[20];
   time_t rawtime;
   struct tm* timeinfo;
   time(&rawtime);
   timeinfo = localtime(&rawtime);
   strftime(date,80,"%d-%b-%Y",timeinfo); 
   
   cout <<"========== BatCalib running  ========== " << endl;
   cout <<"Local time/date is " << asctime(timeinfo) << endl;


  // === Get git tag===
  //cout << "cdmsbats version: " << __CB_GIT_VERSION << endl;
  //cout << "BatCommon version: " << __BC_GIT_VERSION << endl;
  char cbversion[1024];
  char bcversion[1024];
  sprintf(cbversion,"%s",__CB_GIT_VERSION);
  sprintf(bcversion,"%s",__BC_GIT_VERSION);

  //decompose the git tag
  string gitTagStr_cdmsbats;
  int gitTagStr_cdmsbats_past=0;
  string gitTagStr_cdmsbats_hash;
  string gitTagStr_cdmsbats_prefix;
  string gitTagStr_batcommon;
  int gitTagStr_batcommon_past=0;
  string gitTagStr_batcommon_hash;
  string gitTagStr_batcommon_prefix;

  printf("%s\n",cbversion);
  printf("%s\n",bcversion);
  
  //set up the regex match
  regex_t version_regex;
  //string trailing="-([1-9][0-9]*)-g([a-f0-9]{40})";
  string matchversion="((.+)_)?v([0-9]+[.-][0-9]+([.-][0-9]+)?)(-([1-9][0-9]*)-g([a-f0-9]{40}))?";
  regmatch_t matchptr[8];
  char submatch[1024];
  int reti_version = regcomp(&version_regex,matchversion.c_str(),REG_EXTENDED);

  //match the cdmsbats version
  reti_version = regexec(&version_regex,cbversion,8,matchptr,0);
  if(reti_version==0){
   ostringstream gitTagStream;
   int size;

   //the version
   size = matchptr[3].rm_eo - matchptr[3].rm_so;
   strncpy(submatch,(cbversion+matchptr[3].rm_so),size);
   submatch[size]='\0';
   gitTagStream << "v" << submatch;
   gitTagStr_cdmsbats = gitTagStream.str();
   gitTagStream.str("");
   gitTagStream.clear();

   //the commit
   size = matchptr[6].rm_eo - matchptr[6].rm_so;
   strncpy(submatch,(cbversion+matchptr[6].rm_so),size);
   submatch[size]='\0';
   gitTagStream << submatch;
   if(gitTagStream.str()=="")
     gitTagStr_cdmsbats_past=0;
   else{
     istringstream gitTagStreamIn(gitTagStream.str());
     gitTagStreamIn >> gitTagStr_cdmsbats_past;
   }
   gitTagStream.str("");
   gitTagStream.clear();
   
   //the hash
   size = matchptr[7].rm_eo - matchptr[7].rm_so;
   strncpy(submatch,(cbversion+matchptr[7].rm_so),size);
   submatch[size]='\0';
   gitTagStream << submatch;
   gitTagStr_cdmsbats_hash = gitTagStream.str();
   gitTagStream.str("");
   gitTagStream.clear();

   //the prefix 
   size = matchptr[2].rm_eo - matchptr[2].rm_so;
   strncpy(submatch,(cbversion+matchptr[2].rm_so),size);
   submatch[size]='\0';
   gitTagStream << submatch;
   gitTagStr_cdmsbats_prefix = gitTagStream.str();
   gitTagStream.str("");
   gitTagStream.clear();
  }
  else{
    gitTagStr_cdmsbats="";
    gitTagStr_cdmsbats_past=0;
    gitTagStr_cdmsbats_hash="";
    gitTagStr_cdmsbats_prefix="";
  }

  //match the BatCommon version
  reti_version = regexec(&version_regex,bcversion,8,matchptr,0);
  if(reti_version==0){
   ostringstream gitTagStream;
   int size;

   //the version
   size = matchptr[3].rm_eo - matchptr[3].rm_so;
   strncpy(submatch,(bcversion+matchptr[3].rm_so),size);
   submatch[size]='\0';
   gitTagStream << "v" << submatch;
   gitTagStr_batcommon = gitTagStream.str();
   gitTagStream.str("");
   gitTagStream.clear();

   //the commit
   size = matchptr[6].rm_eo - matchptr[6].rm_so;
   strncpy(submatch,(bcversion+matchptr[6].rm_so),size);
   submatch[size]='\0';
   gitTagStream << submatch;
   if(gitTagStream.str()=="")
     gitTagStr_batcommon_past=0;
   else{
     istringstream gitTagStreamIn(gitTagStream.str());
     gitTagStreamIn >> gitTagStr_batcommon_past;
   }
   gitTagStream.str("");
   gitTagStream.clear();
   
   //the hash
   size = matchptr[7].rm_eo - matchptr[7].rm_so;
   strncpy(submatch,(bcversion+matchptr[7].rm_so),size);
   submatch[size]='\0';
   gitTagStream << submatch;
   gitTagStr_batcommon_hash = gitTagStream.str();
   gitTagStream.str("");
   gitTagStream.clear();

   //the prefix 
   size = matchptr[2].rm_eo - matchptr[2].rm_so;
   strncpy(submatch,(bcversion+matchptr[2].rm_so),size);
   submatch[size]='\0';
   gitTagStream << submatch;
   gitTagStr_batcommon_prefix = gitTagStream.str();
   gitTagStream.str("");
   gitTagStream.clear();
  }
  else{
    gitTagStr_batcommon="";
    gitTagStr_batcommon_past=0;
    gitTagStr_batcommon_hash="";
    gitTagStr_batcommon_prefix="";
  }

  if(gitTagStr_cdmsbats_prefix!="")
    cout << "NoiseBuilder::WriteOutputFile WARNING! proceeding with non-standard cdmsbats tag prefix, " << gitTagStr_cdmsbats_prefix << endl;

  if(gitTagStr_cdmsbats_past>0)
    cout << "NoiseBuilder::WriteOutputFile WARNING! NOT at a cdmsbats release, we are " << gitTagStr_cdmsbats_past << " commits past a release, on commit " << gitTagStr_cdmsbats_hash << endl;

  if(gitTagStr_cdmsbats==""){
    cerr << "NoiseBuilder::WriteOutputFile ERROR! attempting to proceed with cdmsbats code of UNKNOWN origin, clone the code again: "
	 << "\n\tgit clone username@nero.stanford.edu:/data/git/Reconstruction/cdmsbats " 
	 << "\n\tcd cdmsbats/BatCommon" 
	 << "\n\tgit pull " << endl;
    exit(1);
   }


  if(gitTagStr_batcommon_prefix!="")
    cout << "NoiseBuilder::WriteOutputFile WARNING! proceeding with non-standard batcommon tag prefix, " << gitTagStr_batcommon_prefix << endl;

  if(gitTagStr_batcommon_past>0)
    cout << "NoiseBuilder::WriteOutputFile WARNING! NOT at a batcommon release, we are " << gitTagStr_batcommon_past << " commits past a release, on commit " << gitTagStr_batcommon_hash << endl;

  if(gitTagStr_batcommon==""){
    cerr << "NoiseBuilder::WriteOutputFile ERROR! proceeding with batcommon code of UNKNOWN origin, clone the code again: "
	 << "\n\tgit clone username@nero.stanford.edu:/data/git/Reconstruction/cdmsbats " 
	 << "\n\tcd cdmsbats/BatCommon" 
	 << "\n\tgit pull " << endl;
    exit(1);
   }


   // Read Configuration file and auxillary files
   // ===============================================
  
   //
   // ===== configuration / user settings  files =====
   //

   UserDataManager myUserData;
  
   // -- analysis and processing configurations --
   
   // autodetect the location based on series number and then set the default configuration file
   string defaultUserSettingsData;
   string defaultCalibrationFile;
   int seriesLength = inputSeries.length(); //series length = 11 for old series format, 13 for new
   int seriesCheck = ( seriesLength > 12 ? atoi(inputSeries.substr(0, (seriesLength-11)).c_str())*10 : 
                       atoi(inputSeries.substr(0, (seriesLength-10)).c_str()) ); 
   int seriesDateAndPrefix = atoi(inputSeries.substr(0, (seriesLength-5)).c_str());

   switch (seriesCheck) 
   {
      case 0:
	 defaultUserSettingsData = "optionsSUFData.Default";
	 defaultCalibrationFile = "calibrationSUFData.Default";
	 break;
	   
      case 1: 
	 defaultUserSettingsData = "optionsSoudanData.Default";
	 defaultCalibrationFile = "calibrationSoudanData.c58";
	 break;
	 
      case 10: 
	 {
	   if(seriesDateAndPrefix > 1111100)
	   {
	     defaultUserSettingsData = "optionsSoudanData.SuperCDMS.Default";
	     defaultCalibrationFile = "calibrationSoudanData.SuperCDMS.Default";
	   }
	   else
	   {
	     if(seriesDateAndPrefix > 1101200)
	     {
	       defaultUserSettingsData = "optionsSoudanData.ITHybrid";
	       defaultCalibrationFile = "calibrationSoudanData.ITHybrid";
	     }
	     else
	     {
	       defaultUserSettingsData = "optionsSoudanData.STHybrid";
	       defaultCalibrationFile = "calibrationSoudanData.STHybrid";
	     }
	   }  
	 } 
	 break;
	 
      case 2: 
	 defaultUserSettingsData = "optionsUCBData.Default";
	 defaultCalibrationFile = "calibrationUCBData.Default";
	 break;
	 
      case 3: 
	 defaultUserSettingsData = "optionsCWRUData.Default";
	 defaultCalibrationFile = "calibrationCWRUData.Default";
	 break;
	 
      case 4:
	 defaultUserSettingsData = "optionsUFData.Default";
	 defaultCalibrationFile = "calibrationUFData.Default";
	 break;
	 
      case 70:
	 defaultUserSettingsData = "optionsUMNData.Default";
	 defaultCalibrationFile = "calibrationUMNData.Default";
         break;
         
      default:
	 cout <<"BatCalib::main() ERROR! series prefix is unrecognized" << endl;
	 exit(1);
   }

   // if desired, overwrite default options with third argument
   string userSettingsData = (argc > 4 ? argv[4] : defaultUserSettingsData);
   // now read the options file
   myUserData.ReadFile(batcalib_proc + userSettingsData); 

   // if desired, overwrite default calibration with fifth argument
   string configFile = (argc > 5 ? argv[5] : defaultCalibrationFile);
   // now read the calibration file
   myUserData.ReadFile(batcalib_const + configFile);


   // set max events to process
   int maxEventsDefault = myUserData.GetMaxEvents();
   int maxEvents = (argc > 3 ? atoi(argv[3]) : maxEventsDefault);
   

   //=========================================
   // Initialize IO Manager
   //=========================================

   //converting dumpNum to "F000#" format
   string dumpNum = argv[2]; 
   string dumpName = myUserData.GetPrefix("FILEINDEX_PREFIX");   

   if(dumpNum != "merged")
   {
     int nZeros = 4 - dumpNum.length();
     for(int zCtr=0; zCtr < nZeros; zCtr++) dumpName += "0";
     dumpName += dumpNum;
   }

   BatCalibIOManager ioManager(myUserData, inputSeries, dumpName);

   // store date/git tag and other processing info
   ostringstream gitTagStream_cdmsbats;
   ostringstream gitTagStream_batcommon;
   gitTagStream_cdmsbats << cbversion;
   gitTagStream_batcommon << bcversion;

   // store date and git tags of the input out and output files
   if(myUserData.GetIntParameter("WRITE_PROCESS_INFO")) 
      ioManager.CreateAndWriteProcessingInfoTree(date,gitTagStream_cdmsbats.str(),gitTagStream_batcommon.str());

   // store the optimal filter resolutions and date/tags
   if(myUserData.GetIntParameter("WRITE_FILTER_INFO")) 
      ioManager.CreateAndWriteFilterTree();

   // Get the detector map, key = detector number, val = detector type.
   //     Detectors set by the DO_PROCESSING flag in the user settings file
   //            AND
   //     available information in the detector config trees (if existent)
   map<int, int> detectorMap = ioManager.GetDetectorMap();

	   
   //===================================
   //  Calibrate and Calculate RRQ's
   //===================================

   //===== Event Level analysis =====

   //This is part of calibration analysis 
   // FIXME - separate out event analysis from this class
   if(myUserData.DoEventTimeProcessing())
   {
      GenRRQDataEvent genRRQEvent(ioManager); //pass ioManager by copy 
      genRRQEvent.DoEventCalc(maxEvents, myUserData);
   }

   //===== Zip Analysis  (loop over zips)

   map<int, int>::iterator it;
   for(it = detectorMap.begin(); it!=detectorMap.end(); it++)
   {    
      int detNum = it->first;
      int detType = it->second;

      cout <<"\nIn BatCalib, on zip " << detNum <<", which is of type = " << detType << endl;
      
      // ------- Apply Charge and Phonon Calibration, make RRQ's
      
      if( myUserData.DoZipAlgorithm(detNum, "ZipCalibration") ) 
      {

	// For CDMSII style detectors and early mercedes - use this for rq files generated before 2011  
	// FIXME add a check for production date and include mZIPs
	if(detType == BatCalibTypes::kZIPDetType || detType == BatCalibTypes::kDualEndcapDetType)
	{
	  GenRRQDataCDMSII genRRQDataCDMSII(ioManager); //pass ioManager by copy 
	  genRRQDataCDMSII.DoCalibration(maxEvents, detNum, myUserData);
	}

	// For Soudan iZIPs
	if(detType == BatCalibTypes::kiZIPSoudanTriFold)
	{
	  GenRRQDataiZIPSoudan genRRQDataiZIPSoudan(ioManager); //pass ioManager by copy 
	  genRRQDataiZIPSoudan.DoCalibration(maxEvents, detNum, myUserData);
	}


	// For mZIPs
	if(detType == BatCalibTypes::kmZIPDetType)
	{
	  GenRRQDatamZIP genRRQDatamZIP(ioManager); //pass ioManager by copy 
	  genRRQDatamZIP.DoCalibration(maxEvents, detNum, myUserData);
	}


	// For endcaps 
	if(detType == BatCalibTypes::kEndcapDetType)
	{
	  GenRRQDataEndcap genRRQDataEndcap(ioManager); //pass ioManager by copy 
	  genRRQDataEndcap.DoCalibration(maxEvents, detNum, myUserData);
	}


        // For CDMSLITE
        if (detType == BatCalibTypes::kCDMSliteIDetType)
         {
           GenRRQDataCDMSliteI  genRRQDataCDMSliteI(ioManager); //pass ioManager by copy 
    	   genRRQDataCDMSliteI.DoCalibration(maxEvents, detNum, myUserData);
	}

	  


      }

   }
   
  cout <<"Goodbye from BatCalib!" << endl;

  return 0;

} //end main()   DONE!!!


