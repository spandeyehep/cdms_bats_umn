//Standard Libaries
#include <iostream>

//ROOT Libraries
#include "TFile.h"
#include "TTree.h"

//CDMS Libraries
#include "RawDataReader.h"
#include "ExtFilesManager.h"

#include "AdminData.h"
#include "ExternalData.h"
#include "HistoryData.h"
#include "PulseData.h"
#include "EventBuilder.h"
#include "TriggerData.h"

#include "BatOutputManager.h"
#include "PulseTools.h"

//for testing only!
#include "PulseFilter.h"
#include "RTFTWalk.h"

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
	   <<"\nThe command line is: ./BatRoot series# dump# nevents#(optional) configFile(optional)"
	   << endl;
      exit(1);
   }

   string inputSeries = argv[1]; 
   string dumpNum = argv[2]; 

   //converting dumpNum to "F000#" format
   int nZeros = 3 - (int)(log10((double)atoi(dumpNum.c_str())));
   string dumpName("F");
   for(int zCtr=0; zCtr < nZeros; zCtr++) dumpName += "0";
   dumpName += dumpNum;
 
   // construct filename from series and dumpNum
   string rawDataFilename = Form("%s_%s", inputSeries.c_str(), dumpName.c_str());

   cout <<"RawDataFilename = " << rawDataFilename << endl;

   //batroot directory
   if(getenv("BATROOTDIR") == NULL) 
   {
     cerr <<"ERROR! environmental variable $BATROOTDIR not set." << endl;
     exit(1);
   }
   string batrootdir = getenv("BATROOTDIR");

   // ===============================================
   // Read Configuration file and auxillary files
   // ===============================================
  
   ExtFilesManager myExtData;

   // autodetect the location based on series number and then set the default configuration file
   long int seriesCheck = atol(inputSeries.c_str());
   string defaultConfigFile;

   if(seriesCheck < 100000) 
      defaultConfigFile = "configSUFData.Default";

   if(seriesCheck >= 100000 && seriesCheck < 200000) 
      defaultConfigFile = "configSoudanData.Default";

   if(seriesCheck >= 200000 && seriesCheck < 300000) 
      defaultConfigFile = "configUCBData.Default";

   if(seriesCheck >= 2000000)
      defaultConfigFile = "configDCRCData.Default";

   // if desired, overwrite default config with fourth argument
   string configFile = (argc > 4 ? argv[4] : defaultConfigFile);

   // now read the configuration file
   cout <<"Using Config File: " << configFile << endl;
   myExtData.ReadConfigFile(batrootdir + "/configuration/" + configFile);

   // read info file
   string infoFile = myExtData.GetProcessingRecord()->GetPath("AUX_FILES") + inputSeries + ".info"; 
   if (myExtData.GetProcessingRecord()->DoRead("INFO_FILE"))
      myExtData.ReadInfoFile(infoFile);

   // read isr file
   string isrFile = myExtData.GetProcessingRecord()->GetPath("AUX_FILES") + inputSeries + ".isr"; 
   if (myExtData.GetProcessingRecord()->DoRead("ISR_FILE"))
      myExtData.ReadIsrFile(isrFile);

   // read noise and pulse templates file, FIXME remove hardcoded filename
   string noiseFile = myExtData.GetProcessingRecord()->GetPath("NOISE_FILES") + "NoiseAndTemplates_" + inputSeries + ".root"; 
   myExtData.ReadFilterFile(noiseFile);

   // set max events to process
   int maxEventsDefault = myExtData.GetProcessingRecord()->GetMaxEvents();
   int maxEvents = (argc > 3 ? atoi(argv[3]) : maxEventsDefault);
  
   //=================================================
   // Initialize Event Builder
   //=================================================
  
   //opens file and reads file header
   EventBuilder eventBuilder(myExtData, rawDataFilename); 

  //=========================================
  // Initialize Output Variables Lists/Trees
  //============================================
    
  //Tree to store traces - Temp
  Int_t nBins = 2048;
  TFile f_out("testhisto.root", "recreate");

  Float_t pulse[nBins];
  Float_t pulseBS[nBins];
  Float_t pulseBSN[nBins];
  Float_t pulseFiltbw1[nBins];
  Float_t pulseFiltbw2[nBins];
  Float_t r100;
  Float_t r20;
  Float_t r40;
  Int_t detectorNum = 999;
  Int_t channelNum = 999;
  Int_t series = 999;
  Int_t event = 999;
  TTree pulseTree("pulseTree","a test tree");
  pulseTree.Branch("series", &series, "series/I");
  pulseTree.Branch("event", &event, "event/I");
  pulseTree.Branch("detectorNum", &detectorNum, "detectorNum/I");
  pulseTree.Branch("channelNum", &channelNum, "channelNum/I");
  pulseTree.Branch("rawPulse", pulse, Form("rawPulse[%d]/F",nBins));
  pulseTree.Branch("bsPulse", pulseBS, Form("bsPulse[%d]/F",nBins));
  pulseTree.Branch("bsnPulse", pulseBSN, Form("bsnPulse[%d]/F",nBins));
  pulseTree.Branch("filtbw1Pulse", pulseFiltbw1, Form("filtbw1Pulse[%d]/F",nBins));
  pulseTree.Branch("filtbw2Pulse", pulseFiltbw2, Form("filtbw2Pulse[%d]/F",nBins));
  pulseTree.Branch("r100", &r100, "r100/F"); 
  pulseTree.Branch("r40", &r40, "r40/F"); 
  pulseTree.Branch("r20", &r20, "r20/F"); 

  //=======================================================
  
  //Loop over events
  int evtCtr = 0;
  while(evtCtr < maxEvents && eventBuilder.ReadNextEvent() != 0)
  {
     // ======= Admin, Veto, Trigger, etc.  ========

     cout <<"eventCtr = " << evtCtr << endl;
     evtCtr++;

     series = eventBuilder.GetAdmin().GetSeries();
     event = eventBuilder.GetAdmin().GetEvent(); 

     cout <<"Series Number = " << series
	  <<"\nEvent Number = " << event
	  << endl;

//     if(event != 60002 && event != 60012) { continue; }
//     if(event != 60088) { continue; }


     // ==============================
     // Start ZIP Analysis
     // ==============================

     // ============== Pulse Analysis Starts Here ==============	 

      eventBuilder.DoBasicPulseCalc();
      //      eventBuilder.DoNoiseSelector(); //  must come before PipeFitPhonon
      eventBuilder.DoRTFTWalk("filtered");
      //eventBuilder.DoPulseIntegral("filtered");  //we may want something other than BW filter here
     
      //  ===== optimal filtering  ======
      eventBuilder.DoOptimalFilterPulse();
      eventBuilder.DoOptimalFilterChargeX(); //only operates on charge
     
      //  ===== time domain =======
      //eventBuilder.DoPipeFitPhonon();

      //  ===== additional user analysis classes - do not modify or copy this comment (for auto_analysis) =====
      //eventBuilder.DoF5ChargeX();
      //eventBuilder.DoInflectionTime();
      
      //  ===== zip timing (w/ dmm) =======

     // ============== Data Writeout  ==============	 

      std::vector<PulseData> zipCollection;
      eventBuilder.FillZipPulseCollection(zipCollection);

      cout <<"the size of the zip collection = " << zipCollection.size() << endl;

      //looping over pulses
      for(uint pulseItr = 0; pulseItr < zipCollection.size(); pulseItr++)
      {
	 PulseData aPulseData = zipCollection[pulseItr];

	 detectorNum = aPulseData.GetDetectorNum();
	 channelNum = aPulseData.GetDetectorChannel();

	 //	 if(detectorNum != 26) { continue; }
	 //if(channelNum != 2) { continue; }

	 cout <<"\nMeanBaseline = " << aPulseData.GetMeanBaseline() 
	      <<"\nChannel = " << aPulseData.GetDetectorChannel() 
	      <<"\nisSaturated = " << aPulseData.IsSaturated()
	      << endl;

	 //create filtered pulses
	 string sensorType = aPulseData.GetChannelType(); 
	 int detNum = aPulseData.GetDetectorNum();
	 ExtDataRecord* detRecord = myExtData.GetDetectorRecord(detNum);
	 
	 int order = detRecord->GetButterworthOrder(sensorType); 
	 double cutoff = detRecord->GetButterworthCutoff(sensorType); 
	 double sampleRate = detRecord->GetSampleRate(sensorType);

	 double maxbin = PulseTools::MaxADCPoint(aPulseData.GetBaselineSubPulse());

	 //Note we are doing the filtering on the BS pulse!
	 vector<double> filteredPulsebw1 = PulseFilter::ButterLowPass(aPulseData.GetBaselineSubPulse(), sampleRate, cutoff, order); 
	 vector<double> filteredPulsebw2 = PulseFilter::ButterLowPass(aPulseData.GetBaselineSubPulse(), sampleRate, cutoff, order); 
	 
	 for(int itr=0; itr<nBins; itr++)
	   {
	     pulse[itr] = (aPulseData.GetRawPulse())[itr];
	     pulseBS[itr] = (aPulseData.GetBaselineSubPulse())[itr];
	     pulseBSN[itr] = (aPulseData.GetBaselineSubNormPulse())[itr];
	     pulseFiltbw1[itr] = filteredPulsebw1[itr];
	     pulseFiltbw2[itr] = filteredPulsebw2[itr];

  //   	     cout <<"\nOldFiltVal = " << filteredPulsebw1[itr]
// 		  <<"\nNewFiltVal = " << filteredPulsebw2[itr]
//  		  << endl;

	   }
	 
	 //Store RTFTwalk values
 	 TCDMSAnalysis tempWalk = aPulseData.GetPulseAnalysis("RTFTWalk");
 	 cout <<"r100 = " << tempWalk.GetRQVal("r100") 
 	      <<"\nr40 = " << tempWalk.GetRQVal("r40") 
 	      <<"\nr20 = " << tempWalk.GetRQVal("r20") 
 	      << endl;

 	 TCDMSAnalysis tempOptimalFilter = (aPulseData.GetChannelType() == "phonon" ?  aPulseData.GetPulseAnalysis("OptimalFilterPulse") : aPulseData.GetPulseAnalysis("OptimalFilterChargeX"));
 	 cout <<"OF delay = " << tempOptimalFilter.GetRQVal("OFdelay")
 	      << endl;

	 pulseTree.Fill();

      }

     // =============End of ZIP analysis  ============

//      cout <<"eventCtr = " << evtCtr << endl;
//      evtCtr++;

     }

  //==========Done looping over events!=================

  pulseTree.Write(); //Temp
  f_out.Close(); //Temp

  return 0;

} //end main()   DONE!!!


