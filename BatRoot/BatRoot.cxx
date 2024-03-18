///////////////////////////////////////////////////////////////////////////////// 
//main()
//Author:  L. Hsu, M. Kos, B. Serfass
//Description: The central routine for running BatRoot
//
//File Import By: L. Hsu
//Creation Date: Nov. 17, 2008
//
//Modifications:
//
////////////////////////////////////////////////////////////////////////////////// 

//Standard Libaries
#include <iostream>
#include <sys/dir.h>
#include <list>
#include "time.h"
#include <regex.h>


//ROOT Libraries
#include "TFile.h"
#include "TTree.h"

//CDMS Libraries

#include "RawDataReader.h"
#include "AdminData.h"
#include "GPSData.h"
#include "HistoryData.h"
#include "PulseData.h"
#include "EventBuilder.h"
#include "TriggerData.h"
#include "DatabaseManager.h"

#include "UserDataManager.h"
#include "DmmDataManager.h"
#include "IsrDataManager.h"
#include "InfoDataManager.h"
#include "GpibDataManager.h"
#include "FilterDataManager.h"

#include "DetectorConfigManager.h"
#include "BatOutputManager.h"
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
      cout <<"ERROR running BatRoot!"
	   <<"\nThe command line is: ./BatRoot series# dump# nevents#(optional) processingOptions(optional) analysisConfig(optional)"
	   << endl;
      exit(1);
   }

   string inputSeries = argv[1]; 


   //cdmsbats directory
   string cdmsbatsdir =
     getenv("CDMSBATSDIR") ? getenv("CDMSBATSDIR") : "./";
 
  
   //batroot directory
   string batrootdir = cdmsbatsdir + "/BatRoot";
    
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
   // Read time/date and BatRoot git tag
   // ===============================================

   //  time/data

   char date[20];
   time_t rawtime;
   struct tm* timeinfo;
   time(&rawtime);
   timeinfo = localtime(&rawtime);
   strftime(date,80,"%d-%b-%Y",timeinfo); 
   
   cout <<"\n========== BatRoot processing  ========== " << endl;
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

  cout << endl;
  cout << "cdmsbats version: "  << gitTagStr_cdmsbats << endl;
  cout << "cdmsbats version prefix: "  << gitTagStr_cdmsbats_prefix << endl;
  if(gitTagStr_cdmsbats_past>0)
    cout << "NOT at a release, we are " << gitTagStr_cdmsbats_past << " commits past a release, on commit "  << gitTagStr_cdmsbats_hash << endl;

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

  cout << endl;
  cout << "batcommon version: "  << gitTagStr_batcommon << endl;
  cout << "batcommon version prefix: "  << gitTagStr_batcommon_prefix << endl;



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


   // ===============================================
   // Read Configuration file and auxillary files
   // Construct Detector list
   // ===============================================
  
   //
   // ===== configuration / user settings  files =====
   //

   UserDataManager myUserData;
  
   // -- analysis and processing configurations --
   
   // autodetect the location based on series number and then set the default configuration file
   string defaultUserOptionsFile = "processingSoudanData.SuperCDMS.Default";
   string defaultAnalysisConfigFile = "configSoudanData.SuperCDMS.Default";
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
	 break;

      case 3: 
	 defaultUserOptionsFile = "processingCWRUData.Default";
	 defaultAnalysisConfigFile = "configCWRUData.Default";
	 break;

      case 4:
	 defaultUserOptionsFile = "processingUFData.Default";
	 defaultAnalysisConfigFile = "configUFData.Default";
	 break;
      
      case 60:
         defaultUserOptionsFile = "processingQueensData.Default";
	 defaultAnalysisConfigFile = "configQueensData.Default";
	 break;

      case 70:
	 defaultUserOptionsFile = "processingUMNData.iZIP100mm";
	 defaultAnalysisConfigFile = "configUMNData.iZIP100mm";
	 break;

      case 510:
	 defaultUserOptionsFile = "processingDMC.iZIPDefault";
	 defaultAnalysisConfigFile = "configDMC.iZIPDefault";
	 break;
	 
      default:
	 cout <<"BatRoot::main() ERROR! series prefix is unrecognized" << endl;
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
   // ===== construct the raw data filename =====
   //

   // construct filename from series and dumpNum
   string dumpNum = argv[2]; 
   int nZeros = 4 - dumpNum.length();
   string dumpName = myUserData.GetPrefix("FILEINDEX_PREFIX");
   for(int zCtr=0; zCtr < nZeros; zCtr++) dumpName += "0";
   dumpName += dumpNum;

   string rawDataFilename = Form("%s_%s", inputSeries.c_str(), dumpName.c_str());  

   
   //
   // ===== ISR file =====
   //

   IsrDataManager myIsrData(myUserData.GetMaxZIPs());

   string isrFile = myUserData.GetPath("AUX_FILES") + inputSeries + ".isr"; 
   if (myUserData.DoRead("ISR_FILE")) 
          myIsrData.ReadFile(isrFile);
    else
       cout <<"\nNOTE:  ISR file is not being read. Values will derive from either raw data or user settings file." << endl;
    

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
   // ===== GPIB change log file ===== 
   //
   
    GpibDataManager myGpibData;
   
    string gpibFile = myUserData.GetPath("GPIB_FILE") + "gpib_states_changed.log"; 
  
    if (myUserData.DoRead("GPIB_FILE"))
             myGpibData.ReadFile(gpibFile);

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
     cerr << "BatRoot: ERROR! could not open raw directory" << endl;
     exit(1); 
   }

   if(fullRawDataFilename==""){
     cerr << "BatRoot: ERROR! requested raw file does not exist" << endl;
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
   // Construct list of ZIP detectors, Read DMM and 
   // noise file
   //=================================================

   // detectors to be processed:
   //    detectors set by the DO_PROCESSING flag in configuration file
   //            AND
   //    available information in the ISR file (if file is read)


   // get the detector map from the detector config manager
   map<int, int> detectorMap = detConfigManager.GetDetectorMap();


   //
   // ===== DMM file =====
   //

   DmmDataManager myDmmData(detectorMap);
 
   string dmmFile = myUserData.GetPath("AUX_FILES") + inputSeries + ".dmm"; 
    
   if (myUserData.DoRead("DMM_FILE"))
             myDmmData.ReadFile(dmmFile);


   //
   // ===== noise and pulse templates file ===== 
   //

   FilterDataManager myFilterData(detectorMap);

   string noisePrefix = myUserData.GetPrefix("NOISE_PREFIX");
   string noiseFile = myUserData.GetPath("NOISE_FILES") + noisePrefix + inputSeries + ".root"; 
   if(myUserData.DoRead("FILTER_FILE"))
      myFilterData.ReadFile(noiseFile);
   
   
   //
   // ===== database (probably remote) ====
   //
   
   CdmsDB::DatabaseManager myDatabase;
   if(myUserData.DoRead("DATABASE")){
     string host = myUserData.GetStringParameter("DATABASE_HOST");
     string user = myUserData.GetStringParameter("DATABASE_USER");
     string name = myUserData.GetStringParameter("DATABASE_NAME");
     string pass = myUserData.GetStringParameter("DATABASE_PASSWD");
     //default is actually empty string, so use NONE as keyword...
     if(pass == "NONE" ) pass="";

     myDatabase.SetDbHost(host);
     myDatabase.SetDbUser(user);
     myDatabase.SetDbName(name);
     myDatabase.SetDbPass(pass);

     if( myDatabase.LoadSeries(inputSeries) ){
       cerr<<"BatRoot ERROR: Unable to load series info from database!\n";
       exit(1);
     }
   }


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
   // Initialize Event Builder
   //=================================================

   //opens raw file and reads file header
   EventBuilder eventBuilder(myUserData, detConfigManager, fullRawDataFilename); 

   //register external data classes - FIXME make sure usage and registration are consistent within EB! (do this after upgrading interface with DetectorConfigManager
   //store external files (Config, ISR, INFO, DMM, GPIB, and NOISE) for later use
   if( myUserData.DoRead("INFO_FILE") ) 
     eventBuilder.RegisterInfo(myInfoData);
   
   if( myUserData.DoRead("ISR_FILE") ) 
     eventBuilder.RegisterIsr(myIsrData);
   
   if( myUserData.DoRead("DMM_FILE") ) 
     eventBuilder.RegisterDmm(myDmmData);

   if( myUserData.DoRead("GPIB_FILE") ) 
     eventBuilder.RegisterGpib(myGpibData);

   if( myUserData.DoRead("FILTER_FILE") ) 
     eventBuilder.RegisterFilter(myFilterData);
   
   if( myUserData.DoRead("DATABASE") )
     eventBuilder.RegisterDatabase(myDatabase);

   //=========================================
   // Initialize Output Variables Lists/Trees
   //=========================================
    
   BatOutputManager outputManager(myUserData, detConfigManager, rawDataFilename);
   outputManager.ConstructOutputLists();

   // store date/git tag and other processing info
   ostringstream gitTagStream_cdmsbats;
   ostringstream gitTagStream_batcommon;
   gitTagStream_cdmsbats << cbversion;
   gitTagStream_batcommon << bcversion;
	
   if(myUserData.GetIntParameter("WRITE_PROCESS_INFO")) 
      outputManager.CreateAndWriteProcessingInfoTree(date,gitTagStream_cdmsbats.str(),gitTagStream_batcommon.str());

   // store user settings
   if(myUserData.GetIntParameter("WRITE_SETTINGS_INFO")) 
      outputManager.CreateAndWriteSettingsTree();

   // store detector configuration
   if(myUserData.GetIntParameter("WRITE_DETCONFIG_INFO")) 
      outputManager.CreateAndWriteDetectorConfigTree();

   // store parameters from filter file
   if(myUserData.GetIntParameter("WRITE_FILTER_INFO") && myUserData.DoRead("FILTER_FILE")) 
   {   

      myFilterData.DoCalc(detConfigManager);         
      outputManager.CreateAndWriteFilterTree(myFilterData);

   }


   
   //========================================
   // Analysis (veto,ZIP,trigger,etc.)
   //========================================

   // Get simulation input file info
   vector<string> libraryFileNames;
   if(myUserData.GetIntParameter("DO_SIM_FROM_PULSE")==1)
   {
       if(myUserData.GetIntParameter("SIM_NO_NOISE") == 1)
	 cout << "Choosing to set Noise Amplitude to Zero (i.e. kill noise and get clean DMC Pulses)" << endl;

       if(!getenv("BATROOT_PULSELIBS"))
       {
           cout << "ERROR: Library-based pulse simulation enabled but BATROOT_PULSELIBS not set!" << endl;
           exit(EXIT_FAILURE);
       }
       else
       {
           // split the environment variable into the parts pointing to each file
           string varToSplit = getenv("BATROOT_PULSELIBS");
           int prevPos = 0, pos;
           while((pos = varToSplit.find(":",prevPos)) != string::npos)
           {
               if(pos > prevPos)
                   libraryFileNames.push_back(varToSplit.substr(prevPos, pos-prevPos));
	       prevPos = pos+1;
           }
           if(prevPos < varToSplit.length())
               libraryFileNames.push_back(varToSplit.substr(prevPos, string::npos));
           eventBuilder.SetPulseLibManager(libraryFileNames);
       }
   }

   int nSimPerEvt = 1;
   if(myUserData.GetIntParameter("DO_SIM_FROM_PULSE")==1 || myUserData.GetIntParameter("DO_SIM_FROM_TEMPLATE")==1)
   {
       std::string energyInputPath = getenv("BATROOT_ENERGYINPUTDIR");
       eventBuilder.SetSimDataManager(energyInputPath + "/pulseSim_input_" + inputSeries + "_" + dumpNum + ".dat");
       nSimPerEvt = myUserData.GetIntParameter("SIM_N_TIMES_USE_RANDOM");
   }


   // Outer loop to allow multiple simulation events to be constructed
   // out of a single random. Default value of nSimPerEvt is 1, so this
   // loop is only run more than once if pulse simulation is activated. [AJA]
   int simEvtCtr = 0;
   for(int jEvtSim = 0; jEvtSim < nSimPerEvt; jEvtSim++)
   {
   cout << "Iterating data events for " << jEvtSim << " time\n" ; 
   //Loop over events   
 
   //all raw data records are read with call to ReadNextEvent
   //at this time history and trigger record analysis is done
   int evtCtr = 0;
   while(evtCtr < maxEvents && eventBuilder.ReadNextEvent() != 0)
   {
      //
      // ======= Getting Admin Info  ========
      //
      uint64_t series = eventBuilder.GetAdmin().GetSeries();
      int event = eventBuilder.GetAdmin().GetEvent(); 

      // if in sim mode, only run on randoms
      if(myUserData.GetIntParameter("DO_SIM_FROM_TEMPLATE")==1 || myUserData.GetIntParameter("DO_SIM_FROM_PULSE")==1)
      {
	if(eventBuilder.GetEventCategory() != 0x1){
	      continue;
	} else{
	      eventBuilder.ReadSimEvent();
        }
	  cout <<"\nSIM Event Number = " << simEvtCtr ;
      }

      cout <<"\nSeries Number = " << series
	   <<"\nEvent Number = " << event
	   << endl;


      //
      // ========= Veto Analysis ==========
      //


      if(myUserData.DoVetoProcessing()) 
  	       eventBuilder.DoVetoAnalysis();

      //
      // ========= NoiseMonitor Analysis ==========
      //


      if(myUserData.DoNoiseMonitorProcessing()) 
  	       eventBuilder.DoNoiseMonitorAnalysis();

      

      //
      // ========= Timing Analysis ==========
      //

      // GPIB (Flash Times) and ISR file (LastISRTime)

      if (myUserData.DoRead("GPIB_FILE"))
                      eventBuilder.DoGpibTimingCalc();

      // ISR (LastISRTime)
      if (myUserData.DoRead("ISR_FILE"))
                      eventBuilder.DoIsrTimingCalc();

      //database
      if ( myUserData.DoRead("DATABASE") )
	eventBuilder.DoDatabaseEventCalc();


      // 
      // =========  ZIP Analysis ===========
      //

      // loop ZIP selected by user in config file 
      //    AND 
      // with either raw data detector config or ISR information
 
      map<int, int>::iterator it;
      for(it = detectorMap.begin(); it!=detectorMap.end(); it++)
      {
        
  	 int detNum = it->first;
         int detType = it->second;
      
         // 
         // ------ Detector related DMM files calculations ----
         // 

          if (myUserData.DoRead("DMM_FILE") & myDmmData.FileExists())
                    eventBuilder.DoDmmCalc(detNum);
                      
	 
	  //-- per-detector database stuff
	  if(myUserData.DoRead("DATABASE"))
	    eventBuilder.DoDatabasePulseCalc(detNum);
	 //
	 //  --------  basic pulse calculations ---------
         //
         
         // Calculations done: 
         //  - baseline substraction
         //  - pulses normalization
         //  - RMS calculation
         //  - calculate sum phonon pulses

	  eventBuilder.DoBasicPulseCalc(detNum);   // not permitted to turn this off !
    


	  //
	  // ------- activate pulse simulation if requested -----
	  //

	  // DO_SIM_FROM_PULSE mode
	  // Write over the random trace with the random plus another event
	  // taken from the pulse library specified in $BATROOT_PULSELIB,
	  // the energy is given in $BATROOT_ENERGYINPUT [AJA] [updated JDM]
	  if(myUserData.GetIntParameter("DO_SIM_FROM_PULSE")==1){
	    if(myUserData.GetIntParameter("RANDOM_SIM_ORDER")==1)
	      eventBuilder.DoSimulateFromPulse(detNum, libraryFileNames,-1);
	    else
	      eventBuilder.DoSimulateFromPulse(detNum, libraryFileNames,simEvtCtr);
	  }

	  // DO_SIM_FROM_TEMPLATE mode
	  // Write over the random trace with a pulse constructed from OF
	  // pulse templates [AJA]
	  if(myUserData.GetIntParameter("DO_SIM_FROM_TEMPLATE")==1)
	  {
	      if(myUserData.GetIntParameter("DO_CHARGESIM")==1)
		  eventBuilder.DoSimulateChargeFromRandoms(detNum);

	      if(myUserData.GetIntParameter("DO_PHONONSIM")==1)
		  eventBuilder.DoSimulatePhononFromRandoms(detNum);
	  }

	      


	 //
	 //  --------- Phonon Pulse Algorithms ---------
         //

	 // ......... Order matters here!  ............. 


         // All channels

         if( myUserData.DoAlgorithm(detNum,"phonon", "InflectionTime") ) 
           eventBuilder.DoInflectionTime(detNum, "phonon");

    	 
  	 if( myUserData.DoAlgorithm(detNum, "phonon", "OptimalFilterPhonon")) 
  	   eventBuilder.DoOptimalFilterPhonon(detNum, "phonon");


	 if(myUserData.DoAlgorithm(detNum, "phonon","OptimalFilterPhononDMC"))
	   eventBuilder.DoOptimalFilterPhononDMC(detNum, "phonon");

	 
	 if( myUserData.DoAlgorithm(detNum, "phonon", "OptimalFilterPhonon1X2") ) 
	   eventBuilder.DoOptimalFilterPhonon1X2(detNum, "phonon");


 	 if( myUserData.DoAlgorithm(detNum, "phonon", "ConstFreqRTFTWalkPhonon") ) 
	   eventBuilder.DoConstFreqRTFTWalkPhonon(detNum, "phonon", "filtered");
	 

  	 if( myUserData.DoAlgorithm(detNum, "phonon", "VarFreqRTFTWalkPhonon") )
  	   eventBuilder.DoVarFreqRTFTWalkPhonon(detNum, "phonon", "filtered"); 


         if( myUserData.DoAlgorithm(detNum, "phonon", "PulseIntegral") )
           eventBuilder.DoPulseIntegral(detNum, "phonon", "filtered");  
	 

  	 if( myUserData.DoAlgorithm(detNum,  "phonon", "NoiseSelector") )
 	   eventBuilder.DoNoiseSelector(detNum, "phonon"); 


  	 if( myUserData.DoAlgorithm(detNum, "phonon", "PipeFitPhonon") )
  	   eventBuilder.DoPipeFitPhonon(detNum, "phonon");  
	 

	 if( myUserData.DoAlgorithm(detNum, "phonon", "WedgeFitPhonon") )
	   eventBuilder.DoWedgeFitPhonon(detNum, "phonon"); 


     
         // PT only
         
         if( myUserData.DoAlgorithm(detNum, "PT", "OptimalFilterPhononGlitch1")) 
  	   eventBuilder.DoOptimalFilterPhononGlitch1(detNum, "phonon");
            

         if( myUserData.DoAlgorithm(detNum, "PT", "OptimalFilterPhononLFnoise1")) 
  	   eventBuilder.DoOptimalFilterPhononLFnoise1(detNum, "phonon");
                        
  
         if( myUserData.DoAlgorithm(detNum, "PT", "OptimalFilterPhononNS") ) 
            eventBuilder.DoOptimalFilterPhononNS(detNum, "phonon");

         
         if( myUserData.DoAlgorithm(detNum, "PT", "PSDIntegralPhonon") ) 
           eventBuilder.DoPSDIntegralPhonon(detNum, "phonon");
	 
 
        


         //
 	 //  --------  Charge Algorithms ---------------
         //

  	 if( myUserData.DoAlgorithm(detNum, "charge", "RTFTWalkCharge") ) 
  	   eventBuilder.DoRTFTWalkCharge(detNum, "charge", "filtered");


  	 if( myUserData.DoAlgorithm(detNum, "charge", "OptimalFilterCharge") ) 
  	   eventBuilder.DoOptimalFilterCharge(detNum);

         
         if( myUserData.DoAlgorithm(detNum, "charge", "OptimalFilterChargeX") ) 
         {
           if (detType==BatRootTypes::kiZIPSoudan) {
              eventBuilder.DoOptimalFilterChargeX(detNum,"S1");
	      eventBuilder.DoOptimalFilterChargeX(detNum,"S2");
           } else {
              eventBuilder.DoOptimalFilterChargeX(detNum);
           }
         }



         if( myUserData.DoAlgorithm(detNum, "charge", "OptimalFilterCharge2X2") )
           eventBuilder.DoOptimalFilterCharge2X2(detNum);
     	 
 
         if( myUserData.DoAlgorithm(detNum, "charge", "F5ChargeX") ) 
         {
           if (detType==BatRootTypes::kiZIPSoudan) {
              eventBuilder.DoF5ChargeX(detNum,"S1");
	      eventBuilder.DoF5ChargeX(detNum,"S2");
           } else {
              eventBuilder.DoF5ChargeX(detNum);
           }
         }
        



         //  ----- Phonon algorithms that might need charge informations -----

         // TailfitPhonon: need charge OF delay
         if( myUserData.DoAlgorithm(detNum, "phonon", "TailFitPhonon") )
	   eventBuilder.DoTailFitPhonon(detNum, "phonon"); 





	 //  ---- additional user analysis classes - DO NOT modify or copy this comment (for auto_analysis) ----



      }  // ===== End of ZIP loop  ======
      if(myUserData.GetIntParameter("DO_SIM_FROM_PULSE")==1)
	if(myUserData.GetIntParameter("RANDOM_SIM_ORDER")!=1)
	      simEvtCtr++;

      outputManager.StoreOutput(eventBuilder);
      evtCtr++;

   }
   // Rewind to the first event if in pulse simulation mode and using
   // each random more than once to construct fake data [AJA]
   if(jEvtSim < nSimPerEvt-1 && (myUserData.GetIntParameter("DO_SIM_FROM_TEMPLATE")==1 || myUserData.GetIntParameter("DO_SIM_FROM_PULSE")==1))
       eventBuilder.ResetDataReader();
   }

  //==========Done looping over events!=================



  cout <<"\nDone looping over events, now storing data!" << endl;

  outputManager.WriteTrees();

  cout <<"Goodbye from BatRoot!" << endl;

  return 0;

} //end main()   DONE!!!


