///////////////////////////////////////////////////////////////////////////////////////////////////////
//Class Name: BatNoise
//Authors: L. Hsu, M. Kos, B. Serfass
//Description:  This main program manages the processing of the noise files
//
//File Import By: L. Hsu
//Creation Date: Nov. 17, 2008
//
//Modifications:
//
//   20171229  N. Mast  --  Add baseline slope noise selection
///////////////////////////////////////////////////////////////////////////////////////////////////////

//Standard Libaries
#include <iostream>
#include <sys/dir.h>
#include <regex.h>

//ROOT Libraries
#include "TFile.h"
#include "TTree.h"

//CDMS Libraries
#include "RawDataReader.h"
#include "UserDataManager.h"
#include "IsrDataManager.h"
#include "InfoDataManager.h"
#include "TemplateDataManager.h"
#include "UserDataManager.h"

#include "AdminData.h"
#include "HistoryData.h"
#include "PulseData.h"
#include "NoiseBuilder.h"
#include "PulseTools.h"

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
      cout <<"ERROR running BatNoise!"
	   <<"\nThe command line is: /BatNoise series# dump# nevents#(optional) processingOptions(optional) analysisConfig(optional)"
	   << endl;
      exit(1);
   }

   string inputSeries = argv[1]; 
   string dumpNum = argv[2]; 

   //converting dumpNum to "F000#" format
   int nZeros = 4 - dumpNum.length();
   string dumpName("F");
   for(int zCtr=0; zCtr < nZeros; zCtr++) dumpName += "0";
   dumpName += dumpNum;

   // construct filename from series and dumpNum
   string rawDataFilename = Form("%s_%s", inputSeries.c_str(), dumpName.c_str());

   cout <<"RawDataFilename = " << rawDataFilename << endl;

   //cdmsbats directory
   string cdmsbatsdir =
     getenv("CDMSBATSDIR") ? getenv("CDMSBATSDIR") : "./";
 

   //batroot directory
   string batnoisedir = cdmsbatsdir + "/BatNoise";
    
   // processing/analysis root directory 
   string batroot_settings = cdmsbatsdir + "/UserSettings/BatRootSettings";
  

   //processing files directory
   string batroot_proc_default = batroot_settings + "/processing";
   string batroot_proc =
     getenv("BATROOT_PROC") ? getenv("BATROOT_PROC")
     : batroot_proc_default;


   //analysis files directory
   string batroot_const_default = batroot_settings + "/analysis";
   string batroot_const =
     getenv("BATROOT_CONST") ? getenv("BATROOT_CONST")
     : batroot_const_default;



   //detector status files directory
   string batroot_detstatus_default = batroot_settings + "/detector_status";
   string batroot_detstatus =
     getenv("BATROOT_DETSTATUS") ? getenv("BATROOT_DETSTATUS")
     : batroot_detstatus_default;





   // ===============================================
   // Read Configuration file and auxillary files
   // ===============================================
  
   UserDataManager myUserData;

   // -- analysis and processing configurations --
   
   // autodetect the location based on series number and then set the default configuration file
   bool   isTFData = false;
   
   string defaultUserOptionsFile = "processingSoudanData.SuperCDMS";
   string defaultAnalysisConfigFile = "configSoudanData.SuperCDMS";
   string defaultDetectorStatusFile = "detectorStatus.SuperCDMS";

   int seriesLength = inputSeries.length(); //series length = 11 for old series format, 13 for new
   int seriesCheck = ( seriesLength > 12 ? atoi(inputSeries.substr(0, (seriesLength-11)).c_str())*10 : 
		       atoi(inputSeries.substr(0, (seriesLength-10)).c_str()) ); 
   int seriesDateAndPrefix = atoi(inputSeries.substr(0, (seriesLength-5)).c_str());

   switch (seriesCheck) 
   {
      case 0:
	 defaultUserOptionsFile = "processingSUFData.Default";
	 defaultAnalysisConfigFile = "configSUFData.Default";
	 break;
	 
      case 1: 
	 defaultUserOptionsFile = "processingSoudanData.Default";
	 defaultAnalysisConfigFile = "configSoudanData.c58";
	 break;

      case 10: 
	 {
	   if(seriesDateAndPrefix > 1111100)
	   {
	     defaultUserOptionsFile = "processingSoudanData.SuperCDMS.Default";
	     defaultAnalysisConfigFile = "configSoudanData.SuperCDMS.Default";
             defaultDetectorStatusFile = "detectorStatus.SuperCDMS";
	   }
	   else
	   {
	     if(seriesDateAndPrefix > 1101200)
	     {
	       defaultUserOptionsFile = "processingSoudanData.ITHybrid";
	       defaultAnalysisConfigFile = "configSoudanData.ITHybrid";
	     }
	     else
	     {
	       defaultUserOptionsFile = "processingSoudanData.STHybrid";
	       defaultAnalysisConfigFile = "configSoudanData.STHybrid";
	     }
	   }
	 }
	 break;
	 
      case 2: 
	 defaultUserOptionsFile = "processingUCBData.Default";
	 defaultAnalysisConfigFile = "configUCBData.Default";
         isTFData = true;
	 break;

      case 3: 
	 defaultUserOptionsFile = "processingCWRUData.Default";
	 defaultAnalysisConfigFile = "configCWRUData.Default"; 
         isTFData = true;
	 break;

      case 4:
	 defaultUserOptionsFile = "processingUFData.Default";
	 defaultAnalysisConfigFile = "configUFData.Default";
         isTFData = true;
	 break;
	 
      case 60:
         defaultUserOptionsFile = "processingQueensData.Default";
	 defaultAnalysisConfigFile = "configQueensData.Default";
         isTFData = true;
	 break;

      case 70:
	 defaultUserOptionsFile = "processingUMNData.iZIP100mm";
	 defaultAnalysisConfigFile = "configUMNData.iZIP100mm";
	 isTFData = true;
	 break;

      case 90:
	 defaultUserOptionsFile = "processingSLACData.iZIP100mm.midas";
	 defaultAnalysisConfigFile = "configSLACData.iZIP100mm.midas";
         isTFData = true;
	 break;
	 
      case 510:
	 defaultUserOptionsFile = "processingDMC.iZIPDefault";
	 defaultAnalysisConfigFile = "configDMC.iZIPDefault";
	 break;

      default:
	 cout <<"BatNoise::main() ERROR! series prefix is unrecognized" << endl;
	 exit(1);
   }

   // if desired, overwrite default processing options with fourth argument
   string userOptionsFile = (argc > 4 ? argv[4] : defaultUserOptionsFile);
   // now read the processing file
   myUserData.ReadFile(batroot_proc + "/" + userOptionsFile); 

   // if desired, overwrite default analysis config with fifth argument
   string configFile = (argc > 5 ? argv[5] : defaultAnalysisConfigFile);
   // now read the analysis configuration file
   myUserData.ReadFile(batroot_const + "/" + configFile);

   // set max events to process
   int maxEventsDefault = myUserData.GetMaxEvents();
   int maxEvents = (argc > 3 ? atoi(argv[3]) : maxEventsDefault);
  
   //
   // ===== INFO file =====
   //

   InfoDataManager myInfoData;

   string infoFile = myUserData.GetPath("AUX_FILES") + inputSeries + ".info"; 
   
   if (myUserData.DoRead("INFO_FILE")) 
        myInfoData.ReadFile(infoFile);
   else
       cout <<"\nNOTE:  INFO is not being read.  Values will derive from either raw data or user settings file."<< endl;
   

   //
   // ===== ISR file =====
   //

   IsrDataManager myIsrData(myUserData.GetMaxZIPs());

   string isrFile = myUserData.GetPath("AUX_FILES") + inputSeries + ".isr"; 
   if (myUserData.DoRead("ISR_FILE")) 
          myIsrData.ReadFile(isrFile);
    else
       cout <<"\nNOTE:  ISR file is not being read. Values will derive from either raw data or user settings file." << endl;
    
   //=================================================
   // Check the directory for the file and get the right extension [ANV] 
   //=================================================
   
   //do some regex matching to parse out series and dump
   regex_t regex;
   string matchfile="("+rawDataFilename+")(\\.mid|\\.mid\\.gz|\\.gz)?$";
   int reti = regcomp(&regex,matchfile.c_str(),REG_EXTENDED);

   //browse the directory for the correct file
   //so we can learn about the filenames in terms of extension [ANV]
   cout << myUserData.GetPath("RAW_DATA") << endl;
   DIR *dir;
   struct dirent *ent;
   string fullRawDataFilename="";
   if ((dir = opendir (myUserData.GetPath("RAW_DATA").c_str())) != NULL) {
     while ((ent = readdir (dir)) != NULL) {
       string filename(ent->d_name);
       regmatch_t matchptr[4];
       reti = regexec(&regex,filename.c_str(),4,matchptr,0);
       if(!reti){
         fullRawDataFilename = filename;
       }
     }
     closedir (dir);
   }
   else{
     cerr << "BatNoise: ERROR! could not open raw directory" << endl;
     exit(1); 
   }

   if(fullRawDataFilename==""){
     cerr << "BatNoise: ERROR! requested raw file does not exist" << endl;
     exit(1); 
   }

   //=================================================
   // Initialize DetectorConfigManager
   //=================================================

   // opens raw file and, if it exists, reads the detector config record
   DetectorConfigManager detConfigManager(myUserData, fullRawDataFilename);
   
   // register external data classes if available
   if( myUserData.DoRead("INFO_FILE") ) 
     detConfigManager.RegisterInfo(myInfoData);
   
   if( myUserData.DoRead("ISR_FILE") ) 
      detConfigManager.RegisterIsr(myIsrData);



   //=================================================
   // Construct list of ZIP detectors 
   //=================================================

   // detectors to be processed:
   //    detectors set by the DO_PROCESSING flag in configuration file
   //            AND
   //    available information in the ISR file (if file is read)


   // get the detector map from the detector config manager
   map<int, int> detectorMap = detConfigManager.GetDetectorMap();


   // get the charge polarity type  map from the detector config manager
   map<int, string> detectorChargePolarityMap = detConfigManager.GetDetectorChargePolarityMap();

   
   //=================================================
   // Detector status RQ (from UserSetting file)
   //=================================================
   
   //  The detector status RQs depend only on the SeriesNumber so
   //  calculation can be done outside the event loop.
   //  However, the RQs will be stored event by event in ZIP
   //  trees

   if (myUserData.DoRead("DET_STATUS_FILE")) {

     // read detector status 
     myUserData.ReadFile(batroot_detstatus + "/" + defaultDetectorStatusFile);
   
     // construct RQ list
     myUserData.ConstructZipRQList(detectorMap);
     
     // Fill RQs 
     myUserData.DoCalcDetectorStatus(inputSeries);


     // Fill broken channel and charge side list
     // (using RQs, which thus need to be calculated)
     //
     // Considered "broken"
     //     Phonon: status = 2  (=> not included in pt)
     //     charge: status>0    (=> single pulse fitting, no 2X2 or Z constraint)
     //        (if either QI or QO broken: charge side = broken 
    
     myUserData.FillBrokenChannelLists();

      
   }


   //=================================================
   // Pulse Template
   //=================================================
   
   string templateDir = 
     getenv("BATNOISE_TEMPLATES") ? getenv("BATNOISE_TEMPLATES")
     : "PulseTemplates";	// Under "cdmsbats"

   // reads only if the detectors are configured 
   //     AND 
   // if CALC_PHONON_TEMPLATE or CALC_CHARGE_TEMPLATE are false in processing settings
   TemplateDataManager myTemplateData;
   myTemplateData.ReadAllFiles(templateDir, inputSeries, detectorMap, detectorChargePolarityMap, myUserData);


   //=================================================
   // Initialize Noise Builder
   //=================================================
  
   //opens file and reads file header
   NoiseBuilder noiseBuilder(myUserData, detConfigManager,
			     myTemplateData); 

   // Initialize Output Variables Lists/Trees   
   noiseBuilder.ConfigureOutputFile(Form("%s.root", inputSeries.c_str())); 

   
   //=======================================================
   // FIRST pass loop over events.  Here we fill histograms 
   // to establish the noise cuts.
   //=======================================================

   noiseBuilder.OpenRawFile(fullRawDataFilename);
   int firstPassEvtCtr = 0;
   while(firstPassEvtCtr < maxEvents && noiseBuilder.ReadNextEvent() != 0)
   {
      int event = noiseBuilder.GetAdmin().GetEvent(); 

      cout <<"\nFilling noise selection histograms, event number = " << event
	   << endl;

      // ==== Loop over requested ZIPs and construct noise selection histograms ====

      map<int, int>::iterator it;
      for(it = detectorMap.begin(); it!=detectorMap.end(); it++)
      {

	 int detNum = it->first;

	 //sum all pulses for each sensor type (for minmax calculation)
	 noiseBuilder.CalcSumOfPulses(detNum); 

	 // === preselection of pulses used to make selection histograms ===

	 // skip all pulses in this event if its not a random trigger - don't apply this cut to TF data
	 if(!isTFData && !noiseBuilder.PassRandomTriggerCut()) { continue; }

	 // apply a saturation cut (in place of old "No Joerger's Railed" cut from DarkPipe)
	 if(!noiseBuilder.PassSaturationCut(detNum)) { continue; }
	 
	 // construct minmax distribution
	 if( myUserData.GetIntParameter(detNum, "DO_MINMAX") )  
	   { noiseBuilder.ConstructMinMaxDistribution(detNum); }

	 // construct pileup distribution
	 if( myUserData.GetIntParameter(detNum, "DO_PILEUP_CUT") )  
	   { noiseBuilder.ConstructPileUpDistribution(detNum); }
      
         // construct Phonon Chisq cut
       	 if( myUserData.GetIntParameter(detNum, "DO_PHONON_CHISQ_CUT") )  
	      noiseBuilder.ConstructPOFchisqDistribution(detNum);

	 // construct Phonon BSslope distribution
       	 if( myUserData.HasIntParameter(detNum, "DO_PTBSslope_CUT") )
	 {
	   if( myUserData.GetIntParameter(detNum, "DO_PTBSslope_CUT") )
	     noiseBuilder.ConstructPTBSslopeDistribution(detNum);
	 }

      
      } //end loop over zips

      firstPassEvtCtr++;

   }  //Done with first pass through events!

   //=======================================================
   // Compute cuts based on stored data
   //=======================================================

   map<int, int>::iterator it;
   for(it = detectorMap.begin(); it!=detectorMap.end(); it++)
   {   
      int detNum = it->first;

      if( myUserData.GetIntParameter(detNum, "DO_MINMAX") )  
	noiseBuilder.CalcMinMaxCut(detNum);

      if( myUserData.GetIntParameter(detNum, "DO_PILEUP_CUT") )  
	noiseBuilder.CalcPileupCut(detNum, isTFData);

      if( myUserData.GetIntParameter(detNum, "DO_PHONON_CHISQ_CUT") )  
	 noiseBuilder.CalcPOFchisqCut(detNum); 

   }   

   //=======================================================
   // Second pass loop over events.  Here we apply noise
   // cuts and compute the average PSD.
   //=======================================================

   noiseBuilder.ResetRawFile();
   int secondPassEvtCtr = 0;
   while(secondPassEvtCtr < maxEvents && noiseBuilder.ReadNextEvent() != 0)
   {
      int event = noiseBuilder.GetAdmin().GetEvent(); 

      cout <<"\nMCF TEST Selecting noise pulses, event number = " << event
	   << endl;

      // ==== Loop over Zips and build average PSD's ====

      map<int, int>::iterator it;
      for(it = detectorMap.begin(); it!=detectorMap.end(); it++)
      {
	 int detNum = it->first;

	 noiseBuilder.CalcSumOfPulses(detNum); //sum all pulses (for each sensor type for minmax calculation)

	 // === noise selection cuts ===

	 //skip all pulses in this event if its not a random trigger - don't apply this cut to TF data
	 if(!isTFData && !noiseBuilder.PassRandomTriggerCut()) { continue; }

	 // Apply a saturation cut (in place of old "No Joerger's Railed" cut from DarkPipe)
	 if(!noiseBuilder.PassSaturationCut(detNum)) { continue; }

	 // Apply MinMax cut
	 if(myUserData.GetIntParameter(detNum, "DO_MINMAX") && !noiseBuilder.PassMinMaxCut(detNum)) 
	   { continue; }

	 // Apply Pileup cut
	 if(myUserData.GetIntParameter(detNum, "DO_PILEUP_CUT") && !noiseBuilder.PassPileupCut(detNum)) 
	   { continue; }

         // Apply OF chisq cut (phonon)
	 if(myUserData.GetIntParameter(detNum, "DO_PHONON_CHISQ_CUT") && !noiseBuilder.PassPOFchisqCut(detNum)) 
	   { continue; }

         // Apply PTBSslope cut
	 if( myUserData.HasIntParameter(detNum, "DO_PTBSslope_CUT") )
	 {
	   if(myUserData.GetIntParameter(detNum, "DO_PTBSslope_CUT") && !noiseBuilder.PassPTBSslopeCut(detNum)) 
	     { continue; }
	 }

	 // ==== option to store the traces ====
	 if( myUserData.GetIntParameter("WRITE_NOISE_PULSES") ) { noiseBuilder.StorePulses(detNum); }


	 // ==== build average PSD ===

	 // if the event on this detector made it to here, add it to the average PSD
	 noiseBuilder.BuildAveragePSD(detNum); 

	 // calculate the average covariance (for special studies only) 
	 if( myUserData.DoAlgorithm(detNum, "charge", "ChargeNoiseCovariance") )
	     noiseBuilder.BuildQIQOCov(detNum);

      }

      secondPassEvtCtr++;

   }  //Done with second pass through events!

   //Calculate noise quantities from average psd's
      
   //==================================================================== 
   // Last loop over Zips.  Calculate noise quantities from average psd's 
   //==================================================================== 

   for(it = detectorMap.begin(); it!=detectorMap.end(); it++)
   {   
      int detNum = it->first;

      noiseBuilder.CalcNoiseQuantities(detNum, secondPassEvtCtr);
   }   

   //Write noise quantities and selection histograms
   noiseBuilder.WriteOutputFile();

   return 0;

} //end main()   DONE!!!


