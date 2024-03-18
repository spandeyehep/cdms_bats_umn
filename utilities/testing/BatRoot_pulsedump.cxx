///////////////////////////////////////////////////////////////////////////////// 
//main()
//Author:  L. Hsu
//Description: This is a modified version of main that will dump all zip pulses
//for a particular event.  No analyses except basic are performed, but they can activated if 
//desired for a particular study
//
//File Import By: L. Hsu
//Creation Date: Nov. 17, 2008
//
//Modifications:
//
////////////////////////////////////////////////////////////////////////////////// 

//Standard Libaries
#include <iostream>
#include "time.h"


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
#include "PulseFilter.h"

using namespace std;

/////////////////// BEGIN MAIN //////////////////////////////

int main(int argc, char* argv[]){

   // ===============
   // assignments
   // ===============

   //reading inputs to main
   //zip numbers are 1-30 (for Soudan, for example)
   //channel numbers is as follows: QI=0, QO=1, PA=2, PB=3, PC=4, PD=5
   if(argc < 4)  
   {
      cout <<"ERROR running BatRoot testing script!"
	   <<"\nThe command line is: ./BatRoot series# dump# eventnumber# zip#(optional) channel#(optional) processingOptions(optional) analysisConfig(optional)"
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
   // Read time/date and BatRoot CVS tag
   // ===============================================

   //  not needed for this script

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
	 defaultAnalysisConfigFile = "configSoudanData.c58";
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
   string userOptionsFile = (argc > 6 ? argv[6] : defaultUserOptionsFile);
   // now read the processing file
   myConfigData.ReadFile(batrootdir + "/configuration/" + userOptionsFile); 

   // if desired, overwrite default analysis config with fifth argument
   string configFile = (argc > 7 ? argv[7] : defaultAnalysisConfigFile);
   // now read the analysis configuration file
   myConfigData.ReadFile(batrootdir + "/configuration/analysis/" + configFile);

   // set max events to process - only allowed to set from config
   int maxEvents = myConfigData.GetMaxEvents();

   // get the eventnumber that is of interest
   int desiredEvent = (argc > 3 ? atoi(argv[3]) : -999999);
   cout <<"\nWill seek out pulses for event = " << desiredEvent << endl;

   // get the zip number that is of interest
   int desiredZip = (argc > 4 ? atoi(argv[4]) : -999999);
   if(desiredZip != -999999) cout <<"Will seek out pulses for detector = " << desiredZip << endl;

   // get the channel number that is of interest
   int desiredChan = (argc > 5 ? atoi(argv[5]) : -999999);
   if(desiredChan != -999999) cout <<"Will seek out pulses for channel = " << desiredChan <<"\n" << endl;

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

   IsrDataManager myIsrData(myConfigData.GetMaxZIPs());

   string isrFile = myConfigData.GetPath("AUX_FILES") + inputSeries + ".isr"; 
   if (myConfigData.DoRead("ISR_FILE")) 
          myIsrData.ReadFile(isrFile);
    else
       cout <<"WARNING!  ISR file is not being read.   Pulse normalizations will all be set to 1!" << endl;
    


   //
   // ===== DMM file =====
   //

   //not needed for this script


//    // ===== GPIB change log file ===== 

   
   //not needed for this script

   // ===== noise and pulse templates file ===== 

   FilterDataManager myFilterData(myConfigData.GetMaxZIPs());
   string noisePrefix = myConfigData.GetPrefix("NOISE_PREFIX");
   string noiseFile = myConfigData.GetPath("NOISE_FILES") + noisePrefix + inputSeries + ".root"; 
   myFilterData.ReadFile(noiseFile);

 
   //=================================================
   // Initialize Event Builder
   //=================================================
  

   //opens raw file and reads file header
   //store external files (Config, ISR, INFO, DMM, GPIB, and NOISE) for later use
   //contruct some dummies for now
    GpibDataManager myGpibData;
    DmmDataManager myDmmData(myConfigData.GetMaxZIPs());

    EventBuilder eventBuilder(myConfigData, myInfoData, myIsrData, myDmmData, myGpibData, myFilterData, rawDataFilename); 

   //=========================================
   // Initialize Output Variables Lists/Trees
   //=========================================
    
    //This will store the pulse histograms
    TFile f_out("dumppulses.root", "recreate");
   
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
      int event = eventBuilder.GetAdmin().GetEvent(); 

      cout <<"\nSeries Number = " << series
	   <<"\nEvent Number = " << event
	   << endl;

      //skip all events except the desired one
      if(event != desiredEvent) { evtCtr++; continue; }

      //
      // ========= Veto Analysis ==========
      //

      //disabled
//       if(myConfigData.DoVetoProcessing()) 
//   	       eventBuilder.DoVetoAnalysis();


      //
      // ========= Timing Analysis ==========
      //

      // GPIB (Flash Times) and ISR file (LastISRTime)

      // not needed for this script

      // ISR (LastISRTime)

      // not needed for this script

      // 
      // =========  ZIP Analysis ===========
      //


      // loop ZIP

      for(int detNum = 1; detNum <= myConfigData.GetMaxZIPs() ; detNum++)
      {
        
	 // check if we want to dump this detector
	 if(desiredZip != detNum && desiredZip != -999999) { continue; }
	 
	 cout <<"detector number = " << detNum << endl;
  
         // 
         // ------ Detector related ISR/DMM/INFO files calculations ----
         // 

//           if (myConfigData.DoRead("DMM_FILE"))
//                     eventBuilder.DoDmmCalc(detNum);
         
          
//           if (myConfigData.DoRead("ISR_FILE"))
//                     eventBuilder.DoIsrCalc(detNum);
         
             
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
	 //  --------- Phonon Pulse Algorithms ---------
         //


//  	 if( myConfigData.DoAlgorithm(detNum, "phonon", "PulseIntegral") ) 
//  	    eventBuilder.DoPulseIntegral(detNum, "phonon", "filtered");
	 
//   	 if( myConfigData.DoAlgorithm(detNum,"phonon", "InflectionTime") ) 
//          	    eventBuilder.DoInflectionTime(detNum, "phonon");

   	 if( myConfigData.DoAlgorithm(detNum, "phonon", "OptimalFilterPulse") ) 
   	    eventBuilder.DoOptimalFilterPulse(detNum, "phonon");

   	 if( myConfigData.DoAlgorithm(detNum, "phonon", "RTFTWalkPhonon") ) 
   	    eventBuilder.DoRTFTWalkPhonon(detNum, "phonon", "filtered");

// 	 if( myConfigData.DoAlgorithm(detNum, "phonon", "ConstFreqRTFTWalkPhonon") ) 
// 	   eventBuilder.DoConstFreqRTFTWalkPhonon(detNum, "phonon", "filtered");

//  	 if( myConfigData.DoAlgorithm(detNum,  "phonon", "NoiseSelector") ) 
//  	    eventBuilder.DoNoiseSelector(detNum, "phonon"); //  must come before PipeFitPhonon 

//   	 if( myConfigData.DoAlgorithm(detNum, "phonon", "PipeFitPhonon") ) 
//   	    eventBuilder.DoPipeFitPhonon(detNum, "phonon");
	 
// 	 if( myConfigData.DoAlgorithm(detNum, "phonon", "WedgeFitPhonon") )
// 	    eventBuilder.DoWedgeFitPhonon(detNum, "phonon");

         //
 	 //  --------  Charge Algorithms ---------------
         //

//   	 if( myConfigData.DoAlgorithm(detNum, "charge", "RTFTWalkCharge") ) 
//   	    eventBuilder.DoRTFTWalkCharge(detNum, "charge", "filtered");

//   	 if( myConfigData.DoAlgorithm(detNum, "charge", "OptimalFilterChargeX") ) 
//   	    eventBuilder.DoOptimalFilterChargeX(detNum);
	 
//   	 if( myConfigData.DoAlgorithm(detNum, "charge", "F5ChargeX") ) 
//   	    eventBuilder.DoF5ChargeX(detNum);
	

	 //Accessing pulses for storage (eventually maybe we can event plot interactively from here)
	 
	 std::vector<PulseData> pulseCollection;
	 eventBuilder.FillSingleZipPulseCollection(pulseCollection, detNum);

	 cout <<"Looping over pulses, size of collection on this detector = " << pulseCollection.size() << endl;
	    
	 for(int pulseItr = 0; pulseItr < pulseCollection.size(); pulseItr++)
	 {
	    PulseData aPulseData = pulseCollection[pulseItr];
	    int channelNum = aPulseData.GetDetectorChannel();	    
	    string channelName = aPulseData.GetChannelName();	    
	   	 
	    // check if we want to dump this detector
	    if(desiredChan != channelNum && desiredChan != -999999) { continue; }

	    // temporary, only do this for phonon channels
	    if(aPulseData.IsChargePulse() || channelName=="PT") { continue; }
	 
	    cout <<"\nChannel = " << aPulseData.GetDetectorChannel() 
		 <<"\nsize of the pulse vector = " << aPulseData.GetRawPulse().size()
		 << endl;

//	    string index = Form("_zip%d_event%d", desiredZip, desiredEvent);

	    //save the baseline subtracted pulse
	    (PulseTools::Vector2TH1D(aPulseData.GetBaselineSubPulse(), channelName+"BaselineSubPulse")).Write();
	   
	    //save the filtered pulse
 	    int order = myConfigData.GetIntParameter(detNum,"P_RTFT_BUTTERWORTH_ORDER");
 	    double sampleRate = myConfigData.GetDoubleParameter(detNum,"P_SAMPLERATE");
	    double cutoff = aPulseData.GetPulseAnalysis("RTFTWalkPhonon").GetRQVal("RTFTCutoff");
	    double oldcutoff =  myConfigData.GetDoubleParameter(detNum,"P_RTFT_BUTTERWORTH_CUTOFF_DEFAULT");

	    cout <<"new rtft cutoff = " << cutoff 
		 <<"\nold rtft cutoff = " << oldcutoff 
		 << endl;

	    if(cutoff > 0)
	    {
	      vector<double> filteredPulse = PulseFilter::ButterLowPass(aPulseData.GetBaselineSubPulse(), 
									sampleRate, cutoff, order); 

	      (PulseTools::Vector2TH1D(filteredPulse, channelName+"NewFilteredPulse")).Write();
	    }
	    else //if cutoff is undefined then just save the original pulse 
	    {
	      cout <<"optimized cutoff was > nyquist, saving unaltered pulse in place of filtered pulse" << endl;
	      vector<double> filteredPulse = aPulseData.GetBaselineSubPulse();
	      (PulseTools::Vector2TH1D(filteredPulse, channelName+"NewFilteredPulse")).Write();
	    }

	    vector<double> filteredPulse = PulseFilter::ButterLowPass(aPulseData.GetBaselineSubPulse(), 
								      sampleRate, oldcutoff, order); 

	    (PulseTools::Vector2TH1D(filteredPulse, channelName+"OldFilteredPulse")).Write();


	 }

      }  // ===== End of ZIP loop  ======


      //Store output


      //Done with pulse dump so exit loop
      break;

     }


  //==========Done looping over events!=================


  return 0;

} //end main()   DONE!!!


