//Standard Libaries
#include <iostream>

//ROOT Libraries
#include "TFile.h"
#include "TTree.h"

//CDMS Libraries

#include "RawDataReader.h"
#include "AdminData.h"
#include "ExternalData.h"
#include "HistoryData.h"
#include "PulseData.h"
#include "EventBuilder.h"
#include "TriggerData.h"


#include "ConfigDataManager.h"
#include "DmmDataManager.h"
#include "IsrDataManager.h"
#include "InfoDataManager.h"
#include "GpibDataManager.h"
#include "FilterDataManager.h"


#include "BatOutputManager.h"
#include "PulseTools.h"

//special for this modification of main
#include "PulseFilter.h"

using namespace std;

/////////////////// BEGIN MAIN //////////////////////////////

int main(int argc, char* argv[]){

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
   string dumpNum = argv[2]; 

   //converting dumpNum to "F000#" format
   int nZeros = 4 - dumpNum.length();
   string dumpName("F");
   for(int zCtr=0; zCtr < nZeros; zCtr++) dumpName += "0";
   dumpName += dumpNum;

   // construct filename from series and dumpNum
   string rawDataFilename = Form("%s_%s", inputSeries.c_str(), dumpName.c_str());

   //batroot directory
   if(getenv("BATROOTDIR") == NULL) 
   {
     cout <<"ERROR! environmental variable $BATROOTDIR not set." << endl;
     exit(1);
   }
   string batrootdir = getenv("BATROOTDIR");



   // ===============================================
   // Read Configuration file and auxillary files
   // ===============================================
  
   //
   // ===== configuration / user settings  files =====
   //

   ConfigDataManager myConfigData;
  
   // -- analysis and processing configurations --
   
   // autodetect the location based on series number and then set the default configuration file
   string defaultUserOptionsFile;
   string defaultAnalysisConfigFile;
   int seriesCheck = atoi(inputSeries.substr(0, (inputSeries.length()-10)).c_str());

   switch (seriesCheck) 
   {
      case 0:
	 defaultUserOptionsFile = "processingSUFData.Default";
	 defaultAnalysisConfigFile = "configSUFData.Default";
	 break;
	 
      case 1: 
	 defaultUserOptionsFile = "processingSoudanData.Default";
	 defaultAnalysisConfigFile = "configSoudanData.Default";
	 break;
	 
      case 2: 
	 defaultUserOptionsFile = "processingUCBData.Default";
	 defaultAnalysisConfigFile = "configUCBData.Default";
	 break;

      case 3: 
	 defaultUserOptionsFile = "processingCWRUData.Default";
	 defaultAnalysisConfigFile = "configCWRUData.Default";
	 break;

      case 20:
	 defaultUserOptionsFile = "processingDCRCData.Default";
	 defaultAnalysisConfigFile = "configDCRCData.Default";
	 break;
	 
      default:
	 cout <<"BatRoot::main() ERROR! series prefix is unrecognized" << endl;
	 exit(1);
   }

   // if desired, overwrite default processing options with fourth argument
   string userOptionsFile = (argc > 4 ? argv[4] : defaultUserOptionsFile);
   // now read the processing file
   myConfigData.ReadFile(batrootdir + "/configuration/" + userOptionsFile); 

   // if desired, overwrite default analysis config with fifth argument
   string configFile = (argc > 5 ? argv[5] : defaultAnalysisConfigFile);
   // now read the analysis configuration file
   myConfigData.ReadFile(batrootdir + "/configuration/analysis/" + configFile);

   // set max events to process
   int maxEventsDefault = myConfigData.GetMaxEvents();
   int maxEvents = (argc > 3 ? atoi(argv[3]) : maxEventsDefault);
  
   //
   // ===== INFO file =====
   //

   InfoDataManager myInfoData;

   string infoFile = myConfigData.GetPath("AUX_FILES") + inputSeries + ".info"; 
   
   if (myConfigData.DoRead("INFO_FILE")) 
        myInfoData.ReadFile(infoFile);
   else
       cout <<"WARNING!  INFO is not being read: using default values from config file!"<< endl;
   

   //
   // ===== ISR file =====
   //

   IsrDataManager myIsrData;

   string isrFile = myConfigData.GetPath("AUX_FILES") + inputSeries + ".isr"; 
   if (myConfigData.DoRead("ISR_FILE")) 
          myIsrData.ReadFile(isrFile);
    else
       cout <<"WARNING!  ISR file is not being read.   Pulse normalizations will all be set to 1!" << endl;
    


   //
   // ===== DMM file =====
   //


   DmmDataManager myDmmData(myConfigData.GetMaxZIPs());
 
   string dmmFile = myConfigData.GetPath("AUX_FILES") + inputSeries + ".dmm"; 
    
   if (myConfigData.DoRead("DMM_FILE"))
             myDmmData.ReadFile(dmmFile);



   // ===== GPIB change log file ===== 

   
    GpibDataManager myGpibData;
   
    string gpibFile = myConfigData.GetPath("GPIB_FILE") + "gpib_states_changed.log"; 
  
    if (myConfigData.DoRead("GPIB_FILE"))
             myGpibData.ReadFile(gpibFile);



   // ===== noise and pulse templates file ===== 

   FilterDataManager myFilterData;

   string noiseFile = myConfigData.GetPath("NOISE_FILES") + "NoiseAndTemplates_" + inputSeries + ".root"; 
   myFilterData.ReadFile(noiseFile);

 
   //=================================================
   // Initialize Event Builder
   //=================================================
  

   //opens raw file and reads file header
   //store external files (Config, ISR, INFO, DMM, GPIB, and NOISE) for later use
  
    EventBuilder eventBuilder(myConfigData, myInfoData, myIsrData, myDmmData, myGpibData, myFilterData, rawDataFilename); 



   //=========================================
   // Initialize Output Variables Lists/Trees
   //=========================================
    
//    BatOutputManager outputManager(myConfigData, rawDataFilename);
//    outputManager.ConstructOutputLists();

    //Tree to store traces - Temp
    Int_t nBins = 2048;
    TFile f_out("testhisto.root", "recreate");
    
    Float_t pulse[nBins];
    Float_t pulseFlt[nBins];
    Int_t detectorNum = 999;
    Int_t channelNum = 999;
    Int_t event = 999;
    TTree pulseTree("pulseTree","a test tree");
    pulseTree.Branch("event", &event, "event/I");
    pulseTree.Branch("detectorNum", &detectorNum, "detectorNum/I");
    pulseTree.Branch("channelNum", &channelNum, "channelNum/I");
    pulseTree.Branch("pulse", pulse, Form("pulse[%d]/F",nBins));
    pulseTree.Branch("pulseFlt", pulseFlt, Form("pulseFlt[%d]/F",nBins));
    

   //========================================
   // Analysis (veto,ZIP,trigger,etc.)
   //========================================
  
  
   //Loop over events
 
   //all raw data records are read with call to ReadNextEvent
   //at this time history and trigger record analysis is done

   int evtCtr = 0;
   while(evtCtr < maxEvents && eventBuilder.ReadNextEvent() != 0)
   {

      //
      // ======= Getting Admin Info  ========
      //
      
      long int series = eventBuilder.GetAdmin().GetSeries();
      event = eventBuilder.GetAdmin().GetEvent(); 

      cout <<"\nSeries Number = " << series
	   <<"\nEvent Number = " << event
	   << endl;


      //
      // ========= Veto Analysis ==========
      //


      if(myConfigData.DoVetoProcessing()) 
  	       eventBuilder.DoVetoAnalysis();



      //
      // ========= GPIB timing Analysis ==========
      //

      if (myConfigData.DoRead("GPIB_FILE"))
                      eventBuilder.DoGpibTimingCalc();


      // 
      // =========  ZIP Analysis ===========
      //


      // loop ZIP

      for(int detNum = 1; detNum <= myConfigData.GetMaxZIPs() ; detNum++)
      {
        
	 // check if we want to process this detector
	 if( !myConfigData.DoZipProcessing(detNum) ) { continue; }
        
        
         //
	 //  --------  basic calculations ---------
         //
         
         // Calculations done: 
         //  - baseline substraction
         //  - pulses normalization
         //  - RMS calculation
         //  - calculate sum phonon pulses

	  eventBuilder.DoBasicPulseCalc(detNum);   // not permitted to turn this off !
	 
         //
	 //  --------- Phonon Pulse Algorithms ---------
         //


//  	 if( myConfigData.DoAlgorithm(detNum, "phonon", "PulseIntegral") ) 
//  	    eventBuilder.DoPulseIntegral(detNum, "phonon", "filtered");
	 
//   	 if( myConfigData.DoAlgorithm(detNum,"phonon", "InflectionTime") ) 
//          	    eventBuilder.DoInflectionTime(detNum, "phonon");

//   	 if( myConfigData.DoAlgorithm(detNum, "phonon", "RTFTWalkPhonon") ) 
//   	    eventBuilder.DoRTFTWalkPhonon(detNum, "phonon", "filtered");

//   	 if( myConfigData.DoAlgorithm(detNum, "phonon", "OptimalFilterPulse") ) 
//   	    eventBuilder.DoOptimalFilterPulse(detNum, "phonon");

//  	 if( myConfigData.DoAlgorithm(detNum,  "phonon", "NoiseSelector") ) 
//  	    eventBuilder.DoNoiseSelector(detNum, "phonon"); //  must come before PipeFitPhonon 

//   	 if( myConfigData.DoAlgorithm(detNum, "phonon", "PipeFitPhonon") ) 
//   	    eventBuilder.DoPipeFitPhonon(detNum, "phonon");
	 

//          //
//  	 //  --------  Charge Algorithms ---------------
//          //

//   	 if( myConfigData.DoAlgorithm(detNum, "charge", "RTFTWalkCharge") ) 
//   	    eventBuilder.DoRTFTWalkCharge(detNum, "charge", "filtered");

//   	 if( myConfigData.DoAlgorithm(detNum, "charge", "OptimalFilterChargeX") ) 
//   	    eventBuilder.DoOptimalFilterChargeX(detNum);
	 
//   	 if( myConfigData.DoAlgorithm(detNum, "charge", "F5ChargeX") ) 
//   	    eventBuilder.DoF5ChargeX(detNum);
	

         
	 //  ---- additional user analysis classes - DO NOT modify or copy this comment (for auto_analysis) ----

	 if(event < 280419) { continue; }
	 
	 std::vector<PulseData> pulseCollection;
	 eventBuilder.FillSingleZipPulseCollection(pulseCollection, detNum);

	 cout <<"Looping over pulses, size of collection on this detector = " << pulseCollection.size() << endl;
	    
	 for(int pulseItr = 0; pulseItr < pulseCollection.size(); pulseItr++)
	 {
	    PulseData aPulseData = pulseCollection[pulseItr];
	    
	    //if(aPulseData.GetChannelName() != "PA" ) { continue; }

	    channelNum = aPulseData.GetDetectorChannel();
	    detectorNum = detNum;

	    cout <<"\nChannel = " << aPulseData.GetDetectorChannel() 
		 <<"\nsize of the pulse vector = " << aPulseData.GetRawPulse().size()
		 << endl;
         
	    int order = myConfigData.GetIntParameter(detNum,"P_RTFT_BUTTERWORTH_ORDER");
	    double cutoff = myConfigData.GetDoubleParameter(detNum,"P_RTFT_BUTTERWORTH_CUTOFF");
	    double sampleRate = myConfigData.GetDoubleParameter(detNum,"P_SAMPLERATE");

 	    vector<double> filteredPulse = PulseFilter::ButterLowPass(aPulseData.GetBaselineSubPulse(), 
 								      sampleRate, cutoff, order); 

	    
	    for(int itr=0; itr<nBins; itr++)
	    {
	       pulse[itr] = aPulseData.GetBaselineSubPulse()[itr];
	       pulseFlt[itr] = filteredPulse[itr];
//	       cout <<"bin = " << itr <<" = " << filteredPulse[itr] << endl;

	    }

	    pulseTree.Fill();
 
	 }
	    

         //
	 //  --------  ZIP timing (w/ dmm) ---------
	 //



      }  // ===== End of ZIP loop  ======


//      outputManager.StoreOutput(eventBuilder);

      evtCtr++;

     }


  //==========Done looping over events!=================



  cout <<"\nDone looping over events, now storing data!" << endl;

  pulseTree.Write();
//  outputManager.WriteTrees();

  cout <<"Goodbye from BatRoot!" << endl;

  return 0;

} //end main()   DONE!!!


